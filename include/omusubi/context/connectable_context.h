#pragma once

namespace omusubi {

// 前方宣言
class SerialContext;
class BluetoothContext;
class WiFiContext;
class BLEContext;

/**
 * @brief 接続可能なデバイスのコンテキスト
 *
 * シリアル、Bluetooth、WiFi、BLEなど、接続機能を持つデバイスへのアクセスを提供します。
 *
 * 使用例:
 * @code
 * ConnectableContext* connectable = ctx.get_connectable_context();
 *
 * // シリアルポート0を取得
 * SerialContext* serial = connectable->get_serial0_context();
 * serial->connect();
 *
 * // Bluetoothを取得
 * BluetoothContext* bt = connectable->get_bluetooth_context();
 * bt->connect();
 * @endcode
 */
class ConnectableContext {
public:
    ConnectableContext() = default;
    virtual ~ConnectableContext() = default;
    ConnectableContext(const ConnectableContext&) = delete;
    ConnectableContext& operator=(const ConnectableContext&) = delete;
    ConnectableContext(ConnectableContext&&) = delete;
    ConnectableContext& operator=(ConnectableContext&&) = delete;

    /** @brief シリアルポート0を取得 */
    virtual SerialContext* get_serial0_context() = 0;

    /** @brief シリアルポート1を取得 */
    virtual SerialContext* get_serial1_context() = 0;

    /** @brief シリアルポート2を取得 */
    virtual SerialContext* get_serial2_context() = 0;

    /** @brief Bluetoothを取得 */
    virtual BluetoothContext* get_bluetooth_context() = 0;

    /** @brief WiFiを取得 */
    virtual WiFiContext* get_wifi_context() = 0;

    /** @brief BLEを取得 */
    virtual BLEContext* get_ble_context() = 0;

    // ========================================
    // テンプレートベースアクセス（C++14対応）
    // ========================================

    /**
     * @brief シリアルポートをテンプレートパラメータで取得
     * @tparam Port ポート番号（0, 1, 2）
     * @return SerialContext* ポートのコンテキスト
     *
     * 使用例:
     * @code
     * // コンパイル時にポート番号を指定
     * auto* serial = connectable->get_serial_context<0>();
     *
     * // ループでの使用
     * for (uint8_t i = 0; i < 3; ++i) {
     *     // ※ループではランタイム値なので使用不可
     * }
     * @endcode
     */
    template<uint8_t Port>
    SerialContext* get_serial_context() {
        static_assert(Port <= 2, "Serial port must be 0, 1, or 2");
        // C++14ではif constexprが使えないため、三項演算子で実装
        return (Port == 0) ? get_serial0_context() :
               (Port == 1) ? get_serial1_context() :
                             get_serial2_context();
    }
};

}  // namespace omusubi
