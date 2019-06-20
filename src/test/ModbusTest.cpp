#include <gtest/gtest.h>

#include "communication/Modbus.h"

#include <vector>

TEST(Modbus, parsePacket) {

    const auto checksum = 10 + 11 + 4 + 1 + 2 + 3 + 4;

    std::vector<std::byte> vecValidPacket = {
        // start signal
        std::byte(0x3a),
        // address
        std::byte(10), std::byte(0),
        // function
        std::byte(11), std::byte(0),
        // data length
        std::byte(4), std::byte(0),
        // data
        std::byte(1), std::byte(2), std::byte(3), std::byte(4),
        // checksum
        std::byte(checksum), std::byte(0),
        // end signal
        std::byte(0x0d), std::byte(0x0a)
    };

    gsl::span<const std::byte> validPacket(vecValidPacket);
    zen::modbus::LpFrameParser lpParser;

    ASSERT_FALSE(lpParser.finished());
    lpParser.parse(validPacket);

    const auto frame = lpParser.frame();
    ASSERT_EQ(10, frame.address);
    ASSERT_EQ(11, frame.function);
    ASSERT_EQ(4, frame.data.size());
    for (size_t i = 0; i < 4; i++) {
        ASSERT_EQ(std::byte(i+1), frame.data[i]);
    }

    ASSERT_TRUE(lpParser.finished());
}