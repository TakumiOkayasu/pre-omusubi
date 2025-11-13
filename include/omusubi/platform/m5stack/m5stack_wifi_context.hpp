#pragma once

#include "omusubi/device/wifi_context.h"

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

    // Scannable interface
    uint8_t scan() override;
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
