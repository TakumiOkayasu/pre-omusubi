#pragma once

#include "omusubi/device/ble_context.h"

namespace omusubi {
namespace platform {
namespace m5stack {

// 前方宣言
class BLEScan;

/**
 * @brief M5Stack用BLE通信実装
 *
 * Connectable + Scannableインターフェースを実装
 */
class M5StackBLEContext : public BLEContext {
private:
    bool initialized_;
    bool connected_;
    BLEScan* scan_;
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
    M5StackBLEContext();
    ~M5StackBLEContext() override;

    // Connectable interface
    bool connect() override;

    // Scannable interface
    uint8_t scan() override;
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
