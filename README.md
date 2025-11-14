# Omusubi Framework

マイコン向けの軽量で型安全なC++フレームワーク

## 概要

Omusubiは、組み込みデバイス向けに設計されたモダンなC++17フレームワークです。
SystemContextを中心としたクリーンなアーキテクチャにより、ハードウェアへの統一的なアクセスを提供します。

## 特徴

- 🎯 **シンプルで直感的** - `snake_case`命名規則、分かりやすいAPI
- 🔒 **型安全** - コンパイル時チェック、nullptrの排除
- 🚀 **ゼロオーバーヘッド** - 動的メモリ確保なし、例外なし
- 🌐 **UTF-8対応** - 日本語や絵文字も扱える文字列処理
- 📦 **インターフェース分離** - ハードウェア非依存なコードが書ける
- 🔌 **SystemContext** - Androidの Application Context に相当する統一的なアクセス

## サポートデバイス

- ✅ M5Stack (Basic, Core2, etc.)
- 🚧 Arduino (計画中)
- 🚧 Raspberry Pi Pico (計画中)

## 開発環境

### Dev Container（推奨）

このプロジェクトはDev Containerに対応しています。VS Code + Docker環境で簡単に開発を開始できます。

**必要なもの:**
- Docker Desktop
- VS Code
- Remote - Containers 拡張機能

**開始方法:**
1. リポジトリをクローン
2. VS Codeでフォルダを開く
3. 「Reopen in Container」を選択

**環境の特徴:**
- **コンパイラ:** Clang（最新版）
- **ビルドツール:** CMake, Ninja, Make
- **デバッグ:** GDB, Valgrind
- **ロケール:** 日本語（ja_JP.UTF-8）
- **タイムゾーン:** Asia/Tokyo

Dev Containerを使用することで、環境構築の手間なく即座に開発を開始できます。

## インストール

### PlatformIO

`platformio.ini` に以下を追加:
```ini
[env:m5stack-core-esp32]
platform = espressif32
board = m5stack-core-esp32
framework = arduino
lib_deps = 
    m5stack/M5Stack@^0.4.3
```

プロジェクトに `omusubi` ディレクトリをコピー:
```
your_project/
├── platformio.ini
├── src/
│   └── main.cpp
└── lib/
    └── omusubi/          # ここに配置
        ├── include/
        └── src/
```

## クイックスタート

### 基本的な使い方
```cpp
#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

SystemContext& ctx = get_system_context();
ConnectableContext* connectable = nullptr;
InputContext* input = nullptr;
SerialContext* serial = nullptr;
Pressable* button = nullptr;

void setup() {
    ctx.begin();

    // Contextの取得
    connectable = ctx.get_connectable_context();
    input = ctx.get_input_context();

    // デバイスの取得
    serial = connectable->get_serial_context(0);
    button = input->get_button_a_context();

    serial->write_line("Hello, Omusubi!"_sv);
}

void loop() {
    ctx.update();

    if (button->was_pressed()) {
        serial->write_line("Button pressed!"_sv);
    }

    ctx.delay(10);
}
```

## アーキテクチャの利点: インターフェース分離

Omusubiの最大の特徴は、**インターフェースと実装の完全な分離**です。
これにより、ハードウェアに依存しないポータブルなコードが書けます。

### 例1: デバイス非依存な関数
```cpp
#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

// ✅ この関数はどのデバイスでも動作する
// インターフェースのみに依存し、具体的な実装には依存しない
void log_message(Writable& output, StringView message) {
    output.write("[LOG] "_sv);
    output.write_line(message);
}

SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;
Displayable* display = nullptr;

void setup() {
    ctx.begin();

    serial = ctx.get_serial(0);
    display = ctx.get_display();

    // ✅ 同じ関数が異なるデバイスで動作
    if (serial) {
        log_message(*serial, "System started"_sv);
    }

    if (display) {
        log_message(*display, "System started"_sv);
    }
}

void loop() {
    // ...
}
```

### 例2: センサーデータの汎用処理
```cpp
#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

// ✅ どの3Dセンサーでも動作する汎用関数
void monitor_sensor(Measurable3D& sensor, Writable& output) {
    Vector3 values = sensor.get_values();

    output.write("X: "_sv);
    output.write(values.x);
    output.write(", Y: "_sv);
    output.write(values.y);
    output.write(", Z: "_sv);
    output.write_line(values.z);
}

SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;
Measurable3D* accel = nullptr;
Measurable3D* gyro = nullptr;

void setup() {
    ctx.begin();

    serial = ctx.get_serial(0);
    accel = ctx.get_accelerometer();
    gyro = ctx.get_gyroscope();
}

void loop() {
    ctx.update();

    if (serial) {
        // ✅ 同じ関数が加速度センサーでもジャイロでも動作
        if (accel) {
            serial->write("Accelerometer: "_sv);
            monitor_sensor(*accel, *serial);
        }

        if (gyro) {
            serial->write("Gyroscope: "_sv);
            monitor_sensor(*gyro, *serial);
        }
    }

    ctx.delay(1000);
}
```

