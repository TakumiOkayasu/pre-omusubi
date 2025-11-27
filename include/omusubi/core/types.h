#pragma once

#include <cmath>
#include <cstdint>

namespace omusubi {

/**
 * @brief 電源状態
 */
enum class PowerState : uint8_t {
    BATTERY,  ///< バッテリー駆動
    USB,      ///< USB給電
    EXTERNAL, ///< 外部電源
    UNKNOWN   ///< 不明
};

/**
 * @brief ボタン状態
 */
enum class ButtonState : uint8_t {
    PRESSED, ///< 押されている
    RELEASED ///< 離されている
};

/**
 * @brief エラーコード
 */
enum class Error : uint8_t {
    OK = 0,            ///< 成功
    NOT_INITIALIZED,   ///< 初期化されていない
    NOT_CONNECTED,     ///< 接続されていない
    TIMEOUT,           ///< タイムアウト
    INVALID_PARAMETER, ///< 無効なパラメータ
    BUFFER_FULL,       ///< バッファフル
    WRITE_FAILED,      ///< 書き込み失敗
    READ_FAILED,       ///< 読み込み失敗
    PERMISSION_DENIED, ///< 権限なし
    FILE_NOT_FOUND,    ///< ファイルが見つからない
    INVALID_DATA,      ///< 無効なデータ
    UNKNOWN            ///< 不明なエラー
};

/**
 * @brief 3次元ベクトル
 */
struct Vector3 {
    float x; ///< X成分
    float y; ///< Y成分
    float z; ///< Z成分

    constexpr Vector3() noexcept : x(0.0F), y(0.0F), z(0.0F) {}

    constexpr Vector3(float x_val, float y_val, float z_val) noexcept : x(x_val), y(y_val), z(z_val) {}

    // ========================================
    // 算術演算子
    // ========================================

    /** @brief ベクトル加算 */
    [[nodiscard]] constexpr Vector3 operator+(const Vector3& other) const noexcept { return {x + other.x, y + other.y, z + other.z}; }

    /** @brief ベクトル減算 */
    [[nodiscard]] constexpr Vector3 operator-(const Vector3& other) const noexcept { return {x - other.x, y - other.y, z - other.z}; }

    /** @brief スカラー乗算 */
    [[nodiscard]] constexpr Vector3 operator*(float scalar) const noexcept { return {x * scalar, y * scalar, z * scalar}; }

    /** @brief スカラー除算 */
    [[nodiscard]] constexpr Vector3 operator/(float scalar) const noexcept { return {x / scalar, y / scalar, z / scalar}; }

    /** @brief 符号反転 */
    [[nodiscard]] constexpr Vector3 operator-() const noexcept { return {-x, -y, -z}; }

    // ========================================
    // 複合代入演算子
    // ========================================

    constexpr Vector3& operator+=(const Vector3& other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr Vector3& operator-=(const Vector3& other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    constexpr Vector3& operator*=(float scalar) noexcept {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    constexpr Vector3& operator/=(float scalar) noexcept {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    // ========================================
    // 比較演算子
    // ========================================

    [[nodiscard]] constexpr bool operator==(const Vector3& other) const noexcept { return x == other.x && y == other.y && z == other.z; }

    [[nodiscard]] constexpr bool operator!=(const Vector3& other) const noexcept { return !(*this == other); }

    // ========================================
    // ベクトル演算
    // ========================================

    /** @brief ベクトルの大きさの2乗（sqrt不要） */
    [[nodiscard]] constexpr float magnitude_squared() const noexcept { return x * x + y * y + z * z; }

    /** @brief ベクトルの大きさ */
    [[nodiscard]] float magnitude() const noexcept { return std::sqrt(magnitude_squared()); }

    /** @brief 正規化（単位ベクトル化） */
    [[nodiscard]] Vector3 normalized() const noexcept {
        const float mag = magnitude();
        if (mag > 0.0F) {
            return *this / mag;
        }
        return {0.0F, 0.0F, 0.0F};
    }

    /** @brief 内積（ドット積） */
    [[nodiscard]] constexpr float dot(const Vector3& other) const noexcept { return x * other.x + y * other.y + z * other.z; }

    /** @brief 外積（クロス積） */
    [[nodiscard]] constexpr Vector3 cross(const Vector3& other) const noexcept { return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x}; }
};

/** @brief スカラー * ベクトル（可換性のため） */
[[nodiscard]] constexpr Vector3 operator*(float scalar, const Vector3& v) noexcept {
    return v * scalar;
}

} // namespace omusubi
