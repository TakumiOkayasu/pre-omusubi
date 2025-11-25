#pragma once

#include <omusubi/core/span.hpp>

namespace omusubi {

/**
 * @brief 固定長バイトバッファ
 *
 * 動的メモリ確保なしのバイトバッファ。
 */
template <uint32_t Capacity>
class FixedBuffer {
public:
    /**
     * @brief デフォルトコンストラクタ
     */
    FixedBuffer() noexcept : length_(0) {}

    /**
     * @brief 容量を取得
     */
    [[nodiscard]] constexpr uint32_t capacity() const noexcept { return Capacity; }

    /**
     * @brief サイズを取得
     */
    [[nodiscard]] uint32_t size() const noexcept { return length_; }

    /**
     * @brief 空か判定
     */
    [[nodiscard]] bool is_empty() const noexcept { return length_ == 0; }

    /**
     * @brief データへのポインタを取得
     */
    [[nodiscard]] const uint8_t* data() const noexcept { return buffer_; }

    /**
     * @brief 1バイト追加
     */
    bool append(uint8_t byte) noexcept {
        if (length_ >= Capacity) {
            return false;
        }

        buffer_[length_++] = byte;
        return true;
    }

    /**
     * @brief クリア
     */
    void clear() noexcept { length_ = 0; }

    /**
     * @brief バイトアクセス
     */
    uint8_t operator[](uint32_t index) const noexcept { return (index < length_) ? buffer_[index] : 0; }

    /**
     * @brief イテレータ（開始）
     */
    [[nodiscard]] const uint8_t* begin() const noexcept { return buffer_; }

    /**
     * @brief イテレータ（終了）
     */
    [[nodiscard]] const uint8_t* end() const noexcept { return buffer_ + length_; }

    /**
     * @brief spanとして取得（書き込み可能）
     */
    [[nodiscard]] span<uint8_t> as_span() noexcept { return span<uint8_t>(buffer_, length_); }

    /**
     * @brief spanとして取得（読み取り専用）
     */
    [[nodiscard]] span<const uint8_t> as_span() const noexcept { return span<const uint8_t>(buffer_, length_); }

    /**
     * @brief spanから構築
     */
    void from_span(span<const uint8_t> s) noexcept {
        length_ = (s.size() < Capacity) ? static_cast<uint32_t>(s.size()) : Capacity;
        for (uint32_t i = 0; i < length_; ++i) {
            buffer_[i] = s[i];
        }
    }

private:
    uint8_t buffer_[Capacity];
    uint32_t length_;
};

} // namespace omusubi
