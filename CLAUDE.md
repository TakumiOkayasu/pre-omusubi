# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## CRITICAL: Japanese Communication Required

**All communication with the user MUST be in Japanese.**

- All responses must be in Japanese
- Comments and explanations must be in Japanese
- Code comments must be in Japanese
- Technical terms can remain in English, but explanations must be in Japanese

**English responses to the user are not acceptable. This is a strict requirement.**

## Response Quality Standards

**When answering questions or addressing uncertainties:**

- **Think deeply before responding** - Consider the question from at least 3 different angles
- **Verify technical accuracy** - Cross-check against documentation, code, and established patterns
- **Provide comparative analysis** - If multiple approaches exist, explain trade-offs
- **Use concrete examples** - Abstract explanations should be accompanied by code examples
- **Cite evidence** - Reference specific files, line numbers, or documentation when making claims

**Thought process should cover:**
1. **Literal interpretation** - What is the user explicitly asking?
2. **Underlying intent** - What problem are they trying to solve?
3. **Contextual factors** - How does this relate to the project's architecture and constraints?

**Example:**
```
User: "Should I use size_t or uint32_t for loop counters?"

Thought 1: Literal - User wants to know which type to use
Thought 2: Intent - They likely care about consistency, performance, or platform compatibility
Thought 3: Context - This project uses uint32_t for fixed containers due to embedded system constraints

Response: このプロジェクトではループカウンタに uint32_t を使用してください。理由は...
```

**This ensures responses are:**
- Accurate and well-reasoned
- Aligned with project conventions
- Helpful for understanding the "why" behind decisions

## Project Overview

Omusubi is a lightweight, type-safe C++14 framework for embedded devices (primarily M5Stack). It provides a clean architecture centered around `SystemContext` for unified hardware access, with complete interface/implementation separation for hardware-independent code.

**Key Characteristics:**
- Zero-overhead abstractions (no dynamic allocation, no exceptions)
- C++14 with clang++
- Header-only implementations use `.hpp`, declarations use `.h`
- Japanese comments and documentation are standard

## Development Environment

### Dev Container

This project is developed in a Dev Container environment.

**Environment Specifications:**
- **Base Image:** `ubuntu:latest`
- **Compiler:** Clang/Clang++ (C++14 support, installed via apt)
- **Build Tools:** Make, CMake, Ninja
- **Debug Tools:** GDB, Valgrind
- **Code Quality Tools:** clang-format, clang-tidy, clangd, bear
- **Locale:** ja_JP.UTF-8 (Japanese environment)
- **Timezone:** Asia/Tokyo

**VS Code Extensions:**
- GitLens
- Claude Code (`ghcr.io/anthropics/devcontainer-features/claude-code:1.0.5`)
- Docker outside of Docker

**Getting Started:**
```bash
# Open in VS Code
code .

# Select "Reopen in Container"
# → Dev Container environment will be automatically built
```

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

## Code Quality Tools

### Formatter: clang-format

**Automatic formatting on save:**
- Enabled in your global VS Code settings
- Formats C++ code automatically when you save a file
- Default formatter is set to clang-format for C++ files

**Manual formatting:**
```bash
# Format a single file
clang-format -i path/to/file.cpp

# Format all C++ files in the project
find include src -name "*.h" -o -name "*.hpp" -o -name "*.cpp" | xargs clang-format -i
```

**VS Code shortcuts:**
- `Shift + Alt + F` (Linux/Windows) or `Shift + Option + F` (Mac) - Format current file
- Right-click → "Format Document"

**Configuration:**
- Style defined in `.clang-format` at project root
- Based on LLVM style with customizations for embedded C++ development
- Enforces consistent naming conventions and indentation
- Format on save is enabled globally (not in devcontainer settings)

### Linter: clang-tidy

**Real-time linting:**
- Integrated with clangd language server
- Shows warnings and suggestions as you type
- Appears as squiggly lines in the editor

**Manual linting:**
```bash
# Lint a single file
clang-tidy path/to/file.cpp -- -Iinclude -std=c++14

# Lint all source files
find src -name "*.cpp" | xargs -I {} clang-tidy {} -- -Iinclude -std=c++14
```

