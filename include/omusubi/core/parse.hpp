#pragma once

#include <omusubi/core/types.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <omusubi/core/result.hpp>
#include <string_view>
#include <type_traits>

namespace omusubi {

/**
 * @brief 文字列から符号なし整数への変換
 *
 * @tparam T 変換先の型（uint8_t, uint16_t, uint32_t, uint64_t, size_t）
 * @param sv 変換元の文字列
 * @param base 基数（2〜36、デフォルト10）
 * @return 変換結果またはエラー
 */
template <typename T>
[[nodiscard]] constexpr auto parse_uint(std::string_view sv, int base = 10) noexcept -> Result<T, Error> {
    static_assert(std::is_unsigned_v<T>, "T must be unsigned integer type");

    if (sv.empty()) {
        return Result<T, Error>::err(Error::INVALID_DATA);
    }

    if (base < 2 || base > 36) {
        return Result<T, Error>::err(Error::INVALID_PARAMETER);
    }

    T result = 0;
    const T max_before_multiply = std::numeric_limits<T>::max() / static_cast<T>(base);

    for (char c : sv) {
        int digit = -1;

        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'z') {
            digit = 10 + (c - 'a');
        } else if (c >= 'A' && c <= 'Z') {
            digit = 10 + (c - 'A');
        }

        if (digit < 0 || digit >= base) {
            return Result<T, Error>::err(Error::INVALID_DATA);
        }

        // オーバーフローチェック
        if (result > max_before_multiply) {
            return Result<T, Error>::err(Error::INVALID_DATA);
        }

        result *= static_cast<T>(base);

        if (result > std::numeric_limits<T>::max() - static_cast<T>(digit)) {
            return Result<T, Error>::err(Error::INVALID_DATA);
        }

        result += static_cast<T>(digit);
    }

    return Result<T, Error>::ok(result);
}

/**
 * @brief 文字列から符号付き整数への変換
 *
 * @tparam T 変換先の型（int8_t, int16_t, int32_t, int64_t, ptrdiff_t）
 * @param sv 変換元の文字列
 * @param base 基数（2〜36、デフォルト10）
 * @return 変換結果またはエラー
 */
template <typename T>
[[nodiscard]] constexpr auto parse_int(std::string_view sv, int base = 10) noexcept -> Result<T, Error> {
    static_assert(std::is_signed_v<T>, "T must be signed integer type");

    if (sv.empty()) {
        return Result<T, Error>::err(Error::INVALID_DATA);
    }

    bool negative = false;
    if (sv[0] == '-') {
        negative = true;
        sv = sv.substr(1);
    } else if (sv[0] == '+') {
        sv = sv.substr(1);
    }

    if (sv.empty()) {
        return Result<T, Error>::err(Error::INVALID_DATA);
    }

    // 符号なし型でパース
    using UnsignedT = std::make_unsigned_t<T>;
    auto unsigned_result = parse_uint<UnsignedT>(sv, base);

    if (!unsigned_result.is_ok()) {
        return Result<T, Error>::err(unsigned_result.error());
    }

    UnsignedT value = unsigned_result.value();

    // 範囲チェック
    if (negative) {
        // 負の最小値の絶対値は正の最大値+1
        UnsignedT abs_min = static_cast<UnsignedT>(std::numeric_limits<T>::max()) + 1U;
        if (value > abs_min) {
            return Result<T, Error>::err(Error::INVALID_DATA);
        }
        return Result<T, Error>::ok(static_cast<T>(-static_cast<T>(value)));
    }

    if (value > static_cast<UnsignedT>(std::numeric_limits<T>::max())) {
        return Result<T, Error>::err(Error::INVALID_DATA);
    }

    return Result<T, Error>::ok(static_cast<T>(value));
}

/**
 * @brief 文字列から浮動小数点数への変換
 *
 * @tparam T 変換先の型（float, double）
 * @param sv 変換元の文字列
 * @return 変換結果またはエラー
 *
 * @note 指数表記（1.5e10）には対応していません
 */
template <typename T>
[[nodiscard]] constexpr auto parse_float(std::string_view sv) noexcept -> Result<T, Error> {
    static_assert(std::is_floating_point_v<T>, "T must be floating point type");

    if (sv.empty()) {
        return Result<T, Error>::err(Error::INVALID_DATA);
    }

    bool negative = false;
    if (sv[0] == '-') {
        negative = true;
        sv = sv.substr(1);
    } else if (sv[0] == '+') {
        sv = sv.substr(1);
    }

    if (sv.empty()) {
        return Result<T, Error>::err(Error::INVALID_DATA);
    }

    T result = 0;
    bool has_decimal = false;
    T decimal_multiplier = static_cast<T>(0.1);
    bool has_digit = false;

    for (char c : sv) {
        if (c == '.') {
            if (has_decimal) {
                // 2つ目の小数点
                return Result<T, Error>::err(Error::INVALID_DATA);
            }
            has_decimal = true;
            continue;
        }

        if (c < '0' || c > '9') {
            return Result<T, Error>::err(Error::INVALID_DATA);
        }

        has_digit = true;
        int digit = c - '0';

        if (has_decimal) {
            result += static_cast<T>(digit) * decimal_multiplier;
            decimal_multiplier *= static_cast<T>(0.1);
        } else {
            result = result * static_cast<T>(10) + static_cast<T>(digit);
        }
    }

    if (!has_digit) {
        return Result<T, Error>::err(Error::INVALID_DATA);
    }

    if (negative) {
        result = -result;
    }

    return Result<T, Error>::ok(result);
}

// === 便利な型エイリアス関数 ===

/**
 * @brief 文字列からint32_tへの変換
 */
[[nodiscard]] inline constexpr auto parse_i32(std::string_view sv, int base = 10) noexcept -> Result<int32_t, Error> {
    return parse_int<int32_t>(sv, base);
}

/**
 * @brief 文字列からuint32_tへの変換
 */
[[nodiscard]] inline constexpr auto parse_u32(std::string_view sv, int base = 10) noexcept -> Result<uint32_t, Error> {
    return parse_uint<uint32_t>(sv, base);
}

/**
 * @brief 文字列からint64_tへの変換
 */
[[nodiscard]] inline constexpr auto parse_i64(std::string_view sv, int base = 10) noexcept -> Result<int64_t, Error> {
    return parse_int<int64_t>(sv, base);
}

/**
 * @brief 文字列からuint64_tへの変換
 */
[[nodiscard]] inline constexpr auto parse_u64(std::string_view sv, int base = 10) noexcept -> Result<uint64_t, Error> {
    return parse_uint<uint64_t>(sv, base);
}

/**
 * @brief 文字列からfloatへの変換
 */
[[nodiscard]] inline constexpr auto parse_f32(std::string_view sv) noexcept -> Result<float, Error> {
    return parse_float<float>(sv);
}

/**
 * @brief 文字列からdoubleへの変換
 */
[[nodiscard]] inline constexpr auto parse_f64(std::string_view sv) noexcept -> Result<double, Error> {
    return parse_float<double>(sv);
}

} // namespace omusubi
