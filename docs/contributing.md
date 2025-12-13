# Omusubiへの貢献

Omusubiプロジェクトへの貢献を歓迎します。このドキュメントでは、プロジェクトへの貢献方法を説明します。

## 目次

- [開発環境のセットアップ](#開発環境のセットアップ)
- [コーディング規約](#コーディング規約)
- [ブランチ戦略](#ブランチ戦略)
- [コミットメッセージ規約](#コミットメッセージ規約)
- [プルリクエストフロー](#プルリクエストフロー)
- [テスト方針](#テスト方針)
- [ドキュメント更新](#ドキュメント更新)

## 開発環境のセットアップ

### 必須要件

- Docker Desktop（Dev Container用）
- Visual Studio Code
- VS Code拡張機能:
  - Dev Containers (`ms-vscode-remote.remote-containers`)
  - clangd (`llvm-vs-code-extensions.vscode-clangd`)

### セットアップ手順

1. **リポジトリのクローン**

```bash
git clone https://github.com/TakumiOkayasu/omusubi.git
cd omusubi
```

2. **Dev Containerで開く**

```bash
# VS Codeで開く
code .

# Command Palette (Ctrl+Shift+P) から
# "Dev Containers: Reopen in Container" を選択
```

Dev Containerが自動的に以下をセットアップします：
- Ubuntu最新環境
- Clang/Clang++コンパイラ
- clang-format、clang-tidy、clangd
- 日本語ロケール（ja_JP.UTF-8）

3. **動作確認**

```bash
# ツールの確認
clang++ --version
clang-format --version
clang-tidy --version

# フォーマット実行
./scripts/format.sh

# Lintチェック
./scripts/lint.sh
```

### 実機ビルド環境

**注意:** Dev Container環境では実機用コードはビルドできません。実機ビルドには以下を使用してください：

**M5Stack向け:**
- PlatformIO（推奨）
- Arduino IDE

**Arduino向け:**
- Arduino IDE
- PlatformIO

**Raspberry Pi Pico向け:**
- Pico SDK
- PlatformIO

## コーディング規約

### C++標準

- **C++17準拠必須**
- C++17の機能を積極的に使用：
  - Nested namespaces (`namespace A::B {}`)
  - `if constexpr`
  - Structured bindings
  - Fold expressions
  - `inline`変数
  - 拡張`constexpr`
  - ジェネリックラムダ
  - 戻り値型の自動推論（`auto`）
  - バイナリリテラル・桁区切り

### 命名規則

```cpp
// ファイル名: snake_case
string_view.h
fixed_string.hpp
m5stack_serial_context.cpp

// クラス・構造体: PascalCase
class SystemContext {};
struct Vector3 {};

// 関数・変数: snake_case
void initialize_device();
int sensor_value = 0;

// 定数・Enum値: UPPER_CASE
constexpr int MAX_BUFFER_SIZE = 256;
enum class PowerState : uint8_t {
    BATTERY,
    USB,
    EXTERNAL
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

// Component層: ドメイン標準用語
class BLECharacteristic {};  // BLE仕様の用語
class HTTPRequest {};        // Web標準の用語
class File {};               // OS標準の用語
```

### 禁止事項

**1. ヒープ割り当て禁止**

```cpp
// ❌ 禁止
auto* obj = new MyObject();
delete obj;
char* buf = (char*)malloc(256);
free(buf);
std::unique_ptr<Device> device;
std::shared_ptr<Data> data;
std::vector<int> vec;

// ✅ 許可
MyObject obj;                  // スタック割り当て
FixedString<256> str;         // 固定サイズバッファ
static MyObject global_obj;   // 静的領域
```

**2. 例外禁止**

```cpp
// ❌ 禁止
throw std::runtime_error("error");
try { } catch { }

// ✅ 許可
bool connect() {
    if (error) {
        return false;  // エラーを返り値で返す
    }
    return true;
}

std::optional<int> get_value();  // C++17標準を使用
```

**3. RTTI禁止**

```cpp
// ❌ 禁止
typeid(obj)
dynamic_cast<Derived*>(base)

// ✅ 許可
static_cast<Derived*>(base)  // 型が確実な場合のみ
```

**4. マクロ禁止**

```cpp
// ❌ 禁止
#define MAX_SIZE 256
#define SQUARE(x) ((x) * (x))

// ✅ 許可
constexpr uint32_t MAX_SIZE = 256;
constexpr int square(int x) { return x * x; }
```

**5. グローバル可変変数の制限**

```cpp
// ❌ 避けるべき
MyDevice device;  // グローバル可変変数

// ✅ 推奨
constexpr int DEFAULT_TIMEOUT = 5000;  // グローバル定数はOK

// ✅ 推奨（Contextパターン）
SystemContext& ctx = get_system_context();
MyDeviceContext* device = ctx.get_device_context();
```

### 推奨事項

**1. constexprの積極的使用**

```cpp
// コンパイル時計算を優先
constexpr size_t buffer_size = 256;
constexpr auto message = static_string("Hello");
constexpr int calculate(int x) { return x * 2 + 1; }
```

**2. Getterのconst修飾**

```cpp
// すべてのGetterはconst
class Context {
public:
    DeviceContext* get_device_context() const;  // ✅
    int get_value() const;                       // ✅
};
```

**3. インターフェース分離**

```cpp
// ✅ 単一責任のインターフェース
class Readable {
    virtual size_t read(span<uint8_t> buffer) = 0;
};

class Writable {
    virtual size_t write(span<const uint8_t> data) = 0;
};

// ✅ 組み合わせて使用
class SerialContext : public Readable, public Writable {
    // ...
};
```

**4. span<T>による非所有参照**

```cpp
// ❌ コピーが発生
void process(FixedString<256> str);

// ✅ ゼロコピー
void process(span<const char> str);
void process(std::string_view str);
```

**5. 不要なコメントを書かない**

```cpp
// ❌ 自明なコメント（不要）
// シリアルポートを取得
SerialContext* get_serial_context(uint8_t port) const;

// ✅ 必要な場合のみ
// ハードウェアレジスタアクセスにはreinterpret_castが必要
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
auto* reg = reinterpret_cast<volatile uint32_t*>(0x40000000);
```

### フォーマット

**自動フォーマット:**
- ファイル保存時に自動的にclang-formatが実行されます
- VS Codeショートカット: `Shift + Alt + F`

**手動フォーマット:**
```bash
./scripts/format.sh
```

**フォーマットチェック（CI用）:**
```bash
./scripts/check-format.sh
```

**Lintチェック:**
```bash
./scripts/lint.sh
```

### よくある警告と修正方法

```cpp
// 1. [[nodiscard]] の追加
[[nodiscard]] int get_value() const { return value_; }

// 2. 未使用パラメータ
void process(int /*unused_param*/) { }

// 3. override の使用（virtualではなく）
void process() override { }

// 4. const の追加
int get_count() const { return count_; }

// 5. else after return の削除
if (error) {
    return false;
}
return true;  // elseを使わない

// 6. = default の使用
Context() = default;
~Context() = default;
```

**警告を抑制する場合:**
```cpp
// NOLINTNEXTLINE(check-name)
auto* reg = reinterpret_cast<volatile uint32_t*>(0x40000000);
```

## ブランチ戦略

### GitHub Flow

Omusubiでは**GitHub Flow**を採用しています。シンプルで継続的デリバリーに適したモデルです。

```
main ─────●─────●─────●─────●───── (常にリリース可能)
           \   /       \   /
            ●─●         ●─● feature/xxx, fix/xxx
```

**特徴:**
- `main`ブランチは常にリリース可能な状態を維持
- 開発はすべて`main`から分岐したブランチで実施
- PRレビュー・CI通過後に`main`へマージ
- リリースはタグで管理

### ブランチ構成

| ブランチ | 用途 |
|----------|------|
| `main` | 安定版。常にビルド・テストが通る状態を維持 |
| `feature/*` | 新機能開発 (`feature/add-wifi-support`) |
| `fix/*` | バグ修正 (`fix/serial-timeout`) |
| `refactor/*` | リファクタリング (`refactor/simplify-context`) |
| `docs/*` | ドキュメント更新 (`docs/update-architecture`) |
| `test/*` | テスト追加・改善 (`test/add-format-tests`) |
| `chore/*` | ビルド設定、依存関係更新等 (`chore/update-clang-tidy`) |

### ワークフロー

1. **mainから分岐**

```bash
# mainから最新を取得
git checkout main
git pull origin main

# 作業ブランチを作成
git checkout -b feature/add-temperature-sensor
```

2. **開発とコミット**

```bash
# 変更を確認
git status

# ファイルをステージング
git add include/omusubi/device/temperature_sensor_context.h

# コミット（pre-commitフックが自動実行される）
git commit -m "feat: add temperature sensor context"
```

3. **プッシュとPR作成**

```bash
# リモートにプッシュ
git push -u origin feature/add-temperature-sensor

# GitHubでPRを作成
gh pr create --title "feat: Add temperature sensor support" --body "..."
```

4. **マージ後のクリーンアップ**

```bash
# mainに戻る
git checkout main
git pull origin main

# マージ済みブランチを削除
git branch -d feature/add-temperature-sensor
```

### リリース管理

リリースはタグで管理します。`release/*`ブランチは使用しません。

```bash
# バージョンタグを作成
git tag -a v0.1.0 -m "Initial release"
git tag -a v0.2.0 -m "Add logger singleton"

# タグをプッシュ
git push origin v0.2.0
```

### なぜGitHub Flowか

| 比較項目 | Git Flow | GitHub Flow |
|----------|----------|-------------|
| 複雑さ | 高（develop, release, hotfix...） | 低（mainのみ） |
| 向いている規模 | 大規模チーム・複雑なリリース | 小〜中規模・継続的デリバリー |
| Omusubi | ❌ 過剰 | ✅ 適切 |

チームが拡大したり、複数バージョンの並行保守が必要になった場合は、`release/*`ブランチを追加する形で段階的に拡張できます。

## コミットメッセージ規約

### Conventional Commits形式

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Type一覧

- `feat`: 新機能追加
- `fix`: バグ修正
- `refactor`: リファクタリング
- `docs`: ドキュメント更新
- `style`: コードフォーマット（機能変更なし）
- `test`: テスト追加・修正
- `chore`: ビルド設定、依存関係更新等
- `perf`: パフォーマンス改善
- `ci`: CI/CD設定変更

### Scope（オプション）

- `core`: コア機能（std::string_view, span等）
- `interface`: インターフェース層
- `context`: Context層
- `device`: Device層
- `platform`: プラットフォーム実装
- `m5stack`: M5Stack固有
- `arduino`: Arduino固有

### 例

```bash
# 新機能
git commit -m "feat(device): add temperature sensor context"

# バグ修正
git commit -m "fix(m5stack): resolve serial timeout issue"

# ドキュメント
git commit -m "docs: update architecture documentation"

# リファクタリング
git commit -m "refactor(context): simplify device ownership pattern"

# フォーマット適用
git commit -m "style: apply clang-format to all files"
```

### コミットメッセージの言語

- **英語必須**
- 簡潔に（50文字以内推奨）
- 命令形を使用（"Add" not "Added"）

## プルリクエストフロー

### PRを作成する前

1. **最新のmainをマージ**

```bash
git checkout main
git pull origin main
git checkout feature/your-branch
git merge main
```

2. **フォーマットとLintを実行**

```bash
./scripts/format.sh
./scripts/lint.sh
```

3. **テストを実行**（該当する場合）

```bash
# 実機でのビルドテスト
pio run

# サンプルコードの動作確認
```

### PR作成

**PRタイトル:**
- コミットメッセージと同じ形式
- 例: `feat(device): add temperature sensor context`

**PR説明テンプレート:**

```markdown
## 概要
何を変更したか、なぜ変更したかを簡潔に説明

## 変更内容
- 追加した機能/修正したバグの詳細
- 影響範囲

## テスト
- [ ] フォーマットチェック通過
- [ ] Lintチェック通過
- [ ] 実機での動作確認（該当する場合）
- [ ] サンプルコード動作確認

## 関連Issue
Closes #123
```

### レビュー基準

**必須チェック項目:**
- [ ] CI/CDがすべて通過
- [ ] コーディング規約に準拠
- [ ] 適切なコミットメッセージ
- [ ] ドキュメント更新（該当する場合）
- [ ] 破壊的変更がない（またはCHANGELOGに記載）

**推奨チェック項目:**
- [ ] テストカバレッジ（該当する場合）
- [ ] サンプルコードの追加（新機能の場合）
- [ ] パフォーマンスへの影響評価

### レビュー後

1. **指摘事項の修正**

```bash
# 修正をコミット
git add .
git commit -m "fix: address review comments"
git push origin feature/your-branch
```

2. **Squash不要**
   - GitHubの"Squash and merge"を使用するため、コミットは整理不要

3. **マージ**
   - レビュアーの承認後、メンテナーがマージ

## テスト方針

### テストフレームワーク

**doctest** を使用します。ヘッダーオンリーで高速なC++テストフレームワークです。

```cpp
#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("Serial - モック動作") {
    MockSerialContext serial;
    uint8_t buffer[256];
    size_t n = serial.read(span<uint8_t>(buffer, 256));
    CHECK_GT(n, 0U);
}
```

### テストの種類

**1. ユニットテスト（doctest使用）**

```bash
# テストをビルド
make tests

# 全テストを実行
make test

# 個別テストを実行
./bin/test_fixed_string
```

**2. プラットフォーム統合テスト（実機）**

実機でのビルドと動作確認：

```bash
# PlatformIOでビルド
pio run

# 実機にアップロード
pio run --target upload

# シリアルモニタで動作確認
pio device monitor
```

**3. サンプルコードによる動作確認**

`examples/platform/`にサンプルコードを追加：

```cpp
// examples/platform/m5stack/temperature_sensor_example.cpp
#include <omusubi/omusubi.h>

using namespace omusubi;

SystemContext& ctx = get_system_context();
TemperatureSensorContext* sensor = nullptr;

void setup() {
    ctx.begin();
    sensor = ctx.get_sensor_context()->get_temperature_sensor_context();
}

void loop() {
    ctx.update();

    float temp = sensor->get_value();
    // 温度表示

    ctx.delay(1000);
}
```

### テスト実行

```bash
# ホスト環境でのテスト（doctest）
make test

# CI環境（自動）
# - フォーマットチェック
# - Lintチェック
# - doctestによるユニットテスト

# 実機環境（手動）
# - ビルドテスト
# - 動作確認
```

## ドキュメント更新

### 更新が必要な場合

以下の変更を行った場合、対応するドキュメントを更新してください：

**新機能追加:**
- `docs/api-reference.md` - API追加
- `docs/architecture.md` - 設計に影響する場合
- `README.md` - 主要機能の場合
- サンプルコード - 使用例

**破壊的変更:**
- `CHANGELOG.md` - 変更内容を記載
- `docs/migration-guide.md` - マイグレーション手順

**プラットフォーム追加:**
- `docs/platform-support.md` - サポート状況
- `docs/architecture.md` - 実装パターン
- サンプルコード

### ドキュメントの配置

```
docs/
├── contributing.md          # 貢献ガイド・開発環境・コード品質（本ファイル）
├── architecture.md          # 設計思想とアーキテクチャ
├── api-reference.md         # APIリファレンス
├── testing.md               # テスト戦略
├── error-handling.md        # エラーハンドリング
├── performance.md           # パフォーマンス最適化
├── platform-support.md      # プラットフォームサポート
├── hardware-abstraction.md  # ハードウェア抽象化ガイドライン（PaHub等）
└── migration-guide.md       # マイグレーションガイド

README.md                    # プロジェクト概要
CHANGELOG.md                 # 変更履歴
```

## 質問とサポート

- **バグ報告:** [GitHub Issues](https://github.com/TakumiOkayasu/omusubi/issues)
- **機能リクエスト:** [GitHub Issues](https://github.com/TakumiOkayasu/omusubi/issues)
- **質問・議論:** [GitHub Discussions](https://github.com/TakumiOkayasu/omusubi/discussions)

## ライセンス

プロジェクトへの貢献は、プロジェクトのライセンスに従います。

---

**Version:** 2.1.0
**Last Updated:** 2025-12-13
