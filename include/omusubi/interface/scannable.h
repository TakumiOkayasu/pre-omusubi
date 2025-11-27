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

    /**
     * @brief スキャンを開始
     * @return 開始成功時 true、失敗時 false
     */
    [[nodiscard]] virtual bool start_scan() = 0;

    /**
     * @brief スキャンを停止
     * @return 停止成功時 true、失敗時 false
     */
    [[nodiscard]] virtual bool stop_scan() = 0;

    /** @brief 発見されたデバイス数を取得 */
    [[nodiscard]] virtual uint8_t get_found_count() const = 0;

    /** @brief 発見されたデバイス名を取得（Java String相当） */
    [[nodiscard]] virtual std::string_view get_found_name(uint8_t index) const = 0;

    /** @brief 発見されたデバイスの信号強度を取得 */
    [[nodiscard]] virtual int32_t get_found_signal_strength(uint8_t index) const = 0;
};

} // namespace omusubi
