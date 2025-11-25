# C++17機能ガイド

このドキュメントでは、Omusubiフレームワークが使用するC++17の機能と、その活用方法について説明します。

## プロジェクトの要件

**OmusubiはC++17に準拠しています。**

- コンパイラ: Clang/Clang++ (C++17サポート)
- 標準: `-std=c++17`
- 組み込みデバイス向けの最適化を重視
- C++20以降の機能は使用しません

## C++17で使用する主要機能

### 1. Nested Namespace定義

**概要**: ネストされた名前空間を簡潔に定義できます。

**使用例**:

```cpp
// ✅ C++17（推奨）
namespace omusubi::utf8 {
    constexpr uint8_t get_char_byte_length(uint8_t first_byte) noexcept;
}

namespace omusubi::detail {
    template <typename T>
    struct Helper { /* ... */ };
}

// ❌ C++14（冗長）
namespace omusubi {
namespace utf8 {
    constexpr uint8_t get_char_byte_length(uint8_t first_byte) noexcept;
}
}
```

**適用箇所**:
- `/workspace/include/omusubi/core/string_view.h` - `omusubi::utf8`
- `/workspace/include/omusubi/core/string_base.hpp` - `omusubi::utf8`
- `/workspace/include/omusubi/core/mcu_config.h` - `omusubi::config`
- `/workspace/include/omusubi/core/logger.hpp` - `omusubi::detail`
- `/workspace/include/omusubi/core/format.hpp` - `omusubi::detail`

### 2. `inline`変数

**概要**: ヘッダーファイル内で変数を定義する際、ODR違反を回避できます。

**使用例**:

```cpp
// ✅ C++17（複数の翻訳単位で安全）
namespace omusubi::config {
    inline constexpr std::size_t MAX_STRING_LENGTH = 256;
    inline constexpr std::size_t MAX_BUFFER_SIZE = 1024;
}

// ❌ C++14（各翻訳単位で内部リンケージ）
namespace omusubi::config {
    constexpr std::size_t MAX_STRING_LENGTH = 256;
}
```

**利点**:
- ヘッダーオンリーライブラリで定数を共有可能
- リンカエラーを回避
- 各翻訳単位で同一のアドレスを保証

**注意**: 現在の実装では、名前空間スコープの`constexpr`変数は各翻訳単位で内部リンケージを持つため、C++14互換性を保っています。必要に応じて`inline constexpr`に変更可能です。

### 3. `constexpr`の拡張（C++14から継承）

**CRITICAL RULE: 可能な限りすべての関数に`constexpr`を付けること**

C++17ではC++14の拡張された`constexpr`機能を継承しています。

**基本方針:**
- すべてのコンストラクタ、メソッド、関数に`constexpr`を付ける
- I/Oやハードウェアアクセスなど、副作用がある関数のみ除外
- `constexpr`を付けない場合は、理由をコメントで明記

**例（FixedString）:**
```cpp
template <uint32_t Capacity>
class FixedString {
public:
    constexpr FixedString() noexcept : byte_length_(0) { buffer_[0] = '\0'; }

    constexpr explicit FixedString(const char* str) noexcept : byte_length_(0) {
        buffer_[0] = '\0';
        if (str != nullptr) {
            append(str);
        }
    }

    constexpr uint32_t byte_length() const noexcept { return byte_length_; }

    constexpr bool append(std::string_view view) noexcept {
        if (byte_length_ + view.size() > Capacity) {
            return false;
        }
        for (uint32_t i = 0; i < view.size(); ++i) {
            buffer_[byte_length_++] = view[i];
        }
        buffer_[byte_length_] = '\0';
        return true;
    }
};
```

**constexprの利点:**
- コンパイル時計算により実行時オーバーヘッドがゼロ
- バイナリサイズの削減
- `static_assert`やテンプレートパラメータで使用可能
- 型安全性の向上

### 4. `if constexpr`

**概要**: テンプレート内でコンパイル時条件分岐を行えます。

**使用例**:

```cpp
// ✅ C++17（推奨）
template <typename T>
void process(T value) {
    if constexpr (std::is_integral_v<T>) {
        // 整数型の処理
    } else if constexpr (std::is_floating_point_v<T>) {
        // 浮動小数点型の処理
    } else {
        // その他の型の処理
    }
}

// ❌ C++14（SFINAE、タグディスパッチが必要）
```

