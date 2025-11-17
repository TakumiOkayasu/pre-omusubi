#pragma once

#include "omusubi/device/ble_context.h"

#include <cstdint>

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
    void disconnect() override;
    bool is_connected() const override;

    // Scannable interface
    void start_scan() override;
    void stop_scan() override;
    uint8_t get_found_count() const override;
    StringView get_found_name(uint8_t index) const override;
    int32_t get_found_signal_strength(uint8_t index) const override;
};

} // namespace m5stack
} // namespace platform
} // namespace omusubi
