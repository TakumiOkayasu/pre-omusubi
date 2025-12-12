#pragma once

#include <type_traits>

namespace omusubi {

/**
 * @brief 値を指定範囲内に制限する
 *
 * @tparam T 数値型
 * @param value 制限する値
 * @param min_val 最小値
 * @param max_val 最大値
 * @return min_val <= result <= max_val
 */
template <typename T>
[[nodiscard]] constexpr T clamp(T value, T min_val, T max_val) noexcept {
    static_assert(std::is_arithmetic_v<T>, "clamp requires arithmetic type");

    if (value < min_val) {
        return min_val;
    }
    if (value > max_val) {
        return max_val;
    }
    return value;
}

/**
 * @brief 値をあるレンジから別のレンジにマッピングする
 *
 * Arduino の map() 関数相当。
 * 入力値を入力範囲から出力範囲に線形変換する。
 *
 * @tparam T 数値型
 * @param value 変換する値
 * @param in_min 入力範囲の最小値
 * @param in_max 入力範囲の最大値
 * @param out_min 出力範囲の最小値
 * @param out_max 出力範囲の最大値
 * @return マッピングされた値
 *
 * @note 入力値が入力範囲外の場合も外挿される（クランプしない）
 */
template <typename T>
[[nodiscard]] constexpr T map_range(T value, T in_min, T in_max, T out_min, T out_max) noexcept {
    static_assert(std::is_arithmetic_v<T>, "map_range requires arithmetic type");

    // ゼロ除算防止
    if (in_max == in_min) {
        return out_min;
    }

    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * @brief 値をあるレンジから別のレンジにマッピングし、出力範囲にクランプする
 *
 * map_range + clamp の組み合わせ。
 *
 * @tparam T 数値型
 * @param value 変換する値
 * @param in_min 入力範囲の最小値
 * @param in_max 入力範囲の最大値
 * @param out_min 出力範囲の最小値
 * @param out_max 出力範囲の最大値
 * @return クランプされたマッピング値
 */
template <typename T>
[[nodiscard]] constexpr T map_range_clamped(T value, T in_min, T in_max, T out_min, T out_max) noexcept {
    T mapped = map_range(value, in_min, in_max, out_min, out_max);

    // out_min と out_max の大小関係を考慮
    T actual_min = (out_min < out_max) ? out_min : out_max;
    T actual_max = (out_min < out_max) ? out_max : out_min;

    return clamp(mapped, actual_min, actual_max);
}

/**
 * @brief 線形補間（lerp）
 *
 * a と b の間を t で補間する。
 *
 * @tparam T 浮動小数点型
 * @param a 開始値
 * @param b 終了値
 * @param t 補間係数（0.0 = a, 1.0 = b）
 * @return 補間された値
 */
template <typename T>
[[nodiscard]] constexpr T lerp(T a, T b, T t) noexcept {
    static_assert(std::is_floating_point_v<T>, "lerp requires floating point type");

    return a + t * (b - a);
}

/**
 * @brief 逆線形補間（inverse lerp）
 *
 * value が a と b の間のどの位置にあるかを返す。
 *
 * @tparam T 浮動小数点型
 * @param a 開始値
 * @param b 終了値
 * @param value 位置を求める値
 * @return 補間係数（0.0 = a, 1.0 = b）
 */
template <typename T>
[[nodiscard]] constexpr T inverse_lerp(T a, T b, T value) noexcept {
    static_assert(std::is_floating_point_v<T>, "inverse_lerp requires floating point type");

    // ゼロ除算防止
    if (b == a) {
        return static_cast<T>(0);
    }

    return (value - a) / (b - a);
}

/**
 * @brief 絶対値を返す
 *
 * @tparam T 数値型
 * @param value 値
 * @return |value|
 */
template <typename T>
[[nodiscard]] constexpr T abs(T value) noexcept {
    static_assert(std::is_arithmetic_v<T>, "abs requires arithmetic type");

    return (value < static_cast<T>(0)) ? -value : value;
}

/**
 * @brief 符号を返す
 *
 * @tparam T 数値型
 * @param value 値
 * @return -1, 0, or 1
 */
template <typename T>
[[nodiscard]] constexpr int sign(T value) noexcept {
    static_assert(std::is_arithmetic_v<T>, "sign requires arithmetic type");

    if (value < static_cast<T>(0)) {
        return -1;
    }
    if (value > static_cast<T>(0)) {
        return 1;
    }
    return 0;
}

/**
 * @brief 2つの値の最小値を返す
 *
 * @tparam T 比較可能な型
 * @param a 値1
 * @param b 値2
 * @return 小さい方の値
 */
template <typename T>
[[nodiscard]] constexpr const T& min(const T& a, const T& b) noexcept {
    return (a < b) ? a : b;
}

/**
 * @brief 2つの値の最大値を返す
 *
 * @tparam T 比較可能な型
 * @param a 値1
 * @param b 値2
 * @return 大きい方の値
 */
template <typename T>
[[nodiscard]] constexpr const T& max(const T& a, const T& b) noexcept {
    return (a > b) ? a : b;
}

} // namespace omusubi
