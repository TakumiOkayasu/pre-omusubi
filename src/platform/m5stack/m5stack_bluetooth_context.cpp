#include "omusubi/platform/m5stack/m5stack_bluetooth_context.hpp"
#include <BluetoothSerial.h>

namespace omusubi {
namespace platform {
namespace m5stack {

M5StackBluetoothContext::M5StackBluetoothContext()
    : connected_(false)
    , found_count_(0)
    , scanning_(false) {

    strncpy(local_name_, "M5Stack", sizeof(local_name_));
    local_name_[sizeof(local_name_) - 1] = '\0';
}

M5StackBluetoothContext::~M5StackBluetoothContext() {
    if (connected_) {
        bt_.disconnect();
    }
}

// ========================================
// Readable実装
// ========================================

FixedBuffer<256> M5StackBluetoothContext::read() {
    FixedBuffer<256> result;
    if (!connected_) {
        return result;
    }

    // 利用可能なバイトを全て読み込む
    while (bt_.available() && result.size() < 256) {
        uint8_t byte = static_cast<uint8_t>(bt_.read());
        result.append(byte);
    }

    return result;
}

// ========================================
// Writable実装
// ========================================

void M5StackBluetoothContext::write(StringView text) {
    if (!connected_) {
        return;
    }

    // StringViewの内容を書き込む
    for (uint32_t i = 0; i < text.byte_length(); ++i) {
        bt_.write(static_cast<uint8_t>(text[i]));
    }
}

// ========================================
// Connectable実装
// ========================================

bool M5StackBluetoothContext::connect() {
    if (connected_) {
        return true;
    }

    // ローカル名でBluetooth開始
    bt_.begin(local_name_);
    connected_ = true;
    return true;
}

// ========================================
// Scannable実装
// ========================================

uint8_t M5StackBluetoothContext::scan() {
    if (scanning_) {
        return found_count_;
    }

    found_count_ = 0;
    scanning_ = true;

    // Bluetoothデバイスをスキャン（5秒間）
    BTScanResults* results = bt_.discover(5000);

    if (results) {
        uint8_t count = results->getCount();
        found_count_ = (count < 10) ? count : 10;

        // 見つかったデバイスの情報を保存
        for (uint8_t i = 0; i < found_count_; ++i) {
            BTAdvertisedDevice* device = results->getDevice(i);

            // デバイス名
            const char* name = device->getName().c_str();
            strncpy(found_devices_[i].name, name, sizeof(found_devices_[i].name) - 1);
            found_devices_[i].name[sizeof(found_devices_[i].name) - 1] = '\0';

            // MACアドレス
            const char* addr = device->getAddress().toString().c_str();
            strncpy(found_devices_[i].address, addr, sizeof(found_devices_[i].address) - 1);
            found_devices_[i].address[sizeof(found_devices_[i].address) - 1] = '\0';

            // RSSI
            found_devices_[i].rssi = device->getRSSI();
        }
    }

    scanning_ = false;
    return found_count_;
}

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
