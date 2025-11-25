# `auto`使用ガイド - 型推論のベストプラクティス

## 基本方針

**特に必要がなければ、変数は`auto`で受け取る。**

C++14の型推論機能（`auto`）を積極的に使用することで、コードの保守性と可読性を向上させます。

## `auto`を使用する理由

### 1. **型の結合度を下げる**

```cpp
// ✗ 型に強く結合している
StaticString<5> str = static_string("Hello");

// ✅ 実装の詳細から独立
auto str = static_string("Hello");
```

**利点:**
- 関数の戻り値の型が変わっても、呼び出し側のコードを修正する必要がない
- リファクタリングが容易

### 2. **テンプレートパラメータの記述を省略**

```cpp
// ✗ 冗長なテンプレートパラメータ
FixedString<64> buffer;
buffer.append("Hello"_sv);

// ✅ テンプレートパラメータを推論
auto msg = static_string("Hello");  // StaticString<5>が推論される
```

### 3. **型名の重複を避ける**

```cpp
// ✗ 型名が2回出現（DRY違反）
FixedString<32> name = FixedString<32>();

// ✅ 1回だけ
auto name = FixedString<32>();

// ✅ さらに良い - 型を完全に隠蔽
auto name = make_some_name();
```

### 4. **複雑な型を簡潔に扱う**

```cpp
// ✗ 長くて読みにくい
span<const char>::iterator it = buffer.begin();

// ✅ 簡潔で読みやすい
auto it = buffer.begin();
```

### 5. **constexprとの相性が良い**

```cpp
// ✅ コンパイル時評価を強制
constexpr auto str = static_string("Hello");
constexpr auto size = str.size();
```

## `auto`を使うべき場面

### ✅ **関数の戻り値を受け取る**

```cpp
// ✅ Recommended
auto ctx = get_system_context();
auto serial = ctx.get_connectable_context()->get_serial_context(0);
auto view = str.view();
```

### ✅ **イテレータ**

```cpp
// ✅ Recommended
for (auto it = buffer.begin(); it != buffer.end(); ++it) {
    // ...
}

// ✅ Even better: range-based for
for (auto c : buffer) {
    // ...
}
```

### ✅ **ラムダ式**

```cpp
// ✅ Recommended
auto handler = [](auto& request) {
    // Generic lambda
};
```

### ✅ **テンプレート戻り値**

```cpp
// ✅ Recommended
auto result = some_template_function<int, float>();
```

### ✅ **コンパイル時文字列**

```cpp
// ✅ Recommended
constexpr auto str = static_string("Hello");
auto concatenated = str + static_string(" World");
```

## 明示的な型を使うべき場面

### ✅ **数値リテラルの初期化**

**Omusubiの推奨: 明示的な型指定**

```cpp
// ✅ 推奨: 明示的な型（ビット幅・符号が明確）
uint32_t retry_count = 0;
uint8_t port_number = 1;
int32_t temperature = -10;
```

**⚠️ 問題: サフィックスなしの`auto`**

```cpp
// ✗ 非推奨: auto deduces 'int' (not uint32_t)
auto retry_count = 0;  // int (意図と異なる可能性)
```

**理由:**
- 組み込みシステムではビット幅と符号の明確化が重要
- `int`は符号付き、`uint32_t`は符号なし
- `auto`では意図しない型になる可能性

> **詳細な型選択ガイドライン:**
> `uint32_t` vs `size_t`の選択基準、サフィックスの使い分け、ラッパー関数の削除理由などは、[型システムの統一](type-system-unification.md)を参照してください。

### ✅ **リテラルで型が自明な場合 - `auto`推奨**

#### 1. **文字列リテラル**

```cpp
// ✅ auto deduces const char*
auto device_name = "M5Stack";     // const char*
auto message = "Hello World";     // const char*

// ✅ Explicit type (equivalent, but verbose)
const char* device_name = "M5Stack";
```

**理由:** 文字列リテラルは`const char*`と推論されるため、`auto`で意図が明確。

**配列との違い:**

```cpp
// String literal → pointer
auto str1 = "Hello";              // const char* (pointer)
decltype(auto) str2 = "Hello";    // const char(&)[6] (array reference)

// Array decay
char arr[] = "Hello";
auto ptr = arr;                   // char* (pointer)
auto& ref = arr;                  // char(&)[6] (array reference)
```

#### 2. **bool リテラル**

```cpp
// ✅ auto for bool (type is obvious)
auto is_enabled = true;           // bool
auto is_connected = false;        // bool
auto has_data = some_function();  // bool (if function returns bool)

// ⚠️ Explicit type (verbose, but acceptable)
bool is_enabled = true;
```

