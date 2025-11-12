#pragma once

#include "omusubi/interface/connectable.h"
#include "omusubi/interface/scannable.h"

namespace omusubi {

/**
 * @brief BLE (Bluetooth Low Energy) 通信デバイス
 *
 * BLE 4.0以降の低電力Bluetooth通信を扱います。
 * Connectable、Scannableインターフェースを実装しています。
 *
 * 使用例:
 * @code
 * BLEContext* ble = ctx.get_connectable()->get_ble_context();
 *
 * // 接続
 * ble->connect();
 *
 * // スキャン
 * uint8_t count = ble->scan();
 * @endcode
 */
class BLEContext
    : public Connectable
    , public Scannable {
public:
    BLEContext() = default;
    ~BLEContext() override = default;
    BLEContext(const BLEContext&) = delete;
    BLEContext& operator=(const BLEContext&) = delete;
    BLEContext(BLEContext&&) = delete;
    BLEContext& operator=(BLEContext&&) = delete;
};

}  // namespace omusubi
