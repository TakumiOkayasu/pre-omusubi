#pragma once

#include "omusubi/context/power_context.h"

namespace omusubi {
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用電源管理コンテキスト
 */
class M5StackPowerContext : public PowerContext {
public:
    M5StackPowerContext() = default;
    ~M5StackPowerContext() override = default;

    PowerState get_power_state() const override;
    uint8_t get_battery_level() const override;
    bool is_charging() const override;
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
