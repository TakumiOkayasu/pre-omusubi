#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

// WiFi設定（自分の環境に合わせて変更）
constexpr const char* WIFI_SSID = "YourSSID";
constexpr const char* WIFI_PASSWORD = "YourPassword";

void setup() {
    SystemContext& ctx = get_system_context();
    ctx.begin();

    SerialCommunication* serial = ctx.get_serial(0);
    WiFiCommunication* wifi = ctx.get_wifi();

    serial->write_line("=== WiFi Connection ==="_sv);
    serial->write("Connecting to "_sv);
    serial->write_line(StringView::from_c_string(WIFI_SSID));

    if (wifi->connect_to(
        StringView::from_c_string(WIFI_SSID),
        StringView::from_c_string(WIFI_PASSWORD),
        10000
    )) {
        serial->write_line("Connected!"_sv);
        serial->write("IP: "_sv);
        serial->write_line(wifi->get_ip_address());
    } else {
        serial->write_line("Failed to connect"_sv);
    }
}

void loop() {
    SystemContext& ctx = get_system_context();
    ctx.update();
    ctx.delay(100);
}