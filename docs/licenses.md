# ライセンスと依存関係

このドキュメントでは、Omusubiフレームワークのライセンスと外部依存関係について説明します。

## 目次

1. [概要](#概要)
2. [Omusubiコアのライセンス](#omusubiコアのライセンス)
3. [外部依存関係](#外部依存関係)
4. [プラットフォーム固有の依存](#プラットフォーム固有の依存)
5. [商用利用について](#商用利用について)
6. [帰属表示](#帰属表示)

---

## 概要

**Omusubiフレームワークのコア部分は外部ライブラリに依存していません。**

- 標準C++のみ使用（C++17）
- すべてのコア機能は自作実装
- ライセンス制約なし
- 商用利用完全自由

プラットフォーム固有の実装（M5Stackなど）のみが外部ライブラリに依存します。

---

## Omusubiコアのライセンス

### 外部依存なしのコンポーネント

以下のコンポーネントは**標準C++のみ**に依存し、外部ライブラリを使用していません：

**コア機能:**
- `Optional<T>` - std::optional互換実装
- `Result<T, E>` - Rust風Result型
- `Logger` - 軽量ログシステム
- `std::string_view` - 文字列参照（UTF-8対応）
- `FixedString<N>` - 固定長文字列
- `FixedBuffer<N>` - 固定長バッファ
- `span<T>` - メモリ範囲
- `StaticString<N>` - コンパイル時文字列

**使用している標準ヘッダー:**
```cpp
#include <cassert>   // アサーション
#include <cstdint>   // uint8_t, uint32_t など
#include <cstddef>   // size_t, nullptr_t
#include <new>       // placement new
#include <cstring>   // memcpy（一部使用）
```

**ライセンス影響:**
- C++標準ライブラリは実行時ライブラリ例外により商用利用可能
- GCC: GPL + Runtime Library Exception
- Clang/LLVM: MIT/NCSA License
- **実質的な制約なし**

---

## 外部依存関係

### 依存関係の分離

Omusubiは以下の2層構造により依存関係を分離しています：

```
┌─────────────────────────────────────────┐
│ Omusubiコア (外部依存なし)                │
│ - Optional, Result, Logger              │
│ - インターフェース定義                    │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│ プラットフォーム実装 (外部ライブラリ依存) │
│ - M5StackSystemContext                  │
│ - ESP32固有実装                          │
└─────────────────────────────────────────┘
```

**利点:**
- コア部分のみ使用する場合、ライセンス制約なし
- プラットフォーム実装は必要な場合のみリンク
- 新しいプラットフォームへの移植が容易

---

## プラットフォーム固有の依存

### M5Stack実装の依存関係

M5Stack向け実装（将来実装予定）は以下のライブラリに依存する予定です：

#### 1. M5Stack Library

**概要:**
- M5Stackハードウェアへのアクセスライブラリ

**ライセンス:**
- MIT License

**リポジトリ:**
- https://github.com/m5stack/M5Stack

**商用利用:**
- ✅ 可能
- 帰属表示が必要

**ライセンス条件:**
```
MIT License

Copyright (c) M5Stack

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction...
```

#### 2. ESP32 Core for Arduino

**概要:**
- ESP32マイコン向けArduino互換フレームワーク

**ライセンス:**
- Apache License 2.0

**リポジトリ:**
- https://github.com/espressif/arduino-esp32

**商用利用:**
- ✅ 可能
- 帰属表示が必要

**ライセンス条件:**
- ソースコード改変時は変更箇所の明示が必要
- NOTICE ファイルの内容を保持

#### 3. ESP32 BLE Library

**概要:**
- Bluetooth Low Energy (BLE) サポート

**ライセンス:**
- Apache License 2.0

**商用利用:**
- ✅ 可能
- 帰属表示が必要

#### 4. WiFi / BluetoothSerial ⚠️

**概要:**
- WiFi および Bluetooth Classic 機能

**ライセンス:**
- Arduino Framework経由: LGPL 2.1
- ESP-IDF直接使用: Apache 2.0

**商用利用:**
- ⚠️ 条件付き（Arduino Framework経由の場合）
- ✅ 推奨: ESP-IDF直接使用（Apache 2.0）

**LGPL 2.1 の要件:**
1. 動的リンク（.so/.dll）として分離、または
2. ユーザーがライブラリを置き換え可能にする、または
3. 商用ライセンスを購入

**回避策:**
- ESP-IDF (Espressif IoT Development Framework) を直接使用
- Apache 2.0 ライセンスのみで動作

---

## 商用利用について

### ✅ 完全に自由な構成

**Omusubiコア + BLE のみ使用:**

```cpp
// この構成は外部ライブラリ依存が MIT + Apache 2.0 のみ
#include <omusubi/omusubi.h>

// BLE使用（Apache 2.0）
auto* ble = ctx.get_connectable_context()->get_ble_context();
```

**ライセンス:**
- Omusubiコア: 制約なし
- M5Stack: MIT
- BLE: Apache 2.0

**商用利用:**
- ✅ 完全に可能
- 帰属表示のみ必要

### ⚠️ 注意が必要な構成

**WiFi / Bluetooth Classic 使用（Arduino Framework経由）:**

```cpp
// この構成は LGPL 2.1 の影響を受ける可能性
auto* wifi = ctx.get_connectable_context()->get_wifi_context();
auto* bt = ctx.get_connectable_context()->get_bluetooth_context();
```

**対処法:**
1. **推奨:** ESP-IDF直接使用への移行（Omusubiの将来対応予定）
2. LGPL要件を満たす（動的リンク分離など）

---

## 帰属表示

### Omusubiフレームワーク使用時の推奨表示

製品のドキュメントまたはクレジット画面に以下を含めることを推奨します：

```
This product uses the following open source software:

Omusubi Framework
Copyright (c) 2025 TakumiOkayasu
Licensed under the MIT License / Apache License 2.0

M5Stack Library
Copyright (c) M5Stack
Licensed under the MIT License
https://github.com/m5stack/M5Stack

ESP32 Arduino Core
Copyright (c) Espressif Systems
Licensed under the Apache License 2.0
https://github.com/espressif/arduino-esp32
```

### Omusubiコアのみ使用する場合

コア機能のみ使用する場合（プラットフォーム実装を使用しない場合）：

```
This product uses the Omusubi Framework
Copyright (c) 2025 TakumiOkayasu

Omusubi core components have no external dependencies
and use only standard C++17 features.
```

---

## ライセンス一覧表

| コンポーネント | 外部依存 | ライセンス | 商用利用 | 帰属表示 |
|--------------|---------|-----------|---------|---------|
| **Omusubiコア** | なし | - | ✅ 完全自由 | 不要 |
| **Logger** | なし | - | ✅ 完全自由 | 不要 |
| **Optional/Result** | なし | - | ✅ 完全自由 | 不要 |
| **std::string_view** | なし | - | ✅ 完全自由 | 不要 |
| **M5Stack実装** | M5Stack Lib | MIT | ✅ 可能 | 必要 |
| **ESP32 Core** | ESP32 Arduino | Apache 2.0 | ✅ 可能 | 必要 |
| **BLE** | ESP32 BLE | Apache 2.0 | ✅ 可能 | 必要 |
| **WiFi (Arduino)** | Arduino Libs | LGPL 2.1 | ⚠️ 条件付き | 必要 |
| **WiFi (ESP-IDF)** | ESP-IDF | Apache 2.0 | ✅ 可能 | 必要 |

---

## よくある質問

### Q1: Omusubiを商用製品に使用できますか？

**A:** はい、可能です。

- **Omusubiコア部分**: 完全に自由に使用可能（外部依存なし）
- **M5Stack実装**: MIT + Apache 2.0 で商用利用可能（帰属表示が必要）

### Q2: WiFiを使いたいのですが、LGPLが心配です

**A:** 以下の対処法があります：

1. **推奨:** ESP-IDF直接使用（Apache 2.0のみ）
2. Arduino Framework使用時はLGPL要件を満たす
3. Omusubiの将来バージョンでESP-IDF対応予定

### Q3: 自社製品にOmusubiを組み込む場合、ソースコード公開は必要ですか？

**A:** いいえ、必要ありません。

- MIT/Apache 2.0 はソースコード公開を要求しません
- 帰属表示（クレジット表示）のみ必要
- ただし、LGPLライブラリ使用時は該当部分の対処が必要

### Q4: Omusubiのライセンスは何ですか？

**A:** Omusubiフレームワーク自体のライセンスは未定です。

- 推奨: MIT License または Apache License 2.0
- コア部分は外部依存なしのため、どちらでも問題なし
- プロジェクトの方針に応じて決定してください

### Q5: 他のプラットフォーム（Arduino、Raspberry Pi Picoなど）への対応は？

**A:** 可能です。

- Omusubiコアは標準C++のみ依存
- 新しいプラットフォーム実装を追加するだけ
- 各プラットフォームのライブラリライセンスに従う

---

## 関連ドキュメント

- [C++17機能ガイド](cpp17-features.md) - 使用している言語機能
- [アーキテクチャガイド](architecture.md) - 依存関係の設計思想
- CLAUDE.md - コーディング規約とプロジェクト方針

---

**Version:** 1.0.1
**Last Updated:** 2025-11-25
