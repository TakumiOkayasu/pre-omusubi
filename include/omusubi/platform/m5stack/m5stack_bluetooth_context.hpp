#pragma once

#include "omusubi/device/bluetooth_context.h"

namespace omusubi {
namespace platform {
namespace m5stack {

// 前方宣言
class BluetoothSerial;
class BTAdvertisedDevice;
class BTScanResults;

/**
 * @brief M5Stack用Bluetooth Classic通信実装
 *
 * Readable + Writable + Connectable + Scannableインターフェースを実装
 */
class M5StackBluetoothContext : public BluetoothContext {
private:
    BluetoothSerial bt_;
    bool connected_;
    char local_name_[64];

    // スキャン結果
    struct FoundDevice {
        char name[64];
        char address[32];
        int32_t rssi;
    };
    FoundDevice found_devices_[10];
    uint8_t found_count_;
    bool scanning_;

public:
    M5StackBluetoothContext();
    ~M5StackBluetoothContext() override;

    // Readable interface
    FixedBuffer<256> read() override;

    // Writable interface
    void write(StringView text) override;

    // Connectable interface
    bool connect() override;

    // Scannable interface
    uint8_t scan() override;
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
