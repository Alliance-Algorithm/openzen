#ifndef ZEN_PROPERTIES_LEGACYIMUPROPERTIES_H_
#define ZEN_PROPERTIES_LEGACYIMUPROPERTIES_H_

#include "communication/SyncedModbusCommunicator.h"
#include "components/ImuComponent.h"

namespace zen
{
    class LegacyImuProperties : public ISensorProperties
    {
    public:
        LegacyImuProperties(SyncedModbusCommunicator& communicator) noexcept;

        /** If successful executes the command, therwise returns an error. */
        ZenError execute(ZenProperty_t property) noexcept override;

        /** If successful fills the buffer with the array of properties and sets the buffer's size.
         * Otherwise, returns an error and potentially sets the desired buffer size - if it is too small.
         */
        std::pair<ZenError, size_t> getArray(ZenProperty_t property, ZenPropertyType type, gsl::span<std::byte> buffer) noexcept override;

        /** If successful fills the value with the property's boolean value, otherwise returns an error. */
        nonstd::expected<bool, ZenError> getBool(ZenProperty_t property) noexcept override;

        /** If successful fills the value with the property's floating-point value, otherwise returns an error. */
        nonstd::expected<float, ZenError> getFloat(ZenProperty_t property) noexcept override;

        /** If successful fills the value with the property's integer value, otherwise returns an error. */
        nonstd::expected<int32_t, ZenError> getInt32(ZenProperty_t property) noexcept override;

        /** If successful fills the value with the property's matrix value, otherwise returns an error. */
        nonstd::expected<ZenMatrix3x3f, ZenError> getMatrix33(ZenProperty_t property) noexcept override;

        /** If successful fills the buffer with the property's string value and sets the buffer's string size.
         * Otherwise, returns an error and potentially sets the desired buffer size - if it is too small.
         */
        std::pair<ZenError, size_t> getString(ZenProperty_t property, gsl::span<char> buffer) noexcept override;

        /** If successful fills the value with the property's unsigned integer value, otherwise returns an error. */
        nonstd::expected<uint64_t, ZenError> getUInt64(ZenProperty_t) noexcept override { return nonstd::make_unexpected(ZenError_UnknownProperty); }

        /** If successful sets the array properties, otherwise returns an error. */
        ZenError setArray(ZenProperty_t property, ZenPropertyType type, gsl::span<const std::byte> buffer) noexcept override;

        /** If successful sets the boolean property, otherwise returns an error. */
        ZenError setBool(ZenProperty_t property, bool value) noexcept override;

        /** If successful sets the floating-point property, otherwise returns an error. */
        ZenError setFloat(ZenProperty_t property, float value) noexcept override;

        /** If successful sets the integer property, otherwise returns an error. */
        ZenError setInt32(ZenProperty_t property, int32_t value) noexcept override;

        /** If successful sets the matrix property, otherwise returns an error. */
        ZenError setMatrix33(ZenProperty_t property, const ZenMatrix3x3f& value) noexcept override;

        /** If successful sets the string property, otherwise returns an error. */
        ZenError setString(ZenProperty_t, gsl::span<const char>) noexcept override { return ZenError_UnknownProperty; }

        /** If successful sets the unsigned integer property, otherwise returns an error. */
        ZenError setUInt64(ZenProperty_t, uint64_t) noexcept override { return ZenError_UnknownProperty; }

        /** Returns whether the property is an array type */
        bool isArray(ZenProperty_t property) const noexcept override;

        /** Returns whether the property can be executed as a command */
        bool isExecutable(ZenProperty_t property) const noexcept override;

        /** Returns whether the property is constant. If so, the property cannot be set */
        bool isConstant(ZenProperty_t property) const noexcept override;

        /** Returns the type of the property */
        ZenPropertyType type(ZenProperty_t property) const noexcept override;

        /** Manually initializes the output-data bitset */
        void setOutputDataBitset(uint32_t bitset) noexcept { m_cache.outputDataBitset = bitset; }

    private:
        bool getConfigDataFlag(unsigned int index) noexcept;

        ZenError setOutputDataFlag(unsigned int index, bool value) noexcept;
        ZenError setPrecisionDataFlag(bool value) noexcept;

        struct IMUState
        {
            std::atomic<LpMatrix3x3f> accAlignMatrix;
            std::atomic<LpMatrix3x3f> gyrAlignMatrix;
            std::atomic<LpMatrix3x3f> softIronMatrix;
            std::atomic<LpVector3f> accBias;
            std::atomic<LpVector3f> gyrBias;
            std::atomic<LpVector3f> hardIronOffset;
            std::atomic_uint32_t outputDataBitset;
            std::atomic_bool gyrAutoCalibration;
            std::atomic_bool gyrUseThreshold;
        } m_cache;

        SyncedModbusCommunicator& m_communicator;

        std::atomic_bool m_streaming;
    };
}

#endif
