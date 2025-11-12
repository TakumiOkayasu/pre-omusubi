// BLE Centralモード（クライアント）のサンプル
// ボタンAを押すと周辺のBLEデバイスをスキャンし、
// デバイス名と信号強度(RSSI)をシリアルに出力します。

#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

// グローバル変数：setup()で一度だけ取得し、loop()で再利用
SystemContext& ctx = get_system_context();
SerialCommunication* serial = nullptr;
BLECommunication* ble = nullptr;
Pressable* button_a = nullptr;

void setup() {
    // システムの初期化
    ctx.begin();

    // デバイスの取得（一度だけ）
    serial = ctx.get_serial(0);
    ble = ctx.get_ble();
    button_a = ctx.get_button(0);

    serial->write_line("=== BLE Client ==="_sv);

    // Centralモードで開始（デバイス名: M5Stack-Central）
    ble->begin_central("M5Stack-Central"_sv);
    serial->write_line("Ready"_sv);
    serial->write_line("Press button A to scan"_sv);
}

void loop() {
    // システムの更新（ボタン状態などを更新）
    ctx.update();

    // ボタンAが押されたらスキャン開始
    if (button_a->was_pressed()) {
        serial->write_line("Scanning..."_sv);

        // BLEスキャンを開始
        ble->start_scan();
        ctx.delay(3000);  // 3秒間スキャン
        ble->stop_scan();

        // 見つかったデバイスの数を取得
        uint8_t count = ble->get_found_count();
        serial->write("Found "_sv);
        serial->write(count);
        serial->write_line(" devices:"_sv);

        // 最大5件のデバイス情報を表示
        for (uint8_t i = 0; i < count && i < 5; ++i) {
            FixedString<64> name = ble->get_found_name(i);
            int32_t rssi = ble->get_found_signal_strength(i);  // 信号強度(dBm)

            serial->write("  "_sv);
            serial->write(i);
            serial->write(": "_sv);
            serial->write(name);
            serial->write(" ("_sv);
            serial->write(rssi);
            serial->write_line(" dBm)"_sv);
        }
    }

    ctx.delay(10);
}
