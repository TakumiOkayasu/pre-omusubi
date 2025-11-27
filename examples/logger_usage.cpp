// Logger使用例
// シングルトンパターン + テンプレートによるログ出力

#include <omusubi/omusubi.h>

using namespace omusubi;

// グローバル変数: setup()で一度だけ取得し、loop()で再利用
SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void setup() {
    // システム初期化
    ctx.begin();

    // Serial取得
    serial = ctx.get_connectable_context().get_serial_context(0);
    serial->connect();

    // Logger初期化（シングルトン）
    // SerialLogOutputはstaticで保持する必要がある
    static SerialLogOutput log_output(serial);
    get_logger().set_output(&log_output);
    get_logger().set_min_level(LogLevel::INFO);

    // ログ出力（テンプレート引数でレベル指定）
    get_logger().log<LogLevel::INFO>(std::string_view("System started", 14));
    get_logger().log<LogLevel::DEBUG>(std::string_view("This is hidden", 14)); // min_level=INFOなので出力されない

    // グローバル関数を使用（どこからでも呼べる）
    log<LogLevel::INFO>(std::string_view("Logger initialized", 18));
}

void loop() {
    ctx.update();

    // 定期的なログ出力（グローバル関数を使用）
    static uint32_t counter = 0;
    ++counter;

    if (counter % 100 == 0) {
        log<LogLevel::INFO>(std::string_view("Loop count: 100", 15));
    }

    if (counter == 1000) {
        log<LogLevel::WARNING>(std::string_view("High loop count", 15));
    }

    if (counter > 10000) {
        log<LogLevel::ERROR>(std::string_view("Loop count exceeded", 19));
        counter = 0; // リセット
    }

    ctx.delay(10);
}

// 別のファイル/モジュールからでも同じシングルトンを使用可能
void some_other_module() {
    // include <omusubi/omusubi.h> さえすれば、どこからでもログ出力可能
    log<LogLevel::INFO>(std::string_view("Called from another module", 26));
    log<LogLevel::DEBUG>(std::string_view("Debug from another module", 25));
    log<LogLevel::CRITICAL>(std::string_view("Critical error!", 15));
}
