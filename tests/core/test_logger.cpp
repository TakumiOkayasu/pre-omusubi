// Logger のユニットテスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/fixed_string.hpp>
#include <omusubi/core/logger.hpp>

#include "../doctest.h"

using namespace omusubi;

// ========================================
// モックLogOutput実装
// ========================================

class MockLogOutput : public LogOutput {
private:
    FixedString<256> last_message_;
    LogLevel last_level_;
    uint32_t write_count_;
    uint32_t flush_count_;

public:
    MockLogOutput() : last_message_(), last_level_(LogLevel::DEBUG), write_count_(0), flush_count_(0) {}

    void write(LogLevel level, std::string_view message) override {
        last_level_ = level;
        last_message_.clear();
        last_message_.append(message);
        write_count_++;
    }

    void flush() override { flush_count_++; }

    // テスト用アクセッサ
    std::string_view get_last_message() const { return last_message_.view(); }

    LogLevel get_last_level() const { return last_level_; }

    uint32_t get_write_count() const { return write_count_; }

    uint32_t get_flush_count() const { return flush_count_; }

    void reset() {
        last_message_.clear();
        last_level_ = LogLevel::DEBUG;
        write_count_ = 0;
        flush_count_ = 0;
    }
};

// ========================================
// 基本的なログ出力
// ========================================

TEST_CASE("Logger - 基本的なログ出力") {
    MockLogOutput output;
    Logger logger(&output, LogLevel::DEBUG);

    logger.log<LogLevel::INFO>(std::string_view("Hello", 5));

    CHECK_EQ(output.get_write_count(), 1U);
    CHECK_EQ(static_cast<uint8_t>(output.get_last_level()), static_cast<uint8_t>(LogLevel::INFO));

    auto msg = output.get_last_message();
    CHECK_EQ(msg.size(), 5U);
    bool msg_equal = (msg.data()[0] == 'H' && msg.data()[1] == 'e' && msg.data()[2] == 'l' && msg.data()[3] == 'l' && msg.data()[4] == 'o');
    CHECK(msg_equal);
}

// ========================================
// ログレベルフィルタリング
// ========================================

TEST_CASE("Logger - ログレベルフィルタリング") {
    SUBCASE("最小レベル未満はブロックされる") {
        MockLogOutput output;
        Logger logger(&output, LogLevel::WARNING);

        logger.log<LogLevel::DEBUG>(std::string_view("debug", 5));
        logger.log<LogLevel::INFO>(std::string_view("info", 4));

        CHECK_EQ(output.get_write_count(), 0U);
    }

    SUBCASE("最小レベル以上は出力される") {
        MockLogOutput output;
        Logger logger(&output, LogLevel::WARNING);

        logger.log<LogLevel::WARNING>(std::string_view("warn", 4));
        logger.log<LogLevel::ERROR>(std::string_view("error", 5));
        logger.log<LogLevel::CRITICAL>(std::string_view("crit", 4));

        CHECK_EQ(output.get_write_count(), 3U);
    }
}

// ========================================
// 各ログレベル（テンプレート）
// ========================================

