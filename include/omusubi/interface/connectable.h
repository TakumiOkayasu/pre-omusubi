#pragma once

namespace omusubi {

/**
 * @brief 接続管理インターフェース
 *
 * デバイスへの接続・切断を管理する抽象インターフェース。
 *
 * @note 接続先の設定について
 *
 * connect() は引数を取りません。接続先（SSIDやBluetoothアドレスなど）は
 * 具体的な実装クラスで別途設定する必要があります。
 *
 * これは以下の理由によります：
 * - 接続先のパラメータはデバイスごとに異なる（WiFi: SSID/パスワード、Bluetooth: アドレス）
 * - 汎用インターフェースでは特定のパラメータ型を強制できない
 *
 * @par 実装例
 * @code
 * class WiFiContext : public Connectable {
 * public:
 *     // 接続先を設定するメソッドを別途提供
 *     void set_credentials(std::string_view ssid, std::string_view password);
 *
 *     // connect() は設定済みの接続先に接続
 *     bool connect() override;
 * };
 * @endcode
 */
class Connectable {
public:
    Connectable() = default;
    virtual ~Connectable() = default;
    Connectable(const Connectable&) = delete;
    Connectable& operator=(const Connectable&) = delete;
    Connectable(Connectable&&) = delete;
    Connectable& operator=(Connectable&&) = delete;

    /**
     * @brief デバイスに接続
     * @return 接続成功時 true、失敗時 false
     * @note 接続先は事前に設定しておく必要がある
     */
    [[nodiscard]] virtual bool connect() = 0;

    /**
     * @brief デバイスから切断
     * @return 切断成功時 true、失敗時 false
     */
    [[nodiscard]] virtual bool disconnect() = 0;

    /**
     * @brief 接続状態を確認
     * @return 接続中の場合 true
     */
    [[nodiscard]] virtual bool is_connected() const = 0;
};

} // namespace omusubi
