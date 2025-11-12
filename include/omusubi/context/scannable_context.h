#pragma once

namespace omusubi {

// 前方宣言
class BluetoothContext;
class WiFiContext;
class BLEContext;

/**
 * @brief スキャン可能なデバイスのコンテキスト
 *
 * Bluetooth、WiFi、BLEなど、周辺デバイス・ネットワークをスキャンする機能を持つデバイスへのアクセスを提供します。
 *
 * 使用例:
 * @code
 * ScannableContext* scannable = ctx.get_scannable();
 *
 * // WiFiを取得
 * WiFiContext* wifi = scannable->get_wifi_context();
 * uint8_t count = wifi->scan();
 * @endcode
 */
class ScannableContext {
public:
    ScannableContext() = default;
    virtual ~ScannableContext() = default;
    ScannableContext(const ScannableContext&) = delete;
    ScannableContext& operator=(const ScannableContext&) = delete;
    ScannableContext(ScannableContext&&) = delete;
    ScannableContext& operator=(ScannableContext&&) = delete;

    /** @brief Bluetoothを取得 */
    virtual BluetoothContext* get_bluetooth_context() = 0;

    /** @brief WiFiを取得 */
    virtual WiFiContext* get_wifi_context() = 0;

    /** @brief BLEを取得 */
    virtual BLEContext* get_ble_context() = 0;
};

}  // namespace omusubi
