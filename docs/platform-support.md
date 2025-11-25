# プラットフォームサポート

このドキュメントでは、Omusubiがサポートするプラットフォームと、各プラットフォーム固有の情報を提供します。

## サポート状況

### 対応済みプラットフォーム

| プラットフォーム | 状態 | 実装状況 | メンテナー |
|------------------|------|----------|-----------|
| M5Stack | ✅ 安定 | 80% | @maintainer |

### 対応予定プラットフォーム

| プラットフォーム | 優先度 | 予定時期 | 備考 |
|------------------|--------|----------|------|
| Arduino | 高 | TBD | ESP32, ESP8266を優先 |
| Raspberry Pi Pico | 中 | TBD | RP2040対応 |
| STM32 | 低 | TBD | 要リクエスト |

## M5Stack

### 概要

**対応デバイス:**
- M5Stack Basic
- M5Stack Core2
- M5Stack Fire
- M5Stack Grey

**C++サポート:**
| 項目 | 詳細 |
|------|------|
| **デフォルトC++** | C++11 |
| **C++14サポート** | ✅ 完全対応 |
| **C++17サポート** | ✅ 完全対応（`-std=c++17`フラグ） |
| **C++20サポート** | ⚠️ 部分対応（一部機能制限あり） |
| **コンパイラ** | GCC 5.2.0+ (xtensa-esp32) |
| **IDE** | Arduino IDE 1.8.6+, PlatformIO |

**実装済み機能:**
- ✅ シリアル通信（UART0/1/2）
- ✅ WiFi接続・スキャン
- ✅ Bluetooth Classic通信・スキャン
- ✅ BLE通信・スキャン
- ✅ 加速度センサー
- ✅ ジャイロセンサー
- ✅ ボタン入力
- ✅ ディスプレイ出力
- ✅ 電源管理
- ✅ システム情報

**未実装機能:**
- ⏳ SDカード
- ⏳ I2C/SPI
- ⏳ タッチパネル（Core2）
- ⏳ スピーカー

### ビルド環境

**PlatformIO（推奨）:**

```ini
[env:m5stack]
platform = espressif32
board = m5stack-core-esp32
framework = arduino

lib_deps =
    m5stack/M5Stack@^0.4.3

build_flags =
    -std=c++17
    -Iinclude

src_filter =
    +<*>
    +<../include/omusubi/>
    +<../src/platform/m5stack/>
```

**Arduino IDE:**

1. ボードマネージャーでESP32をインストール
2. ライブラリマネージャーで`M5Stack`をインストール
3. ボード選択: `M5Stack-Core-ESP32`

### 使用例

```cpp
#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void setup() {
    ctx.begin();
    serial = ctx.get_connectable_context()->get_serial_context(0);
}

void loop() {
    ctx.update();

    serial->write_text("Hello, M5Stack!"_sv);

    ctx.delay(1000);
}
```

### プラットフォーム固有の注意事項

**1. シリアルポート**

```cpp
// Serial0: USB Serial（デバッグ用、自動接続）
SerialContext* usb = ctx.get_connectable_context()->get_serial_context(0);

// Serial1/2: GPIO Serial（手動接続が必要）
SerialContext* gpio = ctx.get_connectable_context()->get_serial_context(1);
gpio->connect();  // ボーレート設定等はデフォルト
```

**2. WiFiとBLEの共存**

ESP32の制約により、WiFiとBLEを同時に使用するとパフォーマンスが低下します：

```cpp
// 推奨: WiFiまたはBLEのどちらか一方を使用
WiFiContext* wifi = ctx.get_connectable_context()->get_wifi_context();
wifi->connect_to("SSID"_sv, "password"_sv);

// 非推奨: WiFiとBLEの同時使用
BLEContext* ble = ctx.get_connectable_context()->get_ble_context();
ble->start_scan();  // WiFi接続中は遅延が発生する可能性
```

**3. メモリ制約**

M5StackはESP32ベースで、RAMが限られています：

- DRAM: 約320KB（システムとスタックで消費）
- 大きな`FixedString`/`FixedBuffer`はグローバル変数として配置を推奨
- 深い再帰は避ける

**4. ディスプレイ更新**

```cpp
DisplayContext* display = ctx.get_output_context()->get_display_context();

// 推奨: 変更箇所のみ更新
display->set_text_color(0xFFFF);
display->draw_text(0, 0, "Hello"_sv);

// 非推奨: 全画面更新（遅い）
display->clear();
display->fill_screen(0x0000);
```

