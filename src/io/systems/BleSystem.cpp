#include "io/systems/BleSystem.h"

#include "io/ble/BleDeviceFinder.h"
#include "io/interfaces/BleInterface.h"

namespace zen
{
    bool BleSystem::available()
    {
        return QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods().testFlag(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    }

    ZenError BleSystem::listDevices(std::vector<ZenSensorDesc>& outDevices)
    {
        BleDeviceFinder finder;
        return finder.listDevices(outDevices);
    }

    std::unique_ptr<BaseIoInterface> BleSystem::obtain(const ZenSensorDesc& desc, ZenSensorInitError& outError)
    {
        auto handle = std::make_unique<BleDeviceHandler>(desc.handle64);
        if (outError = handle->initialize())
            return nullptr;

        auto format = modbus::ModbusFormat::LP;
        auto factory = modbus::make_factory(format);
        auto parser = modbus::make_parser(format);
        if (!factory || !parser)
        {
            outError = ZenSensorInitError_UnsupportedDataFormat;
            return nullptr;
        }

        return std::make_unique<BleInterface>(std::move(handle), std::move(factory), std::move(parser));
    }
}
