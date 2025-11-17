#pragma once

#include "omusubi/core/types.h"

#include <cstdint>

namespace omusubi {

/**
 * @brief 電源管理コンテキスト
 */
class PowerContext {
public:
    PowerContext() = default;
    virtual ~PowerContext() = default;
    PowerContext(const PowerContext&) = delete;
    PowerContext& operator=(const PowerContext&) = delete;
    PowerContext(PowerContext&&) = delete;
    PowerContext& operator=(PowerContext&&) = delete;

    /** @brief 電源状態を取得 */
    virtual PowerState get_power_state() const = 0;

    /** @brief バッテリーレベルを取得（0-100%） */
    virtual uint8_t get_battery_level() const = 0;

    /** @brief 充電中かどうか */
    virtual bool is_charging() const = 0;
};

} // namespace omusubi
