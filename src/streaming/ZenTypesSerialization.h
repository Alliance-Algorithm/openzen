#ifndef ZEN_TYPES_SERIALIZATION_H_
#define ZEN_TYPES_SERIALIZATION_H_

#include "ZenTypes.h"

namespace zen {
    namespace Serialization {
        /**
        Cannot use unions in serialization, therefore dedicated wrapper classes
        for the serialization
        */
        class ZenEventSerializationBase {
        public:
            uint64_t sensor;
            uint64_t component;
        };

        class ZenEventImuSerialization : public ZenEventSerializationBase {
        public:
            ZenImuData data;
        };

        class ZenEventGnssSerialization : public ZenEventSerializationBase {
        public:
            ZenGnssData data;
        };

        template <class Archive>
        void serialize(Archive& ar, ZenEventImuSerialization& wrapper) {
            ar(wrapper.sensor);
            ar(wrapper.component);
            ar(wrapper.data);
        }

        template <class Archive>
        void serialize(Archive& ar, zen::Serialization::ZenEventGnssSerialization& wrapper) {
            ar(wrapper.sensor);
            ar(wrapper.component);
            ar(wrapper.data);
        }
    }
}

template <class Archive>
void serialize(Archive& ar, ZenHeaveMotionData& hmData)
{
    ar(hmData.yHeave);
}

template <class Archive>
void serialize(Archive& ar, ZenImuData & imuData)
{
    ar(imuData.a,
        imuData.g,
        imuData.b,
        imuData.aRaw,
        imuData.gRaw,
        imuData.bRaw,
        imuData.w,
        imuData.r,
        imuData.q,
        imuData.rotationM,
        imuData.rotOffsetM,
        imuData.pressure,
        imuData.frameCount,
        imuData.linAcc,
        imuData.gTemp,
        imuData.altitude,
        imuData.temperature,
        imuData.timestamp, 
        imuData.hm);
}

template <class Archive>
void serialize(Archive& ar, ZenGnssData& gnssData)
{
    ar(gnssData. timestamp,
        gnssData.latitude,
        gnssData.horizontalAccuracy,
        gnssData.longitude,
        gnssData.verticalAccuracy,
        gnssData.height,
        gnssData.heading,
        gnssData.headingAccuracy,
        gnssData.velocity,
        gnssData.velocityAccuracy,
        gnssData.fixType,
        gnssData.carrierPhaseSolution,
        gnssData.numberSatellitesUsed,
        gnssData.year,
        gnssData.month,
        gnssData.day,
        gnssData.hour,
        gnssData.minute,
        gnssData.second,
        gnssData.nanoSecondCorrection);
}

#endif