# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 重要：日本語でのコミュニケーション必須 (IMPORTANT: Japanese Communication Required)

**このプロジェクトでは、ユーザーとのコミュニケーションは日本語で行うことを必須とします。**

- すべての応答は日本語で行ってください
- コメントや説明は日本語で記述してください
- コードのコメントも日本語を使用してください
- 技術用語は英語のままでも構いませんが、説明は日本語で行ってください

**English responses are not acceptable. All communication with the user must be in Japanese.**

## Project Overview

Omusubi is a lightweight, type-safe C++14 framework for embedded devices (primarily M5Stack). It provides a clean architecture centered around `SystemContext` for unified hardware access, with complete interface/implementation separation for hardware-independent code.

**Key Characteristics:**
- Zero-overhead abstractions (no dynamic allocation, no exceptions)
- C++14 with clang++
- Header-only implementations use `.hpp`, declarations use `.h`
- Japanese comments and documentation are standard

## Build Commands

```bash
# Build the project
make

# Clean build artifacts
make clean

# Rebuild from scratch
make rebuild

# Build and run
make run
```

The Makefile uses recursive file discovery with `$(shell find)` to automatically detect all `.cpp` files in `src/` and maintain directory structure in `obj/`.

## Architecture: Method Chain Design

### Core Design Philosophy

**SystemContext is a gateway to organized device access through method chaining.**

All hardware access follows this pattern:
```cpp
get_system_context()->get_[category]_context()->get_[device]_context()->method()
```

Example:
```cpp
// Bluetooth接続
ctx->get_connectable_context()->get_bluetooth_context()->connect()

// シリアル読み取り
ctx->get_readable_context()->get_serial0_context()->read_line()

// 加速度センサー
ctx->get_sensor_context()->get_accelerometer_context()->get_values()
```

### Layer Structure

### 1. Interface Layer (`include/omusubi/interface/`)
**`*able` interfaces define single-method contracts (following Android pattern):**
- `Readable` - Single pure virtual function for reading
- `Writable` - Single pure virtual function for writing
- `Connectable` - Single pure virtual function for connecting
- `Scannable` - Single pure virtual function for scanning
- `Pressable` - Single pure virtual function for button state
- `Measurable` / `Measurable3D` - Single pure virtual function for measurements
- `Displayable` - Single pure virtual function for display output

**Rule:** Each `*able` interface MUST have exactly one pure virtual function.

### 2. Context Layer (`include/omusubi/context/`)
**Middle-tier contexts group devices by category and serve as DI containers:**

Context層は単なるグループ化ではなく、**依存性注入（DI）コンテナ**として機能します：

**DIコンテナとしての役割:**
- デバイスの生成と所有権管理
- テスト時のモック差し替え
- プラットフォーム切り替えの抽象化
- デバイス間の依存関係の管理

**カテゴリ別Context:**

- `ConnectableContext` - All connection-capable devices
  - `SerialContext`, `BluetoothContext`, `WiFiContext`, `BLEContext`
- `ReadableContext` - All readable devices
  - `SerialContext`, `BluetoothContext`
- `WritableContext` - All writable devices
  - `SerialContext`, `BluetoothContext`, `DisplayContext`
- `ScannableContext` - All scannable devices
  - `WiFiContext`, `BluetoothContext`, `BLEContext`
- `SensorContext` - All sensor devices
  - Accelerometer, Gyroscope, Temperature, etc.
- `InputContext` - All input devices
  - Buttons, Touch, etc.
- `OutputContext` - All output devices
  - Display, LED, Speaker, etc.
- `SystemInfoContext` - System information (read-only)
  - Device name, firmware version, chip ID, uptime, free memory
- `PowerContext` - Power management
  - Power state, battery level

### 3. Device Layer (`include/omusubi/device/`)
**Concrete device context classes implementing `*able` interfaces:**
- `SerialContext` (Readable + Writable + Connectable)
- `BluetoothContext` (Readable + Writable + Connectable + Scannable)
- `WiFiContext` (Connectable + Scannable)
- `BLEContext` (Connectable + Scannable)

