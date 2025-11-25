# Omusubi フレームワーク 単体テスト

このディレクトリには、Omusubiフレームワークのコアコンポーネント向けの単体テストが含まれています。

## テスト対象コンポーネント

### 基本型テスト（`tests/`）

| テストファイル | 対象 | 説明 |
|---------------|------|------|
| `test_span.cpp` | `span<T>` | 非所有メモリビュー（C++20 std::spanの簡易版） |
| `test_string_view.cpp` | `StringView` | UTF-8対応の文字列ビュー |
| `test_fixed_string.cpp` | `FixedString<N>` | 固定長UTF-8文字列（ヒープ確保なし） |
| `test_fixed_buffer.cpp` | `FixedBuffer<N>` | 固定長バイトバッファ（ヒープ確保なし） |
| `test_vector3.cpp` | `Vector3` | 3次元ベクトル（センサーデータ用） |
| `test_format.cpp` | `format()` | 型安全な文字列フォーマット |
| `test_format_string.cpp` | `FormatString` | フォーマット文字列パーサー |
| `test_auto_capacity.cpp` | `AutoCapacity` | 自動容量計算ユーティリティ |

### コアライブラリテスト（`tests/core/`）

| テストファイル | 対象 | 説明 |
|---------------|------|------|
| `test_result.cpp` | `Result<T,E>` | Rust風のエラーハンドリング型 |
| `test_logger.cpp` | `Logger` | ログ出力機能 |

## ビルドと実行

### プロジェクトルートから実行（推奨）

```bash
# 全テストをビルド
make tests

# 全テストをビルドして実行
make test

# テストバイナリを削除
make clean-tests
```

### 個別テストの実行

```bash
# ビルド後、個別に実行
./bin/test_fixed_string
./bin/test_result

# doctestオプション
./bin/test_fixed_string --help           # ヘルプ表示
./bin/test_fixed_string -tc="*UTF-8*"    # 特定のテストケースのみ実行
./bin/test_fixed_string -s               # 成功したテストも表示
```

## テストフレームワーク

**doctest** を使用しています。高速で軽量なヘッダーオンリーのC++テストフレームワークです。

**特徴:**
- ヘッダーオンリー（単一ファイル `tests/doctest.h`）
- 例外なしモード対応（`DOCTEST_CONFIG_NO_EXCEPTIONS`）
- 高速コンパイル
- 豊富なアサーションマクロ
- サブケースによるテストの構造化

**主要なマクロ:**
- `CHECK(condition)` - 条件が真であることを検証（失敗しても続行）
- `CHECK_EQ(actual, expected)` - 等価比較
- `CHECK_FALSE(condition)` - 条件が偽であることを検証
- `REQUIRE(condition)` - 条件が真であることを検証（失敗したら停止）
- `TEST_CASE("名前")` - テストケース定義
- `SUBCASE("名前")` - サブケース定義

## 要件

- C++17コンパイラ（clang++推奨）
- Make

## テスト結果の例

```
Running all tests with doctest...

========================================
Running bin/test_fixed_string...
========================================
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:  8 |  8 passed | 0 failed | 0 skipped
[doctest] assertions: 45 | 45 passed | 0 failed |

========================================
All tests passed successfully!
========================================
```

## テストの追加

新しいテストを追加する場合：

1. `tests/test_*.cpp` という名前でテストファイルを作成
2. doctestのヘッダとコンポーネントをインクルード:
   ```cpp
   #define DOCTEST_CONFIG_NO_EXCEPTIONS
   #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
   #include "doctest.h"

   #include <omusubi/core/your_component.hpp>

   using namespace omusubi;

   TEST_CASE("YourComponent - 基本機能") {
       // テストコード
       CHECK(condition);
   }
   ```
3. ルートの `Makefile` に追加（`BASIC_TESTS` または `CORE_TESTS` 変数）

## ディレクトリ構造

```
tests/
├── doctest.h              # doctestフレームワーク（自動ダウンロード）
├── README.md              # このファイル
├── Makefile               # レガシーMakefile（非推奨）
├── test_span.cpp          # span<T>テスト
├── test_string_view.cpp   # StringViewテスト
├── test_fixed_string.cpp  # FixedString<N>テスト
├── test_fixed_buffer.cpp  # FixedBuffer<N>テスト
├── test_vector3.cpp       # Vector3テスト
├── test_format.cpp        # format()テスト
├── test_format_string.cpp # FormatStringテスト
├── test_auto_capacity.cpp # AutoCapacityテスト
└── core/
    ├── test_result.cpp    # Result<T,E>テスト
    └── test_logger.cpp    # Loggerテスト
```

## 関連ドキュメント

- [テストガイド（詳細）](../docs/testing.md) - テストの書き方、ベストプラクティス
- [エラーハンドリングガイド](../docs/error-handling.md) - Result<T,E>の使い方
- [コード品質ガイド](../docs/code-quality.md) - コーディング規約

## 注意事項

- すべてのテストはプラットフォーム非依存です
- ヘッダーオンリーのコンポーネントのみをテスト対象としています
- 組み込みハードウェアに依存するコンポーネント（M5Stack固有など）はテストに含まれていません

---

**Version:** 2.0.0
**Last Updated:** 2025-11-25
