#pragma once

#include <cstdint>

namespace omusubi {

/**
 * @brief ログレベル
 *
 * ログ出力の重要度を表します。
 */
enum class LogLevel : uint8_t {
    DEBUG = 0, ///< デバッグ情報（開発時のみ）
    INFO,      ///< 一般情報
    WARNING,   ///< 警告（エラーではないが注意が必要）
    ERROR,     ///< エラー（機能の一部が失敗）
    CRITICAL   ///< 致命的エラー（システムが継続不能）
};

} // namespace omusubi
