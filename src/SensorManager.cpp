#include "SensorManager.h"

#include "Sensor.h"
#include "communication/ConnectionNegotiator.h"
#include "components/ComponentFactoryManager.h"
#include "io/IoManager.h"
#include "io/can/CanManager.h"
#include "utility/StringView.h"

namespace zen
{
    namespace
    {
        void notifyProgress(std::set<std::reference_wrapper<SensorClient>, ReferenceWrapperCmp<SensorClient>>& subscribers, float progress)
        {
            ZenEvent event{};
            event.eventType = ZenSensorEvent_SensorListingProgress;
            event.data.sensorListingProgress.progress = progress;
            event.data.sensorListingProgress.complete = progress == 1.0f;

            for (auto& subscriber : subscribers)
                subscriber.get().notifyEvent(event);
        }
    }

    SensorManager& SensorManager::get()
    {
        static SensorManager singleton;
        return singleton;
    }

    SensorManager::SensorManager() noexcept
        : m_nextToken(1)
        , m_discovering(false)
        , m_terminate(false)
        , m_sensorThread(&SensorManager::sensorLoop, this)
        , m_sensorDiscoveryThread(&SensorManager::sensorDiscoveryLoop, this)   
    {
#ifdef ZEN_BLUETOOTH
        // Necessary for QBluetooth
        if (QCoreApplication::instance() == nullptr)
        {
            int argv = 0;
            char* argc[]{ nullptr };
            m_app.reset(new QCoreApplication(argv, argc));
        }
#endif

        ComponentFactoryManager::get().initialize();
        IoManager::get().initialize();
    }

    SensorManager::~SensorManager() noexcept
    {
        m_terminate = true;
        m_discoveryCv.notify_all();

        if (m_sensorDiscoveryThread.joinable())
            m_sensorDiscoveryThread.join();

        if (m_sensorThread.joinable())
            m_sensorThread.join();
    }

    nonstd::expected<std::shared_ptr<Sensor>, ZenSensorInitError> SensorManager::obtain(const ZenSensorDesc& const_desc) noexcept
    {
        std::unique_lock<std::mutex> lock(m_sensorsMutex);
        ZenSensorDesc desc = const_desc;

        for (const auto& sensor : m_sensors)
            if (sensor->equals(desc))
                return sensor;

        lock.unlock();

        auto ioSystem = IoManager::get().getIoSystem(desc.ioType);
        if (!ioSystem)
            return nonstd::make_unexpected(ZenSensorInitError_UnsupportedIoType);

        ConnectionNegotiator negotiator;
        auto communicator = std::make_unique<ModbusCommunicator>(negotiator,
          std::make_unique<modbus::LpFrameFactory>(), std::make_unique<modbus::LpFrameParser>());

        // load the default baud rate, if needed
        if (desc.baudRate == 0) {
            desc.baudRate = ioSystem->get().getDefaultBaudrate();
        }

        if (auto ioInterface = ioSystem->get().obtain(desc, *communicator.get()))
            communicator->init(std::move(*ioInterface));
        else
            return nonstd::make_unexpected(ioInterface.error());

        auto agreement = negotiator.negotiate(*communicator.get(), desc.baudRate);
        if (!agreement)
            return nonstd::make_unexpected(agreement.error());

        lock.lock();
        const auto token = m_nextToken++;
        lock.unlock();

        auto sensor = make_sensor(std::move(*agreement), std::move(communicator), token);
        if (!sensor)
            return nonstd::make_unexpected(sensor.error());

        lock.lock();
        m_sensors.insert(*sensor);
        lock.unlock();

        return std::move(*sensor);
    }

    std::shared_ptr<Sensor> SensorManager::release(ZenSensorHandle_t sensorHandle) noexcept
    {
        std::lock_guard<std::mutex> lock(m_sensorsMutex);
        auto it = m_sensors.find(sensorHandle);

        const auto sensor = *it;
        m_sensors.erase(it);
        return sensor;
    }

    void SensorManager::subscribeToSensorDiscovery(SensorClient& client) noexcept
    {
        std::lock_guard<std::mutex> lock(m_discoveryMutex);
        m_discoverySubscribers.insert(client);
        m_discovering = true;
        m_discoveryCv.notify_one();
    }

    void SensorManager::sensorDiscoveryLoop() noexcept
    {
        while (!m_terminate)
        {
            std::unique_lock<std::mutex> lock(m_discoveryMutex);
            m_discoveryCv.wait(lock, [this]() { return m_discovering || m_terminate; });

            lock.unlock();

            const auto ioSystems = IoManager::get().getIoSystems();
            const auto nIoSystems = ioSystems.size();
            for (size_t idx = 0; idx < nIoSystems; ++idx)
            {
                if (m_terminate)
                    return;

                lock.lock();
                notifyProgress(m_discoverySubscribers, (idx + 0.5f) / nIoSystems);
                lock.unlock();

                try
                {
                    ioSystems[idx].get().listDevices(m_devices);
                }
                catch (...)
                {
                    // [TODO] Make listDevices noexcept and move try-catch block into crashing ioSystem
                    continue;
                }
            }


            lock.lock();
            for (auto& device : m_devices)
            {
                ZenEvent event{};
                event.eventType = ZenSensorEvent_SensorFound;
                event.data.sensorFound = device;

                for (auto& subscriber : m_discoverySubscribers)
                    subscriber.get().notifyEvent(event);
            }

            notifyProgress(m_discoverySubscribers, 1.0f);
            
            m_devices.clear();
            m_discovering = false;
            m_discoverySubscribers.clear();
        }
    }

    void SensorManager::sensorLoop()
    {
        while (!m_terminate)
        {
            CanManager::get().poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
