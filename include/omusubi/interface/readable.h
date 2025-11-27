#pragma once

#include <omusubi/core/span.hpp>

namespace omusubi {

/**
 * @brief バイト列読み取りインターフェース
 */
class ByteReadable {
public:
    ByteReadable() = default;
    virtual ~ByteReadable() = default;
    ByteReadable(const ByteReadable&) = delete;
    ByteReadable& operator=(const ByteReadable&) = delete;
    ByteReadable(ByteReadable&&) = delete;
    ByteReadable& operator=(ByteReadable&&) = delete;

    /** @brief バイト列を読み取る */
    [[nodiscard]] virtual size_t read(span<uint8_t> buffer) = 0;

    /** @brief 利用可能なバイト数を取得 */
    [[nodiscard]] virtual size_t available() const = 0;
};

/**
 * @brief テキスト読み取りインターフェース
 */
class TextReadable {
public:
    TextReadable() = default;
    virtual ~TextReadable() = default;
    TextReadable(const TextReadable&) = delete;
    TextReadable& operator=(const TextReadable&) = delete;
    TextReadable(TextReadable&&) = delete;
    TextReadable& operator=(TextReadable&&) = delete;

    /** @brief 行単位でデータを読み取る */
    virtual size_t read_line(span<char> buffer) = 0;
};

} // namespace omusubi
