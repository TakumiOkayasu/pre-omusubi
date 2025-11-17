#pragma once

#include "omusubi/device/wifi_context.h"

#include <cstdint>

namespace omusubi {
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用WiFi通信実装
 *
 * Connectable + Scannableインターフェースを実装
 */
class M5StackWiFiContext : public WiFiContext {
public:
    M5StackWiFiContext();
    ~M5StackWiFiContext() override;

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