TEST_CASE("Logger - 各ログレベル（テンプレート）") {
    SUBCASE("log<DEBUG>") {
        MockLogOutput output;
        Logger logger(&output, LogLevel::DEBUG);

        logger.log<LogLevel::DEBUG>(std::string_view("debug message", 13));

        CHECK_EQ(static_cast<uint8_t>(output.get_last_level()), static_cast<uint8_t>(LogLevel::DEBUG));
#ifdef NDEBUG
        CHECK_EQ(output.get_write_count(), 0U);
#else
        CHECK_EQ(output.get_write_count(), 1U);
#endif
    }

    SUBCASE("log<INFO>") {
        MockLogOutput output;
        Logger logger(&output, LogLevel::DEBUG);

        logger.log<LogLevel::INFO>(std::string_view("info message", 12));

        CHECK_EQ(static_cast<uint8_t>(output.get_last_level()), static_cast<uint8_t>(LogLevel::INFO));
        CHECK_EQ(output.get_write_count(), 1U);
    }

    SUBCASE("log<WARNING>") {
        MockLogOutput output;
        Logger logger(&output, LogLevel::DEBUG);

        logger.log<LogLevel::WARNING>(std::string_view("warning message", 15));

        CHECK_EQ(static_cast<uint8_t>(output.get_last_level()), static_cast<uint8_t>(LogLevel::WARNING));
        CHECK_EQ(output.get_write_count(), 1U);
    }

    SUBCASE("log<ERROR>") {
        MockLogOutput output;
        Logger logger(&output, LogLevel::DEBUG);

        logger.log<LogLevel::ERROR>(std::string_view("error message", 13));

        CHECK_EQ(static_cast<uint8_t>(output.get_last_level()), static_cast<uint8_t>(LogLevel::ERROR));
        CHECK_EQ(output.get_write_count(), 1U);
    }

    SUBCASE("log<CRITICAL>") {
        MockLogOutput output;
        Logger logger(&output, LogLevel::DEBUG);

        logger.log<LogLevel::CRITICAL>(std::string_view("critical message", 16));

        CHECK_EQ(static_cast<uint8_t>(output.get_last_level()), static_cast<uint8_t>(LogLevel::CRITICAL));
        CHECK_EQ(output.get_write_count(), 1U);
    }
}

// ========================================
// 最小ログレベル変更
// ========================================

TEST_CASE("Logger - 最小ログレベル変更") {
    MockLogOutput output;
    Logger logger(&output, LogLevel::DEBUG);

    CHECK_EQ(static_cast<uint8_t>(logger.get_min_level()), static_cast<uint8_t>(LogLevel::DEBUG));

    logger.set_min_level(LogLevel::ERROR);
    CHECK_EQ(static_cast<uint8_t>(logger.get_min_level()), static_cast<uint8_t>(LogLevel::ERROR));

    logger.log<LogLevel::INFO>(std::string_view("info", 4));
    CHECK_EQ(output.get_write_count(), 0U);

    logger.log<LogLevel::ERROR>(std::string_view("error", 5));
    CHECK_EQ(output.get_write_count(), 1U);
}

// ========================================
// nullptr出力先
// ========================================

TEST_CASE("Logger - nullptr出力先") {
    Logger logger(nullptr, LogLevel::DEBUG);

    logger.log<LogLevel::INFO>(std::string_view("test", 4));
    logger.flush();

    CHECK(true);
}

// ========================================
// flush() メソッド
// ========================================

TEST_CASE("Logger - flush() メソッド") {
    MockLogOutput output;
    Logger logger(&output, LogLevel::DEBUG);

    logger.flush();
    CHECK_EQ(output.get_flush_count(), 1U);

    logger.flush();
    logger.flush();
    CHECK_EQ(output.get_flush_count(), 3U);
}

// ========================================
// log_level_to_string() 関数
// ========================================

TEST_CASE("Logger - log_level_to_string()") {
    SUBCASE("DEBUG") {
        auto str = log_level_to_string(LogLevel::DEBUG);
        CHECK_EQ(str.size(), 5U);
        bool equal = (str.data()[0] == 'D' && str.data()[1] == 'E' && str.data()[2] == 'B' && str.data()[3] == 'U' && str.data()[4] == 'G');
        CHECK(equal);
    }

    SUBCASE("INFO") {
        auto str = log_level_to_string(LogLevel::INFO);
        CHECK_EQ(str.size(), 4U);
        bool equal = (str.data()[0] == 'I' && str.data()[1] == 'N' && str.data()[2] == 'F' && str.data()[3] == 'O');
        CHECK(equal);
    }

    SUBCASE("WARNING") {
        auto str = log_level_to_string(LogLevel::WARNING);
        CHECK_EQ(str.size(), 4U);
        bool equal = (str.data()[0] == 'W' && str.data()[1] == 'A' && str.data()[2] == 'R' && str.data()[3] == 'N');
        CHECK(equal);
    }

    SUBCASE("ERROR") {
        auto str = log_level_to_string(LogLevel::ERROR);
        CHECK_EQ(str.size(), 5U);
        bool equal = (str.data()[0] == 'E' && str.data()[1] == 'R' && str.data()[2] == 'R' && str.data()[3] == 'O' && str.data()[4] == 'R');
        CHECK(equal);
    }

    SUBCASE("CRITICAL") {
        auto str = log_level_to_string(LogLevel::CRITICAL);
        CHECK_EQ(str.size(), 4U);
        bool equal = (str.data()[0] == 'C' && str.data()[1] == 'R' && str.data()[2] == 'I' && str.data()[3] == 'T');
        CHECK(equal);
    }
}