**Configuration:**
- Rules defined in `.clang-tidy` at project root
- Enforces:
  - Bug-prone pattern detection
  - Modern C++ best practices (C++14)
  - Performance optimizations
  - Readability guidelines
  - Embedded-specific constraints (no heap allocation, etc.)

**Key checks enabled:**
- `bugprone-*` - Detect common bugs
- `cert-*` - CERT secure coding guidelines
- `cppcoreguidelines-*` - C++ Core Guidelines
- `performance-*` - Performance improvements
- `readability-*` - Code readability
- `modernize-*` - Modern C++14 idioms

**Naming conventions enforced:**
- Classes/Structs: `CamelCase`
- Functions/variables: `snake_case`
- Constants/Enums: `UPPER_CASE`
- Private members: `snake_case_` (trailing underscore)
- Namespaces: `snake_case`

### Best Practices

**Before committing code:**
1. Ensure all files are formatted (automatic on save)
2. Review and fix all clang-tidy warnings
3. Run `make` to verify compilation
4. Check for any diagnostic messages in VS Code

**Ignoring specific warnings:**
```cpp
// NOLINTNEXTLINE(rule-name)
code_that_needs_exception;

// NOLINT
line_to_ignore;
```

Use sparingly and only when absolutely necessary with clear justification.

## Architecture: Method Chain Design

### Core Design Philosophy

**SystemContext is a gateway to organized device access through method chaining.**

Basic hardware access follows this pattern:
```cpp
get_system_context()->get_[category]_context()->get_[device]_context()->method()
```

Complex devices may extend beyond this with additional component layers (see Method Chain Patterns below).

Example (Basic 3-layer):
```cpp
// Bluetooth connection
ctx->get_connectable_context()->get_bluetooth_context()->connect()

// Serial reading (runtime)
ctx->get_readable_context()->get_serial_context(0)->read_line()

// Serial reading (compile-time)
ctx->get_readable_context()->get_serial_context<0>()->read_line()

// Accelerometer sensor
ctx->get_sensor_context()->get_accelerometer_context()->get_values()
```

Example (Hierarchical 4+ layers):
```cpp
// BLE with Service and Characteristic
ble->add_service(uuid)->add_characteristic(uuid, props)->write(data)

// Complex devices may extend beyond DeviceContext
```

### Method Chain Patterns

The framework supports two method chain patterns depending on device complexity:

**Pattern 1: Basic (3-layer) - Most Common**
```cpp
SystemContext → CategoryContext → DeviceContext → method()
```

Applies to simple devices:
```cpp
// Serial communication
ctx->get_connectable_context()->get_serial_context(0)->write("Hello"_sv);

// WiFi connection
ctx->get_connectable_context()->get_wifi_context()->connect_to(ssid, password);

// Sensor reading
ctx->get_sensor_context()->get_accelerometer_context()->get_values();
```

**Pattern 2: Hierarchical (4+ layers) - Component-based Devices**
```cpp
SystemContext → CategoryContext → DeviceContext → Component → method()
```

Applies to complex devices with sub-components:
```cpp
// BLE: Service and Characteristic
BLEContext* ble = ctx->get_connectable_context()->get_ble_context();
BLEService* service = ble->add_service(uuid);
BLECharacteristic* ch = service->add_characteristic(uuid, properties);
ch->write("Hello"_sv);

// Future: HTTP Server
HTTPServer* server = ctx->get_connectable_context()->get_http_server_context();
HTTPRoute* route = server->add_route("/api/data");
route->set_handler([](HTTPRequest& req) { /* ... */ });
```

**When to use hierarchical pattern:**
- Device consists of multiple dynamically created sub-components
- Components can be added/removed at runtime
- Each component has its own state and methods
- Components represent distinct domain concepts (Service/Characteristic, Route/Handler, etc.)

**Component Layer Naming Rules:**

1. **DO NOT use `*Context` suffix** - Component layer is NOT a Context
   ```cpp
   // ✅ Correct
   BLECharacteristic, BLEService, HTTPRequest, File

   // ❌ Wrong
   BLECharacteristicContext, BLEServiceContext
   ```

