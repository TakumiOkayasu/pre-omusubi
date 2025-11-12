#pragma once

namespace omusubi {

// 前方宣言
// TODO: 実際のセンサーContextクラスを作成後、ここに追加
// class AccelerometerContext;
// class GyroscopeContext;
// class TemperatureContext;

/**
 * @brief センサーデバイスのコンテキスト
 *
 * 加速度センサー、ジャイロスコープ、温度センサーなど、
 * 計測機能を持つデバイスへのアクセスを提供します。
 *
 * 使用例:
 * @code
 * SensorContext* sensor = ctx.get_sensor();
 *
 * // 加速度センサーを取得
 * // AccelerometerContext* accel = sensor->get_accelerometer_context();
 * // Vector3 values = accel->get_values();
 * @endcode
 */
class SensorContext {
public:
    SensorContext() = default;
    virtual ~SensorContext() = default;
    SensorContext(const SensorContext&) = delete;
    SensorContext& operator=(const SensorContext&) = delete;
    SensorContext(SensorContext&&) = delete;
    SensorContext& operator=(SensorContext&&) = delete;

    // TODO: センサーデバイスのgetterメソッドをここに追加
    // virtual AccelerometerContext* get_accelerometer_context() = 0;
    // virtual GyroscopeContext* get_gyroscope_context() = 0;
    // virtual TemperatureContext* get_temperature_context() = 0;
};

}  // namespace omusubi
