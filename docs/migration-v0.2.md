# v0.2移行ガイド - プラットフォーム構造の変更

このドキュメントは、Omusubi v0.1からv0.2へのディレクトリ構造変更に伴う移行ガイドです。

## 変更の概要

v0.2では、プラットフォーム固有のコードをコアライブラリから分離し、`platforms/`ディレクトリに移動しました。

### 変更前（v0.1）

```
omusubi/
├── include/omusubi/
│   ├── core/
│   ├── interface/
│   ├── context/
│   ├── device/
│   └── platform/
│       └── m5stack/           # M5Stack固有ヘッダー
├── src/
│   └── platform/
│       └── m5stack/           # M5Stack実装
└── examples/
    └── platform/
        └── m5stack/           # M5Stackサンプル
```

### 変更後（v0.2）

```
omusubi/
├── include/omusubi/           # コアライブラリのみ
│   ├── core/
│   ├── interface/
│   ├── context/
│   └── device/
├── platforms/                 # プラットフォーム実装を集約
│   └── m5stack/
│       ├── include/omusubi/platform/m5stack/
│       ├── src/
│       ├── examples/
│       ├── library.json
│       ├── library.properties
│       ├── platformio.ini
│       └── README.md
└── examples/
    └── core/                  # コアのみのサンプル
```

## 影響範囲

### ✅ 影響なし（変更不要）

1. **ユーザーコードのインクルード文**
   ```cpp
   // 変更前も変更後も同じ
   #include <omusubi/omusubi.h>
   #include <omusubi/platform/m5stack/m5stack_system_context.hpp>
   ```

2. **API呼び出し**
   ```cpp
   // 変更前も変更後も同じ
   SystemContext& ctx = get_system_context();
   SerialContext* serial = ctx.get_connectable_context()->get_serial_context(0);
   ```

3. **コアライブラリの使用**
   ```cpp
   // 変更前も変更後も同じ
   #include <omusubi/core/optional.hpp>
   #include <omusubi/core/result.hpp>
   ```

### ⚠️ 影響あり（変更必要）

1. **PlatformIO `platformio.ini`**

   **変更前:**
   ```ini
   [env:m5stack]
   lib_deps =
       https://github.com/TakumiOkayasu/omusubi.git
   ```

   **変更後:**
   ```ini
   [env:m5stack]
   lib_deps =
       https://github.com/TakumiOkayasu/omusubi.git#platforms/m5stack
   ```

2. **ビルドフラグ（手動でインクルードパスを指定していた場合）**

   **変更前:**
   ```ini
   build_flags =
       -Iinclude
   ```

   **変更後:**
   ```ini
   build_flags =
       -I../../include          ; コアライブラリ
       -Iinclude                ; プラットフォーム固有
   ```

   ただし、`library.json`を使用している場合は自動的に設定されるため変更不要です。

3. **Arduino IDEライブラリ名**

   **変更前:** `Omusubi`

   **変更後:** `Omusubi-M5Stack`

## 移行手順

### 既存プロジェクトの移行

#### PlatformIOプロジェクト

1. `platformio.ini`を編集:

   ```ini
   [env:m5stack]
   platform = espressif32
   board = m5stack-core-esp32
   framework = arduino

   lib_deps =
       https://github.com/TakumiOkayasu/omusubi.git#platforms/m5stack

   build_flags =
       -std=c++14
   ```

2. 依存関係を更新:

   ```bash
   pio lib update
   ```

3. ビルドして動作確認:

   ```bash
   pio run
   ```

#### Arduino IDEプロジェクト

1. 古いライブラリを削除:
   - ライブラリマネージャーから`Omusubi`をアンインストール

2. 新しいライブラリをインストール:
   - ライブラリマネージャーで`Omusubi-M5Stack`を検索してインストール

3. スケッチを再コンパイル

### ローカル開発の移行

Omusubiリポジトリをローカルで開発している場合:

```bash
cd /path/to/omusubi
git pull origin main

# プラットフォーム固有のビルド
cd platforms/m5stack
pio run
```

## 新しい機能

### プラットフォーム固有のビルド設定

各プラットフォームが独自の`platformio.ini`を持つため、プラットフォーム固有のビルド設定が可能になりました:

```bash
cd platforms/m5stack
pio run -e m5stack-core-esp32      # M5Stack Basic向け
pio run -e m5stack-core2           # M5Stack Core2向け
pio run -e m5stack-debug           # デバッグビルド
```

### プラットフォーム固有のドキュメント

各プラットフォームに専用のREADME.mdが追加されました:

- `platforms/m5stack/README.md`

### 将来のプラットフォーム追加が容易に

新しいプラットフォームは`platforms/`に追加するだけ:

```
platforms/
├── m5stack/
├── arduino/       # 追加予定
├── pico/          # 追加予定
└── stm32/         # 追加予定
```

## トラブルシューティング

### エラー: `platform/m5stack/xxx.hpp: No such file or directory`

**原因:** インクルードパスが正しく設定されていない

**解決策:**

1. PlatformIO: `lib_deps`を`#platforms/m5stack`を含むURLに変更
2. Arduino IDE: `Omusubi-M5Stack`ライブラリを再インストール

### エラー: `get_system_context()` が未定義

**原因:** プラットフォーム実装がリンクされていない

**解決策:**

```ini
; platformio.iniに以下を追加
lib_deps =
    https://github.com/TakumiOkayasu/omusubi.git#platforms/m5stack
```

### 既存のサンプルコードが動かない

**原因:** サンプルコードが移動した

**解決策:**

新しい場所を参照:
- 変更前: `examples/platform/m5stack/hello_world/`
- 変更後: `platforms/m5stack/examples/hello_world/`

## 変更の理由

この変更により、以下のメリットがあります:

1. **コアとプラットフォームの明確な分離**
   - プラットフォーム非依存のコードのみを`include/`に配置
   - プラットフォーム固有のコードは`platforms/`に集約

2. **スケーラビリティの向上**
   - 新しいプラットフォームの追加が容易
   - プラットフォーム間の独立性が向上

3. **ビルドシステムの柔軟性**
   - プラットフォーム固有のビルド設定が可能
   - 各プラットフォームが独立したライブラリとして公開可能

4. **ドキュメントの整理**
   - プラットフォーム固有のドキュメントを分離
   - 関連ファイルが一箇所に集約

## サポート

質問や問題がある場合は、以下のリソースを参照してください:

- [GitHub Issues](https://github.com/TakumiOkayasu/omusubi/issues)
- [プラットフォームサポート](platform-support.md)
- [アーキテクチャドキュメント](architecture.md)

---

**Version:** 1.0.0
**Last Updated:** 2025-11-19
