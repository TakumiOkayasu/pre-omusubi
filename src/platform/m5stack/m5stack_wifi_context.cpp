#include "omusubi/platform/m5stack/m5stack_wifi_context.hpp"
#include <WiFi.h>
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
struct WiFiImpl {
    bool connected;
    char last_ssid[64];
    char last_password[64];

    // スキャン結果
    struct FoundNetwork {
        char ssid[64];
        int32_t rssi;
        bool is_open;
        uint8_t encryption_type;
    };
    FoundNetwork found_networks[20];
    uint8_t found_count;
    bool scanning;

    WiFiImpl()
        : connected(false)
        , found_count(0)
        , scanning(false) {
        last_ssid[0] = '\0';
        last_password[0] = '\0';
    }
};

// 静的変数として実装を保持（シングルトン）
static WiFiImpl impl;
}  // namespace

M5StackWiFiContext::M5StackWiFiContext() {
    // 実装は静的変数なので、特に何もしない
}

M5StackWiFiContext::~M5StackWiFiContext() {
    if (impl.connected) {
        WiFi.disconnect(true);
    }
}

// ========================================
// Connectable実装
// ========================================

bool M5StackWiFiContext::connect() {
    
    if (impl.connected) {
        return true;
    }

    // 最後に接続したSSIDがない場合は失敗
    if (impl.last_ssid[0] == '\0') {
        return false;
    }

    // WiFi接続開始
    WiFi.mode(WIFI_STA);
    WiFi.begin(impl.last_ssid, impl.last_password);

    // 接続待機（30秒タイムアウト）
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > 30000) {
            return false;
        }
        delay(100);
    }

    impl.connected = true;
    return true;
}

// ========================================
// Scannable実装
// ========================================

uint8_t M5StackWiFiContext::scan() {
    
    if (impl.scanning) {
        return impl.found_count;
    }

    impl.found_count = 0;
    impl.scanning = true;

    // WiFiネットワークをスキャン
    int16_t count = WiFi.scanNetworks();

    if (count > 0) {
        impl.found_count = (count < 20) ? static_cast<uint8_t>(count) : 20;

        // 見つかったネットワークの情報を保存
        for (uint8_t i = 0; i < impl.found_count; ++i) {
            // SSID
            String ssid = WiFi.SSID(i);
            strncpy(impl.found_networks[i].ssid, ssid.c_str(),
                   sizeof(impl.found_networks[i].ssid) - 1);
            impl.found_networks[i].ssid[sizeof(impl.found_networks[i].ssid) - 1] = '\0';

            // RSSI
            impl.found_networks[i].rssi = WiFi.RSSI(i);

            // 暗号化タイプ
            impl.found_networks[i].encryption_type = WiFi.encryptionType(i);
            impl.found_networks[i].is_open = (impl.found_networks[i].encryption_type == WIFI_AUTH_OPEN);
        }
    }

    impl.scanning = false;
    WiFi.scanDelete();

    return impl.found_count;
}

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