### 4. Platform Layer (`include/omusubi/platform/`, `src/platform/`)
**Platform-specific implementations:**
- `M5StackSystemContext` - SystemContext implementation for M5Stack
- `M5StackConnectableContext` - ConnectableContext implementation
- `M5StackSensorContext` - SensorContext implementation
- `M5StackSerialContext` - SerialContext implementation (inherits from device layer)
- etc.

**Implementation pattern (Pattern A - Context owns devices):**
```cpp
class M5StackConnectableContext : public ConnectableContext {
private:
    M5StackSerialContext serial0_{0};
    M5StackSerialContext serial1_{1};
    M5StackBluetoothContext bluetooth_;
    M5StackWiFiContext wifi_;
    M5StackBLEContext ble_;

public:
    SerialContext* get_serial0_context() override { return &serial0_; }
    SerialContext* get_serial1_context() override { return &serial1_; }
    BluetoothContext* get_bluetooth_context() override { return &bluetooth_; }
    WiFiContext* get_wifi_context() override { return &wifi_; }
    BLEContext* get_ble_context() override { return &ble_; }
};
```

**SystemContext Implementation Pattern:**

プラットフォーム実装クラスは、以下のパターンに従います：

```cpp
// プラットフォーム実装（ユーザーは直接使用しない）
class M5StackSystemContext : public SystemContext {
private:
    // コンストラクタはprivateまたはpublic（どちらでも可）
    // ⚠️ get_instance()等のstaticメソッドは不要
    M5StackSystemContext();

public:
    // SystemContextインターフェースの実装
    void begin() override;
    void update() override;
    // ...
};

// プラットフォーム固有の実装ファイル（src/platform/m5stack/system_context.cpp）
namespace omusubi {
    // フリー関数内でstaticローカル変数として生成（Meyers Singleton）
    SystemContext& get_system_context() {
        static platform::m5stack::M5StackSystemContext inst;
        return inst;
    }
}
```

**コンストラクタのアクセス指定:**
- `private`: より厳密（他からの直接生成を禁止）
- `public`: シンプル（`get_system_context()`以外から生成されることはない前提）
- どちらでも動作するが、`public`の方がシンプル

**他のプラットフォームでも同じパターンを実装:**

**重要:** `get_system_context()`は以下のパターンで実装することを徹底してください：

```cpp
SystemContext& get_system_context() {
    static [DeviceName]SystemContext inst;
    return inst;
}
```

各プラットフォームでの実装例：

```cpp
// M5Stack (src/platform/m5stack/system_context.cpp)
namespace omusubi {
    SystemContext& get_system_context() {
        static platform::m5stack::M5StackSystemContext inst;
        return inst;
    }
}

// Arduino (src/platform/arduino/system_context.cpp)
namespace omusubi {
    SystemContext& get_system_context() {
        static platform::arduino::ArduinoSystemContext inst;
        return inst;
    }
}

// Raspberry Pi Pico (src/platform/pico/system_context.cpp)
namespace omusubi {
    SystemContext& get_system_context() {
        static platform::pico::PicoSystemContext inst;
        return inst;
    }
}
```

**このパターンの利点:**
- シンプルで一貫性がある
- C++11のthread-safe static initialization（Meyers Singleton）を活用
- プラットフォーム実装クラスに`get_instance()`メソッドは不要
- Singletonの実装が`get_system_context()`内に集約される

**ビルドシステムでの切り替え:**
- ビルド時に対象プラットフォームの`system_context.cpp`のみをリンク
- ユーザーコードは変更不要（常に`get_system_context()`を呼び出すだけ）

**重要な原則:**
- ユーザーコードは`get_system_context()`フリー関数のみを使用
- `XXXSystemContext::get_instance()`は内部実装の詳細
- プラットフォーム切り替え時も、ユーザーコードは変更不要

