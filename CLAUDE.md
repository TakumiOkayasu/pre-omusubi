# CLAUDE.md

## CRITICAL: Japanese Communication Required

**All communication with the user MUST be in Japanese.**
- Technical terms can remain in English
- Code comments must be in Japanese

## CRITICAL: Git Commit/Push Policy

**NEVER execute `git commit` or `git push` commands.**
- ❌ `git commit`, `git push`, `git commit --amend`, `git push --force`
- ✅ `git add`, `git rm`, `git mv`, `git status`, `git diff`, `git log`

## Project Overview

Omusubi is a lightweight, type-safe C++17 framework for embedded devices (M5Stack).

**Key Characteristics:**
- Zero-overhead abstractions (no heap, no exceptions)
- C++17 with clang++
- `.hpp` = header-only, `.h` = declarations only
- Japanese comments standard

## Build & Test

```bash
make              # Build
make test         # Build and run all tests
make clean        # Clean
```

**Testing with doctest:**
- Auto-downloaded during devcontainer build
- `DOCTEST_CONFIG_NO_EXCEPTIONS` enabled
- Assertions: `CHECK()`, `CHECK_EQ()`, `CHECK_FALSE()`, `REQUIRE()`

```cpp
#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <omusubi/omusubi.h>

TEST_CASE("Component - 基本機能") {
    SUBCASE("操作") {
        auto result = some_function();
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 42);
    }
}
```

## Code Quality

**clang-format/clang-tidy:**
- Format: `clang-format -i file.cpp`
- Lint: `clang-tidy file.cpp -- -Iinclude -std=c++17`
- Naming: Classes=`PascalCase`, functions/vars=`snake_case`, constants=`UPPER_CASE`, private members=`snake_case_`

**Always fix all Warnings and Hints.** Common fixes:
- Add `[[nodiscard]]` to getters and pure functions
- Name all parameters
- Use `override` instead of `virtual` in derived classes
- Add `const` to non-mutating methods
- Use `= default` for trivial constructors/destructors
- Remove `else` after `return`

## Architecture

**Method Chain Pattern:**
```cpp
ctx->get_connectable_context()->get_serial_context(0)->write("Hello"sv);
```

**Layer Structure:**
1. **Interface** (`interface/`): `*able` interfaces (ISP)
2. **Context** (`context/`): DI container for devices
3. **Device** (`device/`): Concrete implementations
4. **Platform** (`platform/`): Platform-specific (M5Stack, etc.)

**Design Rules:**
- No heap allocation (`new`, `delete`, `malloc`, `free` prohibited)
- Abstract operations only, not data structures
- All getters must be `const`
- Context getters: `get_*_context()` format
- Use `get_system_context()` free function

## Coding Conventions

**C++17 Standard:**
- Use `constexpr` whenever possible
- Prefer `if constexpr` over template specialization
- Prefer `auto` for returns, explicit types for numeric literals

**Prohibited:**
- Macros (use `constexpr`)
- `std::move()` (prevents RVO)
- C-style functions (`printf`, `strlen`, `memcpy`, etc.)

**C-Style Functions → C++ Alternatives:**
```cpp
// ❌ printf/sprintf → ✅ format() + std::cout
std::cout << format("Value: {}\n", value).c_str();

// ❌ strlen/strcpy/strcmp → ✅ std::string_view
sv.size();  sv1 == sv2;

// ❌ memcpy/memset → ✅ std::copy/std::fill
std::copy(src, src + size, dst);
```

**String Formatting:**
```cpp
// ❌ Multiple writes or manual concatenation
// ✅ Always use format()
auto str = format("[{}] {}\r\n", level, msg);
```

**Line Endings:**
- Serial/UART: `\r\n`
- File/Console: `\n`

## Core Types

**文字列・バッファ:**
- `std::string_view`, `FixedString<N>`, `FixedBuffer<N>`

**コンテナ:**
- `span<T>` - 非所有メモリビュー
- `StaticVector<T, N>` - 固定容量の可変長配列
- `RingBuffer<T, N>` - 固定長リングバッファ（FIFO）

**関数・コールバック:**
- `Function<Sig, Size>` - ヒープレス型消去コールバック

**エラーハンドリング:**
- `Result<T, E>` - Rust風エラー型（constexpr対応）
- `std::optional<T>` - 値の有無表現

**数値・数学:**
- `Vector3` - 3次元ベクトル
- `clamp()`, `map_range()`, `lerp()` - 数学ユーティリティ

**パース:**
- `parse_int<T>()`, `parse_uint<T>()`, `parse_float<T>()` - 文字列→数値変換

**フォーマット:**
- `format()` - 型安全な文字列フォーマット

## Usage Pattern

```cpp
#include <omusubi/omusubi.h>
using namespace omusubi;
using namespace std::literals;

SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void setup() {
    ctx.begin();
    serial = ctx.get_connectable_context()->get_serial_context(0);
}

void loop() {
    ctx.update();
    serial->write("Hello"sv);
    ctx.delay(10);
}
```

## Documentation

When creating docs in `docs/`:
1. Add version header: `**Version:** 1.0.0` / `**Last Updated:** YYYY-MM-DD`
2. Update related docs and increment patch versions
3. Maintain consistent terminology
