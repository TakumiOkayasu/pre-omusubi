#pragma once

#include <omusubi/device/serial_context.h>

#include <omusubi/core/format.hpp>
#include <omusubi/core/logger.hpp>

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
    void write(LogLevel level, std::string_view message) override {
        if (serial_ == nullptr) {
            return;
        }

        // format()を使用してログメッセージを整形
        auto formatted = format("[{}] {}\r\n", log_level_to_string(level), message);
        serial_->write_text(span<const char>(formatted.data(), formatted.byte_length()));
    }

    /**
     * @brief 出力をフラッシュ
     */
    void flush() override {
        // Serial出力は通常バッファリングされないため、何もしない
    }
};

} // namespace omusubi
