#pragma once

#include "omusubi/context/scannable_context.h"
#include "m5stack_bluetooth_context.hpp"
#include "m5stack_wifi_context.hpp"
#include "m5stack_ble_context.hpp"

namespace omusubi {
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用スキャン可能デバイスコンテキスト
 */
class M5StackScannableContext : public ScannableContext {
private:
    M5StackBluetoothContext* bluetooth_;
    M5StackWiFiContext* wifi_;
    M5StackBLEContext* ble_;

public:
    M5StackScannableContext(
        M5StackBluetoothContext* bluetooth,
        M5StackWiFiContext* wifi,
        M5StackBLEContext* ble)
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
