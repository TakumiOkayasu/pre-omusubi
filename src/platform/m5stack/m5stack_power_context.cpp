#include "omusubi/platform/m5stack/m5stack_power_context.hpp"
#include <M5Stack.h>

namespace omusubi {
namespace platform {
namespace m5stack {

PowerState M5StackPowerContext::get_power_state() const {
    // M5Stackの電源状態を取得
    // TODO: より正確な実装が必要
    if (M5.Power.isChargeFull()) {
        return PowerState::battery_full;
    } else if (M5.Power.isCharging()) {
        return PowerState::charging;
    } else {
        return PowerState::battery;
    }
}

uint8_t M5StackPowerContext::get_battery_level() const {
    // バッテリー電圧から推定
    float voltage = M5.Power.getBatteryVoltage() / 1000.0f;

    if (voltage >= 4.1f) return 100;
    if (voltage >= 4.0f) return 90;
    if (voltage >= 3.9f) return 80;
    if (voltage >= 3.8f) return 70;
    if (voltage >= 3.7f) return 50;
    if (voltage >= 3.6f) return 30;
    if (voltage >= 3.5f) return 10;

    return 0;
}

bool M5StackPowerContext::is_charging() const {
    return M5.Power.isCharging();
}

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