// ========================================
// シングルトン get_logger()
// ========================================

TEST_CASE("Logger - シングルトン get_logger()") {
    SUBCASE("同じインスタンスを返す") {
        Logger& logger1 = get_logger();
        Logger& logger2 = get_logger();

        CHECK_EQ(&logger1, &logger2);
    }

    SUBCASE("set_output/get_outputが機能する") {
        MockLogOutput output;

        // 初期状態はnullptr
        get_logger().set_output(nullptr);
        CHECK_EQ(get_logger().get_output(), nullptr);

        // 出力先を設定
        get_logger().set_output(&output);
        CHECK_EQ(get_logger().get_output(), &output);

        // テスト後にクリーンアップ
        get_logger().set_output(nullptr);
    }

    SUBCASE("シングルトン経由でログ出力") {
        MockLogOutput output;
        get_logger().set_output(&output);
        get_logger().set_min_level(LogLevel::DEBUG);

        get_logger().log<LogLevel::INFO>(std::string_view("singleton test", 14));

        CHECK_EQ(output.get_write_count(), 1U);
        CHECK_EQ(static_cast<uint8_t>(output.get_last_level()), static_cast<uint8_t>(LogLevel::INFO));

        // テスト後にクリーンアップ
        get_logger().set_output(nullptr);
    }
}

// ========================================
// グローバル log<Level>() 関数
// ========================================

TEST_CASE("Logger - グローバル log<Level>() 関数") {
    MockLogOutput output;
    get_logger().set_output(&output);
    get_logger().set_min_level(LogLevel::DEBUG);

    SUBCASE("log<DEBUG>()") {
        output.reset();
        log<LogLevel::DEBUG>(std::string_view("debug", 5));
#ifdef NDEBUG
        CHECK_EQ(output.get_write_count(), 0U);
#else
        CHECK_EQ(output.get_write_count(), 1U);
        CHECK_EQ(static_cast<uint8_t>(output.get_last_level()), static_cast<uint8_t>(LogLevel::DEBUG));
#endif
    }

    SUBCASE("log<INFO>()") {
        output.reset();
        log<LogLevel::INFO>(std::string_view("info", 4));
        CHECK_EQ(output.get_write_count(), 1U);
        CHECK_EQ(static_cast<uint8_t>(output.get_last_level()), static_cast<uint8_t>(LogLevel::INFO));
    }

    SUBCASE("log<WARNING>()") {
        output.reset();
        log<LogLevel::WARNING>(std::string_view("warning", 7));
        CHECK_EQ(output.get_write_count(), 1U);
        CHECK_EQ(static_cast<uint8_t>(output.get_last_level()), static_cast<uint8_t>(LogLevel::WARNING));
    }

    SUBCASE("log<ERROR>()") {
        output.reset();
        log<LogLevel::ERROR>(std::string_view("error", 5));
        CHECK_EQ(output.get_write_count(), 1U);
        CHECK_EQ(static_cast<uint8_t>(output.get_last_level()), static_cast<uint8_t>(LogLevel::ERROR));
    }

    SUBCASE("log<CRITICAL>()") {
        output.reset();
        log<LogLevel::CRITICAL>(std::string_view("critical", 8));
        CHECK_EQ(output.get_write_count(), 1U);
        CHECK_EQ(static_cast<uint8_t>(output.get_last_level()), static_cast<uint8_t>(LogLevel::CRITICAL));
    }

    SUBCASE("log_flush()") {
        output.reset();
        log_flush();
        CHECK_EQ(output.get_flush_count(), 1U);
    }

    // テスト後にクリーンアップ
    get_logger().set_output(nullptr);
}
