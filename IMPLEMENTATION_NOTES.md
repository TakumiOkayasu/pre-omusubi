# Format実装の詳細

## 設計思想

Omusubiのformat実装は、C++23の`std::format`と`std::basic_format_string`をC++17で再現することを目指しています。

### アーキテクチャ

```
文字列リテラル/StringView
         ↓
basic_format_string<Args...> (型情報を保持)
         ↓
format/format_to/fmt (実装レイヤー)
         ↓
detail::format_impl (コア実装)
```

## 主要コンポーネント

### 1. basic_format_string<Args...>

**役割**: フォーマット文字列を型安全にラップ

```cpp
template <typename... Args>
class basic_format_string {
public:
    template <uint32_t N>
    constexpr basic_format_string(const char (&str)[N]) noexcept;

    constexpr basic_format_string(StringView sv) noexcept;

    constexpr const char* c_str() const noexcept;
    constexpr StringView view() const noexcept;
    constexpr uint32_t length() const noexcept;
    static constexpr uint32_t arg_count() noexcept;
};
```

**特徴**:
- テンプレート引数で期待される型を明示
- constexpr対応（コンパイル時評価可能）
- 型情報を静的に保持（`arg_count()`）
- C++23の`std::basic_format_string`と同等のインターフェース

### 2. format関数ファミリー

#### format<Capacity>(...)

**オーバーロード構成**:

```cpp
// 1. basic_format_string版（主要実装）
template <uint32_t Capacity, typename... FmtArgs, typename... Args>
constexpr FixedString<Capacity> format(
    const basic_format_string<FmtArgs...>& format_str,
    Args&&... args) noexcept;

// 2. 文字列リテラル版（内部でbasic_format_stringを構築）
template <uint32_t Capacity, uint32_t N, typename... Args>
constexpr FixedString<Capacity> format(
    const char (&format_str)[N],
    Args&&... args) noexcept;

// 3. StringView版（実行時の動的文字列対応）
template <uint32_t Capacity, typename... Args>
constexpr FixedString<Capacity> format(
    StringView format_str,
    Args&&... args) noexcept;
```

**設計原則**:
- すべてのオーバーロードは最終的に`basic_format_string`版を呼び出す
- 文字列リテラルは自動的に`basic_format_string<Args...>`に変換
- StringViewは実行時の柔軟性のために提供
- const char*版は削除（文字列リテラル版と競合するため）

#### format_to(...)

出力先バッファを事前に用意する版。パフォーマンス重視の用途向け。

```cpp
FixedString<128> buffer;
format_to(buffer, "Value: {}", 42);  // バッファ再利用可能
```

#### fmt(...)

デフォルト容量256のラッパー。最も簡潔な使用法。

```cpp
auto str = fmt("Hello, {}", "World");  // Capacity指定不要
```

### 3. detail::format_impl

**コア実装**:
- フォーマット文字列の解析
- プレースホルダー`{}`の検出
- エスケープ`{{`, `}}`の処理
- 引数の文字列変換（`detail::formatter`）

## 型変換システム

### detail::formatter<T>

各型に対する文字列変換を提供:

```cpp
template <typename T>
struct formatter {
    static constexpr uint32_t to_string(T value, char* buffer, uint32_t size) noexcept;
};
```

**対応型**:
- 整数型: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
- ブール型: `bool` (`"true"` / `"false"`)
- 文字型: `char`
- 文字列型: `const char*`, `StringView`

### 拡張方法

新しい型のサポートを追加するには、`detail::formatter`を特殊化:

```cpp
namespace omusubi::detail {
    template <>
    struct formatter<MyType> {
        static constexpr uint32_t to_string(MyType value, char* buffer, uint32_t size) noexcept {
            // 変換ロジック
            return written_length;
        }
    };
}
```

## 使い分けガイド

### 1. 一般的な用途: fmt()

```cpp
auto str = fmt("Hello, {}!", "World");
```

**メリット**:
- 最も簡潔
- Capacity指定不要
- 256文字で十分な大半のケースに対応

### 2. パフォーマンス重視: format_to()

```cpp
FixedString<256> buffer;
for (auto& item : items) {
    format_to(buffer, "Item: {}", item);
    process(buffer);
}
```

**メリット**:
- バッファ再利用可能
- メモリアロケーション最小化
- ループ内での使用に最適

### 3. 容量制御: format<N>()

```cpp
auto str = format<32>("ID: {}", id);  // 小さいバッファで効率化
```

**メリット**:
- 必要最小限のメモリ使用
- スタックサイズを厳密に制御

### 4. 型安全性重視: format_string<Args...>

```cpp
format_string<const char*, int> fs("Name: {}, Age: {}");
auto str = fmt(fs, "Alice", 25);
```

**メリット**:
- 期待される型を明示
- フォーマット文字列の再利用
- C++23への移行が容易
- APIの型シグネチャが明確

## C++17での制約と対処

### 制約1: 完全なコンパイル時検証は困難

C++17でも、プレースホルダー数と引数数の完全な一致チェックをコンパイル時に行うことは技術的に困難。

**対処**:
- 型情報の保持（`basic_format_string<Args...>`）
- 実行時の柔軟な型変換
- 将来のC++20/23へのアップグレードパス確保

### 制約2: constexpr関数の制約

C++17のconstexpr関数は複雑なロジックを含められるが、例外は投げられない。

**対処**:
- エラーは無視（空文字列を返す）
- noexcept保証
- 実用性優先の設計

### 制約3: 文字列リテラルとconst char*の曖昧性

文字列リテラル`const char (&)[N]`とポインタ`const char*`は異なる型。

**対処**:
- const char*オーバーロードを削除
- 文字列リテラル版とStringView版のみ提供
- 動的な文字列はStringView経由で使用

## C++23 std::formatとの比較

| 機能 | std::format (C++23) | omusubi::format (C++17) |
|------|---------------------|-------------------------|
| 基本フォーマット | ✅ | ✅ |
| フォーマット指定子 | ✅ ({:d}, {:x}, etc.) | ❌ ({}のみ) |
| basic_format_string | ✅ | ✅ |
| コンパイル時検証 | ✅ 完全 | ⚠️ 部分的 |
| カスタム型対応 | ✅ (formatter特殊化) | ✅ (同様) |
| 動的メモリ | ✅ (必要に応じて) | ❌ (スタックのみ) |
| constexpr対応 | ✅ | ✅ |

## パフォーマンス特性

### メモリ

- **ヒープアロケーション**: なし（完全スタックベース）
- **バッファサイズ**: コンパイル時固定（`FixedString<N>`）
- **オーバーヘッド**: 最小限（ポインタ + サイズのみ）

### 実行時

- **文字列リテラル**: O(n)、nは出力長
- **整数変換**: O(log n)、nは値の大きさ
- **エスケープ処理**: O(n)、nはフォーマット文字列長

### コンパイル時

- **constexpr評価**: 可能（C++14の制約内で）
- **テンプレートインスタンス化**: 型ごとに1回
- **インライン展開**: 小さな関数は積極的にインライン化

## まとめ

Omusubiのformat実装は、以下を達成しています:

1. **C++23互換API**: 将来の標準への移行が容易
2. **型安全性**: `basic_format_string<Args...>`による型情報の保持
3. **ゼロオーバーヘッド**: ヒープアロケーションなし、スタックのみ
4. **C++17準拠**: モダンC++17機能を活用
5. **使いやすさ**: 複数のオーバーロードで柔軟な使用法

埋め込みシステムの制約下で、モダンC++の利便性を提供する実装となっています。
