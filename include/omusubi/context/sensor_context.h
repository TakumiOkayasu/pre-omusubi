#pragma once

namespace omusubi {

// 前方宣言（将来実装予定）
// class AccelerometerContext;
// class GyroscopeContext;
// class TemperatureContext;

/**
 * @brief センサーデバイスのコンテキスト
 */
class SensorContext {
public:
    SensorContext() = default;
    virtual ~SensorContext() = default;
    SensorContext(const SensorContext&) = delete;
    SensorContext& operator=(const SensorContext&) = delete;
    SensorContext(SensorContext&&) = delete;
    SensorContext& operator=(SensorContext&&) = delete;

    // センサーデバイスのgetterメソッド（将来実装予定）
    // virtual AccelerometerContext* get_accelerometer_context() = 0;
    // virtual GyroscopeContext* get_gyroscope_context() = 0;
    // virtual TemperatureContext* get_temperature_context() = 0;
};

} // namespace omusubi
