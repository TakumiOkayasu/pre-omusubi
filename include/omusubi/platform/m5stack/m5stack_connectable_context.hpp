#pragma once

#include "omusubi/context/connectable_context.h"
#include "m5stack_serial_context.hpp"
#include "m5stack_bluetooth_context.hpp"
#include "m5stack_wifi_context.hpp"
#include "m5stack_ble_context.hpp"

namespace omusubi {
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用接続可能デバイスコンテキスト
 *
 * Pattern A: Context owns devices (CLAUDE.mdに準拠)
 * メンバ変数としてデバイスを所有し、それらへのポインタを返す
 */
class M5StackConnectableContext : public ConnectableContext {
private:
    M5StackSerialContext serial0_{0};
    M5StackSerialContext serial1_{1};
    M5StackSerialContext serial2_{2};
    M5StackBluetoothContext bluetooth_;
    M5StackWiFiContext wifi_;
    M5StackBLEContext ble_;

public:
    M5StackConnectableContext() = default;
    ~M5StackConnectableContext() override = default;

    SerialContext* get_serial0_context() override { return &serial0_; }
    SerialContext* get_serial1_context() override { return &serial1_; }
    SerialContext* get_serial2_context() override { return &serial2_; }
    BluetoothContext* get_bluetooth_context() override { return &bluetooth_; }
    WiFiContext* get_wifi_context() override { return &wifi_; }
    BLEContext* get_ble_context() override { return &ble_; }
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
