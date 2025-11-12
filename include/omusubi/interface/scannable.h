#pragma once

#include <cstdint>
#include "omusubi/core/fixed_string.hpp"

namespace omusubi {

/**
 * @brief スキャン機能インターフェース
 *
 * Bluetooth、WiFi、BLEなどの周辺デバイス・ネットワークをスキャンする機能を提供します。
 *
 * 使用例:
 * @code
 * Scannable* device = ctx.get_scannable()->get_wifi_context();
 *
 * // スキャンを実行して見つかったデバイス数を取得
 * uint8_t count = device->scan();
 * @endcode
 *
 * @note このインターフェースを実装するクラス: BluetoothContext, WiFiContext, BLEContext
 */
class Scannable {
public:
    Scannable() = default;
    virtual ~Scannable() = default;
    Scannable(const Scannable&) = delete;
    Scannable& operator=(const Scannable&) = delete;
    Scannable(Scannable&&) = delete;
    Scannable& operator=(Scannable&&) = delete;

    /** @brief スキャンを実行 @return 見つかったデバイス数 */
    virtual uint8_t scan() = 0;
};

}  // namespace omusubi