2. **Prefer domain-standard terminology**
   - BLE: Use official spec terms (`Service`, `Characteristic`)
   - HTTP: Use web standards (`Request`, `Response`, `Route`)
   - File system: Use OS terms (`File`, `Directory`)
   - Graphics: Use industry terms (`Sprite`, `Canvas`, `Layer`)

3. **Add device prefix when necessary**
   - Use prefix to avoid naming conflicts
   - Keep component names clear about ownership
   ```cpp
   BLECharacteristic  // Prefix avoids conflict with other "Characteristic" concepts
   HTTPRequest        // Clear it belongs to HTTP
   ```

4. **Use namespaces for organization (optional)**
   ```cpp
   namespace ble {
       class Service;
       class Characteristic;
   }
   // Usage: ble::Characteristic
   ```

**Key Principle:**
- Component layer represents **domain concepts**, not infrastructure
- Naming should reflect the **user's mental model** of the domain
- Prioritize **familiarity** over framework consistency

### Layer Structure

### 1. Interface Layer (`include/omusubi/interface/`)
**`*able` interfaces define single-responsibility contracts (following Interface Segregation Principle):**
- `Readable` - Reading data from devices
- `Writable` - Writing data to devices
- `Connectable` - Connection management (connect, disconnect, status)
- `Scannable` - Network/device scanning (start, stop, results)
- `Pressable` - Button state management
- `Measurable` / `Measurable3D` - Sensor measurements
- `Displayable` - Display output

**Interface Design Rule (ISP - Interface Segregation Principle):**

Each `*able` interface MUST represent a **single responsibility**, but may contain multiple related methods to fulfill that responsibility.

**Guidelines:**
- ✅ **Single Responsibility**: Interface focuses on ONE capability (reading, writing, connecting, etc.)
- ✅ **Related Methods**: Multiple methods that serve the same purpose are allowed
- ❌ **No Mixed Responsibilities**: Don't combine unrelated capabilities in one interface

**Examples:**

```cpp
// ✅ GOOD - Single responsibility: Scanning
// Multiple methods serve the scanning purpose
class Scannable {
    virtual void start_scan() = 0;
    virtual void stop_scan() = 0;
    virtual uint8_t get_found_count() const = 0;
    virtual FixedString<64> get_found_name(uint8_t index) const = 0;
    virtual int32_t get_found_signal_strength(uint8_t index) const = 0;
};

// ✅ GOOD - Single responsibility: Connection management
class Connectable {
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
};

// ✅ GOOD - Single responsibility: Reading
class Readable {
    virtual FixedBuffer<256> read() = 0;
    virtual uint32_t available() const = 0;
    virtual FixedString<256> read_line() = 0;
};

// ❌ BAD - Multiple responsibilities mixed
class NetworkDevice {
    virtual bool connect() = 0;      // Connection responsibility
    virtual uint8_t scan() = 0;      // Scanning responsibility
    virtual FixedBuffer<256> read() = 0;  // Reading responsibility
};
// Instead: Inherit from Connectable + Scannable + Readable
```

**When to add methods to an interface:**
- Methods directly support the interface's core responsibility
- Methods are commonly used together
- Separating them would make the API awkward

**When to create a new interface:**
- The capability is independent and can be used alone
- Not all implementers need the capability
- The responsibility is conceptually distinct

### 2. Context Layer (`include/omusubi/context/`)
**Middle-tier contexts group devices by category and serve as DI containers:**

The Context layer is not just for grouping - it functions as a **Dependency Injection (DI) container**:

**DI Container Responsibilities:**
- Device creation and ownership management
- Mock substitution for testing
- Platform switching abstraction
- Dependency management between devices

**Contexts by Category:**

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

### 4. Component Layer (`include/omusubi/interface/`) - Optional for Complex Devices
**Sub-components for hierarchical devices (see Method Chain Patterns):**
- `BLECharacteristic` (Readable + Writable) - BLE characteristic
- `BLEService` - BLE service container
- Future: `HTTPRequest`, `HTTPResponse`, `File`, `Directory`, etc.

**Important:** Component layer does NOT use `*Context` naming. Uses domain-standard terminology instead.

