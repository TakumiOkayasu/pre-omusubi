#pragma once

#include "omusubi/context/readable_context.h"
#include "m5stack_serial_context.hpp"
#include "m5stack_bluetooth_context.hpp"

namespace omusubi {
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用読み取り可能デバイスコンテキスト
 *
 * Pattern A: 既存のデバイスインスタンスへの参照を保持
 */
class M5StackReadableContext : public ReadableContext {
private:
    M5StackSerialContext* serial0_;
    M5StackSerialContext* serial1_;
    M5StackSerialContext* serial2_;
    M5StackBluetoothContext* bluetooth_;

public:
    M5StackReadableContext(
        M5StackSerialContext* serial0,
        M5StackSerialContext* serial1,
        M5StackSerialContext* serial2,
        M5StackBluetoothContext* bluetooth)
        : serial0_(serial0)
        , serial1_(serial1)
        , serial2_(serial2)
        , bluetooth_(bluetooth) {}

    ~M5StackReadableContext() override = default;

    SerialContext* get_serial0_context() override { return serial0_; }
    SerialContext* get_serial1_context() override { return serial1_; }
    SerialContext* get_serial2_context() override { return serial2_; }
    BluetoothContext* get_bluetooth_context() override { return bluetooth_; }
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
