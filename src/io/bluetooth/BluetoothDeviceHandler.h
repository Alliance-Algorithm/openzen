#ifndef ZEN_IO_BLUETOOTH_BLUETOOTHDEVICEHANDLER_H_
#define ZEN_IO_BLUETOOTH_BLUETOOTHDEVICEHANDLER_H_

#include <atomic>
#include <future>
#include <optional>

#include "qbluetoothsocket.h"
#include "qthread.h"

#include "nonstd/expected.hpp"

#include "ZenTypes.h"
#include "utility/Ownership.h"
#include "utility/LockingQueue.h"
#include "utility/ThreadFence.h"

namespace zen
{
    class BluetoothDeviceHandler : public QThread
    {
        Q_OBJECT

        static constexpr const unsigned char ZEN_SENSOR_UUID[16] { 0xcc, 0x77, 0xba, 0xf1, 0x42, 0x27, 0x4d, 0x8e, 0x9b, 0x46, 0xcb, 0xe7, 0x66, 0x16, 0xd5, 0x0b };
        static constexpr const unsigned char ZEN_IMUDATA_UUID[16] { 0xc1, 0x3c, 0x35, 0x55, 0x28, 0x11, 0xe1, 0xaa, 0x76, 0x48, 0x42, 0xb0, 0x80, 0xd7, 0xad, 0xe7 };

        using async_read_value_t = nonstd::expected<std::vector<unsigned char>, ZenError>;

    public:
        BluetoothDeviceHandler(uint64_t address);
        ~BluetoothDeviceHandler();

        BluetoothDeviceHandler(BluetoothDeviceHandler&&) = default;

        ZenError initialize();

        nonstd::expected<std::future<async_read_value_t>, ZenAsyncStatus> readAsync();

        ZenError send(const std::vector<unsigned char>& data);

        bool equals(uint64_t handle) const;

    private slots:
        void serviceDiscovered(const QBluetoothServiceInfo& service);
        void receiveData();

    private:
        void reset();
        void run() override;

        QBluetoothAddress m_address;
        std::unique_ptr<QBluetoothSocket> m_socket;

        Owner<std::vector<unsigned char>> m_buffer;
        Owner<std::optional<std::promise<async_read_value_t>>> m_promise;

        ThreadFence m_fence;
        std::atomic_bool m_connected;
        std::atomic<ZenError> m_error;
    };
}

#endif
