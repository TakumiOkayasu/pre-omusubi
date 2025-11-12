// BLE Peripheralモード（サーバー）のサンプル
// BLEサービスとキャラクタリスティックを作成し、
// クライアントからの接続を待ちます。
// ボタンAを押すと接続中のクライアントに通知を送信します。

#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

// BLE UUID定義
constexpr const char* SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
constexpr const char* CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

// グローバル変数：setup()で一度だけ取得し、loop()で再利用
SystemContext& ctx = get_system_context();
ConnectableContext* connectable = nullptr;
InputContext* input = nullptr;
SerialContext* serial = nullptr;
BLEContext* ble = nullptr;
Pressable* button_a = nullptr;
BLEService* service = nullptr;
BLECharacteristic* ch = nullptr;

void setup() {
    // システムの初期化
    ctx.begin();

    // Contextの取得（一度だけ）
    connectable = ctx.get_connectable_context();
    input = ctx.get_input_context();

    // デバイスの取得（一度だけ）
    serial = connectable->get_serial0_context();
    ble = connectable->get_ble_context();
    button_a = input->get_button_a_context();

    serial->write_line("=== BLE Server ==="_sv);

    // Peripheralモードで開始（デバイス名: M5Stack-BLE）
    ble->begin_peripheral("M5Stack-BLE"_sv);
    serial->write("Device: "_sv);
    serial->write_line(ble->get_local_name());

    // サービスを追加
    service = ble->add_service(
        StringView::from_c_string(SERVICE_UUID)
    );

    // キャラクタリスティックを追加（読み取り・書き込み・通知可能）
    ch = service->add_characteristic(
        StringView::from_c_string(CHAR_UUID),
        static_cast<uint16_t>(BLECharacteristicProperty::read) |
        static_cast<uint16_t>(BLECharacteristicProperty::write) |
        static_cast<uint16_t>(BLECharacteristicProperty::notify)
    );
    ch->write_string("Hello!"_sv);

    // アドバタイジング開始（クライアントから発見可能に）
    ble->start_advertising();
    serial->write_line("Advertising..."_sv);
    serial->write_line("Press button A to notify"_sv);
}

void loop() {
    // システムの更新（ボタン状態などを更新）
    ctx.update();

    // ボタンAが押されたら通知を送信
    if (button_a->was_pressed()) {
        if (ble->is_connected()) {
            // 接続中のクライアントに通知を送信
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
