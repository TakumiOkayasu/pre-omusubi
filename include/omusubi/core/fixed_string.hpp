#pragma once

#include <cstddef>
#include <cstdint>

#include "span.hpp"
#include "string_base.hpp"
#include "string_view.h"

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
    FixedString() noexcept : byte_length_(0) { buffer_[0] = '\0'; }

    /**
     * @brief C文字列から構築
     */
    explicit FixedString(const char* str) noexcept : byte_length_(0) {
        buffer_[0] = '\0';
        if (str != nullptr) {
            append(str);
        }
    }

    /**
     * @brief StringViewから構築
     */
    explicit FixedString(StringView view) noexcept : byte_length_(0) {
        buffer_[0] = '\0';
        append(view);
    }

    /**
     * @brief 容量を取得
     */
    constexpr uint32_t capacity() const noexcept { return Capacity; }

    /**
     * @brief バイト長を取得
     */
    uint32_t byte_length() const noexcept { return byte_length_; }

    /**
     * @brief データへのポインタを取得
     */
    const char* data() const noexcept { return buffer_; }

    /**
     * @brief C文字列として取得（null終端保証）
     */
    const char* c_str() const noexcept { return buffer_; }

    /**
     * @brief StringViewに変換
     */
    StringView view() const noexcept { return StringView(buffer_, byte_length_); }

    /**
     * @brief 文字列を追加
     */
    bool append(StringView view) noexcept {
        if (byte_length_ + view.byte_length() > Capacity) {
            return false;
        }

        for (uint32_t i = 0; i < view.byte_length(); ++i) {
            buffer_[byte_length_++] = view[i];
        }

        buffer_[byte_length_] = '\0';

        return true;
    }

    /**
     * @brief C文字列を追加
     */
    bool append(const char* str) noexcept {
        if (str == nullptr) {
            return false;
        }
        return append(StringView::from_c_string(str));
    }

    /**
     * @brief 1文字追加
     */
    bool append(char c) noexcept {
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
    void clear() noexcept {
        byte_length_ = 0;
        buffer_[0] = '\0';
    }

    bool operator==(StringView other) const noexcept { return String<FixedString<Capacity>>::equals(other); }

    bool operator!=(StringView other) const noexcept { return !String<FixedString<Capacity>>::equals(other); }

    StringView get_char(uint32_t char_index) const noexcept {
        uint32_t byte_pos = this->get_char_position(char_index);

        if (byte_pos >= byte_length_) {
            return StringView {};
        }

        uint8_t char_len = utf8::get_char_byte_length(static_cast<uint8_t>(buffer_[byte_pos]));
        return {buffer_ + byte_pos, char_len};
    }

    /**
     * @brief イテレータ（開始）
     */
    const char* begin() const noexcept { return buffer_; }

    /**
     * @brief イテレータ（終了）
     */
    const char* end() const noexcept { return buffer_ + byte_length_; }

    /**
     * @brief spanとして取得（書き込み可能）
     */
    span<char> as_span() noexcept { return span<char>(buffer_, byte_length_); }

    /**
     * @brief spanとして取得（読み取り専用）
     */
    span<const char> as_span() const noexcept { return span<const char>(buffer_, byte_length_); }

    /**
     * @brief spanから構築
     */
    void from_span(span<const char> s) noexcept {
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

} // namespace omusubi