**5. ボタン入力**

```cpp
// M5Stack: ボタンA/B/C
InputContext* input = ctx.get_input_context();
ButtonContext* btn_a = input->get_button_context(0);  // Button A
ButtonContext* btn_b = input->get_button_context(1);  // Button B
ButtonContext* btn_c = input->get_button_context(2);  // Button C

if (btn_a->get_state() == ButtonState::PRESSED) {
    // ボタンA押下時の処理
}
```

### トラブルシューティング

**シリアル接続できない:**

```cpp
// 手動接続を確認
SerialContext* serial = ctx.get_connectable_context()->get_serial_context(0);
if (!serial->is_connected()) {
    serial->connect();
}
```

**WiFi接続が遅い:**

```cpp
// WiFiスキャン前に接続を試みる
WiFiContext* wifi = ctx.get_connectable_context()->get_wifi_context();
wifi->connect_to("SSID"_sv, "password"_sv);  // スキャンより先に接続
```

**メモリ不足:**

```cpp
// 大きなバッファはグローバル変数に
static FixedBuffer<2048> large_buffer;

void loop() {
    // 使用
}
```

## Arduino

### 概要

**対応予定デバイス:**
- Arduino Uno（検討中）
- Arduino Mega（検討中）
- ESP32搭載Arduino
- ESP8266搭載Arduino

**デバイス別C++サポート:**

| デバイス | MCU | デフォルトC++ | C++14サポート | C++17サポート | コンパイラ | 備考 |
|---------|-----|------------|-------------|-------------|----------|------|
| **Arduino Uno** | ATmega328P | C++11 | ✅ 完全対応 | ✅ 完全対応 | avr-gcc 7.3.0+ | RAM 2KB制約あり |
| **Arduino Mega** | ATmega2560 | C++11 | ✅ 完全対応 | ✅ 完全対応 | avr-gcc 7.3.0+ | RAM 8KB |
| **Arduino (ESP32)** | ESP32 | C++11 | ✅ 完全対応 | ✅ 完全対応 | xtensa-esp32-gcc 5.2.0+ | M5Stackと同等 |
| **Arduino (ESP8266)** | ESP8266 | C++11 | ✅ 完全対応 | ✅ 完全対応 | xtensa-lx106-gcc 4.8.2+ | RAM 80KB |

**実装予定機能:**
- シリアル通信
- デジタルI/O
- アナログ入力
- PWM出力

**実装状況:** 未着手

### ビルド環境（予定）

**PlatformIO:**

```ini
[env:arduino_esp32]
platform = espressif32
board = esp32dev
framework = arduino

build_flags =
    -std=c++17
    -Iinclude

src_filter =
    +<*>
    +<../include/omusubi/>
    +<../src/platform/arduino/>
```

### プラットフォーム固有の注意事項（予定）

- Arduino Uno/Mega: メモリが非常に限られているため、機能制限あり
- ESP32/ESP8266: M5Stackと同等の機能を提供予定

## Raspberry Pi Pico

### 概要

**対応予定デバイス:**
- Raspberry Pi Pico
- Raspberry Pi Pico W（WiFi対応）

**C++サポート:**
| 項目 | 詳細 |
|------|------|
| **デフォルトC++** | C++14 |
| **C++14サポート** | ✅ 完全対応（デフォルト） |
| **C++17サポート** | ✅ 完全対応（`-std=c++17`フラグ） |
| **C++20サポート** | ✅ 完全対応（`-std=c++20`フラグ） |
| **C++23サポート** | ⚠️ 部分対応（GCC 10+） |
| **MCU** | RP2040（Cortex-M0+ デュアルコア） |
| **コンパイラ** | arm-none-eabi-gcc 10.3.1+ |
| **RAM** | 264KB SRAM |
| **Flash** | 2MB（外部） |

**実装予定機能:**
- UART通信
- I2C/SPI
- GPIO
- PWM
- ADC
- WiFi（Pico Wのみ）

**実装状況:** 未着手

### ビルド環境（予定）

**Pico SDK:**

```cmake
cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(omusubi_pico C CXX ASM)
set(CMAKE_CXX_STANDARD 14)

pico_sdk_init()

add_executable(omusubi_pico
    src/main.cpp
    src/platform/pico/pico_system_context.cpp
    # ...
)

target_include_directories(omusubi_pico PRIVATE include)
target_link_libraries(omusubi_pico pico_stdlib)
```

### プラットフォーム固有の注意事項（予定）

