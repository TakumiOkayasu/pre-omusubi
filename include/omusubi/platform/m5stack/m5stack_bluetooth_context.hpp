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

    // Readable interface
    FixedBuffer<256> read() override;

    // Writable interface
    void write(StringView text) override;

    // Connectable interface
    bool connect() override;

    // Scannable interface
    uint8_t scan() override;
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
