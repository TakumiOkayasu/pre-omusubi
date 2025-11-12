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
private:
    bool connected_;
    char last_ssid_[64];
    char last_password_[64];

    // スキャン結果
    struct FoundNetwork {
        char ssid[64];
        int32_t rssi;
        bool is_open;
        uint8_t encryption_type;
    };
    FoundNetwork found_networks_[20];
    uint8_t found_count_;
    bool scanning_;

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
