#pragma once

#include "omusubi/core/types.h"

namespace omusubi {

/**
 * @brief ボタン入力インターフェース
 *
 * ボタンやタッチパネルなどの押下可能な入力デバイスを扱います。
 *
 * 使用例:
 * @code
 * Pressable* button = ctx.get_input()->get_button0_context();
 *
 * // ボタン状態を取得
 * ButtonState state = button->get_state();
 * if (state == ButtonState::PRESSED) {
 *     // ボタンが押されている
 * }
 * @endcode
 *
 * @note ctx.update()を呼ばないと状態が更新されないため、loop()内で必ずupdateを呼ぶこと
 */
class Pressable {
public:
    Pressable() = default;
    virtual ~Pressable() = default;
    Pressable(const Pressable&) = delete;
    Pressable& operator=(const Pressable&) = delete;
    Pressable(Pressable&&) = delete;
    Pressable& operator=(Pressable&&) = delete;

    /** @brief ボタン状態を取得 @return ボタンの現在の状態 */
    virtual ButtonState get_state() const = 0;
};

}  // namespace omusubi