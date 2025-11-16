#pragma once

#include <cstddef>
#include <cstdint>

#include "string_base.hpp"

namespace omusubi {

/**
 * @brief コンパイル時文字列クラス
 *
 * - 完全にコンパイル時評価される
 * - 実行時オーバーヘッドゼロ
 * - constexprコンテキストで使用可能
 */
template <uint32_t N>
class StaticString : public String<StaticString<N>> {
public:
    constexpr uint32_t size() const noexcept { return N; }

    constexpr uint32_t byte_length() const noexcept { return N; }

    constexpr const char* data() const noexcept { return data_; }

    constexpr char* data() noexcept { return data_; }

    constexpr const char* c_str() const noexcept { return data_; }

    constexpr char operator[](uint32_t index) const noexcept { return data_[index]; }

    constexpr char& operator[](uint32_t index) noexcept { return data_[index]; }

    template <uint32_t Offset, uint32_t Length>
    constexpr StaticString<Length> substring() const noexcept {
        static_assert(Offset + Length <= N, "Substring out of range");

        StaticString<Length> result {};

        for (uint32_t i = 0; i < Length; ++i) {
            result[i] = data_[Offset + i];
        }

        result[Length] = '\0';

        return result;
    }

    template <int Value>
    static constexpr auto from_int() noexcept {
        constexpr uint32_t DIGITS = count_digits(Value);
        StaticString<DIGITS> result {};

        int val = Value;
        const bool negative = (val < 0);

        if (negative) {
            val = -val;
        }

        uint32_t pos = DIGITS;
        result[pos] = '\0';

        if (val == 0) {
            result[0] = '0';
        } else {
            while (val > 0) {
                --pos;
                result[pos] = '0' + (val % 10);
                val /= 10;
            }

            if (negative) {
                result[0] = '-';
            }
        }

        return result;
    }

private:
    static constexpr uint32_t count_digits(int value) noexcept {
        if (value == 0) {
            return 1;
        }

        uint32_t count = 0;

        if (value < 0) {
            ++count;
            value = -value;
        }

        while (value > 0) {
            ++count;
            value /= 10;
        }

        return count;
    }

    char data_[N + 1];
};

/**
 * @brief 文字列リテラルからコンパイル時文字列を構築
 */
template <uint32_t N>
constexpr StaticString<N - 1> static_string(const char (&str)[N]) noexcept {
    StaticString<N - 1> result {};

    for (uint32_t i = 0; i < N - 1; ++i) {
        result[i] = str[i];
    }

    result[N - 1] = '\0';

    return result;
}

/**
 * @brief 2つの文字列を連結（コンパイル時）
 */
template <uint32_t N1, uint32_t N2>
constexpr StaticString<N1 + N2> operator+(const StaticString<N1>& a, const StaticString<N2>& b) noexcept {
    StaticString<N1 + N2> result {};

    for (uint32_t i = 0; i < N1; ++i) {
        result[i] = a[i];
    }

    for (uint32_t i = 0; i < N2; ++i) {
        result[N1 + i] = b[i];
    }

    result[N1 + N2] = '\0';

    return result;
}

template <uint32_t N1, uint32_t N2>
constexpr bool operator==(const StaticString<N1>& a, const StaticString<N2>& b) noexcept {
    return a.equals(b);
}

template <uint32_t N1, uint32_t N2>
constexpr bool operator!=(const StaticString<N1>& a, const StaticString<N2>& b) noexcept {
    return !a.equals(b);
}

} // namespace omusubi
