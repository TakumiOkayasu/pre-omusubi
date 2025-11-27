#pragma once

#include <omusubi/core/log_level.h>
#include <omusubi/core/string_view.h>
#include <omusubi/interface/log_output.h>

namespace omusubi {

/**
 * @brief シンプルなLogger実装
 *
 * ヒープアロケーションなしで動作する軽量ロガー。
 * LogOutputインターフェースを通じて出力先を抽象化。
 *
 * @note スレッドセーフではありません（組み込みシステム前提）
 */
class Logger {
private:
    LogOutput* output_;
    LogLevel min_level_;

public:
    /**
     * @brief コンストラクタ
     * @param output ログ出力先（nullptrの場合は出力なし）
     * @param min_level 最小ログレベル（これ未満は出力されない）
     */
    constexpr Logger(LogOutput* output, LogLevel min_level = LogLevel::INFO) noexcept : output_(output), min_level_(min_level) {}

    /**
     * @brief デフォルトコンストラクタ（シングルトン用）
     *
     * 出力先なし、最小レベルINFOで初期化。
     * 後からset_output()で出力先を設定可能。
     */
    constexpr Logger() noexcept : output_(nullptr), min_level_(LogLevel::INFO) {}

    /**
     * @brief 出力先を設定
     * @param output ログ出力先（nullptrで出力無効化）
     */
    void set_output(LogOutput* output) noexcept { output_ = output; }

    /**
     * @brief 現在の出力先を取得
     * @return 出力先（未設定の場合nullptr）
     */
    [[nodiscard]] LogOutput* get_output() const noexcept { return output_; }

    /**
     * @brief テンプレートベースのログ出力
     *
     * コンパイル時にログレベルが決定されるため、
     * リリースビルドではDEBUGログが完全に削除される。
     *
     * @tparam Level ログレベル
     * @param message ログメッセージ
     *
     * @par 使用例
     * @code
     * logger.log<LogLevel::INFO>("message"sv);
     * logger.log<LogLevel::ERROR>("error"sv);
     * @endcode
     */
    template <LogLevel Level>
    void log(std::string_view message) const {
#ifdef NDEBUG
        constexpr bool is_debug_build = false;
#else
        constexpr bool is_debug_build = true;
#endif

        if constexpr (Level == LogLevel::DEBUG && !is_debug_build) {
            // リリースビルドではDEBUGログは完全に削除される
            (void)message;
        } else {
            if (Level >= min_level_ && output_ != nullptr) {
                output_->write(Level, message);
            }
        }
    }

    /**
     * @brief 最小ログレベルを設定
     * @param level 新しい最小ログレベル
     */
    void set_min_level(LogLevel level) noexcept { min_level_ = level; }

    /**
     * @brief 現在の最小ログレベルを取得
     * @return 最小ログレベル
     */
    [[nodiscard]] constexpr LogLevel get_min_level() const noexcept { return min_level_; }

    /**
     * @brief 出力をフラッシュ
     */
    void flush() const {
        if (output_ != nullptr) {
            output_->flush();
        }
    }
};

/**
 * @brief ログレベルを文字列に変換
 * @param level ログレベル
 * @return ログレベル文字列
 */
[[nodiscard]] constexpr std::string_view log_level_to_string(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::DEBUG:
            return {"DEBUG", 5};
        case LogLevel::INFO:
            return {"INFO", 4};
        case LogLevel::WARNING:
            return {"WARN", 4};
        case LogLevel::ERROR:
            return {"ERROR", 5};
        case LogLevel::CRITICAL:
            return {"CRIT", 4};
    }
    return {"UNKNOWN", 7};
}

// ========================================
// シングルトンロガー
// ========================================

/**
 * @brief グローバルLoggerインスタンスを取得
 *
 * Meyerのシングルトンパターンを使用。
 * どこからでも同じLoggerインスタンスにアクセス可能。
 *
 * @return Logger& グローバルLoggerへの参照
 *
 * @par 使用例
 * @code
 * // 初期化時（setup()など）
 * static SerialLogOutput log_output(&serial);
 * get_logger().set_output(&log_output);
 * get_logger().set_min_level(LogLevel::DEBUG);
 *
 * // どこからでも使用可能
 * get_logger().log<LogLevel::INFO>("Hello"sv);
 * log<LogLevel::INFO>("Hello"sv);  // グローバル関数も使用可能
 * @endcode
 *
 * @note 出力先を設定する前に呼び出しても安全（何も出力されない）
 */
inline Logger& get_logger() {
    static Logger instance;
    return instance;
}

/**
 * @brief グローバルロガーへのテンプレートログ出力
 *
 * シングルトンロガーを使用してどこからでもログ出力可能。
 * テンプレート引数でログレベルを指定するため、
 * リリースビルドではDEBUGログが完全に削除される。
 *
 * @tparam Level ログレベル
 * @param message ログメッセージ
 *
 * @par 使用例
 * @code
 * log<LogLevel::DEBUG>("debug message"sv);
 * log<LogLevel::INFO>("info message"sv);
 * log<LogLevel::WARNING>("warning"sv);
 * log<LogLevel::ERROR>("error"sv);
 * log<LogLevel::CRITICAL>("critical"sv);
 * @endcode
 */
template <LogLevel Level>
void log(std::string_view message) {
    get_logger().log<Level>(message);
}

/**
 * @brief グローバルロガーをフラッシュ
 */
inline void log_flush() {
    get_logger().flush();
}

} // namespace omusubi
