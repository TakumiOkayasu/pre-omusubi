#pragma once

#include "omusubi/core/types.h"

namespace omusubi {

/**
 * @brief 1次元センサーインターフェース
 *
 * 温度、湿度、気圧など、単一の値を持つセンサーを扱います。
 *
 * 使用例:
 * @code
 * Measurable1D* sensor = ctx.get_temperature_sensor();
 *
 * if (sensor) {
 *     float temperature = sensor->get_value();
 *     serial->write("Temperature: "_sv);
 *     serial->write(temperature);
 *     serial->write_line(" °C"_sv);
 * }
 * @endcode
 */
class Measurable1D {
public:
    Measurable1D() = default;
    virtual ~Measurable1D() = default;
    Measurable1D(const Measurable1D&) = delete;
    Measurable1D& operator=(const Measurable1D&) = delete;
    Measurable1D(Measurable1D&&) = delete;
    Measurable1D& operator=(Measurable1D&&) = delete;

    /** @brief センサー値を取得 @return センサーの測定値 */
    virtual float get_value() const = 0;
};

/**
 * @brief 3次元センサーインターフェース
 *
 * 加速度センサー、ジャイロスコープ、磁気センサーなど、X/Y/Z軸の3次元データを持つセンサーを扱います。
 *
 * 使用例:
 * @code
 * Measurable3D* accel = ctx.get_sensor()->get_accelerometer_context();
 *
 * if (accel) {
 *     // 3軸まとめて取得
 *     Vector3 acc = accel->get_values();
 * }
 * @endcode
 *
 * @note このインターフェースを実装するクラス: AccelerometerContext, GyroscopeContext
 */
class Measurable3D {
public:
    Measurable3D() = default;
    virtual ~Measurable3D() = default;
    Measurable3D(const Measurable3D&) = delete;
    Measurable3D& operator=(const Measurable3D&) = delete;
    Measurable3D(Measurable3D&&) = delete;
    Measurable3D& operator=(Measurable3D&&) = delete;

    /** @brief X/Y/Z軸の値をまとめて取得 @return 3次元ベクトル */
    virtual Vector3 get_values() const = 0;
};

}  // namespace omusubi