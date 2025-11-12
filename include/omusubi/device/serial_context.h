#pragma once

#include "omusubi/interface/readable.h"
#include "omusubi/interface/writable.h"
#include "omusubi/interface/connectable.h"

namespace omusubi {

/**
 * @brief シリアル通信デバイス
 *
 * UART/USBシリアル通信を扱います。
 * Readable、Writable、Connectableインターフェースを実装しています。
 *
 * 使用例:
 * @code
 * SerialContext* serial = ctx.get_connectable_context()->get_serial0_context();
 *
 * // 接続
 * serial->connect();
 *
 * // データ送信
 * serial->write("Hello, Serial!"_sv);
 *
 * // データ受信
 * FixedBuffer<256> data = serial->read();
 * @endcode
 */
class SerialContext
    : public Readable
    , public Writable
    , public Connectable {
public:
    SerialContext() = default;
    ~SerialContext() override = default;
    SerialContext(const SerialContext&) = delete;
    SerialContext& operator=(const SerialContext&) = delete;
    SerialContext(SerialContext&&) = delete;
    SerialContext& operator=(SerialContext&&) = delete;
};

}  // namespace omusubi