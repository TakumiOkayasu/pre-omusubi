#pragma once

#include "omusubi/context/writable_context.h"

namespace omusubi {

// 前方宣言
class SerialContext;
class BluetoothContext;
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用書き込み可能デバイスコンテキスト
 */
class M5StackWritableContext : public WritableContext {
private:
    SerialContext* serial0_;
    SerialContext* serial1_;
    SerialContext* serial2_;
    BluetoothContext* bluetooth_;

public:
    M5StackWritableContext(
        SerialContext* serial0,
        SerialContext* serial1,
        SerialContext* serial2,
        BluetoothContext* bluetooth)
        : serial0_(serial0)
        , serial1_(serial1)
        , serial2_(serial2)
        , bluetooth_(bluetooth) {}

    ~M5StackWritableContext() override = default;

    SerialContext* get_serial0_context() override { return serial0_; }
    SerialContext* get_serial1_context() override { return serial1_; }
    SerialContext* get_serial2_context() override { return serial2_; }
    BluetoothContext* get_bluetooth_context() override { return bluetooth_; }
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