### 5. Platform Layer (`include/omusubi/platform/`, `src/platform/`)
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
    mutable M5StackSerialContext serials_[3]{
        M5StackSerialContext(0),
        M5StackSerialContext(1),
        M5StackSerialContext(2)
    };
    mutable M5StackBluetoothContext bluetooth_;
    mutable M5StackWiFiContext wifi_;
    mutable M5StackBLEContext ble_;

public:
    SerialContext* get_serial_context(uint8_t port) const override {
        return (port < 3) ? &serials_[port] : nullptr;
    }
    uint8_t get_serial_count() const override { return 3; }
    BluetoothContext* get_bluetooth_context() const override { return &bluetooth_; }
    WiFiContext* get_wifi_context() const override { return &wifi_; }
    BLEContext* get_ble_context() const override { return &ble_; }
};
```

**SystemContext Implementation Pattern:**

Platform implementation classes follow this pattern:

```cpp
// Platform implementation (users do not directly use this)
class M5StackSystemContext : public SystemContext {
private:
    // Constructor can be private or public (either works)
    // ⚠️ No need for get_instance() or other static methods
    M5StackSystemContext();

public:
    // SystemContext interface implementation
    void begin() override;
    void update() override;
    // ...
};

// Platform-specific implementation file (src/platform/m5stack/system_context.cpp)
namespace omusubi {
    // Created as static local variable in free function (Meyers Singleton)
    SystemContext& get_system_context() {
        static platform::m5stack::M5StackSystemContext inst;
        return inst;
    }
}
```

**Constructor Access Specifier:**
- `private`: More strict (prevents direct instantiation from elsewhere)
- `public`: Simpler (assumes no instantiation outside `get_system_context()`)
- Either works, but `public` is simpler

**Implement the same pattern for other platforms:**

**IMPORTANT:** Always implement `get_system_context()` using this pattern:

```cpp
SystemContext& get_system_context() {
    static [DeviceName]SystemContext inst;
    return inst;
}
```

Implementation examples for each platform:

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

**Benefits of this pattern:**
- Simple and consistent
- Leverages C++11 thread-safe static initialization (Meyers Singleton)
- No need for `get_instance()` method in platform implementation classes
- Singleton implementation is centralized in `get_system_context()`

**Build system switching:**
- Only the target platform's `system_context.cpp` is linked at build time
- User code remains unchanged (always just calls `get_system_context()`)

**Key principles:**
- User code only uses the `get_system_context()` free function
- `XXXSystemContext::get_instance()` is an internal implementation detail
- User code remains unchanged when switching platforms

**Platform-specific implementation hiding pattern:**

To hide platform-specific types completely from headers:

```cpp
// ❌ Bad: Header exposes implementation details
class M5StackBluetoothContext : public BluetoothContext {
private:
    void* impl_;  // Still exposes pointer
    struct Impl;  // Still exposes type name
};

// ✅ Good: Header has NO implementation details
class M5StackBluetoothContext : public BluetoothContext {
public:
    M5StackBluetoothContext();
    ~M5StackBluetoothContext() override;
    // Methods only
};
```

```cpp
// Implementation file (.cpp) - Platform-specific details hidden
namespace {
// Anonymous namespace - not visible outside this file
struct BluetoothImpl {
    BluetoothSerial bt;  // Platform-specific type
    bool connected;
    // ... implementation details
};

// Static variable - single instance
BluetoothImpl impl;
}  // namespace

void M5StackBluetoothContext::write(StringView text) {
    impl.bt.write(/*...*/);  // Direct access to static variable
}
```

**Benefits:**
- Headers contain zero platform-specific types
- No `void*` pointers or forward declarations needed
- Simple direct access to static variables
- Perfect for singleton contexts (Bluetooth, WiFi, BLE, etc.)

**For multi-instance contexts (Serial ports):**
```cpp
// Header: Minimal identification only
class M5StackSerialContext : public SerialContext {
private:
    uint8_t port_;  // Identification only
public:
    explicit M5StackSerialContext(uint8_t port);
};