**理由:** `true`/`false`は明らかに`bool`なので、`auto`で十分。

**注意:** 条件式は明示的な方が良い場合も

```cpp
// ✅ auto is fine
auto result = (x > 0);            // bool

// ✅ Explicit type for clarity in complex conditions
bool is_valid = (x > 0 && y < 10 && z != 0);  // 複雑な条件式は明示的に
```

#### 3. **文字リテラル**

```cpp
// ✅ auto for char literals
auto separator = ',';             // char
auto newline = '\n';              // char
auto tab = '\t';                  // char

// ⚠️ Explicit type (acceptable for special chars)
char null_char = '\0';            // null終端の意図を強調
```

**理由:** 文字リテラルは`char`と推論されるので`auto`で問題なし。

#### 4. **浮動小数点リテラルにはサフィックス必須**

```cpp
// ✅ auto with suffix
auto pi = 3.14159f;               // float
auto epsilon = 1e-6;              // double
auto large = 1e10L;               // long double

// ⚠️ Without suffix, defaults to double
auto value = 3.14;                // double (not float!)

// ✅ Explicit type when precision matters
float sensor_value = 0.0f;        // 明示的にfloat
double calculation = 0.0;         // 明示的にdouble
```

**理由:**
- サフィックスがあれば型が明確
- 組み込みシステムでは`float`/`double`の違いが重要な場合は明示推奨

#### 5. **nullptr**

```cpp
// ✅ auto for nullptr
auto ptr = nullptr;               // std::nullptr_t

// ✅ Explicit type for pointer variables
SerialContext* serial = nullptr;  // 型が重要
uint8_t* buffer = nullptr;        // 型が重要
```

**理由:**
- `nullptr`自体は型が自明
- ただし、ポインタ変数は明示的な型の方が意図が明確

### ✅ **C APIや低レベルハードウェアとのインターフェース**

```cpp
// ✅ Explicit type for hardware registers
volatile uint32_t* const GPIO_BASE = reinterpret_cast<volatile uint32_t*>(0x40020000);

// ✅ auto is fine for C API string returns
auto device_name = get_device_name_c_api();  // const char*

// ✅ Explicit type when pointer type matters
uint8_t* buffer = get_buffer();  // uint8_t* (not auto)
```

**理由:**
- 文字列は`auto`で`const char*`と推論されるので問題なし
- ハードウェアレジスタは明示的な型が必要（volatile, const修飾）
- バッファポインタは型によってアライメントが変わるため明示推奨

### ✅ **明示的な型変換が必要**

```cpp
// ✅ Explicit conversion for correctness
auto s = some_span();
auto sv = StringView{s.data(), static_cast<uint32_t>(s.size())};
//                                ^^^^^^^^^^^^^^^^^^^^^^^^ 明示的キャスト
```

**理由:** 型変換の意図を明確にする

### ✅ **インターフェースの実装**

```cpp
// ✅ Interface requires explicit return type
class MyReadable : public Readable {
    size_t read(span<uint8_t> buffer) override {  // size_t required by interface
        // ...
    }
};
```

**理由:** インターフェース契約を明示する

## `auto`使用時の注意点

### 注意1: `const auto&` vs `auto`

```cpp
StringView get_view();

// ✗ Dangling reference
const auto& view = get_view();  // 一時オブジェクトへの参照（危険）

// ✅ Copy the value
auto view = get_view();  // 値をコピー（安全）
```

**原則:**
- 一時オブジェクトを受け取る場合は`auto`（値コピー）
- 既存のオブジェクトを参照する場合のみ`const auto&`

### 注意2: ポインタの意図を明確に

```cpp
// ✅ Pointer is clear
auto* serial = ctx.get_connectable_context()->get_serial_context(0);

// ✅ Also acceptable
auto serial = ctx.get_connectable_context()->get_serial_context(0);
```

**推奨:** ポインタを受け取る場合は`auto*`を使うと意図が明確

### 注意3: `auto&&` (Universal Reference) の慎重な使用

```cpp
// ✅ Perfect forwarding in templates
template <typename T>
void process(T&& value) {
    auto&& ref = std::forward<T>(value);
    // ...
}

// ✗ Avoid in normal code
auto&& x = get_value();  // Confusing - use auto or auto& instead
```

**原則:** `auto&&`はテンプレートの完全転送以外では避ける

## 実践例

### 例1: SystemContextの使用

