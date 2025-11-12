#pragma once

namespace omusubi {

// 前方宣言
// TODO: 実際の入力デバイスContextクラスを作成後、ここに追加
// class ButtonContext;
// class TouchContext;

/**
 * @brief 入力デバイスのコンテキスト
 *
 * ボタン、タッチパネルなど、入力機能を持つデバイスへのアクセスを提供します。
 *
 * 使用例:
 * @code
 * InputContext* input = ctx.get_input();
 *
 * // ボタン0を取得
 * // ButtonContext* button = input->get_button0_context();
 * // ButtonState state = button->get_state();
 * @endcode
 */
class InputContext {
public:
    InputContext() = default;
    virtual ~InputContext() = default;
    InputContext(const InputContext&) = delete;
    InputContext& operator=(const InputContext&) = delete;
    InputContext(InputContext&&) = delete;
    InputContext& operator=(InputContext&&) = delete;

    // TODO: 入力デバイスのgetterメソッドをここに追加
    // virtual ButtonContext* get_button0_context() = 0;
    // virtual ButtonContext* get_button1_context() = 0;
    // virtual ButtonContext* get_button2_context() = 0;
    // virtual TouchContext* get_touch_context() = 0;
};

}  // namespace omusubi