// Implementation: Static array of instances
namespace {
struct SerialImpl { /*...*/ };
SerialImpl impl_port0(0);
SerialImpl impl_port1(1);
SerialImpl impl_port2(2);

SerialImpl& get_impl(uint8_t port) {
    switch (port) {
        case 1: return impl_port1;
        case 2: return impl_port2;
        default: return impl_port0;
    }
}
}  // namespace
```

### Critical Design Rules

**1. No Heap Allocation**
- All objects MUST be stack-allocated or use placement new with static buffers
- Never use `new`, `delete`, `malloc`, `free`, `std::unique_ptr`, `std::shared_ptr`
- Use static buffers with placement new when dynamic construction is needed

**1.5. Abstraction Policy**

**Operations (verbs) can be abstracted; Structures (data, task definitions) should NOT be abstracted.**

- ✅ **Operations abstraction (Acceptable):** Abstract actions like `deploy`, `rollback`, `connect`, `disconnect`, `write`, `read` through interfaces
  ```cpp
  // ✅ Good - Operation abstraction
  class Writable {
      virtual size_t write(span<const uint8_t> data) = 0;
  };
  ```

- ❌ **Structure abstraction (High cost, leaky):** Attempting to abstract data structures or task definitions inevitably leads to abstraction leakage
  ```cpp
  // ❌ Bad - Data structure abstraction causes leakage
  class Container {
      virtual void* get_data() = 0;  // Forces concrete details to leak
      virtual size_t get_capacity() = 0;
  };

  // ❌ Bad - Task definition abstraction
  class TaskConfig {
      virtual void set_parameter(const char* key, const char* value) = 0;
      // Concrete task requirements will leak through
  };
  ```

**Rationale:**
- Operations have well-defined contracts that remain stable
- Data structures and task definitions have concrete requirements that leak through any abstraction layer
- Concrete types (like `FixedString<N>`, `Vector3`) avoid abstraction overhead and leakage

**2. Context Getters: Two Access Patterns**

The Context layer functions as a DI container and supports two access patterns:

**Pattern A: Runtime Parameters**
```cpp
SerialContext* serial = ctx.get_connectable_context()->get_serial_context(0);
```
- ✅ Flexible for loops and dynamic port selection
- ✅ Follows C++ getter convention with `get_` prefix
- ✅ Clear indication that a Context object is returned

**Pattern B: Template Parameters (Compile-time)**
```cpp
SerialContext* serial = ctx.get_connectable_context()->get_serial_context<0>();
```
- ✅ Optimized at compile time
- ✅ Supports template metaprogramming
- ⚠️ Port number must be compile-time constant

**For single-instance devices:**
```cpp
BluetoothContext* bt = ctx.get_connectable_context()->get_bluetooth_context();
WiFiContext* wifi = ctx.get_connectable_context()->get_wifi_context();
BLEContext* ble = ctx.get_connectable_context()->get_ble_context();
```
- ✅ Consistent `get_*_context()` naming pattern
- ✅ Aligns with C++ naming conventions
- ✅ Type name matches method name

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

**Context Getter Naming (Strict Rule):**
All Context getter methods MUST follow this exact pattern:
```cpp
get_*_context()  // Required format
```
- ✅ `get_` prefix (C++ getter convention)
- ✅ `_context` suffix (matches return type)
- ❌ No exceptions allowed

Examples:
```cpp
// ✅ Correct
SerialContext* get_serial_context(uint8_t port) const;
BluetoothContext* get_bluetooth_context() const;

// ❌ Wrong - missing get_ prefix
SerialContext* serial_context(uint8_t port) const;

// ❌ Wrong - missing _context suffix
SerialContext* get_serial(uint8_t port) const;
```

**5. Same-Type Getter Prohibition (CRITICAL)**

Context classes MUST NOT contain multiple getter methods that return the same Context type. Use parameters or templates to distinguish instances.

```cpp
// ❌ PROHIBITED - Same return type
class SampleContext {
public:
    virtual HogeContext* get_hoge_1_context() const = 0;
    virtual HogeContext* get_hoge_2_context() const = 0;
};

// ✅ CORRECT - Parameter-based access
class SampleContext {
public:
    virtual HogeContext* get_hoge_context(uint8_t index) const = 0;
    virtual uint8_t get_hoge_count() const = 0;

