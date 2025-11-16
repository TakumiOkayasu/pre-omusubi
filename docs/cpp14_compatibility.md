# C++14互換性ガイド

このドキュメントでは、Omusubiフレームワークが厳格にC++14に準拠していることを説明し、C++17以降の機能を使用しないようにするためのガイドラインを提供します。

## プロジェクトの要件

**Omusubiは厳格にC++14に準拠しています。**

- コンパイラ: Clang/Clang++ (C++14サポート)
- 標準: `-std=c++14`
- **C++17以降の機能は完全に削除されています**
- C++17対応マイコンは使用しないため、条件付きコンパイルも使用しません
- 組み込みデバイスの幅広い互換性を確保

## 修正済みのC++17機能

### 1. `inline constexpr` 変数 → `constexpr` 変数

**問題**: C++17では`inline constexpr`変数が許可されていますが、C++14では変数に`inline`を使用できません。

**修正内容**: `/workspace/include/omusubi/core/mcu_config.h`

```cpp
// ❌ C++17（修正前）
inline constexpr std::size_t MAX_STRING_LENGTH = 256;
inline constexpr bool is_debug_build() { ... }

// ✅ C++14（修正後）
constexpr std::size_t MAX_STRING_LENGTH = 256;
constexpr bool is_debug_build() { ... }
```

**理由**:
- C++14では名前空間スコープの`constexpr`変数は各翻訳単位で内部リンケージを持つため、`inline`は不要
- `constexpr`関数は暗黙的に`inline`なので、明示的な`inline`キーワードは冗長

### 2. テンプレートユーザー定義リテラル → 削除

**問題**: C++17の`template <typename T, T... Chars>`を使用したユーザー定義リテラルはC++14では使用不可。

**修正内容**: `/workspace/include/omusubi/core/static_string.hpp`

C++17のユーザー定義リテラル`operator""_ss`は完全に削除され、C++14互換のヘルパー関数のみを提供します。

```cpp
// C++14互換のヘルパー関数
template <uint32_t N>
constexpr static_string<N> make_static_string(const char (&str)[N]) noexcept {
    return static_string<N>(str);
}
```

**使用方法**:

```cpp
// C++14（唯一の方法）
constexpr auto str = make_static_string("Hello");
```

### 3. Makefile の修正

**修正内容**: `/workspace/Makefile`

```makefile
# ❌ 修正前
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

# ✅ 修正後
CXXFLAGS = -std=c++14 -Wall -Wextra -Iinclude
```

## C++14で使用可能な機能

以下のC++14機能は積極的に使用してください：

### 1. 拡張された `constexpr`
```cpp
// C++14では複数の文、ループ、条件分岐が可能
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}
```

### 2. ジェネリックラムダ
```cpp
auto lambda = [](auto x, auto y) { return x + y; };
```

### 3. 戻り値型の推論
```cpp
auto get_value() {
    return 42;
}
```

### 4. バイナリリテラルと桁区切り
```cpp
int binary = 0b1010;
int large = 1'000'000;
```

### 5. 変数テンプレート
```cpp
template <typename T>
constexpr T pi = T(3.1415926535897932385);
```

## 禁止されているC++17以降の機能

以下の機能は**使用禁止**です：

### ❌ `inline` 変数
```cpp
// ❌ 使用禁止
inline constexpr int value = 42;

// ✅ 代わりにこれを使用
constexpr int value = 42;
```

### ❌ `if constexpr`
```cpp
// ❌ 使用禁止
template <typename T>
void func(T value) {
    if constexpr (std::is_integral_v<T>) {
        // ...
    }
}

// ✅ 代わりにテンプレート特殊化やSFINAEを使用
```

### ❌ 構造化束縛
```cpp
// ❌ 使用禁止
auto [x, y] = get_pair();

// ✅ 代わりにこれを使用
auto pair = get_pair();
auto x = pair.first;
auto y = pair.second;
```

### ❌ Fold expressions
```cpp
// ❌ 使用禁止
template <typename... Args>
auto sum(Args... args) {
    return (... + args);
}

// ✅ 代わりに再帰テンプレートを使用
```

### ❌ `std::optional`, `std::variant`, `std::any`
```cpp
// ❌ 使用禁止
std::optional<int> value;

// ✅ 代わりにカスタム型やポインタを使用
```

### ❌ Nested namespace定義
```cpp
// ❌ 使用禁止
namespace A::B::C {
    // ...
}

// ✅ 代わりにこれを使用
namespace A {
namespace B {
namespace C {
    // ...
}
}
}
```

### ❌ クラステンプレートの引数推論（CTAD）
```cpp
// ❌ 使用禁止
std::pair p(1, 2.0);

// ✅ 代わりに明示的に型を指定
std::pair<int, double> p(1, 2.0);
```

## コンパイル確認

### テストのビルド
```bash
cd test
make clean && make
```

すべてのテストが`-std=c++14`フラグでコンパイルされます。

### プロジェクトのビルド
```bash
make clean && make
```

メインプロジェクトも`-std=c++14`でコンパイルされます。

## まとめ

- ✅ すべてのコードはC++14に準拠
- ✅ C++17以降の機能は条件付きコンパイルまたは削除
- ✅ 代替実装を提供（`make_static_string`など）
- ✅ Makefileは`-std=c++14`を使用
- ✅ すべてのテストがC++14でコンパイル・実行可能

C++14の厳格な準拠により、Omusubiフレームワークは幅広い組み込み環境で使用できます。
