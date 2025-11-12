#pragma once

#include "omusubi/context/sensor_context.h"

namespace omusubi {
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用センサーコンテキスト
 *
 * TODO: センサーデバイスの実装
 */
class M5StackSensorContext : public SensorContext {
public:
    M5StackSensorContext() = default;
    ~M5StackSensorContext() override = default;

    // TODO: センサーデバイスのgetterメソッドを実装
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
