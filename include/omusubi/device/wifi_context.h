#pragma once

#include "omusubi/interface/connectable.h"
#include "omusubi/interface/scannable.h"

namespace omusubi {

/**
 * @brief WiFi通信デバイス
 *
 * WiFi接続、ネットワークスキャンを扱います。
 * Connectable、Scannableインターフェースを実装しています。
 *
 * 使用例:
 * @code
 * WiFiContext* wifi = ctx.get_connectable()->get_wifi_context();
 *
 * // WiFiに接続
 * wifi->connect();
 *
 * // ネットワークスキャン
 * uint8_t count = wifi->scan();
 * @endcode
 */
class WiFiContext
    : public Connectable
    , public Scannable {
public:
    WiFiContext() = default;
    ~WiFiContext() override = default;
    WiFiContext(const WiFiContext&) = delete;
    WiFiContext& operator=(const WiFiContext&) = delete;
    WiFiContext(WiFiContext&&) = delete;
    WiFiContext& operator=(WiFiContext&&) = delete;
};

}  // namespace omusubi