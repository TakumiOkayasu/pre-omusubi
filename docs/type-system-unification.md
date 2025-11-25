# 型システムの統一

## 概要

プロジェクト全体で`size_t`と`uint32_t`の混在を調査し、不要なラッパー関数を削除して型システムを統一しました。

## なぜ`uint32_t`を使用するのか

### 組み込みシステムにおける`size_t`の問題点

**`size_t`の定義はプラットフォーム依存:**
- 8ビットマイコン: `size_t` = `uint8_t` (0～255)
- 16ビットマイコン: `size_t` = `uint16_t` (0～65,535)
- 32ビットマイコン: `size_t` = `uint32_t` (0～4,294,967,295)
- 64ビットシステム: `size_t` = `uint64_t` (0～18,446,744,073,709,551,615)

**問題:**
```cpp
// 16ビットマイコンの場合
template <size_t N>  // size_t = uint16_t
class FixedString {
    char buffer_[N + 1];  // N = 65535の場合、65536バイトが必要
};

// ✗ N = 70000 を指定すると...
FixedString<70000> str;  // オーバーフローして4464になる！
```

### `uint32_t`を使用する理由

1. **プラットフォーム間で一貫したサイズ**
   - どのマイコンでも必ず32ビット
   - バイナリ互換性の確保（シリアル通信、ファイル保存など）

2. **意図的な制限の明示**
   - 文字列長は最大4GB（実用上十分）
   - メモリ制約のある組み込みシステムで妥当な範囲

3. **予測可能な動作**
   - コンパイル時にサイズが確定
   - プラットフォーム移植時の動作変化を防ぐ

4. **オーバーフロー防止**
   ```cpp
   // ✅ uint32_tなら明確
   FixedString<70000> str;  // 32ビットマイコンでは問題なし
                            // 16ビットマイコンではコンパイル時にメモリ不足エラー

   // ✗ size_tだと暗黙の変換が発生
   FixedString<70000> str;  // 16ビットマイコンで意図しない動作
   ```

5. **シリアライズの一貫性**
   ```cpp
   // ネットワークやファイルでデータを送受信する場合
   struct Header {
       uint32_t string_length;  // プラットフォーム間で一貫
   };

   // size_tだとプラットフォームごとにサイズが変わる
   struct Header {
       size_t string_length;  // 2バイト or 4バイト or 8バイト？
   };
   ```

### `size_t`を使う場合

**C++標準ライブラリとの互換性が必要な箇所:**

1. **I/O操作の戻り値**
   - `std::istream::read()`, `std::ostream::write()`が`size_t`を使用
   - POSIXの`read()`, `write()`も`ssize_t`（符号付き`size_t`）

2. **`span<T>`の操作**
   - C++20 `std::span`との互換性
   - 標準ライブラリのコンテナとの連携

3. **文字列リテラル演算子**
   - C++標準では`operator""_sv(const char*, size_t)`が必須

## 型の使い分けルール

| 用途 | 使用する型 | 理由 |
|------|-----------|------|
| **容量・サイズの定数（テンプレートパラメータ）** | `uint32_t` | プラットフォーム間で一貫した32ビット固定サイズ |
| **内部の長さ管理（メンバー変数）** | `uint32_t` | 容量と型を統一、バイナリ互換性の確保 |
| **`FixedString`, `FixedBuffer`, `StaticString`の内部処理** | `uint32_t` | 組み込みシステムでの予測可能な動作 |
| **I/O操作の戻り値（`read()`, `write()`, `available()`）** | `size_t` | C++標準ライブラリとの互換性 |
| **`span<T>`の操作（`size()`, `operator[]`）** | `size_t` | C++20 `std::span`互換性、標準コンテナとの連携 |
| **文字列リテラル演算子（`operator""_sv`）** | `size_t` | C++標準要求 |

## 実施した修正

### 1. `static_string.hpp` - 型システムの完全統一

#### 削除: 不要な`make_static_string()`ラッパー関数

**削除前:**
```cpp
// static_string() - size_tを使用
template <size_t N>
constexpr StaticString<N - 1> static_string(const char (&str)[N]) noexcept { ... }

// make_static_string() - uint32_tを使用（ラッパー）
template <uint32_t N>
constexpr static_string<N> make_static_string(const char (&str)[N]) noexcept {
    return static_string<N>(str);  // ← 単なるラッパー
}
```

**削除後:**
```cpp
// static_string() - uint32_tに統一
template <uint32_t N>
constexpr StaticString<N - 1> static_string(const char (&str)[N]) noexcept { ... }
```

**使用方法:**
```cpp
// ✅ シンプルな関数名
auto str = static_string("Hello");
```

#### 修正: `StaticString`クラスの型を`uint32_t`に統一

**修正内容:**
- テンプレートパラメータ: `template <size_t N>` → `template <uint32_t N>`
- メンバー関数の戻り値: `size_t size()` → `uint32_t size()`
- インデックス引数: `operator[](size_t index)` → `operator[](uint32_t index)`
- ループ変数: すべて`uint32_t`に統一

