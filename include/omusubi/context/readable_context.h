#pragma once

namespace omusubi {

// 前方宣言
class SerialContext;
class BluetoothContext;

/**
 * @brief 読み取り可能なデバイスのコンテキスト
 *
 * シリアル、Bluetoothなど、データ読み取り機能を持つデバイスへのアクセスを提供します。
 *
 * 使用例:
 * @code
 * ReadableContext* readable = ctx.get_readable_context();
 *
 * // シリアルポート0を取得
 * SerialContext* serial = readable->get_serial0_context();
 * FixedBuffer<256> data = serial->read();
 * @endcode
 */
class ReadableContext {
public:
    ReadableContext() = default;
    virtual ~ReadableContext() = default;
    ReadableContext(const ReadableContext&) = delete;
    ReadableContext& operator=(const ReadableContext&) = delete;
    ReadableContext(ReadableContext&&) = delete;
    ReadableContext& operator=(ReadableContext&&) = delete;

    /** @brief シリアルポート0を取得 */
    virtual SerialContext* get_serial0_context() = 0;

    /** @brief シリアルポート1を取得 */
    virtual SerialContext* get_serial1_context() = 0;

    /** @brief シリアルポート2を取得 */
    virtual SerialContext* get_serial2_context() = 0;

    /** @brief Bluetoothを取得 */
    virtual BluetoothContext* get_bluetooth_context() = 0;

    // ========================================
    // テンプレートベースアクセス（C++14対応）
    // ========================================

    /**
     * @brief シリアルポートをテンプレートパラメータで取得
     * @tparam Port ポート番号（0, 1, 2）
     * @return SerialContext* ポートのコンテキスト
     */
    template<uint8_t Port>
    SerialContext* get_serial_context() {
        static_assert(Port <= 2, "Serial port must be 0, 1, or 2");
        return (Port == 0) ? get_serial0_context() :
               (Port == 1) ? get_serial1_context() :
                             get_serial2_context();
    }
};

}  // namespace omusubi
