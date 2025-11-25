#pragma once

#include <cstdint>
#include <string_view>

namespace omusubi {

/**
 * @brief スキャン機能インターフェース
 *
 * Java設計思想: java.util.Scannerに相当
 * - 具体型（FixedString）ではなく抽象型（std::string_view）を返す
 * - 実装の詳細を隠蔽
 */
class Scannable {
public:
    Scannable() = default;
    virtual ~Scannable() = default;
    Scannable(const Scannable&) = delete;
    Scannable& operator=(const Scannable&) = delete;
    Scannable(Scannable&&) = delete;
    Scannable& operator=(Scannable&&) = delete;

    /** @brief スキャンを開始 */
    virtual void start_scan() = 0;

    /** @brief スキャンを停止 */
    virtual void stop_scan() = 0;

    /** @brief 発見されたデバイス数を取得 */
    virtual uint8_t get_found_count() const = 0;

    /** @brief 発見されたデバイス名を取得（Java String相当） */
    virtual std::string_view get_found_name(uint8_t index) const = 0;

    /** @brief 発見されたデバイスの信号強度を取得 */
    virtual int32_t get_found_signal_strength(uint8_t index) const = 0;
};

} // namespace omusubi
