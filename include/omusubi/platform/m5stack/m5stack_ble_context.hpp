#pragma once

#include "omusubi/device/ble_context.h"

namespace omusubi {
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用BLE通信実装
 *
 * Connectable + Scannableインターフェースを実装
 */
class M5StackBLEContext : public BLEContext {
public:
    M5StackBLEContext();
    ~M5StackBLEContext() override;

    // Connectable interface
    bool connect() override;

    // Scannable interface
    uint8_t scan() override;
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
