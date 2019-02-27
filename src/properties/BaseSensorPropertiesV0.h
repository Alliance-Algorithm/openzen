#ifndef ZEN_SENSOR_PROPERTIES_BASESENSORPROPERTIESV0_H_
#define ZEN_SENSOR_PROPERTIES_BASESENSORPROPERTIESV0_H_

#include "InternalTypes.h"
#include "ZenTypes.h"

#include <array>
#include <optional>

#define GET_OR(x) isGetter ? (x) : EDevicePropertyV0::Ack
#define SET_OR(x) isGetter ? EDevicePropertyV0::Ack : (x)
#define GET_SET(x, y) isGetter ? (x) : (y)

namespace zen
{
    namespace base::v0
    {
        namespace internal
        {
            constexpr std::optional<EDevicePropertyInternal> map(uint8_t function)
            {
                if (function <= static_cast<uint8_t>(EDevicePropertyInternal::Nack))
                    return static_cast<EDevicePropertyInternal>(function);
                else if (function == static_cast<uint8_t>(EDevicePropertyInternal::Config))
                    return EDevicePropertyInternal::Config;

                return {};
            }
        }

        constexpr EDevicePropertyV0 mapCommand(ZenProperty_t command)
        {
            switch (command)
            {
            case ZenSensorProperty_StoreSettingsInFlash:
                return EDevicePropertyV0::WriteRegisters;

            case ZenSensorProperty_RestoreFactorySettings:
                return EDevicePropertyV0::RestoreFactorySettings;

            default:
                return EDevicePropertyV0::Ack;
            }
        }

        constexpr EDevicePropertyV0 map(ZenProperty_t property, bool isGetter)
        {
            switch (property)
            {
            case ZenSensorProperty_DeviceName:
                return GET_OR(EDevicePropertyV0::GetDeviceName);

            case ZenSensorProperty_FirmwareInfo:
                return GET_OR(EDevicePropertyV0::GetFirmwareInfo);

            case ZenSensorProperty_FirmwareVersion:
                return GET_OR(EDevicePropertyV0::GetFirmwareVersion);

            case ZenSensorProperty_SerialNumber:
                return GET_OR(EDevicePropertyV0::GetSerialNumber);

            case ZenSensorProperty_BatteryCharging:
                return GET_OR(EDevicePropertyV0::GetBatteryCharging);

            case ZenSensorProperty_BatteryLevel:
                return GET_OR(EDevicePropertyV0::GetBatteryLevel);

            case ZenSensorProperty_BatteryVoltage:
                return GET_OR(EDevicePropertyV0::GetBatteryVoltage);

            case ZenSensorProperty_DataMode:
                return SET_OR(EDevicePropertyV0::SetDataMode);

            case ZenSensorProperty_TimeOffset:
                return GET_SET(EDevicePropertyV0::GetPing, EDevicePropertyV0::SetTimestamp);

            default:
                return EDevicePropertyV0::Ack;
            }
        }

        constexpr bool supportsExecutingDeviceCommand(EDevicePropertyV0 command)
        {
            switch (command)
            {
            case EDevicePropertyV0::WriteRegisters:
            case EDevicePropertyV0::RestoreFactorySettings:
                return true;

            default:
                return false;
            }
        }

        constexpr ZenPropertyType supportsGettingArrayDeviceProperty(EDevicePropertyV0 property)
        {
            switch (property)
            {
            case EDevicePropertyV0::GetSerialNumber:
            case EDevicePropertyV0::GetDeviceName:
            case EDevicePropertyV0::GetFirmwareInfo:
                return ZenPropertyType_String;

            case EDevicePropertyV0::GetFirmwareVersion:
                return ZenPropertyType_Int32;

            default:
                return ZenPropertyType_Invalid;
            }
        }

        constexpr bool supportsGettingFloatDeviceProperty(EDevicePropertyV0 property)
        {
            switch (property)
            {
            case EDevicePropertyV0::GetBatteryLevel:
            case EDevicePropertyV0::GetBatteryVoltage:
                return true;

            default:
                return false;
            }
        }

        constexpr bool supportsGettingInt32DeviceProperty(EDevicePropertyV0 property)
        {
            switch (property)
            {
            case EDevicePropertyV0::GetPing:
                return true;

            default:
                return false;
            }
        }

        constexpr bool supportsSettingInt32DeviceProperty(EDevicePropertyV0 property)
        {
            switch (property)
            {
            case EDevicePropertyV0::SetDataMode:
            case EDevicePropertyV0::SetTimestamp:
                return true;

            default:
                return false;
            }
        }
    }
}

#endif