### Critical Design Rules

**1. No Heap Allocation**
- All objects MUST be stack-allocated or use placement new with static buffers
- Never use `new`, `delete`, `malloc`, `free`, `std::unique_ptr`, `std::shared_ptr`
- Use static buffers with placement new when dynamic construction is needed

**2. Context Getters: Two Access Patterns**

Context層はDIコンテナとして機能し、2つのアクセスパターンをサポートします：

**パターンA: 個別メソッド（基本）**
```cpp
SerialContext* serial = ctx.get_connectable_context()->get_serial0_context();
```
- ✅ 明示的でわかりやすい
- ✅ すべてのデバイスで利用可能

**パターンB: テンプレートパラメータ（C++14）**
```cpp
SerialContext* serial = ctx.get_connectable_context()->get_serial_context<0>();
```
- ✅ コンパイル時にポート番号を指定
- ✅ テンプレートメタプログラミングに対応
- ⚠️ ランタイム値は使用不可（コンパイル時定数のみ）

**ランタイムパラメータは禁止:**
- ❌ Bad: `get_serial_context(port)` - ランタイム引数は不可

**3. Single Primary Access Path**
- Devices with multiple interfaces have ONE primary category
- Communication devices → via `ConnectableContext`
- Sensors → via `SensorContext`
- Input devices → via `InputContext`
- Output devices → via `OutputContext`

**4. Naming Conventions**
- Interface classes: `*able` (Readable, Writable, Connectable)
- Context classes: `*Context` (SerialContext, SystemContext)
- Implementation classes: `M5Stack*Context` (M5StackSerialContext)
- Constants: `UPPER_CASE_WITH_UNDERSCORES`
- Functions/variables: `snake_case`
- Classes: `PascalCase`

**5. SystemContext Access: Free Function Pattern**

**重要:** SystemContextへのアクセスは必ず**フリー関数**`get_system_context()`を使用します。

```cpp
// フリー関数（グローバル名前空間）
SystemContext& get_system_context();
```

**使用方法:**
```cpp
// ✅ 正しい：フリー関数を使用
SystemContext& ctx = get_system_context();

// ❌ 誤り：クラスのstaticメソッドではない
SystemContext& ctx = SystemContext::get_instance();  // これは存在しない
```

**実装の詳細（ユーザーは意識する必要なし）:**
- プラットフォーム実装（M5StackSystemContext等）は内部でSingletonパターンを使用してもよい
- `get_system_context()`フリー関数が内部でプラットフォーム固有の実装を呼び出す
- これにより、プラットフォーム切り替え時にユーザーコードの変更が不要

**6. SystemContext Core Responsibilities**
```cpp
class SystemContext {
public:
    // System control (stays in SystemContext)
    virtual void begin() = 0;
    virtual void update() = 0;
    virtual void delay(uint32_t ms) = 0;
    virtual void reset() = 0;

    // Category context access
    virtual ConnectableContext* get_connectable_context() = 0;
    virtual ReadableContext* get_readable_context() = 0;
    virtual WritableContext* get_writable_context() = 0;
    virtual ScannableContext* get_scannable_context() = 0;
    virtual SensorContext* get_sensor_context() = 0;
    virtual InputContext* get_input_context() = 0;
    virtual OutputContext* get_output_context() = 0;
    virtual SystemInfoContext* get_system_info_context() = 0;
    virtual PowerContext* get_power_context() = 0;
};
```

**Violation Warning:**
If code does not follow these design rules, it violates the framework's architecture philosophy and must be refactored.

## Usage Pattern

### Performance-Critical: Cache Context Pointers

Users retrieve device pointers once in `setup()` and reuse them to avoid method call overhead:

