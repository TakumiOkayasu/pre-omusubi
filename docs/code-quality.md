# コード品質ツール

このドキュメントでは、Omusubiプロジェクトで使用するコード品質ツールの詳細を説明します。

## 概要

Omusubiプロジェクトでは以下のコード品質ツールを使用しています：

1. **clang-format** - 自動コードフォーマッター
2. **clang-tidy** - 静的解析とLinter
3. **clangd** - VS Code言語サーバー（リアルタイム診断）

これらのツールはDev Container環境に統合されており、自動的に動作します。

## 2段階フォーマットシステム

Omusubiでは2段階のフォーマット構成を採用しています：

### 1. 保存時フォーマット（VS Code）
ファイル保存時にclangdが自動的にclang-formatを実行
- 即座にフォーマット適用
- 開発体験の向上

### 2. コミット時フォーマット（pre-commit hook）
ステージされたファイルに対してフォーマット+Lintを実行
- 最終的な品質保証
- エラーがあればコミットをブロック

## clang-format（フォーマッター）

### 機能

clang-formatは定義されたスタイルルールに従ってC++コードを自動的にフォーマットし、コードベース全体の一貫性を保証します。このプロジェクトはC++のみを対象としています（Cコードは対象外）。

### 設定

設定は`.clang-format`ファイルに記述されています。

**主要な設定:**

| 設定 | 値 | 説明 |
|---------|-------|-------------|
| `BasedOnStyle` | LLVM | ベーススタイル |
| `IndentWidth` | 4 | インデントのスペース数 |
| `ColumnLimit` | 180 | 最大行長 |
| `PointerAlignment` | Left | `int* ptr` (not `int *ptr`) |
| `RemoveBracesLLVM` | false | 中括弧の省略を禁止 |
| `AlwaysBreakTemplateDeclarations` | Yes | `template<>`の後ろは改行 |
| `SpaceBeforeCpp11BracedList` | true | 初期化子の`{}`の前にスペース |
| `InsertNewlineAtEOF` | true | ファイル末尾に改行を挿入 |

**手動ルール（clang-formatでは自動化できない）:**
- if/for/while文の開き中括弧の前の行には空行を入れる
- 閉じ中括弧の後ろが閉じ中括弧でなければ空行を入れる
- return文の前に空行を入れる（ただしifブロックなどの中では無視）

### 使用方法

**自動フォーマット（推奨）:**
- VS Codeでファイル保存時に自動実行
- `.vscode/settings.json`で設定済み

**手動フォーマット:**
```bash
# スクリプトを使用（推奨）
./scripts/format.sh

# 単一ファイルをフォーマット
clang-format -i src/main.cpp

# すべてのC++ファイルをフォーマット
find include src -name "*.h" -o -name "*.hpp" -o -name "*.cpp" | xargs clang-format -i

# フォーマットチェック（ファイルを変更しない）
./scripts/check-format.sh
```

**VS Codeショートカット:**
- `Shift + Alt + F` (Linux/Windows)
- `Shift + Option + F` (Mac)

### インクルード順序

フォーマッターは`#include`ディレクティブを以下の順序で自動整理します：

1. プロジェクトヘッダー (`"omusubi/..."`)
2. システムヘッダー (`<*.h>`)
3. C++標準ライブラリ (`<cstdint>`, `<string>`, etc.)
4. その他のヘッダー

例:
```cpp
#include "omusubi/core/string_view.h"
#include "omusubi/system_context.h"

#include <cstdint>
#include <cstddef>
```

**注意:** C++ヘッダー (`<cstdint>`, `<cstdio>`) を優先し、Cヘッダー (`<stdint.h>`, `<stdio.h>`) は避けてください。

## clang-tidy（リンター）

### 機能

clang-tidyはコードの静的解析を行い、以下を検出します：
- 潜在的なバグ
- パフォーマンスの問題
- コーディング標準違反
- モダンC++イディオムの推奨
- 命名規則のチェック

### 設定

設定は`.clang-tidy`ファイルに記述されています。

**有効なチェックカテゴリ:**

| カテゴリ | 目的 | 例 |
|----------|---------|----------------|
| `bugprone-*` | バグ検出 | Use-after-move, 無限ループ |
| `cert-*` | セキュリティ | CERT安全コーディング標準 |
| `cppcoreguidelines-*` | ベストプラクティス | C++ Core Guidelines準拠 |
| `performance-*` | 最適化 | 不要なコピー、ムーブセマンティクス |
| `readability-*` | 可読性 | 命名、複雑度、マジックナンバー |
| `modernize-*` | モダンC++イディオム | `nullptr`, `auto`, `if constexpr`, nested namespacesなど |

### 命名規則

clang-tidyはOmusubiの命名規則を強制します：

```cpp
// クラスと構造体: PascalCase
class SystemContext { };
struct Vector3 { };

// 関数と変数: snake_case
void initialize_device();
int sensor_value = 0;

// 定数とEnum値: UPPER_CASE
constexpr int MAX_BUFFER_SIZE = 256;
enum class PowerState : uint8_t {
    ACTIVE,
    SLEEP
};

// プライベートメンバー: snake_case_ (末尾アンダースコア)
class Device {
private:
    int port_;
    bool enabled_;
};

// 名前空間: snake_case
namespace omusubi {
namespace platform {
namespace m5stack {
}
}
}
```

### 使用方法

**自動Lint（推奨）:**
- clangd言語サーバー経由で自動実行
- VS Codeで波線として警告が表示される
- 警告をホバーして詳細を確認

