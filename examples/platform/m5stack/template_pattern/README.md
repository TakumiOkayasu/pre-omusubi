# テンプレートパラメータパターン

## 概要

コンパイル時にポート番号を指定してデバイスにアクセスするパターンの実装例です。

## 2つのアクセスパターン

### パターンA: 個別メソッド（基本）

```cpp
auto* serial = connectable->get_serial0_context();
```

**利点:**
- 明示的でわかりやすい
- すべてのデバイスで利用可能
- IDEの補完が効きやすい

### パターンB: テンプレートパラメータ

```cpp
auto* serial = connectable->get_serial_context<0>();
```

**利点:**
- コンパイル時にポート番号を指定
- テンプレートメタプログラミングで活用可能
- 複数ポートの一括処理が容易

**制約:**
- ランタイム値は使用不可（コンパイル時定数のみ）

## ユースケース

### ケース1: テンプレート関数でのデバイス初期化

```cpp
template<uint8_t Port>
void setup_serial_port() {
    auto* serial = connectable->get_serial_context<Port>();
    serial->connect();
}

// 使用
setup_serial_port<0>();
setup_serial_port<1>();
```

### ケース2: 可変長テンプレートでの一括処理

```cpp
template<uint8_t... Ports>
void setup_all_ports() {
    int dummy[] = {(setup_serial_port<Ports>(), 0)...};
}

// 使用例：ポート0, 1, 2を一括初期化
setup_all_ports<0, 1, 2>();
```

### ケース3: constexprでの条件分岐

```cpp
template<uint8_t Port>
constexpr const char* get_port_name() {
    return (Port == 0) ? "USB" :
           (Port == 1) ? "External1" :
                         "External2";
}
```

## Context層のDIコンテナとしての価値

このパターンは、Context層が**依存性注入（DI）コンテナ**として機能することを前提としています：

### DIコンテナとしての役割

1. **デバイスの生成と所有権管理**
   - Context層がデバイスの実体を所有
   - ユーザーはポインタのみを取得

2. **テスト時のモック差し替え**
   ```cpp
   class MockConnectableContext : public ConnectableContext {
       // テスト用のモックデバイスを返す
   };
   ```

3. **プラットフォーム切り替えの抽象化**
   - M5Stack → Arduino への切り替えが容易
   - Context層の実装を差し替えるだけ

4. **デバイス間の依存関係の管理**
   - ReadableContext, WritableContextが同じSerialContextを参照
   - 一貫性のある状態管理

## 注意事項

### ランタイム値は使用不可

```cpp
// ❌ NG: ランタイム値は使用不可
uint8_t port = 0;
auto* serial = connectable->get_serial_context<port>();  // コンパイルエラー

// ✅ OK: コンパイル時定数
constexpr uint8_t PORT = 0;
auto* serial = connectable->get_serial_context<PORT>();
```

### ループでの使用

```cpp
// ❌ NG: ループ変数は使用不可
for (uint8_t i = 0; i < 3; ++i) {
    auto* serial = connectable->get_serial_context<i>();  // コンパイルエラー
}

// ✅ OK: 可変長テンプレートを使用
template<uint8_t... Ports>
void process_all_ports() {
    int dummy[] = {(process_port<Ports>(), 0)...};
}
process_all_ports<0, 1, 2>();
```

## まとめ

テンプレートパラメータパターンは、以下の場合に有用です：

- テンプレートメタプログラミングを活用したい場合
- 複数デバイスの一括処理を実装する場合
- コンパイル時の型安全性を最大化したい場合

通常の使用では、個別メソッド（`get_serial0_context()`）の方が直感的でわかりやすいです。
