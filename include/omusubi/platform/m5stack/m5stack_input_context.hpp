#pragma once

#include "omusubi/context/input_context.h"

namespace omusubi {
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用入力デバイスコンテキスト
 *
 * TODO: ボタン等の入力デバイスの実装
 */
class M5StackInputContext : public InputContext {
public:
    M5StackInputContext() = default;
    ~M5StackInputContext() override = default;

    // TODO: 入力デバイスのgetterメソッドを実装
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
