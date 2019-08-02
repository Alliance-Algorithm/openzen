#ifndef ZEN_COMMUNICATION_MOCKMODBUSCOMMUNICATOR_H_
#define ZEN_COMMUNICATION_MOCKMODBUSCOMMUNICATOR_H_

#include "communication/Modbus.h"
#include "communication/ModbusCommunicator.h"

#include "spdlog/spdlog.h"

#include <algorithm>
#include <thread>
#include <future>
#include <chrono>

namespace zen {

class DummyFrameFactory : public modbus::IFrameFactory {
public:
 std::vector<std::byte> makeFrame(uint8_t address, uint8_t function,
   const std::byte* data, uint8_t length) const override{
    return {};
  }
};

class DummyFrameParser : public modbus::IFrameParser {
public:
  modbus::FrameParseError parse(gsl::span<const std::byte>& data) override {
    return modbus::FrameParseError_None;
   }
  void reset() override {}

  bool finished() const override { return true; }
};

class MockbusCommunicator : public ModbusCommunicator {
public:

  /**
    Entries of tuple have these meanings:
    1: address
    2: function
    3: reply buffer to send back
  */
  typedef std::vector< std::tuple< uint8_t, uint8_t, std::vector<std::byte>>> RepliesVector;

  MockbusCommunicator(IModbusFrameSubscriber& subscriber, RepliesVector replies) noexcept :
  ModbusCommunicator( subscriber, std::make_unique<DummyFrameFactory>(),
  std::make_unique<DummyFrameParser>() ), m_replies(replies)
    {

  }

  ZenError send(uint8_t address, uint8_t function, gsl::span<const std::byte> data) noexcept override {
    // check if we can supply that result
    auto itReply = std::find_if(m_replies.begin(), m_replies.end(),
      [address,function](auto const& entry ){
      return std::get<0>(entry) == address && std::get<1>(entry) == function;
    } );

    if (itReply != m_replies.end()) {
      auto local_subscriber = m_subscriber;
      std::vector<std::byte> reply_data = std::get<2>(*itReply);
      m_futureReplies.emplace_back(std::async([local_subscriber, address, function, reply_data]()
      {
        std::vector<std::byte> bb = reply_data;
        std::this_thread::sleep_for( std::chrono::milliseconds(100));
        gsl::span<std::byte> byteSpan(bb.data(), bb.size());
        local_subscriber->processReceivedData(address, function, byteSpan );
      }));

      //aa.set(23);
    } else {
      spdlog::error("Mock reply for address {} and function {} not found",
        address, function);
    }

    return ZenError_None;
  }

  /** Set Baudrate of IO interface (bit/s) */
  ZenError setBaudRate(unsigned int rate) noexcept override {
    return ZenError_None;
  }


private:
  const RepliesVector m_replies;
  std::vector<std::future<void>> m_futureReplies;

};

}

#endif