```cpp
// ✗ Verbose
SystemContext& ctx = get_system_context();
ConnectableContext* connectable = ctx.get_connectable_context();
SerialContext* serial = connectable->get_serial_context(0);

// ✅ Concise with auto
auto& ctx = get_system_context();
auto* connectable = ctx.get_connectable_context();
auto* serial = connectable->get_serial_context(0);

// ✅ Even better: direct chain
auto* serial = get_system_context()
    .get_connectable_context()
    ->get_serial_context(0);
```

### 例2: コンパイル時文字列の連結

```cpp
// ✗ Verbose
constexpr StaticString<11> msg1 = static_string("Hello");
constexpr StaticString<11> msg2 = static_string(" World");
constexpr StaticString<11> combined = msg1 + msg2;

// ✅ Concise with auto
constexpr auto msg1 = static_string("Hello");
constexpr auto msg2 = static_string(" World");
constexpr auto combined = msg1 + msg2;
```

### 例3: ループでの使用

```cpp
FixedString<64> buffer;
buffer.append("Hello"_sv);

// ✗ Verbose
for (FixedString<64>::const_iterator it = buffer.begin();
     it != buffer.end(); ++it) {
    // ...
}

// ✅ Concise with auto
for (auto it = buffer.begin(); it != buffer.end(); ++it) {
    // ...
}

// ✅ Best: range-based for with auto
for (auto c : buffer) {
    // ...
}
```

### 例4: エラー処理

```cpp
// ✅ Explicit type for error codes
enum class ErrorCode : uint8_t {
    SUCCESS = 0,
    TIMEOUT = 1,
    INVALID = 2
};

ErrorCode result = perform_operation();  // 明示的な型
if (result != ErrorCode::SUCCESS) {
    // Handle error
}

// ✗ auto would hide the intent
auto result = perform_operation();  // What type is this?
```

## まとめ

### ✅ `auto`を使う（推奨）

- 関数の戻り値
- イテレータ
- ラムダ式
- テンプレート戻り値
- コンパイル時計算
- **文字列リテラル** (`const char*`と推論される)

### ✅ 明示的な型を使う（例外）

- 数値リテラルの初期化（`uint32_t count = 0;`）
- ハードウェアレジスタ（`volatile`修飾が必要）
- バッファポインタ（型によるアライメントが重要）
- 明示的な型変換
- エラーコードや列挙型
- インターフェース実装

### 判断基準

**型を明示すべきか迷ったら:**

1. **型が自明か？** → はい → `auto`を使う
2. **型が重要か？** → はい → 明示的な型を使う
3. **迷ったら？** → `auto`を使う（後で変更可能）

**`auto`は柔軟性と保守性を向上させる強力なツールです。積極的に活用しましょう！**

---

## 補足: `std::move()`について

### Omusubiフレームワークでは`std::move()`は**不要**

**理由:**

1. **ヒープ確保禁止**
   - Omusubiはヒープ確保を禁止している
   - すべてのオブジェクトはスタック上に存在
   - ムーブセマンティクスによる最適化の余地がない

2. **小さなオブジェクト**
   - `StringView`: ポインタ + 長さ (8～16バイト)
   - `span<T>`: ポインタ + サイズ (8～16バイト)
   - `FixedString<N>`: スタック配列（小さい）
   - コピーコストが極めて低い

3. **constexpr コンテキスト**
   - コンパイル時評価では`std::move()`は無意味
   - すべてコンパイラが最適化する

4. **C++14の制約**
   - ムーブコンストラクタは`constexpr`にできない（C++14）
   - コンパイル時評価で使えない

### `std::move()`とは何か？

**`std::move()`の正体:**

```cpp
// std::move()は単なるキャスト
template <typename T>
constexpr typename std::remove_reference<T>::type&& move(T&& t) noexcept {
    return static_cast<typename std::remove_reference<T>::type&&>(t);
}
```

**重要:** `std::move()`は**何も移動しない**。右辺値参照にキャストするだけ。

### 誤用例とその理由

#### ❌ 誤用1: 小さなオブジェクトでの使用

```cpp
// ✗ 誤用: StringViewは小さい（ポインタ+長さ = 8～16バイト）
StringView get_view() {
    StringView view = "Hello"_sv;
    return std::move(view);  // ← 無意味、むしろ最適化を妨げる
}

// ✅ 正しい: 単純にreturn（RVOが働く）
StringView get_view() {
    StringView view = "Hello"_sv;
    return view;  // ← コンパイラが最適化（NRVO）
}

// ✅ さらに良い: 直接return
StringView get_view() {
    return "Hello"_sv;  // ← 最適
}
```

**なぜ誤用か:**
- `StringView`はポインタと長さだけ（8～16バイト）
- レジスタに収まるサイズ
- コピーコストは極めて低い
- `std::move()`はRVO（Return Value Optimization）を妨げる可能性がある

