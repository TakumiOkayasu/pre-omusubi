#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace omusubi::utf8 {

/**
 * @brief UTF-8文字の先頭バイトからバイト長を取得
 */
constexpr uint8_t get_char_byte_length(uint8_t first_byte) noexcept {
    if ((first_byte & 0x80) == 0x00) {
        return 1;
    }

    if ((first_byte & 0xE0) == 0xC0) {
        return 2;
    }

    if ((first_byte & 0xF0) == 0xE0) {
        return 3;
    }

    if ((first_byte & 0xF8) == 0xF0) {
        return 4;
    }

    return 1;
}

/**
 * @brief UTF-8文字列の文字数を取得
 */
constexpr uint32_t count_chars(const char* str, uint32_t byte_length) noexcept {
    uint32_t char_count = 0;
    uint32_t i = 0;

    while (i < byte_length) {
        const uint8_t char_len = get_char_byte_length(static_cast<uint8_t>(str[i]));
        i += char_len;
        ++char_count;
    }

    return char_count;
}

/**
 * @brief UTF-8文字インデックスからバイト位置を取得
 */
constexpr uint32_t get_char_position(const char* str, uint32_t byte_length, uint32_t char_index) noexcept {
    uint32_t current_char = 0;
    uint32_t i = 0;

    while (i < byte_length && current_char < char_index) {
        const uint8_t char_len = get_char_byte_length(static_cast<uint8_t>(str[i]));
        i += char_len;
        ++current_char;
    }

    return i;
}

} // namespace omusubi::utf8

namespace omusubi {

/**
 * @brief std::string_view の UTF-8 文字数を取得
 */
[[nodiscard]] constexpr uint32_t char_length(std::string_view sv) noexcept {
    return utf8::count_chars(sv.data(), static_cast<uint32_t>(sv.size()));
}

/**
 * @brief std::string_view の UTF-8 文字インデックスからバイト位置を取得
 */
[[nodiscard]] constexpr uint32_t get_char_position(std::string_view sv, uint32_t char_index) noexcept {
    return utf8::get_char_position(sv.data(), static_cast<uint32_t>(sv.size()), char_index);
}

/**
 * @brief std::string_view が空かどうか判定
 */
[[nodiscard]] constexpr bool is_empty(std::string_view sv) noexcept {
    return sv.empty();
}

/**
 * @brief 2つの std::string_view が等しいか判定
 */
[[nodiscard]] constexpr bool equals(std::string_view a, std::string_view b) noexcept {
    return a == b;
}

/**
 * @brief C文字列から std::string_view を構築
 */
[[nodiscard]] inline std::string_view from_c_string(const char* str) noexcept {
    if (str == nullptr) {
        return std::string_view {};
    }

    return std::string_view {str};
}

} // namespace omusubi
