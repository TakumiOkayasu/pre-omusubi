# 移行ガイド

このドキュメントでは、Omusubiの異なるバージョン間での移行方法を説明します。

## 目次

- [v0.1.x → v0.2.x（予定）](#v01x--v02x予定)
- [Pre-release → v0.1.x](#pre-release--v01x)

## v0.1.x → v0.2.x（予定）

**リリース予定:** 未定

### 破壊的変更

現時点で予定されている破壊的変更はありません。

### 非推奨機能

現時点で非推奨になる予定の機能はありません。

## Pre-release → v0.1.x

**リリース日:** 2025-01-XX

### 破壊的変更

#### 1. Interface階層の再設計

**変更内容:**
- `Readable`が`ByteReadable`と`TextReadable`に分離
- `Writable`が`ByteWritable`と`TextWritable`に分離

**影響を受けるコード:**

```cpp
// ❌ 旧API（動作しない）
class MyDevice : public Readable, public Writable {
    // ...
};

// ✅ 新API
class MyDevice : public TextReadable, public TextWritable {
    // ByteReadableとByteWritableは自動的に継承される
};
```

**移行手順:**

1. `Readable`を継承しているクラスを確認
2. バイナリデータのみを扱う場合は`ByteReadable`に変更
3. テキストデータを扱う場合は`TextReadable`に変更（推奨）
4. `Writable`についても同様に変更

**自動移行スクリプト:**

```bash
# 一括置換（確認してから実行）
find . -name "*.h" -o -name "*.hpp" -o -name "*.cpp" | xargs sed -i 's/: public Readable,/: public TextReadable,/g'
find . -name "*.h" -o -name "*.hpp" -o -name "*.cpp" | xargs sed -i 's/: public Writable/: public TextWritable/g'
```

#### 2. Scannableインターフェースの変更

**変更内容:**
- `scan()` → `start_scan()` / `stop_scan()`に変更
- スキャン結果取得メソッドが追加

**影響を受けるコード:**

```cpp
// ❌ 旧API（動作しない）
wifi->scan();
uint8_t count = wifi->get_scan_count();

// ✅ 新API
wifi->start_scan();
ctx.delay(3000);  // スキャン待機
wifi->stop_scan();
uint8_t count = wifi->get_found_count();
```

**移行手順:**

1. `scan()`呼び出しを`start_scan()`に変更
2. スキャン完了を待つため`ctx.delay()`を追加
3. `stop_scan()`を呼び出し
4. `get_scan_count()`を`get_found_count()`に変更
5. `get_scan_result(index)`を以下に置き換え：
   - `get_found_name(index)` - デバイス名/SSID
   - `get_found_signal_strength(index)` - 信号強度

**自動移行スクリプト:**

```bash
# 手動での移行を推奨（ロジック変更が必要）
# 以下は参考例
find . -name "*.cpp" | xargs sed -i 's/->scan()/->start_scan()/g'
find . -name "*.cpp" | xargs sed -i 's/->get_scan_count()/->get_found_count()/g'
```

#### 3. Connectableインターフェースの拡張

**変更内容:**
- `disconnect()`メソッド追加
- `is_connected()`メソッド追加

**影響を受けるコード:**

この変更は基本的に後方互換性があります。既存のコードは動作し続けますが、新しいメソッドを活用することを推奨します。

```cpp
// ✅ 既存コード（動作する）
wifi->connect();

// ✅ 推奨コード（新メソッドを活用）
if (!wifi->is_connected()) {
    wifi->connect();
}

// 使用後
wifi->disconnect();
```

**移行手順:**

必須ではありませんが、以下を推奨：
1. 接続前に`is_connected()`でチェック
2. 使用終了後に`disconnect()`を呼び出し

### 新機能

#### 1. Header依存関係の修正

すべてのContextヘッダーに適切な`#include <cstdint>`が追加されました。これによりコンパイルエラーが解消されます。

#### 2. ドキュメントの整備

包括的なドキュメントが追加されました：
- Architecture design document
- Contributing guide
- Platform support documentation
- API reference
- Testing guide

#### 3. CI/CDパイプライン

GitHub Actionsによる自動テストが追加されました：
- コードフォーマットチェック
- Lintチェック

### 非推奨機能

現時点で非推奨になった機能はありません。

### 移行チェックリスト

以下のチェックリストを使用して、移行が完了したか確認してください：

- [ ] `Readable` → `ByteReadable` / `TextReadable` への変更
- [ ] `Writable` → `ByteWritable` / `TextWritable` への変更
- [ ] `scan()` → `start_scan()` / `stop_scan()` への変更
- [ ] `get_scan_count()` → `get_found_count()` への変更
- [ ] `get_scan_result()` → `get_found_name()` / `get_found_signal_strength()` への変更
- [ ] コンパイルが通ることを確認
- [ ] Lintチェックが通ることを確認（`./scripts/lint.sh`）
- [ ] 実機での動作確認
- [ ] すべてのサンプルコードが動作することを確認

### トラブルシューティング

#### コンパイルエラー: 'Readable' is an ambiguous base

**原因:** `Readable`と`TextReadable`を両方継承している

**解決策:**

```cpp
// ❌ エラー
class MyDevice : public Readable, public TextReadable {
};

// ✅ 修正
class MyDevice : public TextReadable {
    // ByteReadableは自動的に継承される
};
```

#### コンパイルエラー: 'scan' is not a member

**原因:** `scan()`メソッドが削除された

**解決策:**

```cpp
// ❌ エラー
wifi->scan();

// ✅ 修正
wifi->start_scan();
ctx.delay(3000);
wifi->stop_scan();
```

#### Lintエラー: 'uint8_t' is not defined

**原因:** 古いヘッダーファイルを使用している

**解決策:**

最新のヘッダーファイルに更新してください：

```bash
git pull origin main
```

または、手動で`#include <cstdint>`を追加：

```cpp
#pragma once

#include <cstdint>  // 追加

namespace omusubi {
// ...
}
```

## 今後の予定

### 計画中の破壊的変更

現時点で計画中の破壊的変更はありません。

### 非推奨化予定

現時点で非推奨化が予定されている機能はありません。

## バージョン互換性マトリクス

| Omusubiバージョン | 対応プラットフォーム | C++標準 | 推奨コンパイラ |
|------------------|---------------------|---------|---------------|
| 0.1.x | M5Stack | C++17 | Clang 10+, GCC 7+ |
| 0.2.x (予定) | M5Stack, Arduino | C++17 | Clang 10+, GCC 7+ |
| 1.0.x (予定) | M5Stack, Arduino, Pico | C++17 | Clang 11+, GCC 8+ |

## サポートポリシー

### セキュリティ更新

- **最新バージョン:** すべてのセキュリティ更新を提供
- **1つ前のメジャーバージョン:** 重大なセキュリティ更新のみ提供
- **それ以前:** サポート終了

### バグ修正

- **最新バージョン:** すべてのバグ修正を提供
- **1つ前のメジャーバージョン:** 重大なバグ修正のみ提供
- **それ以前:** サポート終了

## 移行サポート

移行で問題が発生した場合：

1. **ドキュメントを確認:** [API Reference](api-reference.md), [Architecture](architecture.md)
2. **既知の問題を確認:** [GitHub Issues](https://github.com/TakumiOkayasu/omusubi/issues)
3. **質問する:** [GitHub Discussions](https://github.com/TakumiOkayasu/omusubi/discussions)
4. **バグ報告:** [GitHub Issues](https://github.com/TakumiOkayasu/omusubi/issues/new)

## フィードバック

このマイグレーションガイドの改善提案は、[GitHub Issues](https://github.com/TakumiOkayasu/omusubi/issues)または[Discussions](https://github.com/TakumiOkayasu/omusubi/discussions)でお待ちしています。

---

**Version:** 1.0.1
**Last Updated:** 2025-11-16
