#pragma once

#include "omusubi/core/fixed_buffer.hpp"

namespace omusubi {

/**
 * @brief データ読み取りインターフェース
 *
 * シリアル通信、Bluetooth、WiFiなどのデバイスからデータを読み取る機能を提供します。
 *
 * 使用例:
 * @code
 * Readable* device = ctx.get_connectable_context()->get_serial0_context();
 *
 * // データを読み取る
 * FixedBuffer<256> data = device->read();
 * @endcode
 *
 * @note このインターフェースを実装するクラス: SerialContext, BluetoothContext
 */
class Readable {
public:
    Readable() = default;
    virtual ~Readable() = default;
    Readable(const Readable&) = delete;
    Readable& operator=(const Readable&) = delete;
    Readable(Readable&&) = delete;
    Readable& operator=(Readable&&) = delete;

    /** @brief データを読み取る @return 読み取ったデータ */
    virtual FixedBuffer<256> read() = 0;
};

}  // namespace omusubi