```cpp
// グローバル変数でキャッシュ（パフォーマンス重視）
SystemContext& ctx = get_system_context();
ConnectableContext* connectable = nullptr;
BluetoothContext* bt = nullptr;

void setup() {
    ctx.begin();

    // 一度だけ取得してキャッシュ
    connectable = ctx.get_connectable_context();
    bt = connectable->get_bluetooth_context();
}

void loop() {
    ctx.update();

    // キャッシュしたポインタを直接使用（オーバーヘッドなし）
    bt->connect();
}
```

### RAII Pattern for Classes

When using contexts in classes, follow RAII principles:

```cpp
class MyDevice {
private:
    BluetoothContext* bt_;

public:
    MyDevice(SystemContext& ctx)
        : bt_(ctx.get_connectable_context()->get_bluetooth_context()) {
        // コンストラクタでContextを取得して保持
    }

    void connect() {
        bt_->connect();
    }
};
```

**Important:**
- Never call `ctx.get_*()->get_*()` chains repeatedly in `loop()` - cache the final pointer
- Context pointers remain valid for the lifetime of SystemContext

## Coding Conventions

- **Naming:**
  - Files: `snake_case.h`, `snake_case.hpp`, `snake_case.cpp`
  - Functions/variables: `snake_case`
  - Classes: `PascalCase`
  - Namespaces: `snake_case`
  - Constants: `UPPER_CASE_WITH_UNDERSCORES`
- **Macros:** Prohibited - Use `constexpr` functions and variables instead
- **String literals:** Use `_sv` suffix (requires `using namespace omusubi::literals`)
- **Header guards:** Use `#pragma once`
- **Memory:** No heap allocation - stack or placement new with static buffers only

## Core Types

- `StringView` - Non-owning string reference (UTF-8 aware)
- `FixedString<N>` - Stack-allocated string with fixed capacity
- `FixedBuffer<N>` - Stack-allocated byte buffer
- `Vector3` - 3D vector for sensor data
- `PowerState` - Enum for power/battery state

## Writing Examples

Examples in `examples/platform/m5stack/` must follow this pattern:

```cpp
// [Japanese comment explaining what the example does]

#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

// グローバル変数：setup()で一度だけ取得し、loop()で再利用
SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void setup() {
    // システムの初期化
    ctx.begin();

    // デバイスの取得（一度だけ）
    // 新しい設計: メソッドチェーン経由でアクセス
    serial = ctx.get_connectable_context()->get_serial0_context();

    // [Setup logic with comments]
}

void loop() {
    // システムの更新
    ctx.update();

    // [Loop logic with comments]

    ctx.delay(10);
}
```

**Example guidelines:**
- Include clear Japanese comments explaining purpose and key steps
- Use method chain to access devices: `ctx.get_[category]_context()->get_[device]_context()`
- Retrieve device pointers once in `setup()`, store globally for performance
- Keep examples focused on single functionality
- Use serial output only (avoid display unless specifically demonstrating display)

## Interface-Based Development

When writing hardware-agnostic code, depend only on interfaces:

```cpp
// ✅ Good: Works with any Writable device
void log_message(Writable& output, StringView message) {
    output.write("[LOG] "_sv);
    output.write_line(message);
}

// Can be used with serial, display, Bluetooth, etc.
log_message(*serial, "Started"_sv);
log_message(*display, "Started"_sv);
```

This enables:
- Code reuse across different hardware
- Testing with mock implementations
- Platform portability

## Header File Distinction

- `.h` files: Declarations only (require corresponding `.cpp`)
- `.hpp` files: Header-only with inline implementations (no `.cpp` needed)

Examples:
- `string_view.h` → needs implementation
- `fixed_string.hpp` → complete implementation in header
- `system_context.h` → interface declaration
- `m5stack_system_context.hpp` → platform-specific implementation

## Platform Support

Currently implemented:
- M5Stack (Basic, Core2, etc.) via `M5StackSystemContext`

Planned:
- Arduino
- Raspberry Pi Pico

## Language

- Code comments: Japanese preferred
- Variable names: English
- Documentation: Japanese in README, mixed in code comments
