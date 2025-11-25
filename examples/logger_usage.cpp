// Logger使用例
// SerialLogOutputを使用してシリアル出力にログを記録

#include <omusubi/omusubi.h>

using namespace omusubi;

// グローバル変数: setup()で一度だけ取得し、loop()で再利用
SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;
SerialLogOutput* log_output = nullptr;
Logger* logger = nullptr;

void setup() {
    // システム初期化
    ctx.begin();

    // Serial取得
    serial = ctx.get_connectable_context().get_serial_context(0);
    serial->connect();

    // Logger初期化（スタック変数として保持）
    static SerialLogOutput log_out(serial);
    static Logger log(&log_out, LogLevel::INFO);
    log_output = &log_out;
    logger = &log;

    // ログ出力テスト
    logger->info(std::string_view("System started", 14));
    logger->debug(std::string_view("This debug message is hidden", 28)); // min_level=INFOなので出力されない

    // テンプレート関数を使用した出力（リリースビルドで最適化）
    log_at<LogLevel::INFO>(*logger, std::string_view("Logger initialized", 18));
}

void loop() {
    ctx.update();

    // 定期的なログ出力
    static uint32_t counter = 0;
    ++counter;

    if (counter % 100 == 0) {
        log_at<LogLevel::INFO>(*logger, std::string_view("Loop count: 100", 15));
    }

    if (counter == 1000) {
        log_at<LogLevel::WARNING>(*logger, std::string_view("High loop count", 15));
    }

    if (counter > 10000) {
        log_at<LogLevel::ERROR>(*logger, std::string_view("Loop count exceeded", 19));
        counter = 0; // リセット
    }

    ctx.delay(10);
}
