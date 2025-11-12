#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

constexpr const char* SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
constexpr const char* CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

void setup() {
    SystemContext& ctx = get_system_context();
    ctx.begin();

    SerialCommunication* serial = ctx.get_serial(0);
    BLECommunication* ble = ctx.get_ble();

    serial->write_line("=== BLE Server ==="_sv);

    ble->begin_peripheral("M5Stack-BLE"_sv);
    serial->write("Device: "_sv);
    serial->write_line(ble->get_local_name());

    BLEService* service = ble->add_service(
        StringView::from_c_string(SERVICE_UUID)
    );

    BLECharacteristic* ch = service->add_characteristic(
        StringView::from_c_string(CHAR_UUID),
        static_cast<uint16_t>(BLECharacteristicProperty::read) |
        static_cast<uint16_t>(BLECharacteristicProperty::write) |
        static_cast<uint16_t>(BLECharacteristicProperty::notify)
    );
    ch->write_string("Hello!"_sv);

    ble->start_advertising();
    serial->write_line("Advertising..."_sv);
    serial->write_line("Press button A to notify"_sv);
}

void loop() {
    SystemContext& ctx = get_system_context();
    ctx.update();

    Pressable* button_a = ctx.get_button(0);

    if (button_a->was_pressed()) {
        SerialCommunication* serial = ctx.get_serial(0);
        BLECommunication* ble = ctx.get_ble();

        if (ble->is_connected()) {
            BLEService* service = ble->get_service(
                StringView::from_c_string(SERVICE_UUID)
            );
            BLECharacteristic* ch = service->get_characteristic(
                StringView::from_c_string(CHAR_UUID)
            );

            const char* msg = "Button pressed!";
            ch->notify(
                reinterpret_cast<const uint8_t*>(msg),
                15
            );

            serial->write_line("Notified!"_sv);
        } else {
            serial->write_line("Not connected"_sv);
        }
    }

    ctx.delay(10);
}