    // Optional: Template version for compile-time
    template<uint8_t Index>
    HogeContext* get_hoge_context() const {
        return get_hoge_context(Index);
    }
};
```

**Rationale:**
- DRY (Don't Repeat Yourself) principle
- Scalability (no method explosion)
- Type safety (compiler can validate parameter ranges)
- Maintainability

**6. SystemContext Access: Free Function Pattern**

**IMPORTANT:** Always access SystemContext using the **free function** `get_system_context()`.

```cpp
// Free function (global namespace)
SystemContext& get_system_context();
```

**Usage:**
```cpp
// ✅ Correct: Use free function
SystemContext& ctx = get_system_context();

// ❌ Wrong: Not a class static method
SystemContext& ctx = SystemContext::get_instance();  // This does not exist
```

**Implementation details (users don't need to worry about this):**
- Platform implementations (M5StackSystemContext, etc.) may use Singleton pattern internally
- The `get_system_context()` free function calls platform-specific implementation internally
- This ensures user code remains unchanged when switching platforms

**7. SystemContext Core Responsibilities**
```cpp
class SystemContext {
public:
    // System control (stays in SystemContext)
    virtual void begin() = 0;
    virtual void update() = 0;
    virtual void delay(uint32_t ms) = 0;
    virtual void reset() = 0;

    // Category context access (all must have const)
    virtual ConnectableContext* get_connectable_context() const = 0;
    virtual ReadableContext* get_readable_context() const = 0;
    virtual WritableContext* get_writable_context() const = 0;
    virtual ScannableContext* get_scannable_context() const = 0;
    virtual SensorContext* get_sensor_context() const = 0;
    virtual InputContext* get_input_context() const = 0;
    virtual OutputContext* get_output_context() const = 0;
    virtual SystemInfoContext* get_system_info_context() const = 0;
    virtual PowerContext* get_power_context() const = 0;
};
```

**Violation Warning:**
If code does not follow these design rules, it violates the framework's architecture philosophy and must be refactored.

## Usage Pattern

### Performance-Critical: Cache Context Pointers

Users retrieve device pointers once in `setup()` and reuse them to avoid method call overhead:

```cpp
// Cache in global variables (performance-oriented)
SystemContext& ctx = get_system_context();
ConnectableContext* connectable = nullptr;
BluetoothContext* bt = nullptr;

void setup() {
    ctx.begin();

    // Retrieve once and cache
    connectable = ctx.get_connectable_context();
    bt = connectable->get_bluetooth_context();
}

