# ドキュメンテーションガイド

このドキュメントでは、Omusubiフレームワークのドキュメント作成方針と管理方法を定義します。

## 目次

1. [ドキュメント方針](#ドキュメント方針)
2. [ドキュメント構成](#ドキュメント構成)
3. [コードコメント](#コードコメント)
4. [Markdown 形式](#markdown-形式)
5. [サンプルコード](#サンプルコード)
6. [ドキュメント管理](#ドキュメント管理)

---

## ドキュメント方針

### 1. ドキュメントの重要性

**コードと同等に重要。**

**ドキュメントの目的:**
- ユーザーがフレームワークを理解できる
- 設計判断の理由を記録
- 新しい開発者のオンボーディング
- API の正しい使い方を示す

### 2. ドキュメントの種類

**ユーザー向け:**
- README.md - プロジェクト概要とクイックスタート
- API リファレンス - インターフェース仕様
- チュートリアル - 段階的な学習
- サンプルコード - 実用例

**開発者向け:**
- CLAUDE.md - コーディング規約
- Architecture - アーキテクチャ設計
- 各種ガイドライン - 実装指針
- CHANGELOG - 変更履歴

### 3. 言語

**日本語と英語の使い分け:**

- **日本語:** ドキュメント本文、説明文
- **英語:** コード例、変数名、技術用語

---

## ドキュメント構成

### 1. docs/ ディレクトリ構成

```
docs/
├── error-handling.md         # エラーハンドリング
├── testing.md                 # テスト
├── performance.md             # パフォーマンス
├── debug.md                   # デバッグ
├── security.md                # セキュリティ
├── build-system.md            # ビルドシステム
├── versioning.md              # バージョニング
├── documentation.md           # このファイル
├── hardware-abstraction.md    # ハードウェア抽象化
└── abstraction-principles.md  # 抽象化原則
```

### 2. ドキュメントの構造

**すべてのドキュメントは以下の構造に従う:**

```markdown
# タイトル

このドキュメントでは、[概要]を定義します。

## 目次

1. [セクション1](#セクション1)
2. [セクション2](#セクション2)
...

---

## セクション1

### 1. サブセクション

説明文

```cpp
// コード例
```

---

## 関連ドキュメント

- [関連ドキュメント1](related-doc1.md) - 関連する内容の説明
- [関連ドキュメント2](related-doc2.md) - 関連する内容の説明

---

**Version:** 1.0.0
**Last Updated:** 2025-11-17
```

### 3. バージョン情報

**すべてのドキュメントに必須:**

```markdown
---

**Version:** 1.0.0
**Last Updated:** 2025-11-17
```

**バージョンの更新:**
- Major (X.0.0): 大幅な書き直し、セクション削除
- Minor (0.X.0): 新セクション追加、大幅な内容追加
- Patch (0.0.X): 誤字修正、明確化、クロスリファレンス追加

---

## コードコメント

### 1. コメントの原則

**コードは自己文書化されるべき。コメントは「なぜ」を説明。**

```cpp
// ❌ 悪い例: 何をしているかを説明 (コードを見れば分かる)
// iを0から9まで増やす
for (uint32_t i = 0; i < 10; ++i) {
    process(i);
}

// ✅ 良い例: なぜそうするかを説明
// ハードウェアの初期化には10回のリトライが必要
for (uint32_t i = 0; i < 10; ++i) {
    if (init_hardware()) {
        break;
    }
    delay(100);
}
```

### 2. クラスとインターフェースのコメント

**Doxygen 形式を使用。**

```cpp
/**
 * @brief シリアル通信のインターフェース
 *
 * ByteReadable, ByteWritable, Connectable インターフェースを継承し、
 * シリアル通信の完全な機能を提供します。
 *
 * @note プラットフォーム固有の実装が必要です
 * @see M5StackSerialContext
 */
class SerialContext
    : public ByteReadable
    , public ByteWritable
    , public Connectable {
public:
    // ...
};
```

### 3. 関数のコメント

**パブリックAPIには必須。**

```cpp
/**
 * @brief バッファにデータを追加
 *
 * @param data 追加するデータ
 * @return 追加に成功した場合true、容量不足の場合false
 *
 * @note 容量を超える場合は何も追加せずfalseを返す
 * @warning データの長さが0の場合もfalseを返す
 */
bool append(std::string_view data);
```

### 4. 複雑なロジックのコメント

**アルゴリズムの説明:**

```cpp
// UTF-8 の文字境界を検出
// - 0b0xxxxxxx: 1バイト文字
// - 0b110xxxxx: 2バイト文字の先頭
// - 0b1110xxxx: 3バイト文字の先頭
// - 0b11110xxx: 4バイト文字の先頭
// - 0b10xxxxxx: 継続バイト
bool is_utf8_boundary(uint8_t byte) {
    return (byte & 0b10000000) == 0 ||  // 1バイト文字
           (byte & 0b11000000) == 0b11000000;  // マルチバイト文字の先頭
}
```

### 5. TODO コメント

**未実装や改善点を記録。**

```cpp
// TODO(username): BLE スキャン機能を実装
// FIXME: バッファオーバーフローの可能性あり
// HACK: 一時的な回避策、後で修正が必要
// NOTE: この実装はESP32専用
```

---

## Markdown 形式

### 1. 見出し

```markdown
# h1 - ドキュメントタイトル (1つのみ)

## h2 - 主要セクション

### h3 - サブセクション

#### h4 - 詳細セクション (使用は最小限に)
```

### 2. コードブロック

**言語を明示:**

```markdown
```cpp
// C++ コード
```

```bash
# Bash コマンド
```

```makefile
# Makefile
```
```

### 3. リスト

**順序なしリスト:**

```markdown
- 項目1
- 項目2
  - サブ項目2.1
  - サブ項目2.2
```

**順序ありリスト:**

```markdown
1. ステップ1
2. ステップ2
3. ステップ3
```

### 4. 強調

```markdown
**太字** - 重要な用語、強調
*イタリック* - 軽い強調
`code` - コード、ファイル名、コマンド
```

### 5. リンク

**相対パスを使用:**

```markdown
[エラーハンドリングガイド](error-handling.md)
[テストガイド](testing.md)
```

**外部リンク:**

```markdown
[Semantic Versioning](https://semver.org/)
```

### 6. 表

```markdown
| 項目 | 説明 | 例 |
|------|------|-----|
| MAJOR | 破壊的変更 | 2.0.0 |
| MINOR | 機能追加 | 1.3.0 |
| PATCH | バグ修正 | 1.2.1 |
```

### 7. 引用

```markdown
> **重要:** この機能はバージョン2.0で削除されます。
```

---

## サンプルコード

### 1. サンプルコードの構成

**すべてのサンプルは以下の構造:**

```cpp
// [日本語の説明: 何をするサンプルか]

#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace std::literals;

// グローバル変数: setup() で一度だけ取得
SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void setup() {
    // システム初期化
    ctx.begin();

    // デバイス取得 (一度だけ)
    serial = ctx.get_connectable_context()->get_serial_context(0);

    // [セットアップロジック]
}

void loop() {
    // システム更新
    ctx.update();

    // [ループロジック]

    ctx.delay(10);
}
```

### 2. サンプルコードの原則

**完全で実行可能:**

```cpp
// ❌ 悪い例: 不完全なコード
serial->write("Hello");  // setup() がない

// ✅ 良い例: 完全なコード
void setup() {
    ctx.begin();
    serial = ctx.get_connectable_context()->get_serial_context(0);
}

void loop() {
    serial->write("Hello"sv);
    ctx.delay(1000);
}
```

**コメントで説明:**

```cpp
void setup() {
    ctx.begin();

    // Serial0 を取得 (USB Serial)
    serial = ctx.get_connectable_context()->get_serial_context(0);

    // WiFi に接続
    auto* wifi = ctx.get_connectable_context()->get_wifi_context();
    wifi->connect_to("SSID"sv, "password"sv);
}
```

### 3. サンプルコードの配置

```
examples/
├── platform/
│   ├── m5stack/
│   │   ├── serial_echo.cpp
│   │   ├── wifi_scan.cpp
│   │   └── ble_advertise.cpp
│   ├── arduino/
│   │   └── serial_echo.cpp
│   └── pico/
│       └── serial_echo.cpp
└── README.md
```

---

## ドキュメント管理

### 1. ドキュメントの更新

**関連するコード変更時に必ず更新:**

```bash
# コード変更
git add include/omusubi/device/serial_context.h

# 関連ドキュメントも更新
git add docs/api-reference.md

git commit -m "feat(serial): add read_line() method

- SerialContext に read_line() メソッド追加
- ドキュメント更新"
```

### 2. クロスリファレンス

**関連ドキュメントにリンクを追加:**

```markdown
## 関連ドキュメント

- [エラーハンドリングガイド](error-handling.md) - エラー処理パターン
- [テストガイド](testing.md) - テストの書き方
- CLAUDE.md - コーディング規約
```

### 3. ドキュメントのレビュー

**プルリクエストでレビュー:**

```markdown
# PR Description

## Changes
- WiFi スキャン機能を追加

## Documentation
- [x] API リファレンス更新
- [x] サンプルコード追加
- [x] README 更新
```

### 4. ドキュメントのバージョン管理

**ドキュメント末尾にバージョン情報:**

```markdown
---

**Version:** 1.0.0
**Last Updated:** 2025-11-17
```

**更新時にバージョンを更新:**

```markdown
# Before
**Version:** 1.0.0
**Last Updated:** 2025-11-17

# After (新セクション追加)
**Version:** 1.1.0
**Last Updated:** 2025-11-20
```

---

## ドキュメンテーションチェックリスト

新規ドキュメント作成時:

- [ ] タイトルが明確
- [ ] 目次を含む
- [ ] セクションが論理的に構成されている
- [ ] コード例が完全で実行可能
- [ ] コメントが日本語
- [ ] 関連ドキュメントへのリンクがある
- [ ] バージョン情報を含む
- [ ] 誤字脱字がない
- [ ] Markdown フォーマットが正しい

既存ドキュメント更新時:

- [ ] バージョン番号を更新した
- [ ] 更新日を更新した
- [ ] 関連する他のドキュメントも更新した
- [ ] 矛盾する記述を修正した
- [ ] 古い情報を削除または更新した

---

## 関連ドキュメント

- [バージョニングガイド](versioning.md) - ドキュメントのバージョン管理
- CLAUDE.md - ドキュメント管理方針

---

**Version:** 1.0.0
**Last Updated:** 2025-11-17
