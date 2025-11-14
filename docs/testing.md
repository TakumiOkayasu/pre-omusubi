# Testing Guide

このドキュメントでは、Omusubiプロジェクトのテスト戦略とテスト方法を説明します。

## テスト戦略

Omusubiは組み込みシステム向けフレームワークのため、以下の3層テスト戦略を採用しています：

1. **インターフェーステスト** - モックを使用した単体テスト
2. **実機統合テスト** - 実機での動作確認
3. **サンプルコードテスト** - 実用的な使用例の動作確認

## テストの種類

### 1. インターフェーステスト（モック使用）

**目的:** インターフェース設計の妥当性とロジックの正確性を検証

**環境:** Dev Container（Ubuntu）

**特徴:**
- ヒープ割り当てなし
- 実機不要
- 高速実行
- CI/CDで自動実行可能

**実装例:**

```cpp
// tests/mock/mock_serial_context.hpp
#include "omusubi/device/serial_context.h"

class MockSerialContext : public SerialContext {
private:
    FixedBuffer<256> write_buffer_;
    FixedBuffer<256> read_buffer_;
    bool connected_ = false;

public:
    // ByteReadable
    size_t read(span<uint8_t> buffer) override {
        size_t n = (buffer.size() < read_buffer_.length())
                   ? buffer.size()
                   : read_buffer_.length();

        for (size_t i = 0; i < n; ++i) {
            buffer[i] = read_buffer_[i];
        }

        return n;
    }

    size_t available() const override {
        return read_buffer_.length();
    }

    // TextReadable
    size_t read_line(span<char> buffer) override {
        // 改行までを読み取る実装
        return 0;
    }

    // ByteWritable
    size_t write(span<const uint8_t> data) override {
        for (size_t i = 0; i < data.size(); ++i) {
            write_buffer_.append(data[i]);
        }
        return data.size();
    }

    // TextWritable
    size_t write_text(span<const char> text) override {
        for (size_t i = 0; i < text.size(); ++i) {
            write_buffer_.append(static_cast<uint8_t>(text[i]));
        }
        return text.size();
    }

    // Connectable
    bool connect() override {
        connected_ = true;
        return true;
    }

    void disconnect() override {
        connected_ = false;
    }

    bool is_connected() const override {
        return connected_;
    }

    // テスト用ヘルパー
    void set_read_data(span<const uint8_t> data) {
        read_buffer_.clear();
        for (size_t i = 0; i < data.size(); ++i) {
            read_buffer_.append(data[i]);
        }
    }

    span<const uint8_t> get_written_data() const {
        return write_buffer_.as_span();
    }
};
```

**テストコード例:**

```cpp
// tests/test_serial.cpp
#include "mock/mock_serial_context.hpp"

void test_serial_write() {
    MockSerialContext serial;

    // 接続テスト
    assert(!serial.is_connected());
    assert(serial.connect());
    assert(serial.is_connected());

    // 書き込みテスト
    const char* msg = "Hello";
    size_t n = serial.write_text(span<const char>(msg, 5));
    assert(n == 5);

    // 書き込みデータ確認
    span<const uint8_t> written = serial.get_written_data();
    assert(written.size() == 5);
    assert(written[0] == 'H');
    assert(written[4] == 'o');
}

void test_serial_read() {
    MockSerialContext serial;

    // 読み取りデータを設定
    uint8_t data[] = {'W', 'o', 'r', 'l', 'd'};
    serial.set_read_data(span<const uint8_t>(data, 5));

    // 利用可能バイト数確認
    assert(serial.available() == 5);

    // 読み取りテスト
    uint8_t buffer[10];
    size_t n = serial.read(span<uint8_t>(buffer, 10));
    assert(n == 5);
    assert(buffer[0] == 'W');
    assert(buffer[4] == 'd');
}

int main() {
    test_serial_write();
    test_serial_read();

    return 0;
}
```

### 2. 実機統合テスト

**目的:** 実際のハードウェアでの動作確認

**環境:** M5Stack、Arduino、Raspberry Pi Pico等

**特徴:**
- 実機が必要
- ハードウェア固有の問題を検出
- 手動実行

**実装例:**

```cpp
// tests/integration/m5stack/test_wifi.cpp
#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

SystemContext& ctx = get_system_context();
WiFiContext* wifi = nullptr;
SerialContext* serial = nullptr;

void setup() {
    ctx.begin();

    wifi = ctx.get_connectable_context()->get_wifi_context();
    serial = ctx.get_connectable_context()->get_serial_context(0);

    serial->write_text("=== WiFi Integration Test ==="_sv);
}

void loop() {
    ctx.update();

    // テスト1: WiFiスキャン
    serial->write_text("Test 1: WiFi Scan..."_sv);
    wifi->start_scan();
    ctx.delay(3000);
    wifi->stop_scan();

    uint8_t count = wifi->get_found_count();
    serial->write_text("Found "_sv);
    // count出力
    serial->write_text(" networks"_sv);

    if (count > 0) {
        serial->write_text("PASS"_sv);
    } else {
        serial->write_text("FAIL"_sv);
    }

    // テスト2: WiFi接続
    serial->write_text("Test 2: WiFi Connect..."_sv);
    bool connected = wifi->connect_to("TestSSID"_sv, "password"_sv);

    if (connected && wifi->is_connected()) {
        serial->write_text("PASS"_sv);
        wifi->disconnect();
    } else {
        serial->write_text("FAIL"_sv);
    }

    serial->write_text("=== Test Complete ==="_sv);

    // テスト終了
    while (true) {
        ctx.delay(1000);
    }
}
```

