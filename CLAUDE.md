# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## CRITICAL: Japanese Communication Required

**All communication with the user MUST be in Japanese.**
- All responses, comments, and explanations must be in Japanese
- Technical terms can remain in English
- Code comments must be in Japanese

## CRITICAL: Git Commit/Push Policy

**NEVER execute `git commit` or `git push` commands.**
- ❌ **PROHIBITED:** `git commit`, `git push`, `git commit --amend`, `git push --force`
- ✅ **ALLOWED:** `git add`, `git rm`, `git mv`, `git status`, `git diff`, `git log`
- User handles all commit/push operations

## Project Overview

Omusubi is a lightweight, type-safe C++17 framework for embedded devices (M5Stack). It provides a clean architecture centered around `SystemContext` for unified hardware access.

**Key Characteristics:**
- Zero-overhead abstractions (no heap, no exceptions)
- C++17 with clang++
- `.hpp` = header-only implementations, `.h` = declarations only
- Japanese comments standard

## Build System

```bash
make              # Build
make clean        # Clean
make rebuild      # Rebuild from scratch
make run          # Build and run
```

## Code Quality

### clang-format (Automatic on save)
- Format manually: `clang-format -i file.cpp`
- Format all: `find include src -name "*.h" -o -name "*.hpp" -o -name "*.cpp" | xargs clang-format -i`

### clang-tidy (Real-time via clangd)
- Lint manually: `clang-tidy file.cpp -- -Iinclude -std=c++17`
- Enforces naming: Classes=`CamelCase`, functions/vars=`snake_case`, constants/enums=`UPPER_CASE`, private members=`snake_case_`

## Architecture: Method Chain Design

### Core Pattern

Hardware access via method chains:
```cpp
get_system_context()->get_[category]_context()->get_[device]_context()->method()
```

**Two patterns:**

**Pattern 1: Basic (3-layer)** - Most devices
```cpp
ctx->get_connectable_context()->get_serial_context(0)->write("Hello"_sv);
ctx->get_sensor_context()->get_accelerometer_context()->get_values();
```

**Pattern 2: Hierarchical (4+ layers)** - Complex devices with sub-components
```cpp
BLEContext* ble = ctx->get_connectable_context()->get_ble_context();
BLEService* svc = ble->add_service(uuid);
BLECharacteristic* ch = svc->add_characteristic(uuid, props);
ch->write(data);
```

Use hierarchical when:
- Device has dynamically created sub-components
- Components have independent state/methods
- Components represent distinct domain concepts (Service/Characteristic, Route/Handler)

**Component naming:** Use domain-standard terms (`BLECharacteristic`, `HTTPRequest`), NOT `*Context` suffix.

### Layer Structure

**1. Interface Layer** (`include/omusubi/interface/`)
- `*able` interfaces: `ByteReadable`, `TextReadable`, `ByteWritable`, `TextWritable`, `Connectable`, `Scannable`, etc.
- Each interface = single responsibility (ISP)
- Return abstract types (`StringView`) not concrete (`FixedString<N>`)
- Separate text/byte I/O (Java `Reader`/`Writer` vs `InputStream`/`OutputStream` pattern)

**2. Context Layer** (`include/omusubi/context/`)
- Acts as DI container (device creation, ownership, mock substitution, platform switching)
- Categories: `ConnectableContext`, `ScannableContext`, `SensorContext`, `InputContext`, `OutputContext`, `SystemInfoContext`, `PowerContext`
- Single primary access path per device

**3. Device Layer** (`include/omusubi/device/`)
- Concrete device contexts implementing `*able` interfaces
- Examples: `SerialContext`, `BluetoothContext`, `WiFiContext`, `BLEContext`

**4. Component Layer** - Optional for hierarchical devices
- Sub-components: `BLECharacteristic`, `BLEService`, etc.
- Use domain terminology, NOT `*Context` suffix

