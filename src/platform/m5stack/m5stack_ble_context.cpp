#include "omusubi/platform/m5stack/m5stack_ble_context.hpp"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <new>  // placement new用
#include <cstring>

namespace omusubi {
namespace platform {
namespace m5stack {

// ========================================
// 実装の詳細（プラットフォーム固有の型を完全に隠蔽）
// ========================================

namespace {
// 無名名前空間で実装クラスを完全に隠蔽
struct BLEImpl {
    bool initialized;
    bool connected;
    BLEScan* scan;
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

    BLEImpl()
        : initialized(false)
        , connected(false)
        , scan(nullptr)
        , found_count(0)
        , scanning(false) {
        strncpy(local_name, "M5Stack-BLE", sizeof(local_name));
        local_name[sizeof(local_name) - 1] = '\0';
    }
};

// 静的変数として実装を保持（シングルトン）
static BLEImpl impl;
}  // namespace

M5StackBLEContext::M5StackBLEContext() {
    // 実装は静的変数なので、特に何もしない
}

M5StackBLEContext::~M5StackBLEContext() {
    if (impl.initialized) {
        BLEDevice::deinit(true);
    }
}

// ========================================
// Connectable実装
// ========================================

bool M5StackBLEContext::connect() {
    
    if (!impl.initialized) {
        // BLEデバイスの初期化
        BLEDevice::init(impl.local_name);
        impl.scan = BLEDevice::getScan();
        impl.scan->setActiveScan(true);
        impl.scan->setInterval(100);
        impl.scan->setWindow(99);
        impl.initialized = true;
    }

    impl.connected = true;
    return true;
}

// ========================================
// Scannable実装
// ========================================

uint8_t M5StackBLEContext::scan() {
    
    if (!impl.initialized || impl.scanning) {
        return impl.found_count;
    }

    impl.found_count = 0;
    impl.scanning = true;

    // BLEデバイスをスキャン（5秒間）
    BLEScanResults results = impl.scan->start(5, false);

    uint8_t count = results.getCount();
    impl.found_count = (count < 10) ? count : 10;

    // 見つかったデバイスの情報を保存
    for (uint8_t i = 0; i < impl.found_count; ++i) {
        BLEAdvertisedDevice device = results.getDevice(i);

        // デバイス名
        if (device.haveName()) {
            strncpy(impl.found_devices[i].name, device.getName().c_str(),
                   sizeof(impl.found_devices[i].name) - 1);
        } else {
            strncpy(impl.found_devices[i].name, "Unknown",
                   sizeof(impl.found_devices[i].name) - 1);
        }
        impl.found_devices[i].name[sizeof(impl.found_devices[i].name) - 1] = '\0';

        // MACアドレス
        strncpy(impl.found_devices[i].address, device.getAddress().toString().c_str(),
               sizeof(impl.found_devices[i].address) - 1);
        impl.found_devices[i].address[sizeof(impl.found_devices[i].address) - 1] = '\0';

        // RSSI
        impl.found_devices[i].rssi = device.getRSSI();
    }

    impl.scan->clearResults();
    impl.scanning = false;

    return impl.found_count;
}

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