void loop() {
    ctx.update();

    // Use cached pointer directly (no overhead)
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
        // Retrieve and hold Context in constructor
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

- **C++ Standard:** ALL code MUST conform to C++14. Actively use C++14 features when generating code:
  - Extended `constexpr` (functions with multiple statements, loops, conditionals)
  - Generic lambdas with `auto` parameters
  - Return type deduction with `auto`
  - Binary literals (`0b1010`) and digit separators (`1'000'000`)
  - `std::make_unique` for rare heap allocations (though heap allocation is generally prohibited)
  - Variable templates
- **constexpr Usage (CRITICAL RULE):**
  - **ALWAYS use `constexpr` whenever possible** - This is a strict requirement
  - Apply `constexpr` to all functions, constructors, and methods that can be evaluated at compile time
  - Benefits:
    - Compile-time computation reduces runtime overhead
    - Smaller binary size
    - Type safety improvements
    - Enables use in constant expressions and `static_assert`
  - Examples:
    ```cpp
    // ✅ All possible functions are constexpr
    constexpr uint32_t byte_length() const noexcept { return byte_length_; }
    constexpr bool append(StringView view) noexcept { /* ... */ }
    constexpr StringView view() const noexcept { return StringView(buffer_, byte_length_); }

    // ✅ Constructors
    constexpr FixedString() noexcept : byte_length_(0) { buffer_[0] = '\0'; }

    // ✅ Free functions
    constexpr FixedString<N - 1> fixed_string(const char (&str)[N]) noexcept { /* ... */ }
    ```
  - Only omit `constexpr` when:
    - Function has side effects (I/O, hardware access, etc.)
    - Function cannot be evaluated at compile time due to technical limitations
    - Provide clear justification in comments if `constexpr` is not used
- **Type Deduction with `auto`:**
  - **Prefer `auto` for variable declarations** unless there is a specific reason to use explicit types
  - `auto` improves maintainability and reduces coupling to specific types
  - Use explicit types only when:
    - Type clarity is critical for readability (e.g., numeric literals: `uint32_t count = 0;`)
    - Interfacing with C APIs or hardware registers
    - Explicit type conversion is needed for correctness
  - **Numeric literals:** Use explicit types for bit width/signedness clarity in embedded systems
    ```cpp
    // ✅ Preferred: explicit type for numeric literals
    uint32_t retry_count = 0;    // Clear bit width and signedness
    uint8_t port_number = 1;     // Clear 8-bit unsigned

    // ⚠️ Alternative: auto with suffix/initialization (use with caution)
    auto retry_count = 0U;           // unsigned int (platform-dependent size)
    auto retry_count = uint32_t{0};  // uint32_t (explicit)

    // ✗ Avoid: auto without suffix
    auto retry_count = 0;  // int (wrong type - signed, platform-dependent)
    ```
  - Examples:
    ```cpp
    // ✅ Preferred: auto for function returns
    auto str = static_string("Hello");
    auto view = str.view();
    auto ctx = get_system_context();

    // ✅ Explicit type for numeric literals
    uint32_t retry_count = 0;

    // ✅ auto for literals when type is obvious
    auto device_name = "M5Stack";  // const char* (string literal)
    auto is_enabled = true;        // bool
    auto separator = ',';          // char
    auto pi = 3.14159f;            // float (with suffix)

    // ✗ Avoid: unnecessary explicit types for function returns
    StaticString<5> str = static_string("Hello");  // auto is better
    StringView view = str.view();                  // auto is better
    ```
- **Naming:**
  - Files: `snake_case.h`, `snake_case.hpp`, `snake_case.cpp`
  - Functions/variables: `snake_case`
  - Classes: `PascalCase`
  - Namespaces: `snake_case`
  - Constants: `UPPER_CASE_WITH_UNDERSCORES`
  - Enum class values: `UPPER_CASE_WITH_UNDERSCORES` (treated as immutable constants)
  - Component layer: Domain-standard terminology (e.g., `BLECharacteristic`, `HTTPRequest`), NOT `*Context`
- **Macros:** Prohibited - Use `constexpr` functions and variables instead
- **constexpr Usage:** Prefer `constexpr` wherever possible to move runtime computations to compile-time. This reduces runtime overhead and binary size. Examples: `constexpr` constructors, `constexpr` member functions in `span<T>`, `FixedString<N>`, etc.
- **String literals:** Use `_sv` suffix (requires `using namespace omusubi::literals`)
- **Header guards:** Use `#pragma once`
- **Memory:** No heap allocation - stack or placement new with static buffers only
- **`std::move()` Usage:** DO NOT use `std::move()` in Omusubi code
  - All objects are small and stack-allocated (no heap)
  - Copy cost is negligible (pointers + small data)
  - `std::move()` can prevent RVO/NRVO optimizations
  - Simply return values - compiler will optimize
  - Example:
    ```cpp
    // ✅ Correct: simple return (RVO applies)
    StringView get_view() {
        return "Hello"_sv;
    }

    // ✗ Wrong: std::move() is unnecessary and harmful
    StringView get_view() {
        StringView view = "Hello"_sv;
        return std::move(view);  // Prevents RVO
    }
    ```
- **Getter Methods:** ALL getter methods MUST be marked `const`. Example: `HogeContext* get_hoge_context() const = 0;`. If a getter cannot be const, you MUST provide a clear justification explaining why
- **Implementation Hiding:** When hiding platform-specific implementation details, do NOT expose implementation in headers (no `void* impl_`, no `struct Impl;` forward declarations). Place all implementation details in `.cpp` files using anonymous namespaces with static variables
- **Static Variables in Implementation Files:** File-scope static variables in `.cpp` files must use the `static` keyword explicitly, even inside anonymous namespaces. Example: `static BluetoothImpl impl;` not just `BluetoothImpl impl;`
- **Comments:** Do not write unnecessary comments. Only add comments when they provide essential information that cannot be inferred from the code itself. Implementation details should be self-evident from the code structure

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

// Global variables: retrieve once in setup(), reuse in loop()
SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void setup() {
    // Initialize system
    ctx.begin();

    // Retrieve device (once only)
    serial = ctx.get_connectable_context()->get_serial_context(0);

    // [Setup logic with comments]
}

void loop() {
    // Update system
    ctx.update();

    // [Loop logic with comments]

    ctx.delay(10);
}
```

**Example guidelines:**
- Include clear Japanese comments explaining purpose and key steps
- Use method chain: `ctx.get_[category]_context()->get_[device]_context()`
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

## Documentation Management

**CRITICAL: When creating new documentation, you MUST update existing related documents to maintain consistency.**

### Workflow for Creating New Documentation

1. **Pre-Creation: Survey Existing Documentation**
   ```bash
   ls docs/  # List all existing documentation
   grep -r "related_concept" docs/  # Search for related content
   ```
   - Read all files in `docs/` directory
   - Identify documents that cover related topics
   - Note any conflicting or overlapping content

2. **Create New Documentation**
   - Write the new document with clear structure
   - Add version and date at the end (see format below)

3. **Post-Creation: Update Existing Documents** ⚠️ MANDATORY
   - **Update all related documents** based on the new document's content
   - Add cross-references to the new document
   - Resolve any contradictions or overlapping explanations
   - Increment version numbers of updated documents (Patch: 0.0.X)
   - Update "Last Updated" date

### Version and Date Tracking (REQUIRED FOR ALL DOCS)

**Every documentation file in `docs/` MUST end with:**
```markdown
---

**Version:** 1.0.0
**Last Updated:** 2025-11-16
```

**Version Increment Rules:**
- **Major (X.0.0)**: Breaking changes, complete rewrites, or removed sections
- **Minor (0.X.0)**: New sections or significant content additions
- **Patch (0.0.X)**: Corrections, clarifications, cross-reference updates, minor edits

**When to increment:**
- Creating new doc → Start at `1.0.0`
- Adding cross-reference from existing doc → Increment patch version
- Updating existing doc with new concepts → Increment minor version
- Major refactoring → Increment major version

### Documentation Structure

- `docs/` - Technical documentation, guides, and specifications
- `CLAUDE.md` - AI assistant guidelines and project conventions (no version needed)
- `README.md` - User-facing project overview and quick start (no version needed)

### Consistency Rules

1. **Terminology**: Use identical terms across all documents
2. **Cross-References**: Link related documents with relative paths
3. **Update Cascade**: When updating one document, check if others need updates
4. **No Contradictions**: New content must not contradict existing documentation

**Example: Creating `docs/auto_usage_guide.md`**
```bash
# 1. Check existing docs
ls docs/  # Found: cpp14_compatibility.md, type_system_unification.md

# 2. Read existing docs to find related content
# Found: type_system_unification.md mentions uint32_t vs size_t

# 3. Create new document: docs/auto_usage_guide.md
# (Include version 1.0.0 at the end)

# 4. Update existing documents:
# - Add cross-reference in type_system_unification.md to auto_usage_guide.md
# - Increment version: 1.0.0 → 1.0.1
# - Update date: 2025-11-16
```

# Embedded Development Design Principles

## Addressing Static Writable Area Constraints

In embedded systems, it is often impossible to link static writable areas into executables.
(Examples: BREW, Android Application Context, etc.)

In environments with such constraints, designs that depend on static or global variables are not viable.

### Solution: Context Pattern Design

**Design Principles**
- Access data through interfaces
- Divide interfaces by category and expose only necessary functionality
- Obtain required interfaces from a parent instance (Context)
- Hide interface implementations from callers

**Design Benefits**
- Interfaces themselves have code efficiency nearly equivalent to direct access in executables
- Forward declarations minimize impact on unrelated code
- Implementation is independent of data placement (static area, dynamic area, external memory, etc.)
- Interfaces can be nested to build hierarchical structures as needed
- Interfaces hide whether data is a single block or has overlapping parts

**Important Notes**
- Singleton is an implementation choice for the implementation side
- The caller should not expect the same instance to always be returned
- The implementation side decides to adopt it for benefits like reduced creation costs

**Reference Designs**
- Android Application Context
- Design Pattern: Abstract Factory Pattern

This design pattern is a guideline for balancing flexibility and efficiency in memory-constrained embedded environments.
