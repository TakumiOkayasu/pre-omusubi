#pragma once

#include <omusubi/core/string_view.h>

#include <cstdint>
#include <omusubi/core/span.hpp>
#include <omusubi/core/string_base.hpp>
#include <string_view>

namespace omusubi {

/**
 * @brief 固定長UTF-8文字列
 *
 * - UTF-8エンコーディング
 * - バイト長と文字数を両方管理
 * - null終端を保証
 * - 動的メモリ確保なし
 */
template <uint32_t Capacity>
class FixedString : public String<FixedString<Capacity>> {
public:
    /**
     * @brief デフォルトコンストラクタ
     */
    constexpr FixedString() noexcept : byte_length_(0) { buffer_[0] = '\0'; }

    /**
     * @brief C文字列から構築
     */
    constexpr explicit FixedString(const char* str) noexcept : byte_length_(0) {
        buffer_[0] = '\0';

        if (str != nullptr) {
            append(std::string_view {str});
        }
    }

    /**
     * @brief std::string_viewから構築
     */
    constexpr explicit FixedString(std::string_view view) noexcept : byte_length_(0) {
        buffer_[0] = '\0';
        append(view);
    }

    /**
     * @brief 容量を取得
     */
    [[nodiscard]] constexpr uint32_t capacity() const noexcept { return Capacity; }

    /**
     * @brief バイト長を取得
     */
    [[nodiscard]] constexpr uint32_t byte_length() const noexcept { return byte_length_; }

    /**
     * @brief サイズを取得（byte_length()のエイリアス、std::string互換）
     */
    [[nodiscard]] constexpr size_t size() const noexcept { return byte_length_; }

    /**
     * @brief データへのポインタを取得
     */
    [[nodiscard]] constexpr const char* data() const noexcept { return buffer_; }

    /**
     * @brief C文字列として取得（null終端保証）
     */
    [[nodiscard]] constexpr const char* c_str() const noexcept { return buffer_; }

    /**
     * @brief std::string_viewに変換
     */
    [[nodiscard]] constexpr std::string_view view() const noexcept { return std::string_view {buffer_, byte_length_}; }

    /**
     * @brief std::string_viewへの暗黙変換
     */
    [[nodiscard]] constexpr operator std::string_view() const noexcept { return view(); }

    /**
     * @brief 文字列を追加
     */
    constexpr bool append(std::string_view view) noexcept {
        const auto view_size = static_cast<uint32_t>(view.size());

        if (byte_length_ + view_size > Capacity) {
            return false;
        }

        for (uint32_t i = 0; i < view_size; ++i) {
            buffer_[byte_length_++] = view[i];
        }

        buffer_[byte_length_] = '\0';

        return true;
    }

    /**
     * @brief C文字列を追加
     */
    constexpr bool append(const char* str) noexcept {
        if (str == nullptr) {
            return false;
        }

        return append(from_c_string(str));
    }

    /**
     * @brief 1文字追加
     */
    constexpr bool append(char c) noexcept {
        if (byte_length_ >= Capacity) {
            return false;
        }

        buffer_[byte_length_++] = c;
        buffer_[byte_length_] = '\0';

        return true;
    }

    /**
     * @brief クリア
     */
    constexpr void clear() noexcept {
        byte_length_ = 0;
        buffer_[0] = '\0';
    }

    [[nodiscard]] bool operator==(std::string_view other) const noexcept { return String<FixedString<Capacity>>::equals(other); }

    [[nodiscard]] bool operator!=(std::string_view other) const noexcept { return !String<FixedString<Capacity>>::equals(other); }

    [[nodiscard]] constexpr std::string_view get_char(uint32_t char_index) const noexcept {
        uint32_t byte_pos = this->get_char_position(char_index);

        if (byte_pos >= byte_length_) {
            return std::string_view {};
        }

        uint8_t char_len = utf8::get_char_byte_length(static_cast<uint8_t>(buffer_[byte_pos]));

        return std::string_view {buffer_ + byte_pos, char_len};
    }

    /**
     * @brief イテレータ（開始）
     */
    [[nodiscard]] constexpr const char* begin() const noexcept { return buffer_; }

    /**
     * @brief イテレータ（終了）
     */
    [[nodiscard]] constexpr const char* end() const noexcept { return buffer_ + byte_length_; }

    /**
     * @brief spanとして取得（書き込み可能）
     */
    [[nodiscard]] constexpr span<char> as_span() noexcept { return span<char>(buffer_, byte_length_); }

    /**
     * @brief spanとして取得（読み取り専用）
     */
    [[nodiscard]] constexpr span<const char> as_span() const noexcept { return span<const char>(buffer_, byte_length_); }

    /**
     * @brief spanから構築
     */
    constexpr void from_span(span<const char> s) noexcept {
        byte_length_ = (s.size() < Capacity) ? static_cast<uint32_t>(s.size()) : Capacity;

        for (uint32_t i = 0; i < byte_length_; ++i) {
            buffer_[i] = s[i];
        }

        buffer_[byte_length_] = '\0';
    }

private:
    char buffer_[Capacity + 1]; // +1 for null terminator
    uint32_t byte_length_;
};

/**
 * @brief 文字列リテラルから固定長文字列を構築
 */
template <uint32_t N>
constexpr FixedString<N - 1> fixed_string(const char (&str)[N]) noexcept {
    return FixedString<N - 1>(str);
}

} // namespace omusubi
