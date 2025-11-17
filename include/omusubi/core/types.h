#pragma once

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
};

} // namespace omusubi