**影響範囲:**
- `StaticString`クラス全体
- `substring()`テンプレートパラメータ
- `from_int()`メソッド
- `count_digits()`ヘルパー関数
- 演算子オーバーロード（`operator+`, `operator==`, `operator!=`）

---

### 2. `span.hpp` - 不要なラッパー関数を削除

#### 削除: `to_span()`と`to_string_view()`

**削除前:**
```cpp
constexpr span<const char> to_span(StringView sv) noexcept {
    return {sv.data(), sv.byte_length()};
}

constexpr StringView to_string_view(span<const char> s) noexcept {
    return {s.data(), static_cast<uint32_t>(s.size())};  // ← 不要なキャスト隠蔽
}
```

**削除理由:**
1. 型の不一致を隠蔽しており、問題を見えにくくする
2. ユーザーは直接コンストラクタを使うべき
3. 冗長な抽象化層

**代替方法:**
```cpp
// span → StringView（明示的なキャストで意図を明確に）
StringView sv{s.data(), static_cast<uint32_t>(s.size())};

// StringView → span（そのまま渡せる）
span<const char> sp{sv.data(), sv.byte_length()};
```

---

### 3. `fixed_string.hpp` - ループ変数の統一

#### 修正: `from_span()`メソッド

**修正前:**
```cpp
void from_span(span<const char> s) noexcept {
    byte_length_ = (s.size() < Capacity) ? s.size() : Capacity;
    for (size_t i = 0; i < byte_length_; ++i) {  // ← size_t
        buffer_[i] = s[i];
    }
    buffer_[byte_length_] = '\0';
}
```

**修正後:**
```cpp
void from_span(span<const char> s) noexcept {
    byte_length_ = (s.size() < Capacity) ? static_cast<uint32_t>(s.size()) : Capacity;
    for (uint32_t i = 0; i < byte_length_; ++i) {  // ← uint32_t
        buffer_[i] = s[i];
    }
    buffer_[byte_length_] = '\0';
}
```

**変更点:**
- `s.size()`の代入時に明示的キャスト
- ループ変数を`uint32_t`に変更

---

### 4. `fixed_buffer.hpp` - ループ変数の統一

#### 修正: `from_span()`メソッド

**修正前:**
```cpp
void from_span(span<const uint8_t> s) noexcept {
    length_ = (s.size() < Capacity) ? s.size() : Capacity;
    for (size_t i = 0; i < length_; ++i) {  // ← size_t
        buffer_[i] = s[i];
    }
}
```

**修正後:**
```cpp
void from_span(span<const uint8_t> s) noexcept {
    length_ = (s.size() < Capacity) ? static_cast<uint32_t>(s.size()) : Capacity;
    for (uint32_t i = 0; i < length_; ++i) {  // ← uint32_t
        buffer_[i] = s[i];
    }
}
```

---

## 削除されたAPI

### 削除された関数

1. **`make_static_string()`** - 不要なラッパー
   ```cpp
   // ❌ 削除
   template <uint32_t N>
   constexpr static_string<N> make_static_string(const char (&str)[N]) noexcept;

   // ✅ 代わりにこれを使用
   auto str = static_string("Hello");
   ```

2. **`to_span()`** - 不要な変換関数
   ```cpp
   // ❌ 削除
   constexpr span<const char> to_span(StringView sv) noexcept;

   // ✅ 代わりにこれを使用
   span<const char> s{sv.data(), sv.byte_length()};
   ```

3. **`to_string_view()`** - 不要な変換関数
   ```cpp
   // ❌ 削除
   constexpr StringView to_string_view(span<const char> s) noexcept;

   // ✅ 代わりにこれを使用
   StringView sv{s.data(), static_cast<uint32_t>(s.size())};
   ```

---

## 影響と利点

### 削減されたコード
- 不要なラッパー関数: **3つ削除**
- コード行数: **約30行削減**

### 改善点
1. **シンプルな API**: `make_static_string()` → `static_string()`
2. **型の一貫性**: すべての固定長コンテナで`uint32_t`を使用
3. **明示的なキャスト**: 型変換が必要な箇所で意図を明確化
4. **保守性の向上**: 不要な抽象化を排除

### 後方互換性
- ❌ **`make_static_string()`を使用していたコードは修正が必要**
  - 修正は簡単: `make_static_string("Hello")` → `static_string("Hello")`
- ❌ **`to_span()`/`to_string_view()`を使用していたコードは修正が必要**
  - 明示的なコンストラクタ呼び出しに置き換え

---

## まとめ

✅ **型システムを完全に統一**
- 固定長コンテナ: `uint32_t`
- `span`操作: `size_t`（C++標準互換）
- I/O操作: `size_t`（標準ライブラリ互換）

✅ **不要なラッパー関数を削除**
- `make_static_string()` → 削除
- `to_span()` → 削除
- `to_string_view()` → 削除

✅ **コードの品質向上**
- シンプルで直感的なAPI
- 明示的な型変換で意図を明確化
- 保守性の向上

プロジェクト全体の型システムが統一され、不要な抽象化が排除されました。

---

**Version:** 1.0.0
**Last Updated:** 2025-11-16