**利点**:
- コードの可読性向上
- SFINAEよりシンプル
- コンパイルエラーの削減

**注意**: 現在のコードベースではまだ使用していませんが、テンプレートメタプログラミングで活用可能です。

### 5. 構造化束縛（Structured Bindings）

**概要**: タプルや構造体を分解して複数の変数に代入できます。

**使用例**:

```cpp
// ✅ C++17（推奨）
auto [x, y] = get_coordinates();

// ❌ C++14（冗長）
auto pair = get_coordinates();
auto x = pair.first;
auto y = pair.second;
```

**注意**: 現在のコードベースではペアやタプルをほとんど使用していませんが、将来的に活用可能です。

### 6. Fold Expressions

**概要**: 可変長テンプレート引数を展開して演算を適用できます。

**使用例**:

```cpp
// ✅ C++17（推奨）
template <typename... Args>
auto sum(Args... args) {
    return (... + args);
}

// ❌ C++14（再帰テンプレートが必要）
```

**注意**: 現在のコードベースではまだ使用していませんが、format関数などで活用可能です。

## C++14から継承された機能

以下のC++14機能も積極的に使用してください：

### 1. ジェネリックラムダ
```cpp
auto lambda = [](auto x, auto y) { return x + y; };
```

### 2. 戻り値型の推論
```cpp
auto get_value() {
    return 42;
}
```

### 3. バイナリリテラルと桁区切り
```cpp
int binary = 0b1010;
int large = 1'000'000;
```

### 4. 変数テンプレート
```cpp
template <typename T>
constexpr T pi = T(3.1415926535897932385);
```

## 禁止されているC++20以降の機能

以下の機能は**使用禁止**です：

### ❌ Concepts
```cpp
// ❌ 使用禁止
template <typename T>
concept Integral = std::is_integral_v<T>;

// ✅ 代わりにSFINAEやstatic_assertを使用
```

### ❌ Ranges
```cpp
// ❌ 使用禁止
auto result = values | std::views::filter(...) | std::views::transform(...);

// ✅ 代わりに従来のイテレータを使用
```

### ❌ Coroutines
```cpp
// ❌ 使用禁止（組み込み環境では不適切）
```

### ❌ Modules
```cpp
// ❌ 使用禁止
import std.core;

// ✅ 代わりに従来の#includeを使用
```

### ❌ Three-way comparison operator
```cpp
// ❌ 使用禁止
auto operator<=>(const T&) const = default;

// ✅ 代わりに個別の比較演算子を定義
```

## コンパイル確認

### プロジェクトのビルド
```bash
make clean && make
```

### テストのビルドと実行
```bash
make tests
make test
```

すべてのコードが`-std=c++17`フラグでコンパイルされます。

## 今後の方針

1. **C++17のみを使用**: C++20以降の機能は使用しない
2. **Nested Namespaces活用**: コードの簡潔性を向上
3. **`if constexpr`の導入**: テンプレートメタプログラミングの改善
4. **constexpr推奨**: 可能な限りすべての関数に`constexpr`を付ける
5. **組み込み環境対応**: ゼロオーバーヘッドと小さいバイナリサイズを維持

## まとめ

- ✅ すべてのコードはC++17に準拠
- ✅ Nested namespaceでコードを簡潔化
- ✅ `inline`変数でヘッダーオンリー実装をサポート
- ✅ `constexpr`を最大限活用
- ✅ 組み込みデバイスの幅広い互換性を確保

**プロジェクト全体がC++17に準拠し、モダンな機能を活用しながら、組み込み環境に最適化されています。**

C++17の機能により、Omusubiフレームワークはより簡潔で保守性の高いコードを実現します。

---

## 関連ドキュメント

- [`auto`使用ガイド](auto-usage-guide.md) - C++14/17の型推論活用方法
- [CLAUDE.md](../CLAUDE.md) - コーディング規約全般
- [アーキテクチャ](architecture.md) - フレームワーク設計

---

**Version:** 1.0.0
**Last Updated:** 2025-11-23
