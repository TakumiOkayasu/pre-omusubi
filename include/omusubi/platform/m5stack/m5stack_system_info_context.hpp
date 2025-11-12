#pragma once

#include "omusubi/context/system_info_context.h"

namespace omusubi {
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用システム情報コンテキスト
 */
class M5StackSystemInfoContext : public SystemInfoContext {
public:
    M5StackSystemInfoContext() = default;
    ~M5StackSystemInfoContext() override = default;

    FixedString<64> get_device_name() const override;
    FixedString<32> get_firmware_version() const override;
    uint64_t get_chip_id() const override;
    uint32_t get_uptime_ms() const override;
    uint32_t get_free_memory() const override;
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
