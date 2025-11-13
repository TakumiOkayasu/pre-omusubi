#pragma once

#include "omusubi/context/scannable_context.h"

namespace omusubi {

// 前方宣言
class BluetoothContext;
class WiFiContext;
class BLEContext;
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用スキャン可能デバイスコンテキスト
 */
class M5StackScannableContext : public ScannableContext {
private:
    BluetoothContext* bluetooth_;
    WiFiContext* wifi_;
    BLEContext* ble_;

public:
    M5StackScannableContext(
        BluetoothContext* bluetooth,
        WiFiContext* wifi,
        BLEContext* ble)
        : bluetooth_(bluetooth)
        , wifi_(wifi)
        , ble_(ble) {}

    ~M5StackScannableContext() override = default;

    BluetoothContext* get_bluetooth_context() override { return bluetooth_; }
    WiFiContext* get_wifi_context() override { return wifi_; }
    BLEContext* get_ble_context() override { return ble_; }
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
