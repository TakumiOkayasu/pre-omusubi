#pragma once

#include <cstdint>

namespace omusubi {

// 前方宣言
class ConnectableContext;
class ReadableContext;
class WritableContext;
class ScannableContext;
class SensorContext;
class InputContext;
class OutputContext;
class SystemInfoContext;
class PowerContext;

/**
 * @brief システムコンテキスト
 *
 * デバイス全体のシステム制御とカテゴリ別コンテキストへのアクセスを提供します。
 * すべてのハードウェアアクセスはこのクラスを経由します。
 *
 * 使用例:
 * @code
 * SystemContext& ctx = get_system_context();
 *
 * // システム初期化
 * ctx.begin();
 *
 * // デバイスアクセス
 * SerialContext* serial = ctx.get_connectable_context()->get_serial0_context();
 * serial->connect();
 * serial->write("Hello"_sv);
 *
 * // システム更新
 * ctx.update();
 * @endcode
 */
class SystemContext {
public:
    virtual ~SystemContext() = default;

    SystemContext(const SystemContext&) = delete;
    SystemContext& operator=(const SystemContext&) = delete;
    SystemContext(SystemContext&&) = delete;
    SystemContext& operator=(SystemContext&&) = delete;

    // ========================================
    // システム制御
    // ========================================

    /** @brief システムを初期化 */
    virtual void begin() = 0;

    /** @brief システム状態を更新 */
    virtual void update() = 0;

    /** @brief 待機（ミリ秒） */
    virtual void delay(uint32_t ms) = 0;

    /** @brief システムをリセット */
    virtual void reset() = 0;

    // ========================================
    // カテゴリ別コンテキストアクセス
    // ========================================

    /** @brief 接続可能なデバイスのコンテキスト */
    virtual ConnectableContext* get_connectable_context() = 0;

    /** @brief 読み取り可能なデバイスのコンテキスト */
    virtual ReadableContext* get_readable_context() = 0;

    /** @brief 書き込み可能なデバイスのコンテキスト */
    virtual WritableContext* get_writable_context() = 0;

    /** @brief スキャン可能なデバイスのコンテキスト */
    virtual ScannableContext* get_scannable_context() = 0;

    /** @brief センサーデバイスのコンテキスト */
    virtual SensorContext* get_sensor_context() = 0;

    /** @brief 入力デバイスのコンテキスト */
    virtual InputContext* get_input_context() = 0;

    /** @brief 出力デバイスのコンテキスト */
    virtual OutputContext* get_output_context() = 0;

    /** @brief システム情報コンテキスト */
    virtual SystemInfoContext* get_system_info_context() = 0;

    /** @brief 電源管理コンテキスト */
    virtual PowerContext* get_power_context() = 0;

protected:
    SystemContext() = default;
};

/**
 * @brief グローバルSystemContextを取得
 */
SystemContext& get_system_context();

}  // namespace omusubi