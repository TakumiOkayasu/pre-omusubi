#pragma once

#include "../core/log_level.h"
#include "../core/string_view.h"

namespace omusubi {

/**
 * @brief ログ出力インターフェース
 *
 * ログメッセージの出力先を抽象化します。
 * 実装例: Serial出力, ファイル出力, リングバッファ
 */
class LogOutput {
public:
    /**
     * @brief ログメッセージを出力
     * @param level ログレベル
     * @param message ログメッセージ
     */
    virtual void write(LogLevel level, StringView message) = 0;

    /**
     * @brief 出力をフラッシュ（オプション）
     *
     * バッファリングされた出力を強制的に書き出します。
     * デフォルト実装は何もしません。
     */
    virtual void flush() {}

    virtual ~LogOutput() = default;
};

} // namespace omusubi
