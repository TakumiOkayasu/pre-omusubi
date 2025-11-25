#pragma once

#include <cstdint>
#include <string_view>

namespace omusubi {

/**
 * @brief ディスプレイ出力インターフェース
 */
class Displayable {
public:
    Displayable() = default;
    virtual ~Displayable() = default;
    Displayable(const Displayable&) = delete;
    Displayable& operator=(const Displayable&) = delete;
    Displayable(Displayable&&) = delete;
    Displayable& operator=(Displayable&&) = delete;

    /** @brief 文字列を表示 */
    virtual void display(std::string_view text) = 0;
};

} // namespace omusubi
