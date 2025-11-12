#pragma once

#include "omusubi/core/types.h"

namespace omusubi {

/**
 * @brief 電源管理コンテキスト
 *
 * 電源状態、バッテリーレベルなどの電源管理機能へのアクセスを提供します。
 *
 * 使用例:
 * @code
 * PowerContext* power = ctx.get_power();
 *
 * // 電源状態を取得
 * PowerState state = power->get_power_state();
 *
 * // バッテリーレベルを取得
 * uint8_t level = power->get_battery_level();
 * @endcode
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

}  // namespace omusubi