- デュアルコアを活用した並行処理サポート検討中
- PIOブロックによる高速I/O対応検討中

## STM32

### 概要

**対応予定デバイス:**
- STM32F4シリーズ
- STM32F7シリーズ

**C++サポート:**
| 項目 | STM32F4 | STM32F7 |
|------|---------|---------|
| **デフォルトC++** | C++11 | C++11 |
| **C++14サポート** | ✅ 完全対応 | ✅ 完全対応 |
| **C++17サポート** | ✅ 完全対応 | ✅ 完全対応 |
| **C++20サポート** | ✅ 完全対応 | ✅ 完全対応 |
| **MCU** | Cortex-M4F | Cortex-M7F |
| **コンパイラ** | arm-none-eabi-gcc 9.0+ | arm-none-eabi-gcc 9.0+ |
| **RAM** | 128KB～256KB | 256KB～512KB |
| **Flash** | 512KB～2MB | 1MB～2MB |
| **FPU** | ✅ Single Precision | ✅ Double Precision |

**実装状況:** 未着手（要リクエスト）

リクエストがあれば対応を検討します。

## 新しいプラットフォームの追加

新しいプラットフォームを追加したい場合は、以下のドキュメントを参照してください：

- [Architecture - 新しいプラットフォームの追加](architecture.md#新しいプラットフォームの追加)
- [Contributing](contributing.md)

または、[GitHub Issues](https://github.com/your-org/omusubi/issues)でリクエストを作成してください。

## プラットフォーム比較表

### 機能実装状況

| 機能 | M5Stack | Arduino | Pico | STM32 |
|------|---------|---------|------|-------|
| シリアル通信 | ✅ | ⏳ | ⏳ | - |
| WiFi | ✅ | ⏳ | ⏳ | - |
| Bluetooth | ✅ | ⏳ | - | - |
| BLE | ✅ | ⏳ | ⏳ | - |
| ディスプレイ | ✅ | - | - | - |
| センサー | ✅ | ⏳ | ⏳ | - |
| ボタン | ✅ | ⏳ | ⏳ | - |
| GPIO | ⏳ | ⏳ | ⏳ | - |
| I2C/SPI | ⏳ | ⏳ | ⏳ | - |
| ADC | ⏳ | ⏳ | ⏳ | - |
| PWM | ⏳ | ⏳ | ⏳ | - |

**凡例:**
- ✅ 実装済み
- ⏳ 実装予定
- - 未定

### C++バージョンサポート比較

| プラットフォーム | デフォルト | C++14 | C++17 | C++20 | C++23 | 備考 |
|-----------------|----------|-------|-------|-------|-------|------|
| **M5Stack (ESP32)** | C++11 | ✅ | ✅ | ⚠️ | ❌ | `-std=c++17`フラグ推奨 |
| **Arduino Uno** | C++11 | ✅ | ✅ | ❌ | ❌ | RAM 2KB制約 |
| **Arduino Mega** | C++11 | ✅ | ✅ | ❌ | ❌ | RAM 8KB |
| **Arduino ESP32** | C++11 | ✅ | ✅ | ⚠️ | ❌ | M5Stackと同等 |
| **Arduino ESP8266** | C++11 | ✅ | ✅ | ⚠️ | ❌ | RAM 80KB |
| **Raspberry Pi Pico** | C++14 | ✅ | ✅ | ✅ | ⚠️ | デフォルトC++14 |
| **STM32F4** | C++11 | ✅ | ✅ | ✅ | ⚠️ | 128-256KB RAM |
| **STM32F7** | C++11 | ✅ | ✅ | ✅ | ⚠️ | 256-512KB RAM |

**凡例:**
- ✅ 完全対応
- ⚠️ 部分対応（一部機能制限あり）
- ❌ 非対応

## サポートリクエスト

新しいプラットフォームのサポートをリクエストする場合は、以下の情報を含めて[GitHub Issues](https://github.com/your-org/omusubi/issues)に投稿してください：

**必須情報:**
- プラットフォーム名とバージョン
- 主な用途
- 必要な機能（シリアル、WiFi、センサー等）
- 開発環境（Arduino IDE、PlatformIO等）

**任意情報:**
- 参考ドキュメントやサンプルコード
- コミュニティの規模
- 貢献意向（実装サポート可能か）

## コミュニティ貢献

プラットフォーム実装への貢献を歓迎します。詳細は[Contributing Guide](contributing.md)を参照してください。

---

**Version:** 1.3.0
**Last Updated:** 2025-11-25
