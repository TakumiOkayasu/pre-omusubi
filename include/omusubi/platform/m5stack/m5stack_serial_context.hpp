#pragma once

#include "omusubi/device/serial_context.h"

namespace omusubi {
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用シリアル通信実装
 *
 * Readable + Writable + Connectableインターフェースを実装
 */
class M5StackSerialContext : public SerialContext {
private:
    uint8_t port_;  // ポート識別のみ保持

public:
    explicit M5StackSerialContext(uint8_t port);
    ~M5StackSerialContext() override;

    // Readable interface
    FixedBuffer<256> read() override;

    // Writable interface
    void write(StringView text) override;

    // Connectable interface
    bool connect() override;
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
