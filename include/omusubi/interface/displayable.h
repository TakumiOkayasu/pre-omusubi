#pragma once

#include "omusubi/core/string_view.h"

namespace omusubi {

/**
 * @brief ディスプレイ出力インターフェース
 *
 * LCD、OLED、TFTなどのディスプレイに文字を表示する機能を提供します。
 *
 * 使用例:
 * @code
 * Displayable* display = ctx.get_output()->get_display_context();
 *
 * if (display) {
 *     // 文字列を表示
 *     display->display("Hello, World!"_sv);
 * }
 * @endcode
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
    virtual void display(StringView text) = 0;
};

}  // namespace omusubi