// WiFiネットワークスキャンのサンプル
// ボタンAを押すと周辺のWiFiネットワークをスキャンし、
// 見つかったSSIDをシリアルに出力します

#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

// グローバル変数：setup()で一度だけ取得し、loop()で再利用
SystemContext& ctx = get_system_context();
SerialCommunication* serial = nullptr;
WiFiCommunication* wifi = nullptr;
Pressable* button_a = nullptr;

void setup() {
    // システムの初期化
    ctx.begin();

    // デバイスの取得（一度だけ）
    serial = ctx.get_serial(0);
    wifi = ctx.get_wifi();
    button_a = ctx.get_button(0);

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