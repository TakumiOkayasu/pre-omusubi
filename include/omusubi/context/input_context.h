#pragma once

namespace omusubi {

// 前方宣言（将来実装予定）
// class ButtonContext;
// class TouchContext;

/**
 * @brief 入力デバイスのコンテキスト
 *
 * @note このクラスは現在プレースホルダーです。
 *       将来のバージョンで以下の入力コンテキストが追加予定:
 *       - ButtonContext（ボタン入力）
 *       - TouchContext（タッチ入力）
 *
 * @warning 現在このクラスには機能がありません。
 *          プラットフォーム実装で具体的な入力デバイスアクセスを提供してください。
 */
class InputContext {
public:
    InputContext() = default;
    virtual ~InputContext() = default;
    InputContext(const InputContext&) = delete;
    InputContext& operator=(const InputContext&) = delete;
    InputContext(InputContext&&) = delete;
    InputContext& operator=(InputContext&&) = delete;

    // 入力デバイスのgetterメソッド（将来実装予定）
    // virtual ButtonContext* get_button_context(uint8_t index) = 0;
    // virtual TouchContext* get_touch_context() = 0;
};

} // namespace omusubi
