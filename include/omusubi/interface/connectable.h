#pragma once

namespace omusubi {

/**
 * @brief 接続管理インターフェース
 *
 * シリアル、Bluetooth、WiFiなどのデバイスの接続を管理します。
 *
 * 使用例:
 * @code
 * Connectable* device = ctx.get_connectable()->get_bluetooth_context();
 *
 * // 接続
 * if (device->connect()) {
 *     // 接続成功
 * }
 * @endcode
 *
 * @note このインターフェースを実装するクラス: SerialContext, BluetoothContext, WiFiContext, BLEContext
 */
class Connectable {
public:
    Connectable() = default;
    virtual ~Connectable() = default;
    Connectable(const Connectable&) = delete;
    Connectable& operator=(const Connectable&) = delete;
    Connectable(Connectable&&) = delete;
    Connectable& operator=(Connectable&&) = delete;

    /** @brief デバイスに接続 @return 接続成功ならtrue */
    virtual bool connect() = 0;
};

}  // namespace omusubi