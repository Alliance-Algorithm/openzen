#include "io/ble/BleDeviceFinder.h"

#include <cstring>

#include "io/systems/BleSystem.h"

namespace zen
{
    ZenError BleDeviceFinder::listDevices(std::vector<ZenSensorDesc>& outDevices)
    {
        start();
        wait();

        if (m_agent->error() != QBluetoothDeviceDiscoveryAgent::NoError)
            return ZenError_Device_ListingFailed;

        const auto devices = m_agent->discoveredDevices();
        for (const auto& device : devices)
        {
            if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
            {
                const auto name = device.name().toStdString();
                const auto address = device.address().toString().toStdString();

                ZenSensorDesc desc;
                auto length = std::min(sizeof(ZenSensorDesc::name) - 1, name.size());
                std::memcpy(desc.name, name.c_str(), length);
                desc.name[length] = '\0';

                length = std::min(sizeof(ZenSensorDesc::serialNumber) - 1, static_cast<size_t>(address.size()));
                std::memcpy(desc.serialNumber, address.c_str(), length);
                desc.serialNumber[length] = '\0';

                std::memcpy(desc.ioType, BleSystem::KEY, sizeof(BleSystem::KEY));
                desc.handle64 = device.address().toUInt64();
                outDevices.emplace_back(desc);
            }
        }

        return ZenError_None;
    }

    void BleDeviceFinder::run()
    {
        m_agent = std::make_unique<QBluetoothDeviceDiscoveryAgent>();
        m_agent->setLowEnergyDiscoveryTimeout(5000);

        connect(m_agent.get(), &QBluetoothDeviceDiscoveryAgent::finished, this, &QThread::quit, Qt::DirectConnection);

        m_agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

        // Exec, so we can process events
        exec();
    }
}