**5. Platform Layer** (`include/omusubi/platform/`, `src/platform/`)
- Platform-specific implementations: `M5StackSystemContext`, `M5StackConnectableContext`, etc.
- Pattern: Contexts own devices as members
- `get_system_context()` implemented as Meyers Singleton in platform `.cpp`

### Critical Design Rules

**1. No Heap Allocation**
- Stack-allocated or placement new with static buffers only
- Never `new`, `delete`, `malloc`, `free`, `std::unique_ptr`, `std::shared_ptr`

**2. Abstraction Policy**
- ✅ Abstract operations (verbs): `connect`, `disconnect`, `write`, `read`
- ❌ Do NOT abstract structures (data, task definitions) - causes leakage
- Use concrete types: `FixedString<N>`, `Vector3`

**3. Context Getters**
```cpp
// Runtime parameter
SerialContext* serial = ctx.get_connectable_context()->get_serial_context(0);

// Compile-time template
SerialContext* serial = ctx.get_connectable_context()->get_serial_context<0>();
```

**4. Naming Conventions**
- Interfaces: `*able`
- Contexts: `*Context`
- Platform implementations: `M5Stack*Context`
- Files: `snake_case.h/hpp/cpp`
- Functions/vars: `snake_case`
- Classes: `PascalCase`
- Constants/enums: `UPPER_CASE_WITH_UNDERSCORES`
- Component layer: Domain terms (NOT `*Context`)

**5. Context Getter Naming (Strict)**
ALL Context getters MUST follow: `get_*_context()` format
```cpp
// ✅ Correct
SerialContext* get_serial_context(uint8_t port) const;

// ❌ Wrong
SerialContext* serial_context(uint8_t port) const;      // Missing get_
SerialContext* get_serial(uint8_t port) const;          // Missing _context
```

**6. Same-Type Getter Prohibition**
MUST NOT have multiple getters returning same type - use parameters/templates:
```cpp
// ❌ PROHIBITED
virtual HogeContext* get_hoge_1_context() const = 0;
virtual HogeContext* get_hoge_2_context() const = 0;

// ✅ CORRECT
virtual HogeContext* get_hoge_context(uint8_t index) const = 0;
virtual uint8_t get_hoge_count() const = 0;
```

**7. SystemContext Access**
Always use free function `get_system_context()` (NOT a class static method)

**8. All Getters Must Be const**
ALL getter methods MUST be `const`. If not possible, provide clear justification.

**9. Implementation Hiding**
- Headers: NO `void* impl_`, NO `struct Impl;` forward declarations
- Implementation: Use anonymous namespaces with static variables in `.cpp`
- Static variables in `.cpp`: Explicitly use `static` keyword

## Coding Conventions

**C++17 Standard (Strict)**
- All C++14 features plus: nested namespaces (`namespace A::B {}`), `if constexpr`, structured bindings, inline variables, fold expressions
- **Modern C++ First:** Actively use latest C++17 features to write simple, concise code
- **constexpr Rule:** ALWAYS use `constexpr` when possible - apply to functions, constructors, methods
- Only omit when function has side effects or technical limitations - provide justification
- **Prefer `if constexpr` over template specialization** when appropriate for better readability

**Type Deduction**
- **Prefer `auto`** for function returns
- **Explicit types for numeric literals** (embedded clarity):
```cpp
// ✅ Preferred
auto str = static_string("Hello");
uint32_t retry_count = 0;  // Clear bit width

// ✗ Avoid
StaticString<5> str = static_string("Hello");  // auto is better
auto retry_count = 0;  // Wrong - int, not uint32_t
```

**Other Rules**
- **Macros:** Prohibited - use `constexpr`
- **String literals:** Use `_sv` suffix (`using namespace omusubi::literals`)
- **Header guards:** `#pragma once`
- **`std::move()`:** DO NOT use - prevents RVO, unnecessary for small stack objects
- **Comments:** Only when essential - code should be self-explanatory
- **String formatting:** ALWAYS use `format()` for string composition - avoid multiple write calls or manual concatenation