**実行方法:**

```bash
# PlatformIOでビルド
pio run -e m5stack

# アップロード
pio run -e m5stack --target upload

# シリアルモニタで結果確認
pio device monitor
```

### 3. サンプルコードテスト

**目的:** 実用的な使用例の動作確認とドキュメント化

**環境:** 実機

**特徴:**
- ユーザー向けの実用例
- ドキュメントとしても機能
- 手動実行

**実装例:**

```cpp
// examples/platform/m5stack/serial_echo.cpp
// シリアルエコーのサンプルコード

#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void setup() {
    ctx.begin();

    // Serial0を取得（USB Serial、自動接続済み）
    serial = ctx.get_connectable_context()->get_serial_context(0);

    serial->write_text("=== Serial Echo Example ==="_sv);
    serial->write_text("Type something..."_sv);
}

void loop() {
    ctx.update();

    // データが利用可能かチェック
    if (serial->available() > 0) {
        // 1行読み取り
        char buffer[256];
        size_t n = serial->read_line(span<char>(buffer, 256));

        if (n > 0) {
            // エコーバック
            serial->write_text("Echo: "_sv);
            serial->write_text(span<const char>(buffer, n));
        }
    }

    ctx.delay(10);
}
```

## モックの作成方法

### 基本パターン

```cpp
// 1. インターフェースを継承
class MockDeviceContext : public DeviceContext {
private:
    // 2. テストデータを保持
    FixedBuffer<256> buffer_;
    bool connected_ = false;

public:
    // 3. インターフェースメソッドを実装
    bool connect() override {
        connected_ = true;
        return true;
    }

    // 4. テスト用ヘルパーメソッドを追加
    void set_test_data(span<const uint8_t> data) {
        buffer_.clear();
        for (size_t i = 0; i < data.size(); ++i) {
            buffer_.append(data[i]);
        }
    }

    span<const uint8_t> get_captured_data() const {
        return buffer_.as_span();
    }
};
```

### テスト用SystemContextの作成

```cpp
// tests/mock/mock_system_context.hpp
class MockSystemContext : public SystemContext {
private:
    mutable MockConnectableContext connectable_;
    mutable MockSensorContext sensor_;
    // ... 他のContext

public:
    MockSystemContext() = default;

    void begin() override {
        // モックの初期化
    }

    void update() override {
        // 何もしない
    }

    void delay(uint32_t ms) override {
        // 何もしない（テストでは待機不要）
    }

    void reset() override {
        // 何もしない
    }

    ConnectableContext* get_connectable_context() const override {
        return &connectable_;
    }

    // ... 他のgetter
};
```

**使用例:**

```cpp
void test_with_mock_context() {
    MockSystemContext ctx;
    ctx.begin();

    // モックデバイスを取得
    MockSerialContext* serial = static_cast<MockSerialContext*>(
        ctx.get_connectable_context()->get_serial_context(0)
    );

    // テストデータ設定
    serial->set_test_data(...);

    // テスト実行
    // ...
}
```

## 実機テスト手順

### M5Stack

**1. 環境準備**

```bash
# PlatformIO環境確認
pio --version

# プロジェクトディレクトリに移動
cd omusubi/tests/integration/m5stack
```

**2. platformio.iniの設定**

```ini
[env:m5stack]
platform = espressif32
board = m5stack-core-esp32
framework = arduino

lib_deps =
    m5stack/M5Stack@^0.4.3

build_flags =
    -std=c++14
    -I../../../include

src_filter =
    +<test_*.cpp>
    +<../../../src/platform/m5stack/>
```

**3. ビルドとアップロード**

```bash
# ビルド
pio run -e m5stack

# アップロード（M5Stack接続済み）
pio run -e m5stack --target upload

# シリアルモニタで確認
pio device monitor
```

**4. テスト結果の確認**

シリアル出力で以下を確認：
- テストケース名
- PASS/FAIL表示
- エラーメッセージ（失敗時）

### Arduino

（実装予定）

### Raspberry Pi Pico

（実装予定）

## CI/CDでのテスト

### GitHub Actions設定

```yaml
# .github/workflows/test.yml
name: Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  unit-test:
    runs-on: ubuntu-latest

    steps:
      - name: Checkout code
        uses: actions/checkout@v3

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y clang++ cmake

      - name: Build tests
        run: |
          cd tests
          cmake .
          make

      - name: Run tests
        run: |
          cd tests
          ./run_all_tests
```

