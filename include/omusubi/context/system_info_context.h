#pragma once

#include <cstdint>
#include <string_view>

namespace omusubi {

/**
 * @brief システム情報コンテキスト
 *
 * Java設計思想: java.lang.Systemに相当
 * - 具体型（FixedString）ではなく抽象型（std::string_view）を返す
 * - getProperty()のような設計
 */
class SystemInfoContext {
public:
    SystemInfoContext() = default;
    virtual ~SystemInfoContext() = default;
    SystemInfoContext(const SystemInfoContext&) = delete;
    SystemInfoContext& operator=(const SystemInfoContext&) = delete;
    SystemInfoContext(SystemInfoContext&&) = delete;
    SystemInfoContext& operator=(SystemInfoContext&&) = delete;

    /** @brief デバイス名を取得（Java String相当） */
    virtual std::string_view get_device_name() const = 0;

    /** @brief ファームウェアバージョンを取得（Java String相当） */
    virtual std::string_view get_firmware_version() const = 0;

    /** @brief チップIDを取得 */
    virtual uint64_t get_chip_id() const = 0;

    /** @brief 稼働時間を取得（ミリ秒） */
    virtual uint32_t get_uptime_ms() const = 0;

    /** @brief 空きメモリを取得（バイト） */
    virtual uint32_t get_free_memory() const = 0;
};

} // namespace omusubi
