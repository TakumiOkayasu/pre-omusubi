# テストガイド

このドキュメントでは、Omusubiフレームワークにおけるテストの書き方と実行方法を定義します。

## 目次

1. [テスト方針](#テスト方針)
2. [テストフレームワーク](#テストフレームワーク)
3. [テストの書き方](#テストの書き方)
4. [命名規則](#命名規則)
5. [テスト構造](#テスト構造)
6. [アサーション](#アサーション)
7. [テストカバレッジ](#テストカバレッジ)
8. [モックとスタブ](#モックとスタブ)
9. [組み込み環境でのテスト](#組み込み環境でのテスト)
10. [実機テスト](#実機テスト)

---

## テスト方針

### 1. 単体テストの重要性

**Omusubiはヘッダーオンリーの抽象化フレームワークのため、単体テストが品質保証の要。**

**テストが必要な理由:**
- コンパイル時エラーを早期検出
- `constexpr`関数の正しさを検証
- 型安全性の保証
- リファクタリング時の回帰防止

### 2. テストファースト開発

**新機能実装前にテストを書く習慣を推奨。**

```cpp
// 1. テストを先に書く
TEST_CASE("FixedString - starts_with機能") {
    FixedString<32> s("Hello World");
    CHECK(s.starts_with("Hello"sv));
    CHECK_FALSE(s.starts_with("World"sv));
}

// 2. 実装を追加
// 3. テストが通ることを確認
```

**利点:**
- 要件を明確化できる
- 実装の設計が改善される
- テスト可能なコードになる

### 3. テスト範囲

**単体テスト対象:**
- Core types (`span<T>`, `std::string_view`, `FixedString<N>`, `FixedBuffer<N>`, `StaticVector<T, N>`, `RingBuffer<T, N>`, `Function<Sig, Size>`, `BitSet<N>`, `Flags<Enum>`, `Result<T, E>`, `Vector3`)
- Utility functions (`format()`, `clamp()`, `map_range()`, `lerp()`, `parse_int()`, etc.)
- Interface contracts

**統合テスト対象:**
- Platform implementations (M5Stack, Arduino, etc.)
- Device contexts (Serial, WiFi, Bluetooth, etc.)
- System context

**テスト不要:**
- Pure virtual interfaces (コンパイル時にチェック済み)
- Trivial getters/setters (実装がほぼない)

---

## テストフレームワーク

Omusubiは**doctest**を使用します。高速で軽量なヘッダーオンリーのC++テストフレームワークです。

### doctestの特徴

```cpp
// 基本的なテスト構造
#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("テストケース名") {
    // テストコード
    CHECK(condition);
    CHECK_EQ(actual, expected);
}
```

**利点:**
- ヘッダーオンリー（単一ファイル `test/doctest.h`）
- 例外なしモード対応（`DOCTEST_CONFIG_NO_EXCEPTIONS`）
- 高速コンパイル
- 豊富なアサーションマクロ
- サブケースによるテストの構造化

### doctestのインストール

**自動インストール（推奨）:**
- Dev Container起動時に最新版が自動的にダウンロードされます
- `test/doctest.h` に配置されます

**手動更新:**
```bash
# 最新版をダウンロード
curl -L https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h -o test/doctest.h
```

---

## テストの書き方

### 基本パターン

**すべてのテストファイルはこのパターンに従う:**

```cpp
// tests/test_example.cpp

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omusubi/core/example.hpp>

using namespace omusubi;

TEST_CASE("Example - 基本機能") {
    // Arrange (準備)
    Example obj;

    // Act (実行)
    auto result = obj.do_something();

    // Assert (検証)
    CHECK(result);
}

TEST_CASE("Example - エッジケース") {
    SUBCASE("空の入力") {
        // テストコード
    }

    SUBCASE("最大値") {
        // テストコード
    }
}
```

### AAA パターン (Arrange-Act-Assert)

**すべてのテストケースはAAA パターンに従う:**

```cpp
TEST_CASE("FixedString - 追加操作") {
    // Arrange - テスト準備
    FixedString<32> s;

    // Act - 操作実行
    bool success = s.append("Hello"sv);

    // Assert - 結果検証
    CHECK(success);
    CHECK_EQ(s.byte_length(), 5U);
}
```

**AAA の利点:**
- テストの意図が明確
- 読みやすい構造
- デバッグが容易

### SUBCASEによる関連テストのグループ化

**関連するテストケースをSUBCASEでまとめる:**

```cpp
TEST_CASE("FixedString - 容量管理") {
    FixedString<10> s;

    SUBCASE("容量内の追加") {
        CHECK(s.append("Hello"));
        CHECK_EQ(s.byte_length(), 5U);
    }

    SUBCASE("容量いっぱいまで追加") {
        CHECK(s.append("1234567890"));
        CHECK_EQ(s.byte_length(), 10U);
    }

    SUBCASE("容量超過時の失敗") {
        s.append("1234567890");
        CHECK_FALSE(s.append("X"));
    }
}
```

**SUBCASEの利点:**
- 共通のセットアップを共有
- 関連テストを論理的にグループ化
- 独立した実行（各SUBCASEは新しい状態から開始）

---

## 命名規則

### テストファイル名

**`test_[対象].cpp` の形式:**

```
tests/
├── test_span.cpp           # span<T>のテスト
├── test_string_view.cpp    # std::string_viewのテスト
├── test_fixed_string.cpp   # FixedString<N>のテスト
├── test_fixed_buffer.cpp   # FixedBuffer<N>のテスト
├── test_vector3.cpp        # Vector3のテスト
├── test_format.cpp         # format()のテスト
├── test_format_string.cpp  # FormatStringのテスト
├── test_auto_capacity.cpp  # AutoCapacityのテスト
├── doctest.h               # doctestフレームワーク
└── core/
    ├── test_result.cpp     # Result<T,E>のテスト
    └── test_logger.cpp     # Loggerのテスト
```

### TEST_CASE名

**`対象 - 機能説明` の形式:**

```cpp
// 良い例: 明確で具体的
TEST_CASE("span - 基本機能")
TEST_CASE("span - イテレーション")
TEST_CASE("span - サブスパン取得")
TEST_CASE("FixedString - 追加操作")
TEST_CASE("FixedString - UTF-8処理")
TEST_CASE("Result - 成功値の取得")

// 悪い例: 抽象的で不明確
TEST_CASE("test1")
TEST_CASE("misc")
TEST_CASE("everything")
```

### SUBCASE名

**機能や条件を説明する簡潔な名前:**

```cpp
TEST_CASE("FixedString - 初期化") {
    SUBCASE("デフォルトコンストラクタ") { }
    SUBCASE("C文字列からの構築") { }
    SUBCASE("std::string_viewからの構築") { }
}
```

---

## テスト構造

### テストファイルの基本構成

```cpp
// ヘッダー部分
#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

// テスト対象のインクルード
#include <omusubi/core/fixed_string.hpp>

// 名前空間
using namespace omusubi;
using namespace std::literals;

// ========================================
// テストケース
// ========================================

TEST_CASE("FixedString - 基本機能") {
    SUBCASE("デフォルト構築") {
        FixedString<32> s;
        CHECK(s.empty());
        CHECK_EQ(s.byte_length(), 0U);
    }

    SUBCASE("C文字列からの構築") {
        FixedString<32> s("Hello");
        CHECK_EQ(s.byte_length(), 5U);
    }
}

TEST_CASE("FixedString - 追加操作") {
    FixedString<32> s;

    SUBCASE("std::string_view追加") {
        CHECK(s.append("Hello"sv));
        CHECK_EQ(s.byte_length(), 5U);
    }

    SUBCASE("連続追加") {
        CHECK(s.append("Hello"sv));
        CHECK(s.append(" World"sv));
        CHECK_EQ(s.byte_length(), 11U);
    }
}

TEST_CASE("FixedString - UTF-8処理") {
    FixedString<64> s;
    s.append("こんにちは"sv);

    CHECK_EQ(s.byte_length(), 15U);    // 5文字 × 3バイト
    CHECK_EQ(s.char_length(), 5U);     // 5文字
}
```

### テストセクションの分割基準

- 1つのTEST_CASEは1つの機能カテゴリに集中
- SUBCASEで具体的なシナリオを分割
- 5〜15個のチェックを目安に
- 長すぎる場合は別のTEST_CASEに分割

---

## アサーション

### 基本アサーション

**`CHECK(condition)`** - 条件が真であることを検証（失敗しても続行）

```cpp
CHECK(s.empty());
CHECK(!s.empty());
CHECK(result);
```

**`REQUIRE(condition)`** - 条件が真であることを検証（失敗したら停止）

```cpp
REQUIRE(ptr != nullptr);  // nullptrならここで停止
ptr->method();            // 安全にアクセス可能
```

### 比較アサーション

**`CHECK_EQ(actual, expected)`** - 等価比較

```cpp
CHECK_EQ(s.byte_length(), 5U);
CHECK_EQ(count, 3);
CHECK_EQ(result.value(), 42);
```

**`CHECK_NE(actual, expected)`** - 非等価比較

```cpp
CHECK_NE(ptr, nullptr);
CHECK_NE(status, Error::OK);
```

**`CHECK_LT / CHECK_LE / CHECK_GT / CHECK_GE`** - 大小比較

```cpp
CHECK_LT(value, 100);   // value < 100
CHECK_LE(value, 100);   // value <= 100
CHECK_GT(value, 0);     // value > 0
CHECK_GE(value, 0);     // value >= 0
```

### 否定アサーション

**`CHECK_FALSE(condition)`** - 条件が偽であることを検証

```cpp
CHECK_FALSE(s.empty());
CHECK_FALSE(buffer.full());
CHECK_FALSE(result.is_err());
```

### 浮動小数点比較

**`CHECK(a == doctest::Approx(b))`** - 許容誤差付き比較

```cpp
CHECK(v.x == doctest::Approx(1.0F).epsilon(0.0001F));
CHECK(angle == doctest::Approx(3.14159F).epsilon(0.00001F));
```

### 文字列比較

```cpp
// std::string_viewとの比較
CHECK_EQ(s.view(), "Hello"sv);

// C文字列との比較
CHECK(strcmp(s.c_str(), "Hello") == 0);
```

### アサーション使い分けガイド

```cpp
// 条件チェック
CHECK(result.is_ok());          // booleanの検証

// 数値比較（型を明示）
CHECK_EQ(count, 5);             // int
CHECK_EQ(length, 5U);           // unsigned

// 浮動小数点
CHECK(x == doctest::Approx(1.0F));

// ポインタ
CHECK_NE(ptr, nullptr);
REQUIRE_NE(ptr, nullptr);       // 後続のコードが依存する場合

// 否定
CHECK_FALSE(error_occurred);
```

---

## テストカバレッジ

### カバレッジ目標

**コアライブラリ:**
- ステートメントカバレッジ: **90%以上**
- ブランチカバレッジ: **80%以上**

**プラットフォーム実装:**
- ステートメントカバレッジ: **70%以上**
- ブランチカバレッジ: **60%以上**

### テストすべきケース

**1. 正常系 (Happy Path)**

```cpp
TEST_CASE("FixedString - 正常系") {
    FixedString<32> s;
    CHECK(s.append("Hello"sv));
    CHECK_EQ(s.byte_length(), 5U);
}
```

**2. 境界値 (Boundary)**

```cpp
TEST_CASE("FixedString - 境界値") {
    FixedString<10> s;

    SUBCASE("容量いっぱい") {
        CHECK(s.append("1234567890"));
        CHECK_EQ(s.byte_length(), 10U);
    }

    SUBCASE("容量超過") {
        s.append("1234567890");
        CHECK_FALSE(s.append("X"));
    }
}
```

**3. エラー系 (Error Cases)**

```cpp
TEST_CASE("span - エラー系") {
    span<int> empty_span;
    CHECK(empty_span.empty());
    CHECK_EQ(empty_span.size(), 0U);
}
```

**4. エッジケース (Edge Cases)**

```cpp
TEST_CASE("FixedString - エッジケース") {
    SUBCASE("空文字列") {
        FixedString<32> empty;
        CHECK_EQ(empty.byte_length(), 0U);
        CHECK(empty.empty());
    }

    SUBCASE("1文字") {
        FixedString<32> single("A");
        CHECK_EQ(single.byte_length(), 1U);
    }

    SUBCASE("マルチバイト文字") {
        FixedString<32> multibyte("あ");
        CHECK_EQ(multibyte.byte_length(), 3U);
        CHECK_EQ(multibyte.char_length(), 1U);
    }
}
```

**5. constexpr 検証**

```cpp
TEST_CASE("FixedString - constexpr対応") {
    // コンパイル時検証
    constexpr auto s = fixed_string("Hello");
    static_assert(s.byte_length() == 5, "constexpr byte_length()");
    static_assert(s.capacity() == 5, "constexpr capacity()");

    // 実行時検証も追加
    CHECK_EQ(s.byte_length(), 5U);
    CHECK_EQ(s.capacity(), 5U);
}
```

---

## モックとスタブ

### モックの基本方針

**組み込み環境でのモックは軽量に保つ。**

**モック実装パターン:**

```cpp
// テスト用モック実装
class MockSerialContext : public SerialContext {
private:
    FixedBuffer<256> read_buffer_;
    FixedBuffer<256> write_buffer_;
    bool connected_ = false;

public:
    // Connectable interface
    bool connect() override {
        connected_ = true;
        return true;
    }

    void disconnect() override {
        connected_ = false;
    }

    [[nodiscard]] bool is_connected() const override {
        return connected_;
    }

    // ByteReadable interface
    size_t read(span<uint8_t> buffer) override {
        auto count = (buffer.size() < read_buffer_.size())
            ? buffer.size()
            : read_buffer_.size();
        for (size_t i = 0; i < count; ++i) {
            buffer[i] = read_buffer_[i];
        }
        return count;
    }

    [[nodiscard]] size_t available() const override {
        return read_buffer_.size();
    }

    // テスト用ヘルパー
    void set_read_data(std::string_view data) {
        read_buffer_.clear();
        for (char c : data) {
            read_buffer_.append(static_cast<uint8_t>(c));
        }
    }

    [[nodiscard]] std::string_view get_written_data() const {
        return std::string_view(
            reinterpret_cast<const char*>(write_buffer_.data()),
            write_buffer_.size()
        );
    }
};
```

### モックを使ったテスト

```cpp
TEST_CASE("Serial - モックを使ったテスト") {
    MockSerialContext mock_serial;
    mock_serial.set_read_data("Hello"sv);

    uint8_t buffer[10];
    size_t bytes_read = mock_serial.read(span<uint8_t>(buffer, 10));

    CHECK_EQ(bytes_read, 5U);
    CHECK_EQ(buffer[0], 'H');
}
```

---

## 組み込み環境でのテスト

### 組み込み環境の制約

**組み込みシステムでのテストは以下の制約を考慮:**

1. **メモリ制約**: ヒープアロケーション禁止
2. **実行時間**: リアルタイム性を損なわない
3. **ハードウェア依存**: モック/スタブで分離
4. **例外なし**: `DOCTEST_CONFIG_NO_EXCEPTIONS`を使用

### ホスト環境でのテスト (推奨)

**開発マシン (Linux/macOS/Windows) でテストを実行:**

```bash
# Makefileを使用してテストをビルド・実行
make test

# 出力例:
# Running all tests with doctest...
#
# ========================================
# Running bin/test_result...
# ========================================
# [doctest] doctest version is "2.4.11"
# [doctest] run with "--help" for options
# ===============================================================================
# [doctest] test cases:  5 |  5 passed | 0 failed | 0 skipped
# [doctest] assertions: 42 | 42 passed | 0 failed |
# ...
# ========================================
# All tests passed successfully!
# ========================================
```

**個別テストの実行:**

```bash
# テストをビルド
make tests

# 個別テスト実行
./bin/test_fixed_string

# doctestオプション
./bin/test_fixed_string --help           # ヘルプ表示
./bin/test_fixed_string -tc="*UTF-8*"    # 特定のテストケースのみ実行
./bin/test_fixed_string -s               # 成功したテストも表示
```

**利点:**
- 高速なイテレーション
- デバッガが使える
- メモリチェッカー (Valgrind, ASan) が使える

---

## 実機テスト

### ターゲット環境でのテスト

**実機 (M5Stack, Arduino) でのテスト:**

```cpp
// examples/platform/m5stack/test_on_device.cpp

#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace std::literals;

SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void test_serial_loopback() {
    serial->write_text("Hello"sv);
    ctx.delay(100);

    uint8_t buffer[10];
    size_t bytes = serial->read(span<uint8_t>(buffer, 10));

    // 結果をシリアル出力で確認
    if (bytes == 5) {
        serial->write_text("[PASS] Serial loopback\r\n"sv);
    } else {
        serial->write_text("[FAIL] Serial loopback\r\n"sv);
    }
}

void setup() {
    ctx.begin();
    serial = ctx.get_connectable_context()->get_serial_context(0);
    serial->connect();

    test_serial_loopback();
}

void loop() {
    ctx.update();
}
```

**実機テストの用途:**
- ハードウェア統合テスト
- タイミング検証
- リソース使用量測定

### PlatformIOでの実機テスト実行

```bash
# ビルド
pio run -e m5stack

# アップロード
pio run -e m5stack --target upload

# シリアルモニタで結果確認
pio device monitor
```

---

## ベストプラクティス

### 1. 1つのテストは1つのことを検証

```cpp
// 良い例: 1つの機能を検証
TEST_CASE("append - 成功") {
    FixedString<32> s;
    CHECK(s.append("Hello"sv));
}

TEST_CASE("append - 容量超過") {
    FixedString<10> s("1234567890");
    CHECK_FALSE(s.append("X"));
}

// 悪い例: 複数の機能を混在
TEST_CASE("append") {
    FixedString<32> s1;
    CHECK(s1.append("Hello"sv));

    FixedString<10> s2("1234567890");
    CHECK_FALSE(s2.append("X"));
    // 2つの異なるケースが混在している
}
```

### 2. テストは独立させる

```cpp
// 良い例: 各テストで新しいオブジェクトを作成
TEST_CASE("clear") {
    FixedString<32> s("Hello");
    s.clear();
    CHECK_EQ(s.byte_length(), 0U);
}

TEST_CASE("append") {
    FixedString<32> s;  // 新しいオブジェクト
    CHECK(s.append("World"sv));
}

// 悪い例: グローバル変数でテスト間に依存
FixedString<32> global_str;  // テスト間で共有

TEST_CASE("append") {
    global_str.append("Hello");  // 前のテストの状態に依存
}
```

### 3. CHECKとREQUIREの使い分け

```cpp
TEST_CASE("ポインタ操作") {
    auto* ptr = get_device();

    // REQUIRE: 後続のコードが依存する場合
    REQUIRE_NE(ptr, nullptr);

    // CHECK: 独立した検証
    CHECK(ptr->is_connected());
    CHECK_EQ(ptr->get_status(), Status::OK);
}
```

### 4. constexpr 関数は static_assert でも検証

```cpp
TEST_CASE("constexpr検証") {
    // コンパイル時検証
    constexpr auto s = fixed_string("Hello");
    static_assert(s.byte_length() == 5, "constexpr計算");
    static_assert(s.capacity() == 5, "constexpr容量");

    // 実行時検証（必須）
    CHECK_EQ(s.byte_length(), 5U);
    CHECK_EQ(s.capacity(), 5U);
}
```

### 5. テストコードも読みやすく

```cpp
// 良い例: AAA パターンで構造化
TEST_CASE("example") {
    // Arrange
    FixedString<32> s;

    // Act
    auto result = s.append("Hello"sv);

    // Assert
    CHECK(result);
    CHECK_EQ(s.byte_length(), 5U);
}
```

---

## テスト実行

### Makefileコマンド

```bash
# 全テストをビルド
make tests

# 全テストをビルドして実行
make test

# テストバイナリを削除
make clean-tests

# 全てを削除（ビルド成果物 + テスト）
make clean-all
```

### doctestコマンドラインオプション

```bash
# ヘルプ表示
./bin/test_fixed_string --help

# 特定のテストケースのみ実行
./bin/test_fixed_string -tc="*UTF-8*"

# 成功したテストも表示
./bin/test_fixed_string -s

# 特定のサブケースのみ実行
./bin/test_fixed_string -sc="空文字列"

# テストケース一覧表示
./bin/test_fixed_string -ltc
```

### CI での自動テスト

**GitHub Actions で自動実行:**

```yaml
# .github/workflows/test.yml
name: Run Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install Clang
        run: sudo apt-get install -y clang
      - name: Download doctest
        run: |
          curl -L https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h -o test/doctest.h
      - name: Build and Run Tests
        run: make test
```

---

## チェックリスト

テスト作成時のチェックリスト:

- [ ] 正常系をテストしている
- [ ] 境界値をテストしている
- [ ] エラー系をテストしている
- [ ] エッジケースをテストしている
- [ ] constexpr 関数を static_assert で検証している
- [ ] AAA パターンに従っている
- [ ] テストは独立している
- [ ] TEST_CASE/SUBCASE名が適切である
- [ ] ホスト環境でテストが通る
- [ ] 実機でも検証した (プラットフォーム実装の場合)

---

## 関連ドキュメント

- [エラーハンドリングガイド](error-handling.md) - テスト時のエラーケース検証
- [CLAUDE.md](../CLAUDE.md) - コーディング規約全般
- [デバッグガイド](debug.md) - テスト失敗時のデバッグ手法
- [doctest公式ドキュメント](https://github.com/doctest/doctest/blob/master/doc/markdown/readme.md)

---

**Version:** 3.0.2
**Last Updated:** 2025-12-14
