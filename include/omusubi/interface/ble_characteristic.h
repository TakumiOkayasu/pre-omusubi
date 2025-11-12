#pragma once

#include "readable.h"
#include "writable.h"

namespace omusubi {

/**
 * @brief BLE Characteristic インターフェース
 *
 * BLE Characteristicの読み書き機能を提供します。
 * ReadableとWritableを継承しています。
 *
 * 使用例:
 * @code
 * BLECharacteristic* characteristic = ble->get_characteristic(...);
 *
 * // 読み取り
 * FixedBuffer<256> data = characteristic->read();
 *
 * // 書き込み
 * characteristic->write("Hello"_sv);
 * @endcode
 */
class BLECharacteristic : public Readable, public Writable {
public:
    BLECharacteristic() = default;
    ~BLECharacteristic() override = default;
    BLECharacteristic(const BLECharacteristic&) = delete;
    BLECharacteristic& operator=(const BLECharacteristic&) = delete;
    BLECharacteristic(BLECharacteristic&&) = delete;
    BLECharacteristic& operator=(BLECharacteristic&&) = delete;
};

}  // namespace omusubi