**手動Lint:**
```bash
# スクリプトを使用（推奨）
./scripts/lint.sh

# 単一ファイルをLint
clang-tidy src/main.cpp -- -Iinclude -std=c++17

# すべてのソースファイルをLint
find src -name "*.cpp" -exec clang-tidy {} -- -Iinclude -std=c++17 \;

# 自動修正（注意して使用）
clang-tidy -fix src/main.cpp -- -Iinclude -std=c++17
```

### 警告の抑制

特定の警告を抑制する必要がある場合は、以下のアノテーションを使用します（慎重に使用）：

```cpp
// 次の行のみ警告を抑制
// NOLINTNEXTLINE(check-name)
void* ptr = reinterpret_cast<void*>(address);

// 1行のみ警告を抑制
int result = legacy_function(); // NOLINT

// 特定のチェックを抑制
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
auto* hardware = reinterpret_cast<HardwareRegister*>(0x40000000);
```

**警告を抑制すべき場合:**
- プラットフォーム固有のハードウェアアクセス（メモリマップドレジスタ等）
- Cライブラリとのインタラクション
- パフォーマンスクリティカルなコードでチェックが適用不可能
- 誤検知のエッジケース

**常に理由をコメントで説明:**
```cpp
// ハードウェアレジスタアクセスにはreinterpret_castが必要
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
auto* reg = reinterpret_cast<volatile uint32_t*>(0x40000000);
```

## 組み込み向けチェック

組み込み開発向けの特別な考慮事項があります。

### 組み込み向けに無効化されたチェック

組み込み制約と衝突するため、一部のチェックを無効化しています：

| チェック | 無効化理由 |
|-------|--------------|
| `cppcoreguidelines-owning-memory` | ヒープではなくスタック割り当てを使用 |
| `cppcoreguidelines-pro-bounds-pointer-arithmetic` | 低レベルハードウェアアクセスに必要 |
| `cppcoreguidelines-avoid-magic-numbers` | ハードウェアレジスタはマジックナンバーを使用 |
| `modernize-avoid-c-arrays` | 組み込みではC配列が許容される |

### カスタム閾値

| 設定 | 値 | 理由 |
|---------|-------|--------|
| 関数行数 | 100 | 組み込み関数は長くなる傾向 |
| 関数ステートメント数 | 50 | ハードウェア初期化は複雑 |

## スクリプト

### format.sh

すべてのC++ファイルを自動的にフォーマットします：

```bash
./scripts/format.sh
```

### lint.sh

コードの問題をチェックします：

```bash
./scripts/lint.sh
```

エラーがある場合は終了コード1で終了します。

### check-format.sh

フォーマットが正しいかチェックします（修正はしません）：

```bash
./scripts/check-format.sh
```

CI/CD用のスクリプトです。

## Gitコミットワークフロー

### Pre-commit Hook

コミット時に自動的に以下が実行されます：

1. ステージされたC++ファイルを自動フォーマット（clang-format）
2. Lintチェック（clang-tidy）
3. エラーがあればコミットをブロック

**フロー:**
```
ファイル編集 → 保存（自動フォーマット） → git add → git commit（フォーマット+Lint） → git push（CI/CD）
```

### バイパス方法

緊急時にチェックをスキップする場合：

```bash
git commit --no-verify
```

⚠️ 推奨されません。CI/CDで品質チェックが失敗します。

## CI/CDパイプライン

GitHub Actionsが自動的に以下をチェックします：

- ✅ フォーマットチェック
- ✅ Lintチェック
- ✅ ビルドテスト

プッシュ前に以下を実行することを推奨：

```bash
# フォーマット適用
./scripts/format.sh

# Lintチェック
./scripts/lint.sh

# ステージング
git add .

# コミット（自動チェック実行）
git commit -m "your message"
```

## トラブルシューティング

### clang-formatが動作しない

1. clangd拡張機能がインストールされているか確認：
   - 拡張機能パネルを開く (`Ctrl+Shift+X`)
   - "clangd"を検索
   - `llvm-vs-code-extensions.vscode-clangd`がインストールされているか確認

2. VS Code設定を確認：
   - `Ctrl+,`で設定を開く
   - "format on save"を検索
   - 有効になっているか確認

3. clang-formatがPATHにあるか確認：
   ```bash
   which clang-format
   # 出力: /usr/bin/clang-format
   ```

### clang-tidyが警告を表示しない

1. clangdが動作しているか確認：
   - ステータスバーに"clangd"が表示されているか確認
   - 表示されていない場合、ウィンドウをリロード (`Ctrl+Shift+P` → "Reload Window")

2. clangd出力を確認：
   - `Ctrl+Shift+P` → "Output"
   - ドロップダウンから"Clangd"を選択

### 誤検知

特定のチェックで一貫して誤検知が発生する場合：

1. 問題を文書化
2. `.clang-tidy`の無効化チェックに追加
3. 無効化理由をコメント

例:
```yaml
Checks: >
  ...,
  -bugprone-suspicious-check,  # ハードウェアレジスタで誤検知
  ...
```

## 関連ドキュメント

- [開発ガイド](DEVELOPMENT.md) - コード品質管理の実践ガイド
- [C++17機能ガイド](cpp17_features.md) - C++17機能の活用
- [CLAUDE.md](../CLAUDE.md) - 命名規則とコーディング規約

## 参考資料

- [clang-format documentation](https://clang.llvm.org/docs/ClangFormat.html)
- [clang-tidy documentation](https://clang.llvm.org/extra/clang-tidy/)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [CERT C++ Coding Standard](https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=88046682)

---

**Version:** 1.1.0
**Last Updated:** 2025-11-25
