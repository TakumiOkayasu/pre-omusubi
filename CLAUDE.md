# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Omusubi is a lightweight, type-safe C++17 framework for embedded devices (primarily M5Stack). It provides a clean architecture centered around `SystemContext` for unified hardware access, with complete interface/implementation separation for hardware-independent code.

**Key Characteristics:**
- Zero-overhead abstractions (no dynamic allocation, no exceptions)
- C++17 with clang++
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

## Architecture: Three-Layer Design

### 1. Interface Layer (`include/omusubi/interface/`)
Pure abstract interfaces defining capabilities:
- `Readable` / `Writable` - I/O operations
- `Connectable` / `Scannable` - Network operations
- `Pressable` - Button/input
- `Measurable` / `Measurable3D` - Sensor data
- `Displayable` - Screen output

### 2. Device Layer (`include/omusubi/device/`)
Device-specific abstractions that inherit from interfaces:
- `SerialCommunication` (Readable + Writable + Connectable)
- `BluetoothCommunication` (Readable + Writable + Connectable + Scannable)
- `WiFiCommunication` (Connectable + Scannable)
- `BLECommunication` (Connectable + Scannable + BLE-specific)

### 3. Platform Layer (`include/omusubi/platform/`, `src/platform/`)
Platform-specific implementations (e.g., `M5StackSystemContext`) that implement the `SystemContext` interface and provide concrete device instances.

## SystemContext Pattern

`SystemContext` is the single point of access to all hardware (similar to Android's Application Context). Users retrieve device pointers once in `setup()` and reuse them:

```cpp
// Retrieve once globally - critical for performance
SystemContext& ctx = get_system_context();
SerialCommunication* serial = nullptr;

void setup() {
    ctx.begin();
    serial = ctx.get_serial(0);  // Get device pointer once
}

void loop() {
    ctx.update();
    serial->write_line("..."_sv);  // Reuse pointer (no function call overhead)
}
```

**Important:** Never call `ctx.get_*()` repeatedly in `loop()` - this creates unnecessary overhead in high-frequency execution.

## Coding Conventions

- **Naming:**
  - Files: `snake_case.h`, `snake_case.hpp`, `snake_case.cpp`
  - Functions/variables: `snake_case`
  - Classes: `PascalCase`
  - Namespaces: `snake_case`
- **Macros:** Prohibited
- **String literals:** Use `_sv` suffix (requires `using namespace omusubi::literals`)
- **Header guards:** Use `#pragma once`

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
SerialCommunication* serial = nullptr;

void setup() {
    // システムの初期化
    ctx.begin();

    // デバイスの取得（一度だけ）
    serial = ctx.get_serial(0);

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
- Retrieve device pointers once in `setup()`, store globally
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
