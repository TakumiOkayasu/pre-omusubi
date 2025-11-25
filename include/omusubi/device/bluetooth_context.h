#pragma once

#include <omusubi/interface/connectable.h>
#include <omusubi/interface/readable.h>
#include <omusubi/interface/scannable.h>
#include <omusubi/interface/writable.h>

namespace omusubi {

/**
 * @brief Bluetooth Classic通信デバイス
 */
class BluetoothContext : public ByteReadable, public TextReadable, public ByteWritable, public TextWritable, public Connectable, public Scannable {
public:
    BluetoothContext() = default;
    ~BluetoothContext() override = default;
    BluetoothContext(const BluetoothContext&) = delete;
    BluetoothContext& operator=(const BluetoothContext&) = delete;
    BluetoothContext(BluetoothContext&&) = delete;
    BluetoothContext& operator=(BluetoothContext&&) = delete;

    // ByteReadable interface
    size_t read(span<uint8_t> buffer) override = 0;
    size_t available() const override = 0;

    // TextReadable interface
    size_t read_line(span<char> buffer) override = 0;

    // ByteWritable interface
    size_t write(span<const uint8_t> data) override = 0;

    // TextWritable interface
    size_t write_text(span<const char> text) override = 0;

    // Connectable interface
    bool connect() override = 0;
    void disconnect() override = 0;
    bool is_connected() const override = 0;

    // Scannable interface
    void start_scan() override = 0;
    void stop_scan() override = 0;
    uint8_t get_found_count() const override = 0;
    std::string_view get_found_name(uint8_t index) const override = 0;
    int32_t get_found_signal_strength(uint8_t index) const override = 0;
};

} // namespace omusubi
