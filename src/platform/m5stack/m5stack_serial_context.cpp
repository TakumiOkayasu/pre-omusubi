#include "omusubi/platform/m5stack/m5stack_serial_context.hpp"
#include <M5Stack.h>

namespace omusubi {
namespace platform {
namespace m5stack {

M5StackSerialContext::M5StackSerialContext(uint8_t port)
    : serial_(nullptr)
    , port_(port)
    , baud_rate_(115200)
    , connected_(false) {

    // ポート番号に応じてHardwareSerialを割り当て
    switch (port_) {
        case 0: serial_ = &Serial; break;
        case 1: serial_ = &Serial1; break;
        case 2: serial_ = &Serial2; break;
        default: break;
    }
}

M5StackSerialContext::~M5StackSerialContext() {
    if (connected_) {
        serial_->end();
    }
}

// ========================================
// Readable実装
// ========================================

FixedBuffer<256> M5StackSerialContext::read() {
    FixedBuffer<256> result;
    if (!serial_ || !connected_) {
        return result;
    }

    // 利用可能なバイトを全て読み込む
    while (serial_->available() && result.size() < 256) {
        uint8_t byte = static_cast<uint8_t>(serial_->read());
        result.append(byte);
    }

    return result;
}

// ========================================
// Writable実装
// ========================================

void M5StackSerialContext::write(StringView text) {
    if (!serial_ || !connected_) {
        return;
    }

    // StringViewの内容を書き込む
    for (uint32_t i = 0; i < text.byte_length(); ++i) {
        serial_->write(static_cast<uint8_t>(text[i]));
    }
}

// ========================================
// Connectable実装
// ========================================

bool M5StackSerialContext::connect() {
    if (!serial_) {
        return false;
    }

    if (connected_) {
        return true;
    }

    serial_->begin(baud_rate_);
    connected_ = true;
    return true;
}

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
