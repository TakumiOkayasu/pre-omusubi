#pragma once

#include "omusubi/interface/readable.h"
#include "omusubi/interface/writable.h"
#include "omusubi/interface/connectable.h"
#include "omusubi/interface/scannable.h"

namespace omusubi {

/**
 * @brief Bluetooth Classic通信デバイス
 *
 * Bluetooth 2.x/3.x (SPP: Serial Port Profile) による通信を扱います。
 * Readable、Writable、Connectable、Scannableインターフェースを実装しています。
 *
 * 使用例:
 * @code
 * BluetoothContext* bt = ctx.get_connectable()->get_bluetooth_context();
 *
 * // 接続
 * bt->connect();
 *
 * // スキャン
 * uint8_t count = bt->scan();
 *
 * // データ送信
 * bt->write("Hello Bluetooth!"_sv);
 *
 * // データ受信
 * FixedBuffer<256> data = bt->read();
 * @endcode
 */
class BluetoothContext
    : public Readable
    , public Writable
    , public Connectable
    , public Scannable {
public:
    BluetoothContext() = default;
    ~BluetoothContext() override = default;
    BluetoothContext(const BluetoothContext&) = delete;
    BluetoothContext& operator=(const BluetoothContext&) = delete;
    BluetoothContext(BluetoothContext&&) = delete;
    BluetoothContext& operator=(BluetoothContext&&) = delete;
};

}  // namespace omusubi