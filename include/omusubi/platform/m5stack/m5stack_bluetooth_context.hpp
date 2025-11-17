#pragma once

#include "omusubi/device/bluetooth_context.h"

namespace omusubi {
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用Bluetooth Classic通信実装
 *
 * Readable + Writable + Connectable + Scannableインターフェースを実装
 */
class M5StackBluetoothContext : public BluetoothContext {
public:
    M5StackBluetoothContext();
    ~M5StackBluetoothContext() override;

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

    // Scannable interface
    void start_scan() override;
    void stop_scan() override;
    uint8_t get_found_count() const override;
    StringView get_found_name(uint8_t index) const override;
    int32_t get_found_signal_strength(uint8_t index) const override;
};

} // namespace m5stack
} // namespace platform
} // namespace omusubi
