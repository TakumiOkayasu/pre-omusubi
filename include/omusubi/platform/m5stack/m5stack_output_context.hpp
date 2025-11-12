#pragma once

#include "omusubi/context/output_context.h"

namespace omusubi {
namespace platform {
namespace m5stack {

/**
 * @brief M5Stack用出力デバイスコンテキスト
 *
 * TODO: ディスプレイ等の出力デバイスの実装
 */
class M5StackOutputContext : public OutputContext {
public:
    M5StackOutputContext() = default;
    ~M5StackOutputContext() override = default;

    // TODO: 出力デバイスのgetterメソッドを実装
};

}  // namespace m5stack
}  // namespace platform
}  // namespace omusubi