### 例3: 通信プロトコルの抽象化
```cpp
#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

// ✅ 通信方式に依存しないプロトコル実装
class SimpleProtocol {
private:
    Readable& input_;
    Writable& output_;

public:
    SimpleProtocol(Readable& input, Writable& output)
        : input_(input), output_(output) {}

    void send_command(StringView command) {
        output_.write("CMD:"_sv);
        output_.write_line(command);
    }

    FixedString<256> receive_response() {
        if (!input_.has_data()) {
            return FixedString<256>();
        }

        FixedString<256> line = input_.read_line();
        if (line.view().starts_with("RES:"_sv)) {
            return FixedString<256>(line.view().substring(4, line.byte_length() - 4));
        }
        return FixedString<256>();
    }
};

SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;
BluetoothContext* bt = nullptr;
WiFiContext* wifi = nullptr;

void setup() {
    ctx.begin();

    serial = ctx.get_serial(0);
    bt = ctx.get_bluetooth();
    wifi = ctx.get_wifi();

    // ✅ 同じプロトコルを異なる通信手段で使用
    if (serial) {
        SimpleProtocol protocol(*serial, *serial);
        protocol.send_command("STATUS"_sv);
    }

    if (bt && bt->is_connected()) {
        SimpleProtocol protocol(*bt, *bt);
        protocol.send_command("STATUS"_sv);
    }

    // WiFi経由でも同じプロトコルが使える
    // （WiFiCommunicationがReadable/Writableを実装していれば）
}

void loop() {
    // ...
}
```

### 例4: テスト可能な設計

インターフェース分離により、実際のハードウェアなしでもテストが可能です。
```cpp
// テスト用のモック実装
class MockSerial : public SerialContext {
private:
    FixedString<1024> buffer_;
    
public:
    // Readableの実装
    FixedString<256> read_string() override {
        // テスト用の実装
        return buffer_;
    }
    
    // Writableの実装
    void write(StringView text) override {
        // テスト用の実装
        buffer_.append(text);
    }
    
    // その他の必要な実装...
};

// ✅ ハードウェアなしでロジックをテストできる
void test_log_function() {
    MockSerial mock;
    log_message(mock, "Test message"_sv);
    
    // モックの内容を検証
    // assert(mock.buffer_ == "[LOG] Test message\r\n");
}
```

## 主な機能

### SystemContext

すべてのハードウェアへの統一的なアクセスポイント:
```cpp
SystemContext& ctx = get_system_context();

// システム情報
const char* device = ctx.get_device_name();
uint32_t free_mem = ctx.get_free_memory();
uint32_t uptime = ctx.get_uptime_ms();

// 通信デバイス
SerialContext* serial = ctx.get_serial(0);
BluetoothContext* bt = ctx.get_bluetooth();
WiFiContext* wifi = ctx.get_wifi();
BLEContext* ble = ctx.get_ble();

// 入力デバイス
Pressable* button = ctx.get_button(0);
uint8_t button_count = ctx.get_button_count();

// センサー
Measurable3D* accel = ctx.get_accelerometer();
Measurable3D* gyro = ctx.get_gyroscope();

// ディスプレイ
Displayable* display = ctx.get_display();

// システム制御
ctx.begin();
ctx.update();
ctx.delay(100);
ctx.reset();
```

### 通信

#### シリアル通信
```cpp
SerialContext* serial = ctx.get_serial(0);

serial->set_baud_rate(115200);
serial->connect();

serial->write_line("Hello"_sv);
serial->write(42);
serial->write(3.14f);

if (serial->has_data()) {
    FixedString<256> line = serial->read_line();
}
```

#### Bluetooth通信
```cpp
BluetoothContext* bt = ctx.get_bluetooth();

// ローカル名を設定
bt->set_local_name("M5Stack-BT"_sv);

// デバイスに接続
if (bt->connect_to("RemoteDevice"_sv, 5000)) {
    bt->write_line("Hello Bluetooth!"_sv);
}

// スキャン
bt->start_scan();
ctx.delay(3000);
bt->stop_scan();

uint8_t count = bt->get_found_count();
for (uint8_t i = 0; i < count; ++i) {
    FixedString<64> name = bt->get_found_name(i);
    int32_t rssi = bt->get_found_signal_strength(i);
}
```

#### Wi-Fi通信
```cpp
WiFiContext* wifi = ctx.get_wifi();

// WiFi接続
if (wifi->connect_to("SSID"_sv, "Password"_sv, 10000)) {
    FixedString<16> ip = wifi->get_ip_address();
    FixedString<64> ssid = wifi->get_connected_ssid();
}

// ネットワークスキャン
wifi->start_scan();
ctx.delay(3000);
wifi->stop_scan();

uint8_t count = wifi->get_found_count();
for (uint8_t i = 0; i < count; ++i) {
    FixedString<64> ssid = wifi->get_found_name(i);
}
```

