#ifndef ZEN_SENSORCOMPONENT_H_
#define ZEN_SENSORCOMPONENT_H_

#include <memory>
#include <string_view>

#include <gsl/span>

#include "ISensorProperties.h"

namespace zen
{
    class SensorComponent
    {
    public:
        SensorComponent(std::unique_ptr<ISensorProperties> properties) noexcept
            : m_properties(std::move(properties))
        {}

        virtual ~SensorComponent() noexcept = default;

        /** Tries to initialize settings of the sensor's component that can fail.
         * After succesfully completing init, m_properties should be set.
         */
        virtual ZenSensorInitError init() noexcept = 0;

        virtual ZenError processData(uint8_t function, gsl::span<const std::byte> data) noexcept = 0;
        virtual ZenError processEvent(ZenEvent event, gsl::span<const std::byte> data) noexcept = 0;

        virtual std::string_view type() const noexcept = 0;

        ISensorProperties* properties() noexcept { return m_properties.get(); }

    protected:
        std::unique_ptr<ISensorProperties> m_properties;
    };
}

#endif