### テストの自動実行

現在、CI環境では以下のみ実行：
- フォーマットチェック
- Lintチェック

実機テストはローカルまたは手動で実行する必要があります。

## テストカバレッジ

### 重点テスト項目

**1. インターフェース契約の検証**

```cpp
// すべてのインターフェースメソッドが正しく動作するか
void test_interface_contract() {
    MockDevice device;

    // 初期状態の確認
    assert(!device.is_connected());

    // 接続テスト
    assert(device.connect());
    assert(device.is_connected());

    // 切断テスト
    device.disconnect();
    assert(!device.is_connected());
}
```

**2. エラーハンドリング**

```cpp
// エラー条件での動作確認
void test_error_handling() {
    MockDevice device;

    // 接続前の操作
    uint8_t buffer[10];
    size_t n = device.read(span<uint8_t>(buffer, 10));
    assert(n == 0);  // 接続前は読み取れない

    // 容量超過
    FixedString<8> small;
    bool result = small.append("1234567890");
    assert(!result);  // 容量不足でfalse
}
```

**3. 境界値テスト**

```cpp
// 境界値での動作確認
void test_boundary_values() {
    FixedString<256> str;

    // 最大容量まで追加
    for (uint32_t i = 0; i < 256; ++i) {
        assert(str.append('x'));
    }

    // 容量超過
    assert(!str.append('y'));
}
```

**4. メモリ安全性**

```cpp
// ヒープ割り当てなしの確認
void test_no_heap_allocation() {
    // スタック割り当てのみ
    FixedString<256> str;
    FixedBuffer<1024> buffer;
    Vector3 vec;

    // すべてスタックに配置されている
    assert(true);
}
```

## テストのベストプラクティス

### 1. テストは独立させる

```cpp
// ❌ 悪い例：テスト間で状態を共有
MockDevice global_device;

void test1() {
    global_device.connect();
    // ...
}

void test2() {
    // test1の状態に依存
    assert(global_device.is_connected());
}

// ✅ 良い例：各テストで独立したインスタンス
void test1() {
    MockDevice device;
    device.connect();
    // ...
}

void test2() {
    MockDevice device;
    // 独立した状態
}
```

### 2. 意図を明確にする

```cpp
// ✅ 良い例：テストの意図が明確
void test_serial_write_increments_buffer() {
    MockSerialContext serial;

    // Given: 空の状態
    assert(serial.get_written_data().size() == 0);

    // When: データを書き込む
    serial.write_text("Hello"_sv);

    // Then: バッファに追加される
    assert(serial.get_written_data().size() == 5);
}
```

### 3. エッジケースをテストする

```cpp
void test_edge_cases() {
    FixedString<256> str;

    // 空文字列
    assert(str.byte_length() == 0);

    // null文字列
    str.append(nullptr);
    assert(str.byte_length() == 0);

    // 容量ちょうど
    for (uint32_t i = 0; i < 256; ++i) {
        str.append('x');
    }
    assert(str.byte_length() == 256);

    // 容量超過
    assert(!str.append('y'));
}
```

### 4. 実機テストの自動化（推奨）

```cpp
// 実機テスト結果をシリアル出力
void run_integration_tests() {
    serial->write_text("=== Integration Tests ==="_sv);

    uint32_t passed = 0;
    uint32_t failed = 0;

    // Test 1
    if (test_wifi_scan()) {
        serial->write_text("PASS: WiFi Scan"_sv);
        passed++;
    } else {
        serial->write_text("FAIL: WiFi Scan"_sv);
        failed++;
    }

    // Test 2
    // ...

    // 結果サマリー
    serial->write_text("Passed: "_sv);
    // passed出力
    serial->write_text("Failed: "_sv);
    // failed出力
}
```

## トラブルシューティング

### モックが正しく動作しない

**原因:** インターフェースメソッドの実装漏れ

**解決策:** すべての純粋仮想関数を実装

```cpp
// コンパイルエラーで未実装メソッドが検出される
class MockDevice : public DeviceContext {
    // すべてのメソッドを実装する必要がある
};
```

### 実機テストがタイムアウトする

**原因:** ハードウェア初期化の遅延

**解決策:** 十分な待機時間を設定

```cpp
void setup() {
    ctx.begin();
    ctx.delay(1000);  // 初期化待機

    wifi = ctx.get_wifi_context();
    ctx.delay(500);  // WiFi初期化待機
}
```

### メモリ不足エラー

**原因:** スタック上の大きなバッファ

**解決策:** グローバル変数として配置

```cpp
// ❌ スタックオーバーフロー
void test() {
    FixedBuffer<4096> large_buffer;  // スタック上
}

// ✅ 静的領域に配置
static FixedBuffer<4096> large_buffer;

void test() {
    large_buffer.clear();
    // 使用
}
```

## 参考資料

- [Contributing Guide](contributing.md) - テストの要件
- [Architecture](architecture.md) - モック設計のガイドライン
- [API Reference](api-reference.md) - インターフェース仕様