#### ❌ 誤用2: constexprコンテキスト

```cpp
// ✗ 誤用: constexprでは無意味
constexpr auto create_string() {
    auto str = static_string("Hello");
    return std::move(str);  // ← C++14ではconstexprムーブ不可
}

// ✅ 正しい: 単純にreturn
constexpr auto create_string() {
    auto str = static_string("Hello");
    return str;  // ← コンパイル時に最適化される
}
```

**なぜ誤用か:**
- C++14ではムーブコンストラクタを`constexpr`にできない
- コンパイル時評価では`std::move()`は無意味
- すべてコンパイラが最適化

#### ❌ 誤用3: 参照を返す関数

```cpp
// ✗ 危険: ローカル変数への参照をムーブ
StringView& get_view_ref() {
    StringView view = "Hello"_sv;
    return std::move(view);  // ← ダングリング参照！
}

// ✅ 正しい: 値で返す
StringView get_view() {
    StringView view = "Hello"_sv;
    return view;
}
```

**なぜ危険か:**
- ローカル変数は関数終了時に破棄される
- `std::move()`しても寿命は延びない
- ダングリング参照（未定義動作）

#### ❌ 誤用4: ポインタの「ムーブ」

```cpp
// ✗ 誤解: ポインタは「ムーブ」できない
SerialContext* serial = get_serial();
auto moved = std::move(serial);  // ← 単なるポインタのコピー
// serialはまだ有効（nullにならない）
```

**なぜ誤解か:**
- ポインタは単なる数値（アドレス）
- `std::move()`してもコピーされるだけ
- 元のポインタは無効にならない

### `std::move()`が有用な場合（Omusubi外）

**一般的なC++では以下の場合に有用:**

#### ✅ 大きなオブジェクトの移動

```cpp
// 一般的なC++（Omusubi外）
std::vector<int> create_large_vector() {
    std::vector<int> v(1'000'000);  // 大きなベクタ
    // ... データを設定 ...
    return v;  // ムーブされる（C++11以降）
}

void use_vector() {
    auto v = create_large_vector();  // ムーブコンストラクタが呼ばれる
}
```

**なぜ有用か:**
- `std::vector`は内部でヒープ確保している
- コピーすると大量のメモリコピーが発生
- ムーブならポインタのコピーだけ

#### ✅ ユニークな所有権の移動

```cpp
// 一般的なC++（Omusubi外）
std::unique_ptr<Resource> ptr = std::make_unique<Resource>();
auto moved_ptr = std::move(ptr);  // 所有権を移動
// ptrはnullptrになる
```

**なぜ有用か:**
- `unique_ptr`はコピー不可
- 所有権の明示的な移動が必要
- `std::move()`で所有権を譲渡

### Omusubiでの推奨事項

#### ✅ DO: 単純にreturn

```cpp
// ✅ Recommended
StringView get_view() {
    return "Hello"_sv;
}

auto create_buffer() {
    FixedBuffer<64> buf;
    // ... 処理 ...
    return buf;  // std::move()不要
}
```

#### ✅ DO: autoで受け取る

```cpp
// ✅ Recommended
auto view = get_view();  // コピーされるが、コストは低い
auto buffer = create_buffer();  // スタック上のコピー
```

#### ❌ DON'T: std::move()を使う

```cpp
// ✗ Avoid
auto view = std::move(get_view());  // 無意味
return std::move(local_var);  // RVOを妨げる可能性
```

### まとめ

**Omusubiフレームワークでは:**

| 項目 | 推奨 | 理由 |
|------|------|------|
| `std::move()`の使用 | ❌ 不要 | ヒープ確保禁止、小さなオブジェクト |
| 単純なreturn | ✅ 推奨 | RVO/NRVOによる最適化 |
| 値渡し/値返し | ✅ 推奨 | コピーコストが低い |
| constexpr | ✅ 推奨 | コンパイル時最適化 |

**一般的なC++では:**
- 大きなオブジェクト（`std::vector`, `std::string`）: `std::move()`有用
- ユニークな所有権（`std::unique_ptr`）: `std::move()`必須
- 小さなオブジェクト（POD、ポインタ）: `std::move()`不要

**結論: Omusubiでは`std::move()`を使わない。単純にreturnすれば、コンパイラが最適化する。**

---

## 関連ドキュメント

- [型システムの統一](type-system-unification.md) - `uint32_t` vs `size_t`の使い分けルール
- CLAUDE.md - コーディング規約全般

---

**Version:** 1.1.0
**Last Updated:** 2025-11-17
