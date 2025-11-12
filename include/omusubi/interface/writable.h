#pragma once

#include "omusubi/core/string_view.h"

namespace omusubi {

/**
 * @brief データ書き込みインターフェース
 *
 * シリアル通信、Bluetooth、ディスプレイなどにデータを出力する機能を提供します。
 *
 * 使用例:
 * @code
 * Writable* device = ctx.get_connectable()->get_serial0_context();
 *
 * // 文字列を出力
 * device->write("Hello"_sv);
 * @endcode
 *
 * @note このインターフェースを実装するクラス: SerialContext, BluetoothContext
 */
class Writable {
public:
    Writable() = default;
    virtual ~Writable() = default;
    Writable(const Writable&) = delete;
    Writable& operator=(const Writable&) = delete;
    Writable(Writable&&) = delete;
    Writable& operator=(Writable&&) = delete;

    /** @brief 文字列を出力 */
    virtual void write(StringView text) = 0;
};

}  // namespace omusubi