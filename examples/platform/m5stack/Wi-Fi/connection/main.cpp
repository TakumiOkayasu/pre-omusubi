// WiFi接続のサンプル
// 指定したSSIDとパスワードでWiFiに接続し、
// 接続成功時にIPアドレスを表示します

#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

// WiFi設定（自分の環境に合わせて変更してください）
constexpr const char* WIFI_SSID = "YourSSID";
constexpr const char* WIFI_PASSWORD = "YourPassword";

// グローバル変数：setup()で一度だけ取得し、loop()で再利用
SystemContext& ctx = get_system_context();
ConnectableContext* connectable = nullptr;
SerialContext* serial = nullptr;
WiFiContext* wifi = nullptr;

void setup() {
    // システムの初期化
    ctx.begin();

    // Contextの取得（一度だけ）
    connectable = ctx.get_connectable_context();

    // デバイスの取得（一度だけ）
    serial = connectable->get_serial0_context();
    wifi = connectable->get_wifi_context();

    // 接続開始
    serial->write_line("=== WiFi Connection ==="_sv);
    serial->write("Connecting to "_sv);
    serial->write_line(StringView::from_c_string(WIFI_SSID));

    // WiFiに接続（タイムアウト: 10秒）
    if (wifi->connect_to(
        StringView::from_c_string(WIFI_SSID),
        StringView::from_c_string(WIFI_PASSWORD),
        10000
    )) {
        // 接続成功：IPアドレスを表示
        serial->write_line("Connected!"_sv);
        serial->write("IP: "_sv);
        serial->write_line(wifi->get_ip_address());
    } else {
        // 接続失敗
        serial->write_line("Failed to connect"_sv);
    }
}

void loop() {
    // システムの更新
    ctx.update();
    ctx.delay(100);
}