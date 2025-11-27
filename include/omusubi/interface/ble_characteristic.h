#pragma once

#include <omusubi/interface/readable.h>
#include <omusubi/interface/writable.h>

namespace omusubi {

/**
 * @brief BLE Characteristic インターフェース
 *
 * BLEデバイスの特性（Characteristic）を表す抽象クラス。
 * データの読み書きが可能なBLE特性を実装する際に継承する。
 */
class BLECharacteristic : public ByteReadable, public ByteWritable {
public:
    BLECharacteristic() = default;
    ~BLECharacteristic() override = default;
    BLECharacteristic(const BLECharacteristic&) = delete;
    BLECharacteristic& operator=(const BLECharacteristic&) = delete;
    BLECharacteristic(BLECharacteristic&&) = delete;
    BLECharacteristic& operator=(BLECharacteristic&&) = delete;

    // ========================================
    // ByteReadable インターフェース
    // ========================================

    /** @brief 特性からバイト列を読み取る */
    [[nodiscard]] size_t read(span<uint8_t> buffer) override = 0;

    /** @brief 読み取り可能なバイト数を取得 */
    [[nodiscard]] size_t available() const override = 0;

    // ========================================
    // ByteWritable インターフェース
    // ========================================

    /** @brief 特性にバイト列を書き込む */
    [[nodiscard]] size_t write(span<const uint8_t> data) override = 0;
};

} // namespace omusubi
