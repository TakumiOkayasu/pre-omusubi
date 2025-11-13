#include "omusubi/platform/m5stack/m5stack_bluetooth_context.hpp"
#include <BluetoothSerial.h>
#include <new>  // placement new用
#include <cstring>

namespace omusubi {
namespace platform {
namespace m5stack {

// ========================================
// 実装の詳細（プラットフォーム固有の型を完全に隠蔽）
// ========================================

namespace {
// 無名名前空間で実装を完全に隠蔽
struct BluetoothImpl {
    // プラットフォーム固有のメンバー
    BluetoothSerial bt;
    bool connected;
    char local_name[64];

    // スキャン結果
    struct FoundDevice {
        char name[64];
        char address[32];
        int32_t rssi;
    };
    FoundDevice found_devices[10];
    uint8_t found_count;
    bool scanning;

    BluetoothImpl()
        : connected(false)
        , found_count(0)
        , scanning(false) {
        strncpy(local_name, "M5Stack", sizeof(local_name));
        local_name[sizeof(local_name) - 1] = '\0';
    }
};

// 静的変数として実装を保持（シングルトン）
static BluetoothImpl impl;
}  // namespace

M5StackBluetoothContext::M5StackBluetoothContext() {
    // 実装は静的変数なので、特に何もしない
}

M5StackBluetoothContext::~M5StackBluetoothContext() {
    if (impl.connected) {
        impl.bt.disconnect();
    }
}

// ========================================
// Readable実装
// ========================================

FixedBuffer<256> M5StackBluetoothContext::read() {
    FixedBuffer<256> result;
    if (!impl.connected) {
        return result;
    }

    // 利用可能なバイトを全て読み込む
    while (impl.bt.available() && result.size() < 256) {
        uint8_t byte = static_cast<uint8_t>(impl.bt.read());
        result.append(byte);
    }

    return result;
}

// ========================================
// Writable実装
// ========================================

void M5StackBluetoothContext::write(StringView text) {
    if (!impl.connected) {
        return;
    }

    // StringViewの内容を書き込む
    for (uint32_t i = 0; i < text.byte_length(); ++i) {
        impl.bt.write(static_cast<uint8_t>(text[i]));
    }
}

// ========================================
// Connectable実装
// ========================================

bool M5StackBluetoothContext::connect() {
    if (impl.connected) {
        return true;
    }

    // ローカル名でBluetooth開始
    impl.bt.begin(impl.local_name);
    impl.connected = true;
    return true;
}

// ========================================
// Scannable実装
// ========================================

uint8_t M5StackBluetoothContext::scan() {
    if (impl.scanning) {
        return impl.found_count;
    }

    impl.found_count = 0;
    impl.scanning = true;

    // Bluetoothデバイスをスキャン（5秒間）
    BTScanResults* results = impl.bt.discover(5000);

    if (results) {
        uint8_t count = results->getCount();
        impl.found_count = (count < 10) ? count : 10;

        // 見つかったデバイスの情報を保存
        for (uint8_t i = 0; i < impl.found_count; ++i) {
            BTAdvertisedDevice* device = results->getDevice(i);

            // デバイス名
            const char* name = device->getName().c_str();
            strncpy(impl.found_devices[i].name, name, sizeof(impl.found_devices[i].name) - 1);
            impl.found_devices[i].name[sizeof(impl.found_devices[i].name) - 1] = '\0';

            // MACアドレス
            const char* addr = device->getAddress().toString().c_str();
            strncpy(impl.found_devices[i].address, addr, sizeof(impl.found_devices[i].address) - 1);
            impl.found_devices[i].address[sizeof(impl.found_devices[i].address) - 1] = '\0';

            // RSSI
            impl.found_devices[i].rssi = device->getRSSI();
        }
    }

    impl.scanning = false;
    return impl.found_count;
}

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
