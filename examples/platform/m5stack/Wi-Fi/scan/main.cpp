#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

void setup() {
    SystemContext& ctx = get_system_context();
    ctx.begin();

    SerialCommunication* serial = ctx.get_serial(0);
    serial->write_line("=== WiFi Scanner ==="_sv);
    serial->write_line("Press button A to scan"_sv);
}

void loop() {
    SystemContext& ctx = get_system_context();
    ctx.update();

    Pressable* button_a = ctx.get_button(0);

    if (button_a->was_pressed()) {
        SerialCommunication* serial = ctx.get_serial(0);
        WiFiCommunication* wifi = ctx.get_wifi();

        serial->write_line("Scanning..."_sv);

        wifi->start_scan();
        ctx.delay(3000);
        wifi->stop_scan();

        uint8_t count = wifi->get_found_count();
        serial->write("Found "_sv);
        serial->write(count);
        serial->write_line(" networks:"_sv);

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