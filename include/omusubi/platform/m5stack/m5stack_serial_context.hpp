#pragma once

#include "omusubi/device/serial_context.h"

#include <cstdint>

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
    uint8_t port_; // ポート識別のみ保持

public:
    explicit M5StackSerialContext(uint8_t port);
    ~M5StackSerialContext() override;

    // ByteReadable interface
    size_t read(span<uint8_t> buffer) override;
    size_t available() const override;

    // TextReadable interface
    size_t read_line(span<char> buffer) override;

    // ByteWritable interface
    size_t write(span<const uint8_t> data) override;

    // TextWritable interface
    size_t write_text(span<const char> text) override;

    // Connectable interface
    bool connect() override;
    void disconnect() override;
    bool is_connected() const override;
};

} // namespace m5stack
} // namespace platform
} // namespace omusubi
