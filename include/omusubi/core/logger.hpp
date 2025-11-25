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
     * @brief ログを出力
     * @param level ログレベル
     * @param message ログメッセージ
     */
    void log(LogLevel level, std::string_view message) const {
        if (level >= min_level_ && output_ != nullptr) {
            output_->write(level, message);
        }
    }

    /**
     * @brief DEBUGレベルでログ出力
     * @param message ログメッセージ
     */
    void debug(std::string_view message) const { log(LogLevel::DEBUG, message); }

    /**
     * @brief INFOレベルでログ出力
     * @param message ログメッセージ
     */
    void info(std::string_view message) const { log(LogLevel::INFO, message); }

    /**
     * @brief WARNINGレベルでログ出力
     * @param message ログメッセージ
     */
    void warning(std::string_view message) const { log(LogLevel::WARNING, message); }

    /**
     * @brief ERRORレベルでログ出力
     * @param message ログメッセージ
     */
    void error(std::string_view message) const { log(LogLevel::ERROR, message); }

    /**
     * @brief CRITICALレベルでログ出力
     * @param message ログメッセージ
     */
    void critical(std::string_view message) const { log(LogLevel::CRITICAL, message); }

    /**
     * @brief 最小ログレベルを設定
     * @param level 新しい最小ログレベル
     */
    void set_min_level(LogLevel level) noexcept { min_level_ = level; }

    /**
     * @brief 現在の最小ログレベルを取得
     * @return 最小ログレベル
     */
    constexpr LogLevel get_min_level() const noexcept { return min_level_; }

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
constexpr std::string_view log_level_to_string(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::DEBUG:
            return std::string_view("DEBUG", 5);
        case LogLevel::INFO:
            return std::string_view("INFO", 4);
        case LogLevel::WARNING:
            return std::string_view("WARN", 4);
        case LogLevel::ERROR:
            return std::string_view("ERROR", 5);
        case LogLevel::CRITICAL:
            return std::string_view("CRIT", 4);
    }
    return std::string_view("UNKNOWN", 7);
}

/**
 * @brief テンプレートベースのログ出力ヘルパー（C++17 if constexpr版）
 *
 * 使用例:
 *   log_at<LogLevel::DEBUG>(logger, "Debug message"_sv);
 *   log_at<LogLevel::ERROR>(logger, "Error occurred"_sv);
 *
 * リリースビルド（NDEBUG定義時）では、DEBUGレベルのログは
 * コンパイラの最適化により完全に削除されます。
 */
template <LogLevel Level>
void log_at(const Logger& logger, std::string_view message) {
#ifdef NDEBUG
    constexpr bool is_debug_build = false;
#else
    constexpr bool is_debug_build = true;
#endif

    // if constexprによりコンパイル時に分岐が決定される
    if constexpr (Level == LogLevel::DEBUG && !is_debug_build) {
        // リリースビルドではDEBUGログは完全に削除される
        (void)logger;
        (void)message;
    } else {
        logger.log(Level, message);
    }
}

} // namespace omusubi