**String Formatting with format() (CRITICAL)**
- **ALWAYS use `format()` for string composition** - this is mandatory, not optional
- Avoid redundant code: multiple `write()` calls, manual string concatenation, or temporary buffers
- `format()` provides type-safe, efficient formatting with minimal code

```cpp
// ❌ PROHIBITED: Multiple write calls
write_string("[");
write_string(level_name);
write_string("] ");
write_string(message);
write_string("\r\n");

// ✅ REQUIRED: Use format()
auto formatted = format("[{}] {}\r\n", level_name, message);
write_text(span<const char>(formatted.data(), formatted.byte_length()));

// ❌ PROHIBITED: Manual concatenation
FixedString<64> str;
str.append("[");
str.append(level);
str.append("] ");
str.append(msg);

// ✅ REQUIRED: Use format()
auto str = format("[{}] {}", level, msg);
```

## Usage Pattern

**Performance-Critical: Cache Context Pointers**
```cpp
SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void setup() {
    ctx.begin();
    serial = ctx.get_connectable_context()->get_serial_context(0);  // Cache once
}

void loop() {
    ctx.update();
    serial->write("Hello"_sv);  // Use cached pointer
    ctx.delay(10);
}
```

**RAII Pattern for Classes**
```cpp
class MyDevice {
private:
    BluetoothContext* bt_;
public:
    MyDevice(SystemContext& ctx)
        : bt_(ctx.get_connectable_context()->get_bluetooth_context()) {}
    void connect() { bt_->connect(); }
};
```

## Core Types

- `StringView` - Non-owning string reference (UTF-8)
- `FixedString<N>` - Stack string with fixed capacity
- `FixedBuffer<N>` - Stack byte buffer
- `Vector3` - 3D vector for sensors
- `PowerState` - Power/battery state enum
- `Result<T, E>` - Rust-style result type for error handling
- `std::optional<T>` - C++17 standard optional (use directly, no wrapper)
- `span<T>` - Non-owning memory view (C++20 std::span backport)
- `format()` - Type-safe string formatting

## Writing Examples

Pattern for `examples/platform/m5stack/`:
```cpp
// [Japanese comment explaining what the example does]
#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void setup() {
    ctx.begin();
    serial = ctx.get_connectable_context()->get_serial_context(0);
    // [Setup logic with Japanese comments]
}

void loop() {
    ctx.update();
    // [Loop logic with Japanese comments]
    ctx.delay(10);
}
```

Guidelines:
- Clear Japanese comments
- Retrieve device pointers once in `setup()`, cache globally
- Focus on single functionality
- Prefer serial output over display

## Interface-Based Development

Depend only on interfaces for hardware-agnostic code:
```cpp
void log_message(TextWritable& output, StringView message) {
    output.write("[LOG] "_sv);
    output.write_line(message);
}

log_message(*serial, "Started"_sv);    // Works with any TextWritable
log_message(*display, "Started"_sv);
```

## Documentation Management

**CRITICAL: When creating new docs in `docs/`, MUST update existing related docs.**

**Version Tracking (Required for all `docs/`):**
```markdown
---
**Version:** 1.0.0
**Last Updated:** 2025-11-16
```

**Version Increment:**
- Major (X.0.0): Breaking changes, complete rewrites
- Minor (0.X.0): New sections, significant additions
- Patch (0.0.X): Corrections, clarifications, cross-references

**Workflow:**
1. Survey existing docs: `ls docs/`, `grep -r "topic" docs/`
2. Create new doc with version 1.0.0
3. **MANDATORY:** Update related docs, add cross-refs, increment patch versions

**Consistency Rules:**
- Identical terminology across all docs
- Link related docs with relative paths
- No contradictions

## Embedded Design Principles

**Context Pattern for Static Writable Area Constraints:**
- Access data through interfaces
- Divide by category, expose only necessary functionality
- Obtain interfaces from parent Context
- Hide implementations from callers
- Singleton is implementation choice (caller should NOT assume same instance)

**Reference:** Android Application Context, Abstract Factory Pattern
