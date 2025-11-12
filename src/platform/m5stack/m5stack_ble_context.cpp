#include "omusubi/platform/m5stack/m5stack_ble_context.hpp"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

namespace omusubi {
namespace platform {
namespace m5stack {

M5StackBLEContext::M5StackBLEContext()
    : initialized_(false)
    , connected_(false)
    , scan_(nullptr)
    , found_count_(0)
    , scanning_(false) {

    strncpy(local_name_, "M5Stack-BLE", sizeof(local_name_));
    local_name_[sizeof(local_name_) - 1] = '\0';
}

M5StackBLEContext::~M5StackBLEContext() {
    if (initialized_) {
        BLEDevice::deinit(true);
    }
}

// ========================================
// Connectable実装
// ========================================

bool M5StackBLEContext::connect() {
    if (!initialized_) {
        // BLEデバイスの初期化
        BLEDevice::init(local_name_);
        scan_ = BLEDevice::getScan();
        scan_->setActiveScan(true);
        scan_->setInterval(100);
        scan_->setWindow(99);
        initialized_ = true;
    }

    connected_ = true;
    return true;
}

// ========================================
// Scannable実装
// ========================================

uint8_t M5StackBLEContext::scan() {
    if (!initialized_ || scanning_) {
        return found_count_;
    }

    found_count_ = 0;
    scanning_ = true;

    // BLEデバイスをスキャン（5秒間）
    BLEScanResults results = scan_->start(5, false);

    uint8_t count = results.getCount();
    found_count_ = (count < 10) ? count : 10;

    // 見つかったデバイスの情報を保存
    for (uint8_t i = 0; i < found_count_; ++i) {
        BLEAdvertisedDevice device = results.getDevice(i);

        // デバイス名
        if (device.haveName()) {
            strncpy(found_devices_[i].name, device.getName().c_str(),
                   sizeof(found_devices_[i].name) - 1);
        } else {
            strncpy(found_devices_[i].name, "Unknown",
                   sizeof(found_devices_[i].name) - 1);
        }
        found_devices_[i].name[sizeof(found_devices_[i].name) - 1] = '\0';

        // MACアドレス
        strncpy(found_devices_[i].address, device.getAddress().toString().c_str(),
               sizeof(found_devices_[i].address) - 1);
        found_devices_[i].address[sizeof(found_devices_[i].address) - 1] = '\0';

        // RSSI
        found_devices_[i].rssi = device.getRSSI();
    }

    scan_->clearResults();
    scanning_ = false;

    return found_count_;
}

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
