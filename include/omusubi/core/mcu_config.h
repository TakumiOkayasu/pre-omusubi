#pragma once

#include <cstddef>

/**
 * @file mcu_config.h
 * @brief MCU（マイコン）固有の設定定義
 *
 * デバイスやプラットフォームに応じた設定値を定義します。
 * コンパイル時に各種制約や機能を調整できます。
 */

namespace omusubi {
namespace config {

// ========================================
// メモリ制約のあるMCU向けの設定
// ========================================

/**
 * @brief 文字列の最大長（デフォルト値）
 */
constexpr std::size_t MAX_STRING_LENGTH = 256;

/**
 * @brief バッファの最大サイズ（デフォルト値）
 */
constexpr std::size_t MAX_BUFFER_SIZE = 1024;

// ========================================
// デバッグビルドの判定
// ========================================

/**
 * @brief デバッグモードが有効かどうか
 */
constexpr bool is_debug_build() {
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    return true;
#else
    return false;
#endif
}

// ========================================
// プラットフォーム判定
// ========================================

/**
 * @brief ESP32/ESP8266プラットフォームかどうか
 */
constexpr bool is_platform_esp() {
#if defined(ESP32) || defined(ESP8266)
    return true;
#else
    return false;
#endif
}

/**
 * @brief Arduinoプラットフォームかどうか
 */
constexpr bool is_platform_arduino() {
#if defined(ARDUINO)
    return true;
#else
    return false;
#endif
}

/**
 * @brief 汎用プラットフォームかどうか
 */
constexpr bool is_platform_generic() {
    return !is_platform_esp() && !is_platform_arduino();
}

} // namespace config
} // namespace omusubi
