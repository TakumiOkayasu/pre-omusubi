// WiFiネットワークスキャンのサンプル
// ボタンAを押すと周辺のWiFiネットワークをスキャンし、
// 見つかったSSIDをシリアルに出力します

#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

// グローバル変数：setup()で一度だけ取得し、loop()で再利用
SystemContext& ctx = get_system_context();
ConnectableContext* connectable = nullptr;
InputContext* input = nullptr;
SerialContext* serial = nullptr;
WiFiContext* wifi = nullptr;
Pressable* button_a = nullptr;

void setup() {
    // システムの初期化
    ctx.begin();

    // Contextの取得（一度だけ）
    connectable = ctx.get_connectable_context();
    input = ctx.get_input_context();

    // デバイスの取得（一度だけ）
    serial = connectable->get_serial0_context();
    wifi = connectable->get_wifi_context();
    button_a = input->get_button_a_context();

    // 起動メッセージ
    serial->write_line("=== WiFi Scanner ==="_sv);
    serial->write_line("Press button A to scan"_sv);
}

void loop() {
    // システムの更新（ボタン状態などを更新）
    ctx.update();

    // ボタンAが押されたらスキャン開始
    if (button_a->was_pressed()) {
        serial->write_line("Scanning..."_sv);

        // WiFiスキャンを開始
        wifi->start_scan();
        ctx.delay(3000);  // 3秒間スキャン
        wifi->stop_scan();

        // 見つかったネットワークの数を取得
        uint8_t count = wifi->get_found_count();
        serial->write("Found "_sv);
        serial->write(count);
        serial->write_line(" networks:"_sv);

        // 最大10件のSSIDを表示
        for (uint8_t i = 0; i < count && i < 10; ++i) {
            FixedString<64> ssid = wifi->get_found_name(i);
            serial->write("  "_sv);
            serial->write(i);
            serial->write(": "_sv);
            serial->write_line(ssid);
        }
    }

    ctx.delay(10);
}