#pragma once

#include "../core/logger.hpp"
#include "../device/serial_context.h"

namespace omusubi {

/**
 * @brief Serial出力を使用したログ出力実装
 *
 * SerialContextを通じてログをシリアル出力します。
 * フォーマット: [LEVEL] message
 */
class SerialLogOutput : public LogOutput {
private:
    SerialContext* serial_;

public:
    /**
     * @brief コンストラクタ
     * @param serial シリアルコンテキスト（nullptrの場合は出力なし）
     */
    explicit SerialLogOutput(SerialContext* serial) noexcept : serial_(serial) {}

    /**
     * @brief ログメッセージを出力
     * @param level ログレベル
     * @param message ログメッセージ
     */
    void write(LogLevel level, StringView message) override {
        if (serial_ == nullptr) {
            return;
        }

        // フォーマット: [LEVEL] message
        write_string("[");
        write_string(log_level_to_string(level));
        write_string("] ");
        write_string(message);
        write_string("\r\n");
    }

private:
    /**
     * @brief StringViewをシリアルに出力
     * @param str 出力する文字列
     */
    void write_string(StringView str) { serial_->write_text(span<const char>(str.data(), str.byte_length())); }

    /**
     * @brief 出力をフラッシュ
     */
    void flush() override {
        // Serial出力は通常バッファリングされないため、何もしない
    }
};

} // namespace omusubi