#### BLE通信
```cpp
BLEContext* ble = ctx.get_ble();

// Peripheralモード（サーバー）
ble->begin_peripheral("M5Stack-BLE"_sv);
BLEService* service = ble->add_service("SERVICE_UUID"_sv);
BLECharacteristic* ch = service->add_characteristic(
    "CHAR_UUID"_sv,
    static_cast<uint16_t>(BLECharacteristicProperty::read) |
    static_cast<uint16_t>(BLECharacteristicProperty::notify)
);
ble->start_advertising();

// Centralモード（クライアント）
ble->begin_central("M5Stack-Central"_sv);
ble->start_scan();
ctx.delay(3000);
ble->stop_scan();

uint8_t count = ble->get_found_count();
for (uint8_t i = 0; i < count; ++i) {
    FixedString<64> name = ble->get_found_name(i);
}
```

### センサー
```cpp
Measurable3D* accel = ctx.get_accelerometer();

if (accel) {
    Vector3 acc = accel->get_values();
    float x = accel->get_x();
    float y = accel->get_y();
    float z = accel->get_z();
}
```

### ディスプレイ
```cpp
Displayable* display = ctx.get_display();

display->clear();
display->set_cursor(0, 0);
display->set_text_size(2);
display->set_text_color(0xFFFF);  // White

display->write_line("Hello"_sv);
display->write(42);
display->write(3.14f);
```

### 入力
```cpp
Pressable* button = ctx.get_button(0);

if (button->was_pressed()) {
    // ボタンが押された瞬間
}

if (button->is_pressed()) {
    // ボタンが押されている間
}

if (button->was_released()) {
    // ボタンが離された瞬間
}
```

## コーディング規約

- ファイル名: `snake_case.hpp`, `snake_case.cpp`
- 関数/変数: `snake_case`
- クラス: `PascalCase`
- 名前空間: `snake_case`
- マクロ: 使用禁止

## プロジェクト構造
```
omusubi/
├── include/omusubi/
│   ├── omusubi.h                    # メインヘッダ
│   ├── system_context.h             # システムコンテキスト
│   ├── core/                        # コア機能
│   │   ├── string_view.h           # 文字列ビュー
│   │   ├── fixed_string.hpp        # 固定長文字列（実装あり）
│   │   ├── fixed_buffer.hpp        # 固定長バッファ（実装あり）
│   │   ├── types.h                 # 基本型定義
│   │   └── mcu_config.h            # MCU設定
│   ├── interface/                   # インターフェース層
│   │   ├── readable.h
│   │   ├── writable.h
│   │   ├── connectable.h
│   │   ├── scannable.h
│   │   ├── pressable.h
│   │   ├── measurable.h
│   │   └── displayable.h
│   ├── device/                      # デバイス層
│   │   ├── serial_communication.h
│   │   ├── bluetooth_communication.h
│   │   ├── wifi_communication.h
│   │   └── ble_communication.h
│   └── platform/m5stack/           # プラットフォーム実装
│       └── m5stack_system_context.hpp
├── src/platform/m5stack/           # M5Stack実装
│   └── m5stack_system_context.cpp
├── examples/platform/m5stack/      # サンプルコード
│   ├── Wi-Fi/
│   │   ├── scan/main.cpp
│   │   └── connection/main.cpp
│   ├── bluetooth/main.cpp
│   ├── ble_server/main.cpp
│   └── ble_client/main.cpp
├── main.cpp                        # エントリーポイント
└── Makefile                        # ビルド設定
```

## ライセンス

未定（検討中）

## コントリビュート

Issue や Pull Request を歓迎します！

## ドキュメント

### 開発者向けドキュメント

- **[開発ガイド](DEVELOPMENT.md)** - 開発環境のセットアップとワークフロー
- **[貢献ガイド](docs/contributing.md)** - プロジェクトへの貢献方法
- **[アーキテクチャ](docs/architecture.md)** - 設計思想とアーキテクチャの詳細
- **[APIリファレンス](docs/api-reference.md)** - 完全なAPIドキュメント
- **[コード品質](docs/code-quality.md)** - フォーマットとLintツール
- **[テストガイド](docs/testing.md)** - テスト戦略とガイドライン

### プラットフォーム

- **[プラットフォームサポート](docs/platform-support.md)** - 対応ハードウェアと実装状況
- **[マイグレーションガイド](docs/migration-guide.md)** - バージョン間の移行方法

### プロジェクト管理

- **[CHANGELOG](CHANGELOG.md)** - バージョン履歴と変更内容
- **[SECURITY](SECURITY.md)** - セキュリティポリシーと脆弱性報告

## コミュニティ

### 質問・議論

- **[GitHub Discussions](https://github.com/your-org/omusubi/discussions)** - 質問、アイデア、一般的な議論

### バグ報告・機能リクエスト

- **[Bug Report](https://github.com/your-org/omusubi/issues/new?template=bug_report.md)** - バグを報告
- **[Feature Request](https://github.com/your-org/omusubi/issues/new?template=feature_request.md)** - 新機能を提案
- **[Platform Support](https://github.com/your-org/omusubi/issues/new?template=platform_support.md)** - プラットフォームサポートをリクエスト

### 貢献する

プルリクエストを歓迎します！貢献前に[Contributing Guide](docs/contributing.md)をご確認ください。

## ライセンス

未定（検討中）

## 作者

Murata

---

**開発中のプロジェクトです。** APIは変更される可能性があります。
