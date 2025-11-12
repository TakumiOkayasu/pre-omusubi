#include "omusubi/system_context.h"
#include "omusubi/platform/m5stack/m5stack_system_context.hpp"

namespace omusubi {

// ========================================
// グローバルSystemContext取得（フリー関数）
// ========================================

// get_system_context()はフリー関数であり、クラスのstaticメソッドではない
// staticローカル変数（Meyers Singleton）でインスタンスを生成
// プラットフォーム実装の詳細を隠蔽し、ユーザーコードをプラットフォームから独立させる
SystemContext& get_system_context() {
    static platform::m5stack::M5StackSystemContext inst;
    return inst;
}

}  // namespace omusubi
