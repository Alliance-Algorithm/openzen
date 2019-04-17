#include "io/systems/SiUsbSystem.h"

#include <iostream>

#include <spdlog/spdlog.h>

#include "SensorManager.h"
#include "io/interfaces/SiUsbInterface.h"
#include "utility/IPlatformDll.h"

namespace zen
{
    namespace
    {
        nonstd::expected<std::unique_ptr<IIoInterface>, ZenSensorInitError> make_interface(IIoDataSubscriber& subscriber, DWORD deviceId)
        {
            HANDLE handle;
            if (auto error = SiUsbSystem::fnTable.open(deviceId, &handle))
                return nonstd::make_unexpected(ZenSensorInitError_IoFailed);

            // Create an overlapped object for asynchronous communication
            OVERLAPPED ioReader{0};
            ioReader.hEvent = ::CreateEventA(nullptr, false, false, nullptr);
            if (!ioReader.hEvent)
                return nonstd::make_unexpected(ZenSensorInitError_IoFailed);

            auto ioInterface = std::make_unique<SiUsbInterface>(subscriber, handle, ioReader);
            //if (auto error = ioInterface->setBaudrate(921600))
            //    return std::make_pair(error, nullptr);

            if (auto error = SiUsbSystem::fnTable.setFlowControl(handle, SI_HANDSHAKE_LINE, SI_FIRMWARE_CONTROLLED, SI_HELD_INACTIVE, SI_STATUS_INPUT, SI_STATUS_INPUT, 0))
                return nonstd::make_unexpected(ZenSensorInitError_IoFailed);

            return std::move(ioInterface);
        }
    }

    SiUsbFnTable SiUsbSystem::fnTable = {};

    SiUsbSystem::~SiUsbSystem()
    {
        IPlatformDll::get().unload(m_handle);
    }

    bool SiUsbSystem::available()
    {
        if (m_handle != nullptr)
            return true;

        auto& dll = IPlatformDll::get();
        if (auto handle = dll.load("SiUSBXp.dll"))
        {
            m_handle = handle;

            fnTable.checkRxQueue = reinterpret_cast<SiUsbFnTable::CheckRxQueueFn>(dll.procedure(m_handle, "SI_CheckRXQueue"));
            fnTable.cancelIo = reinterpret_cast<SiUsbFnTable::CancelIoFn>(dll.procedure(m_handle, "SI_CancelIo"));
            fnTable.close = reinterpret_cast<SiUsbFnTable::CloseFn>(dll.procedure(m_handle, "SI_Close"));
            fnTable.getDeviceProductString = reinterpret_cast<SiUsbFnTable::GetDeviceProductStringFn>(dll.procedure(m_handle, "SI_GetDeviceProductString"));
            fnTable.getNumDevices = reinterpret_cast<SiUsbFnTable::GetNumDevicesFn>(dll.procedure(m_handle, "SI_GetNumDevices"));
            fnTable.getProductStringSafe = reinterpret_cast<SiUsbFnTable::GetProductStringSafeFn>(dll.procedure(m_handle, "SI_GetProductStringSafe"));
            fnTable.open = reinterpret_cast<SiUsbFnTable::OpenFn>(dll.procedure(m_handle, "SI_Open"));
            fnTable.read = reinterpret_cast<SiUsbFnTable::ReadFn>(dll.procedure(m_handle, "SI_Read"));
            fnTable.setBaudrate = reinterpret_cast<SiUsbFnTable::SetBaudrateFn>(dll.procedure(m_handle, "SI_SetBaudRate"));
            fnTable.setFlowControl = reinterpret_cast<SiUsbFnTable::SetFlowControlFn>(dll.procedure(m_handle, "SI_SetFlowControl"));
            fnTable.write = reinterpret_cast<SiUsbFnTable::WriteFn>(dll.procedure(m_handle, "SI_Write"));
            spdlog::info("Loaded SiLabs driver from Dll");
            return true;
        }

        spdlog::warn("Cannot load SiLabs SiUSBXp.dll, USB express connected sensor will not be available.");

        return false;
    }

    ZenError SiUsbSystem::listDevices(std::vector<ZenSensorDesc>& outDevices)
    {
        DWORD nDevices;
        if (auto error = SiUsbSystem::fnTable.getNumDevices(&nDevices))
            return ZenError_Io_GetFailed;

        for (DWORD idx = 0; idx < nDevices; ++idx)
        {
            ZenSensorDesc desc;
            std::memcpy(desc.ioType, SiUsbSystem::KEY, sizeof(SiUsbSystem::KEY));

            if (auto error = SiUsbSystem::fnTable.getProductStringSafe(idx, desc.serialNumber, sizeof(ZenSensorDesc::serialNumber), SI_RETURN_SERIAL_NUMBER))
                return ZenError_Io_GetFailed;

            if (auto error = SiUsbSystem::fnTable.getProductStringSafe(idx, desc.name, sizeof(ZenSensorDesc::name), SI_RETURN_SERIAL_NUMBER))
                return ZenError_Io_GetFailed;

            outDevices.emplace_back(desc);
        }

        return ZenError_None;
    }

    nonstd::expected<std::unique_ptr<IIoInterface>, ZenSensorInitError> SiUsbSystem::obtain(const ZenSensorDesc& desc, IIoDataSubscriber& subscriber) noexcept
    {
        DWORD nDevices;
        if (auto error = SiUsbSystem::fnTable.getNumDevices(&nDevices))
            return nonstd::make_unexpected(ZenSensorInitError_IoFailed);

        const std::string_view target = desc.serialNumber;

        bool found = false;
        char serialNumber[sizeof(ZenSensorDesc::serialNumber)];
        DWORD idx = 0;
        for (; idx < nDevices; ++idx)
        {
            if (auto error = SiUsbSystem::fnTable.getProductStringSafe(idx, serialNumber, sizeof(ZenSensorDesc::serialNumber), SI_RETURN_SERIAL_NUMBER))
                continue;

            found = serialNumber == target;
            if (found)
                break;
        }

        if (!found)
            return nonstd::make_unexpected(ZenSensorInitError_InvalidAddress);

        return make_interface(subscriber, idx);
    }
}
