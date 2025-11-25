#pragma once

#include <omusubi/interface/connectable.h>
#include <omusubi/interface/scannable.h>

namespace omusubi {

/**
 * @brief WiFi通信デバイス
 */
class WiFiContext : public Connectable, public Scannable {
public:
    WiFiContext() = default;
    ~WiFiContext() override = default;
    WiFiContext(const WiFiContext&) = delete;
    WiFiContext& operator=(const WiFiContext&) = delete;
    WiFiContext(WiFiContext&&) = delete;
    WiFiContext& operator=(WiFiContext&&) = delete;

    // Connectable interface
    bool connect() override = 0;
    void disconnect() override = 0;
    bool is_connected() const override = 0;

    // Scannable interface
    void start_scan() override = 0;
    void stop_scan() override = 0;
    uint8_t get_found_count() const override = 0;
    std::string_view get_found_name(uint8_t index) const override = 0;
    int32_t get_found_signal_strength(uint8_t index) const override = 0;
};

} // namespace omusubi
