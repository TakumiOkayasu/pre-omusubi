#include "omusubi/platform/m5stack/m5stack_system_info_context.hpp"
#include <M5Stack.h>
#include <esp_system.h>

namespace omusubi {
namespace platform {
namespace m5stack {

FixedString<64> M5StackSystemInfoContext::get_device_name() const {
    return FixedString<64>("M5Stack");
}

FixedString<32> M5StackSystemInfoContext::get_firmware_version() const {
    return FixedString<32>("Omusubi 0.1.0");
}

uint64_t M5StackSystemInfoContext::get_chip_id() const {
    return ESP.getEfuseMac();
}

uint32_t M5StackSystemInfoContext::get_uptime_ms() const {
    return millis();
}

uint32_t M5StackSystemInfoContext::get_free_memory() const {
    return esp_get_free_heap_size();
}

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
