#ifndef ZEN_SENSOR_H_
#define ZEN_SENSOR_H_

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include "nonstd/expected.hpp"

#include "InternalTypes.h"

#include "SensorConfig.h"
#include "SensorComponent.h"
#include "communication/SyncedModbusCommunicator.h"

namespace zen
{
    nonstd::expected<std::shared_ptr<class Sensor>, ZenSensorInitError> make_sensor(SensorConfig config, std::unique_ptr<ModbusCommunicator> communicator, uintptr_t token) noexcept;

    class Sensor : private IModbusFrameSubscriber
    {
    public:
        Sensor(SensorConfig config, std::unique_ptr<ModbusCommunicator> communicator, uintptr_t token);
        ~Sensor();

        /** Allow the sensor to initialize variables, that require an active IO interface */
        ZenSensorInitError init();

        /** On first call, tries to initialize a firmware update, and returns an error on failure.
         * Subsequent calls do not require a valid buffer and buffer size, and only report the current status:
         * Returns ZenAsync_Updating while busy updating firmware.
         * Returns ZenAsync_Finished once the entire firmware has been written to the sensor.
         * Returns ZenAsync_Failed if an error has occured while updating.
         */
        ZenAsyncStatus updateFirmwareAsync(gsl::span<const std::byte> buffer) noexcept;

        /** On first call, tries to initialize an IAP update, and returns an error on failure.
         * Subsequent calls do not require a valid buffer and buffer size, and only report the current status:
         * Returns ZenAsync_Updating while busy updating IAP.
         * Returns ZenAsync_Finished once the entire IAP has been written to the sensor.
         * Returns ZenAsync_Failed if an error has occured while updating.
         */
        ZenAsyncStatus updateIAPAsync(gsl::span<const std::byte> buffer) noexcept;

        ISensorProperties* properties() { return m_properties.get(); }

        /** If successful, directs the outComponents pointer to a list of sensor components and sets its length to outLength, otherwise, returns an error.
         * If the type variable points to a string, only components of that type are returned. If it is a nullptr, all components are returned, irrespective of type.
         */
        const std::vector<std::unique_ptr<SensorComponent>>& components() const noexcept { return m_components; }

        /** Returns the sensor's IO type */
        std::string_view ioType() const noexcept { return m_communicator.ioType(); }

        /** Returns whether the sensor is equal to the sensor description */
        bool equals(const ZenSensorDesc& desc) const;

    private:
        ZenError processReceivedData(uint8_t address, uint8_t function, gsl::span<const std::byte> data) noexcept override;

        void upload(std::vector<std::byte> firmware);

        SensorConfig m_config;

        std::vector<std::unique_ptr<SensorComponent>> m_components;
        std::unique_ptr<ISensorProperties> m_properties;
        SyncedModbusCommunicator m_communicator;

        uintptr_t m_token;

        std::atomic_bool m_updatingFirmware;
        std::atomic_bool m_updatedFirmware;
        ZenError m_updateFirmwareError;

        std::atomic_bool m_updatingIAP;
        std::atomic_bool m_updatedIAP;
        ZenError m_updateIAPError;

        std::thread m_uploadThread;

        // [LEGACY]
        std::atomic_bool m_initialized;
    };
}

#endif
