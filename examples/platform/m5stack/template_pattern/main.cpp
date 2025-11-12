// テンプレートパラメータを使ったデバイスアクセスの例
//
// この例では、コンパイル時にポート番号を指定してシリアルポートにアクセスする
// テンプレートメタプログラミングのパターンを示します。

#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

// グローバル変数：setup()で一度だけ取得し、loop()で再利用
SystemContext& ctx = get_system_context();
ConnectableContext* connectable = nullptr;

// テンプレート関数：コンパイル時にポート番号を指定してシリアルポートを初期化
template<uint8_t Port>
void setup_serial_port() {
    // コンパイル時にポート番号が解決される
    auto* serial = connectable->get_serial_context<Port>();
    serial->connect();

    // ポート番号を文字列として表示するため、条件分岐
    if (Port == 0) {
        serial->write_line("Serial Port 0 initialized"_sv);
    } else if (Port == 1) {
        serial->write_line("Serial Port 1 initialized"_sv);
    } else if (Port == 2) {
        serial->write_line("Serial Port 2 initialized"_sv);
    }
}

// テンプレート関数：複数のシリアルポートを一括初期化
template<uint8_t... Ports>
void setup_serial_ports_impl() {
    // C++14の展開を使って各ポートを初期化
    int dummy[] = {(setup_serial_port<Ports>(), 0)...};
    (void)dummy;  // 未使用警告を抑制
}

// 使いやすいインターフェース
template<uint8_t Port0, uint8_t Port1>
void setup_two_ports() {
    setup_serial_ports_impl<Port0, Port1>();
}

void setup() {
    // システムの初期化
    ctx.begin();

    // デバイスの取得（一度だけ）
    connectable = ctx.get_connectable_context();

    // パターン1: 個別メソッド（従来型）
    auto* serial0_old = connectable->get_serial0_context();
    serial0_old->connect();
    serial0_old->write_line("Pattern 1: Individual method"_sv);

    // パターン2: テンプレートパラメータ（新機能）
    auto* serial0_new = connectable->get_serial_context<0>();
    serial0_new->write_line("Pattern 2: Template parameter"_sv);

    // パターン3: テンプレート関数での活用
    // コンパイル時にポート0と1を初期化
    setup_two_ports<0, 1>();

    // パターン4: ReadableContext経由でも同じパターンが使える
    auto* readable = ctx.get_readable_context();
    auto* serial1 = readable->get_serial_context<1>();
    auto data = serial1->read();

    // DIコンテナとしてのContext層の利点：
    // - デバイスの生成と管理をContext層に委譲
    // - テスト時にモックContext層を差し替え可能
    // - プラットフォーム切り替えが容易
}

void loop() {
    // システムの更新
    ctx.update();

    // テンプレートパラメータでアクセス
    auto* serial = connectable->get_serial_context<0>();
    serial->write("Hello from template pattern!"_sv);

    ctx.delay(1000);
}
