#include "omusubi/platform/m5stack/m5stack_system_context.hpp"
#include <M5Stack.h>
#include <esp_system.h>

namespace omusubi {
namespace platform {
namespace m5stack {

// ========================================
// コンストラクタ
// ========================================

M5StackSystemContext::M5StackSystemContext()
    : connectable_()
    , readable_(
        connectable_.get_serial0_context(),
        connectable_.get_serial1_context(),
        connectable_.get_serial2_context(),
        connectable_.get_bluetooth_context())
    , writable_(
        connectable_.get_serial0_context(),
        connectable_.get_serial1_context(),
        connectable_.get_serial2_context(),
        connectable_.get_bluetooth_context())
    , scannable_(
        connectable_.get_bluetooth_context(),
        connectable_.get_wifi_context(),
        connectable_.get_ble_context())
    , sensor_()
    , input_()
    , output_()
    , system_info_()
    , power_() {
}

// ========================================
// システム制御
// ========================================

void M5StackSystemContext::begin() {
    // M5Stackの初期化
    M5.begin();

    // Serial0を自動的に接続
    connectable_.get_serial0_context()->connect();
}

void M5StackSystemContext::update() {
    // M5Stackの更新
    M5.update();
}

void M5StackSystemContext::delay(uint32_t ms) {
    ::delay(ms);
}

void M5StackSystemContext::reset() {
    esp_restart();
}

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
