# バージョニングガイド

このドキュメントでは、Omusubiフレームワークのバージョン管理方針とリリースプロセスを定義します。

## 目次

1. [バージョニング方針](#バージョニング方針)
2. [セマンティックバージョニング](#セマンティックバージョニング)
3. [リリースプロセス](#リリースプロセス)
4. [ブランチ戦略](#ブランチ戦略)
5. [変更ログ](#変更ログ)
6. [後方互換性](#後方互換性)

---

## バージョニング方針

### 1. セマンティックバージョニング採用

**Omusubiは Semantic Versioning 2.0.0 に従う。**

**バージョン形式: `MAJOR.MINOR.PATCH`**

```
1.2.3
│ │ └─ PATCH: バグ修正
│ └─── MINOR: 機能追加 (後方互換性あり)
└───── MAJOR: 破壊的変更 (後方互換性なし)
```

### 2. バージョン番号の意味

**MAJOR (破壊的変更):**
- インターフェースの削除
- インターフェースのシグネチャ変更
- デフォルト動作の変更
- サポート終了

**MINOR (機能追加):**
- 新しいインターフェース追加
- 新しいデバイスサポート
- 新しいプラットフォームサポート
- パフォーマンス改善

**PATCH (バグ修正):**
- バグ修正
- ドキュメント修正
- テスト追加
- 内部リファクタリング

### 3. プレリリースバージョン

**開発版・テスト版の表記:**

```
1.0.0-alpha.1  # アルファ版
1.0.0-beta.1   # ベータ版
1.0.0-rc.1     # リリース候補
```

**プレリリースラベル:**
- `alpha` - 開発初期 (機能未完成)
- `beta` - 機能完成、テスト中
- `rc` - リリース候補 (最終テスト)

---

## セマンティックバージョニング

### 1. MAJOR バージョンアップ

**破壊的変更の例:**

```cpp
// v1.0.0 (旧バージョン)
class SerialContext {
    virtual size_t read(uint8_t* buffer, size_t size) = 0;
};

// v2.0.0 (破壊的変更)
class SerialContext {
    virtual size_t read(span<uint8_t> buffer) = 0;  // シグネチャ変更
};
```

**MAJOR バージョンアップの基準:**
- インターフェースの削除・変更
- クラス名変更
- 必須パラメータの追加
- デフォルト動作の変更

### 2. MINOR バージョンアップ

**後方互換性のある機能追加:**

```cpp
// v1.0.0 (既存)
class WiFiContext : public Connectable {
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
};

// v1.1.0 (機能追加)
class WiFiContext : public Connectable, public Scannable {
    virtual bool connect() = 0;
    virtual void disconnect() = 0;

    // 新機能追加 (既存コードは動作する)
    virtual void start_scan() = 0;
    virtual void stop_scan() = 0;
};
```

**MINOR バージョンアップの基準:**
- 新しいインターフェース追加
- 新しいメソッド追加 (デフォルト実装あり)
- 新しいオプション追加
- パフォーマンス改善

### 3. PATCH バージョンアップ

**バグ修正のみ:**

```cpp
// v1.0.0 (バグあり)
bool append(std::string_view str) {
    if (length_ + str.size() >= capacity_) {  // ❌ >= は間違い
        return false;
    }
    // ...
}

// v1.0.1 (バグ修正)
bool append(std::string_view str) {
    if (length_ + str.size() > capacity_) {  // ✅ > が正しい
        return false;
    }
    // ...
}
```

**PATCH バージョンアップの基準:**
- バグ修正
- タイポ修正
- ドキュメント修正
- テスト追加

---

## リリースプロセス

### 1. リリースフロー

```
開発 (develop ブランチ)
  ↓
機能完成
  ↓
バージョン番号決定
  ↓
リリースブランチ作成 (release/vX.Y.Z)
  ↓
テスト・バグ修正
  ↓
リリースタグ作成 (vX.Y.Z)
  ↓
main ブランチにマージ
  ↓
GitHub Release 作成
  ↓
develop ブランチにマージバック
```

### 2. リリース手順

**Step 1: バージョン番号決定**

```bash
# 破壊的変更あり → MAJOR
# 機能追加あり   → MINOR
# バグ修正のみ   → PATCH

# 例: v1.2.3 → v1.3.0 (機能追加)
```

**Step 2: リリースブランチ作成**

```bash
git checkout develop
git pull origin develop
git checkout -b release/v1.3.0
```

**Step 3: バージョン更新**

```cpp
// include/omusubi/version.h
#define OMUSUBI_VERSION_MAJOR 1
#define OMUSUBI_VERSION_MINOR 3
#define OMUSUBI_VERSION_PATCH 0

#define OMUSUBI_VERSION "1.3.0"
```

**Step 4: CHANGELOG 更新**

```markdown
# CHANGELOG.md

## [1.3.0] - 2025-11-17

### Added
- WiFi スキャン機能追加
- BLE サポート追加

### Changed
- シリアル通信のパフォーマンス改善

### Fixed
- FixedString のバッファオーバーフロー修正
```

**Step 5: テスト実行**

```bash
# すべてのテストを実行
make test

# PlatformIO でビルド確認
pio run -e m5stack
pio run -e arduino
```

**Step 6: コミット**

```bash
git add .
git commit -m "chore: bump version to 1.3.0"
git push origin release/v1.3.0
```

**Step 7: Pull Request 作成**

```
Title: Release v1.3.0

Description:
- WiFi スキャン機能追加
- BLE サポート追加
- バグ修正

Checklist:
- [x] テストが通る
- [x] ドキュメント更新済み
- [x] CHANGELOG 更新済み
```

**Step 8: main ブランチにマージ**

```bash
# レビュー後にマージ
git checkout main
git merge --no-ff release/v1.3.0
git push origin main
```

**Step 9: タグ作成**

```bash
git tag -a v1.3.0 -m "Release version 1.3.0"
git push origin v1.3.0
```

**Step 10: GitHub Release 作成**

```markdown
# Release v1.3.0

## 新機能

- WiFi スキャン機能を追加
- BLE デバイスのサポートを追加

## 変更点

- シリアル通信のパフォーマンスを改善

## バグ修正

- FixedString のバッファオーバーフロー問題を修正

## アップグレード方法

```cpp
// v1.2.x から v1.3.0 へのアップグレード
// 破壊的変更はありません
// 既存コードはそのまま動作します
```

**Step 11: develop にマージバック**

```bash
git checkout develop
git merge --no-ff v1.3.0
git push origin develop
```

---

## ブランチ戦略

### 1. ブランチ構成

**主要ブランチ:**
- `main` - 安定版 (リリース済み)
- `develop` - 開発版 (次期リリース)

**サポートブランチ:**
- `feature/*` - 機能開発
- `release/*` - リリース準備
- `hotfix/*` - 緊急修正

### 2. ブランチの使い分け

**feature ブランチ:**

```bash
# develop から作成
git checkout develop
git checkout -b feature/add-ble-support

# 開発
git commit -m "feat: add BLE context interface"

# develop にマージ
git checkout develop
git merge --no-ff feature/add-ble-support
git push origin develop
```

**release ブランチ:**

```bash
# develop から作成
git checkout develop
git checkout -b release/v1.3.0

# バージョン更新・テスト
git commit -m "chore: bump version to 1.3.0"

# main にマージ
git checkout main
git merge --no-ff release/v1.3.0
git tag v1.3.0

# develop にマージバック
git checkout develop
git merge --no-ff release/v1.3.0
```

**hotfix ブランチ:**

```bash
# main から作成
git checkout main
git checkout -b hotfix/v1.2.1

# 緊急修正
git commit -m "fix: critical buffer overflow"

# バージョン更新 (PATCH)
git commit -m "chore: bump version to 1.2.1"

# main にマージ
git checkout main
git merge --no-ff hotfix/v1.2.1
git tag v1.2.1

# develop にもマージ
git checkout develop
git merge --no-ff hotfix/v1.2.1
```

---

## 変更ログ

### 1. CHANGELOG.md 形式

```markdown
# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- 開発中の新機能

### Changed
- 開発中の変更

### Deprecated
- 将来削除予定の機能

### Removed
- 削除された機能

### Fixed
- 修正されたバグ

### Security
- セキュリティ修正

## [1.3.0] - 2025-11-17

### Added
- WiFi スキャン機能追加
- BLE サポート追加

### Changed
- シリアル通信のパフォーマンス改善

### Fixed
- FixedString のバッファオーバーフロー修正

## [1.2.0] - 2025-10-15

### Added
- M5Stack サポート追加
- 加速度センサーサポート追加

## [1.1.0] - 2025-09-01

### Added
- std::string_view UTF-8ヘルパー関数
- FixedString 実装

## [1.0.0] - 2025-08-01

### Added
- 初回リリース
- span<T> 実装
- SystemContext 実装
```

### 2. コミットメッセージ規約

**Conventional Commits に従う:**

```
<type>(<scope>): <subject>

<body>

<footer>
```

**type:**
- `feat`: 新機能
- `fix`: バグ修正
- `docs`: ドキュメント
- `style`: コードスタイル
- `refactor`: リファクタリング
- `perf`: パフォーマンス改善
- `test`: テスト追加
- `chore`: ビルド・ツール関連

**例:**

```bash
git commit -m "feat(ble): add BLE scanning support"
git commit -m "fix(serial): fix buffer overflow in read()"
git commit -m "docs(readme): update installation instructions"
git commit -m "perf(string): optimize UTF-8 character counting"
```

---

## 後方互換性

### 1. 互換性保証

**MINOR/PATCH バージョンアップは後方互換性を保証:**

```cpp
// v1.0.0 のコード
SerialContext* serial = ctx.get_serial_context(0);
serial->write("Hello"sv);

// v1.1.0, v1.2.0, v1.0.1 でも同じコードが動作
```

### 2. 非推奨化 (Deprecation)

**機能削除は2段階:**

**Step 1: 非推奨化 (MINOR バージョン)**

```cpp
// v1.3.0 で非推奨化
class OldAPI {
    [[deprecated("Use NewAPI instead")]]
    void old_method();
};
```

**Step 2: 削除 (MAJOR バージョン)**

```cpp
// v2.0.0 で削除
// OldAPI は存在しない
```

### 3. 移行ガイド

**MAJOR バージョンアップ時は移行ガイドを提供:**

```markdown
# Migration Guide: v1.x → v2.0

## Breaking Changes

### 1. SerialContext::read() signature changed

**Before (v1.x):**
```cpp
size_t read(uint8_t* buffer, size_t size);
```

**After (v2.0):**
```cpp
size_t read(span<uint8_t> buffer);
```

**Migration:**
```cpp
// v1.x
uint8_t buffer[256];
size_t n = serial->read(buffer, 256);

// v2.0
uint8_t buffer[256];
size_t n = serial->read(span<uint8_t>(buffer, 256));
```
```

---

## バージョニングチェックリスト

リリース前のチェックリスト:

- [ ] バージョン番号が適切 (MAJOR/MINOR/PATCH)
- [ ] version.h を更新した
- [ ] CHANGELOG.md を更新した
- [ ] すべてのテストが通る
- [ ] ドキュメントを更新した
- [ ] 破壊的変更の場合、移行ガイドを作成した
- [ ] GitHub Release を作成した
- [ ] develop ブランチにマージバックした

---

## 関連ドキュメント

- [ドキュメンテーションガイド](documentation.md) - ドキュメントのバージョン管理
- CLAUDE.md - コーディング規約

---

**Version:** 1.0.0
**Last Updated:** 2025-11-17
