# Omusubi Framework

**⚠️ アルファ版 - 現在開発中**

このプロジェクトはアルファ版です。APIは予告なく変更される可能性があります。
本番環境での使用は推奨されません。

---

マイコン向けの軽量で型安全なC++14フレームワーク

## 概要

Omusubiは、組み込みデバイス向けに設計されたC++14フレームワークです。
SystemContextを中心としたクリーンなアーキテクチャにより、ハードウェアへの統一的なアクセスを提供します。

## 特徴

- ゼロオーバーヘッド抽象化（動的メモリ確保なし、例外なし）
- C++14標準準拠
- インターフェース/実装の完全分離
- SystemContextによる統一的なハードウェアアクセス
- プラットフォーム独立型アーキテクチャ

## ディレクトリ構造

### コアフレームワーク（このリポジトリ）

```
omusubi/                      # プラットフォーム非依存のコアライブラリ
├── include/omusubi/          # ヘッダーファイル（プラットフォーム非依存）
│   ├── core/                 # コアユーティリティ（Optional, Result, Logger等）
│   ├── interface/            # インターフェース定義（*able）
│   ├── context/              # Context層（DI container）
│   ├── device/               # デバイス層（具象デバイスContext）
│   ├── output/               # 出力実装（SerialLogOutput等）
│   ├── omusubi.h            # 統合ヘッダー
│   └── system_context.h     # SystemContext定義
├── examples/                 # サンプルコード（コア機能デモ）
├── test/                     # ユニットテスト
├── docs/                     # ドキュメント
├── scripts/                  # ビルド/品質管理スクリプト
└── bin/                      # ビルド成果物
```

### プラットフォーム固有実装（別リポジトリ）

プラットフォーム固有実装は、**コアライブラリに依存する形**で別リポジトリとして管理されます:

```
workspace/                    # 開発環境のルート
├── omusubi/                  # ← コアライブラリ（このリポジトリ）
│   ├── include/omusubi/
│   ├── examples/
│   └── ...
│
├── omusubi-m5stack/          # ← M5Stack向け実装（別リポジトリ）
│   ├── include/
│   │   └── omusubi/
│   │       └── platform/
│   │           └── m5stack/
│   │               ├── m5stack_system_context.h
│   │               ├── m5stack_connectable_context.h
│   │               ├── m5stack_sensor_context.h
│   │               └── ...
│   ├── src/
│   │   └── platform/
│   │       └── m5stack/
│   │           ├── m5stack_system_context.cpp  # get_system_context()実装
│   │           ├── m5stack_serial_context.cpp
│   │           └── ...
│   ├── examples/
│   │   ├── serial_hello.cpp
│   │   ├── ble_peripheral.cpp
│   │   └── ...
│   └── platformio.ini        # omusubiへのパスを../omusubi/includeで参照
│
└── my-m5stack-project/       # ユーザープロジェクト
    ├── src/
    │   └── main.cpp
    └── platformio.ini        # ../omusubi/include と ../omusubi-m5stack/include を参照
```

**依存関係:**
```
ユーザープロジェクト
    ↓ depends on
omusubi-m5stack (Platform Layer)
    ↓ depends on
omusubi (Core Framework)
```

**platformio.iniの設定例:**
```ini
[env:m5stack]
lib_extra_dirs =
    ../omusubi/include
    ../omusubi-m5stack/include
```

**他のプラットフォーム例:**
- `omusubi-esp32` - ESP32向け実装
- `omusubi-arduino` - Arduino向け実装
- `omusubi-stm32` - STM32向け実装

### レイヤー構造

1. **Core Layer** (`core/`) - プラットフォーム非依存の基本型
2. **Interface Layer** (`interface/`) - `*able`インターフェース（単一責任）
3. **Context Layer** (`context/`) - カテゴリー別Context（DIコンテナ）
4. **Device Layer** (`device/`) - デバイス固有Context実装

詳細は[アーキテクチャドキュメント](docs/architecture.md)を参照してください。

## 開発環境

Dev Container環境で開発できます。

```bash
# VS Codeで開く
code .

# "Reopen in Container"を選択
```

詳細は[CLAUDE.md](CLAUDE.md)を参照してください。

## クイックスタート

コアライブラリのサンプルコードは`examples/`ディレクトリを参照してください。

## ドキュメント

詳細は以下を参照してください:

- **[CLAUDE.md](CLAUDE.md)** - 開発ガイドとアーキテクチャ詳細
- **[DEVELOPMENT.md](DEVELOPMENT.md)** - 開発環境とワークフロー
- **[docs/](docs/)** - 技術ドキュメント
  - [architecture.md](docs/architecture.md) - アーキテクチャ設計
  - [licenses.md](docs/licenses.md) - ライセンスと依存関係
  - [error-handling.md](docs/error-handling.md) - エラーハンドリング
  - [testing.md](docs/testing.md) - テストガイド
  - [performance.md](docs/performance.md) - パフォーマンス最適化
  - [debug.md](docs/debug.md) - デバッグガイド

## ライセンス

**Omusubiコア部分は外部ライブラリに依存していません。**

- コア機能（Optional, Result, Logger, StringViewなど）は標準C++のみ使用
- ライセンス制約なし、商用利用完全自由
- プラットフォーム実装（M5Stackなど）のみが外部ライブラリに依存

詳細は [docs/licenses.md](docs/licenses.md) を参照してください。

**Omusubiフレームワーク自体のライセンス:** 未定（MIT または Apache 2.0 を検討中）

## 作者

Takumi Okayasu
