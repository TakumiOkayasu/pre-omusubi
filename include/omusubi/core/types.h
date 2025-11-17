#pragma once

#include <cstdint>

namespace omusubi {

/**
 * @brief 電源状態
 */
enum class PowerState : uint8_t {
    BATTERY,  ///< バッテリー駆動
    USB,      ///< USB給電
    EXTERNAL, ///< 外部電源
    UNKNOWN   ///< 不明
};

/**
 * @brief ボタン状態
 */
enum class ButtonState : uint8_t {
    PRESSED, ///< 押されている
    RELEASED ///< 離されている
};

/**
 * @brief 3次元ベクトル
 */
struct Vector3 {
    float x; ///< X成分
    float y; ///< Y成分
    float z; ///< Z成分

    constexpr Vector3() noexcept : x(0.0f), y(0.0f), z(0.0f) {}

    constexpr Vector3(float x_val, float y_val, float z_val) noexcept : x(x_val), y(y_val), z(z_val) {}
};

} // namespace omusubi
