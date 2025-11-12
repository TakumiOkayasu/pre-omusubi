#include "omusubi/platform/m5stack/m5stack_wifi_context.hpp"
#include <WiFi.h>

namespace omusubi {
namespace platform {
namespace m5stack {

M5StackWiFiContext::M5StackWiFiContext()
    : connected_(false)
    , found_count_(0)
    , scanning_(false) {

    last_ssid_[0] = '\0';
    last_password_[0] = '\0';
}

M5StackWiFiContext::~M5StackWiFiContext() {
    if (connected_) {
        WiFi.disconnect(true);
    }
}

// ========================================
// Connectable実装
// ========================================

bool M5StackWiFiContext::connect() {
    if (connected_) {
        return true;
    }

    // 最後に接続したSSIDがない場合は失敗
    if (last_ssid_[0] == '\0') {
        return false;
    }

    // WiFi接続開始
    WiFi.mode(WIFI_STA);
    WiFi.begin(last_ssid_, last_password_);

    // 接続待機（30秒タイムアウト）
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > 30000) {
            return false;
        }
        delay(100);
    }

    connected_ = true;
    return true;
}

// ========================================
// Scannable実装
// ========================================

uint8_t M5StackWiFiContext::scan() {
    if (scanning_) {
        return found_count_;
    }

    found_count_ = 0;
    scanning_ = true;

    // WiFiネットワークをスキャン
    int16_t count = WiFi.scanNetworks();

    if (count > 0) {
        found_count_ = (count < 20) ? static_cast<uint8_t>(count) : 20;

        // 見つかったネットワークの情報を保存
        for (uint8_t i = 0; i < found_count_; ++i) {
            // SSID
            String ssid = WiFi.SSID(i);
            strncpy(found_networks_[i].ssid, ssid.c_str(),
                   sizeof(found_networks_[i].ssid) - 1);
            found_networks_[i].ssid[sizeof(found_networks_[i].ssid) - 1] = '\0';

            // RSSI
            found_networks_[i].rssi = WiFi.RSSI(i);

            // 暗号化タイプ
            found_networks_[i].encryption_type = WiFi.encryptionType(i);
            found_networks_[i].is_open = (found_networks_[i].encryption_type == WIFI_AUTH_OPEN);
        }
    }

    scanning_ = false;
    WiFi.scanDelete();

    return found_count_;
}

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
