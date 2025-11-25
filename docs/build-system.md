# ビルドシステムガイド

このドキュメントでは、Omusubiフレームワークのビルドシステムと構成方法を定義します。

## 目次

1. [ビルドシステム概要](#ビルドシステム概要)
2. [Makefile](#makefile)
3. [コンパイラオプション](#コンパイラオプション)
4. [ビルド構成](#ビルド構成)
5. [クロスコンパイル](#クロスコンパイル)
6. [PlatformIO](#platformio)
7. [CI/CD](#cicd)

---

## ビルドシステム概要

### 1. サポートするビルドシステム

**Omusubiは複数のビルドシステムをサポート:**

- **Make** - ホスト環境での開発・テスト (推奨)
- **PlatformIO** - 組み込みターゲット (M5Stack, Arduino, etc.)
- **CMake** - 大規模プロジェクト (オプション)

### 2. ビルドターゲット

**ホスト環境:**
- `make` - デフォルトビルド
- `make test` - テスト実行
- `make clean` - クリーンアップ
- `make rebuild` - リビルド

**組み込み環境 (PlatformIO):**
- `pio run` - ビルド
- `pio run --target upload` - アップロード
- `pio test` - テスト実行
- `pio device monitor` - シリアルモニタ

---

## Makefile

### 1. 基本構成

```makefile
# Compiler and flags
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Target executable
TARGET = main

# Source files
MAIN_SRC = main.cpp
LIB_SRCS = $(shell find $(SRC_DIR) -type f -name '*.cpp')

# Header files
HEADERS = $(shell find $(INC_DIR) -type f \( -name '*.h' -o -name '*.hpp' \))

# Object files (maintain directory structure in obj/)
LIB_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(LIB_SRCS))

# Default target
all: $(TARGET)

# Link main.cpp with library object files to create executable
$(TARGET): $(MAIN_SRC) $(LIB_OBJS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(MAIN_SRC) $(LIB_OBJS)

# Compile library source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Rebuild everything
rebuild: clean all

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run
```

### 2. ビルド構成の切り替え

**デバッグビルドとリリースビルド:**

```makefile
# Build configuration (debug or release)
BUILD ?= debug

ifeq ($(BUILD),debug)
    CXXFLAGS += -g -O0 -DDEBUG
else ifeq ($(BUILD),release)
    CXXFLAGS += -O2 -DNDEBUG
else
    $(error Unknown BUILD configuration: $(BUILD))
endif

# Usage:
# make BUILD=debug    # デバッグビルド
# make BUILD=release  # リリースビルド
```

### 3. テストビルド（doctest使用）

```makefile
# Test configuration with doctest
TEST_DIR = tests
BIN_DIR = bin

# Tests in tests/core/ directory
CORE_TESTS = test_result test_logger
CORE_TEST_BINS = $(patsubst %,$(BIN_DIR)/%,$(CORE_TESTS))

# Tests in tests/ directory
BASIC_TESTS = test_auto_capacity test_format test_format_string test_fixed_string test_fixed_buffer test_span test_string_view test_vector3
BASIC_TEST_BINS = $(patsubst %,$(BIN_DIR)/%,$(BASIC_TESTS))

# All test binaries
ALL_TEST_BINS = $(CORE_TEST_BINS) $(BASIC_TEST_BINS)

# Build all tests
tests: $(ALL_TEST_BINS)

# Build tests in tests/ directory
$(BIN_DIR)/test_%: $(TEST_DIR)/test_%.cpp $(TEST_DIR)/doctest.h $(HEADERS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Run all tests
test: tests
	@for test in $(ALL_TEST_BINS); do \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done

.PHONY: tests test
```

**doctest の特徴:**
- ヘッダーオンリー（`test/doctest.h`）
- 例外なしモード対応（`DOCTEST_CONFIG_NO_EXCEPTIONS`）
- 高速コンパイル
- 各テストファイルは独立した実行可能ファイルとしてビルド

---

## コンパイラオプション

### 1. 基本オプション

```makefile
# C++ standard
CXXFLAGS += -std=c++17

# Warnings
CXXFLAGS += -Wall -Wextra -Wpedantic

# Include paths
CXXFLAGS += -Iinclude

# Optimization (release)
CXXFLAGS += -O2

# Debug symbols (debug)
CXXFLAGS += -g
```

### 2. 組み込み向けオプション

```makefile
# Embedded-specific flags
CXXFLAGS += -fno-exceptions   # 例外無効化
CXXFLAGS += -fno-rtti          # RTTI無効化
CXXFLAGS += -ffunction-sections  # 関数ごとにセクション分割
CXXFLAGS += -fdata-sections      # データごとにセクション分割

# Linker flags
LDFLAGS += -Wl,--gc-sections   # 未使用セクション削除
```

### 3. 最適化レベル

```makefile
# -O0: 最適化なし (デバッグ用)
CXXFLAGS_DEBUG = -O0 -g

# -O1: 基本的な最適化
CXXFLAGS_OPT1 = -O1

# -O2: 標準的な最適化 (推奨)
CXXFLAGS_OPT2 = -O2

# -O3: 積極的な最適化 (コードサイズ増大)
CXXFLAGS_OPT3 = -O3

# -Os: サイズ優先の最適化 (組み込み推奨)
CXXFLAGS_SIZE = -Os
```

### 4. LTO (Link Time Optimization)

```makefile
# LTO有効化
CXXFLAGS_LTO = -flto
LDFLAGS_LTO = -flto

# Usage:
# make CXXFLAGS="$(CXXFLAGS_LTO)" LDFLAGS="$(LDFLAGS_LTO)"
```

---

## ビルド構成

### 1. ディレクトリ構造

```
omusubi/
├── include/           # ヘッダーファイル
│   └── omusubi/
│       ├── core/      # コア型定義
│       ├── interface/ # インターフェース
│       ├── context/   # コンテキスト
│       ├── device/    # デバイスコンテキスト
│       └── platform/  # プラットフォーム実装
├── src/               # 実装ファイル
│   └── platform/
│       ├── m5stack/   # M5Stack実装
│       ├── arduino/   # Arduino実装
│       └── pico/      # Raspberry Pi Pico実装
├── tests/             # テストコード
├── examples/          # サンプルコード
├── docs/              # ドキュメント
├── obj/               # オブジェクトファイル (生成)
├── Makefile           # ビルド設定
└── README.md
```

### 2. プラットフォーム切り替え

```makefile
# Platform selection
PLATFORM ?= m5stack

# Platform-specific source files
ifeq ($(PLATFORM),m5stack)
    PLATFORM_SRCS = $(shell find src/platform/m5stack -name '*.cpp')
else ifeq ($(PLATFORM),arduino)
    PLATFORM_SRCS = $(shell find src/platform/arduino -name '*.cpp')
else ifeq ($(PLATFORM),pico)
    PLATFORM_SRCS = $(shell find src/platform/pico -name '*.cpp')
else
    $(error Unknown PLATFORM: $(PLATFORM))
endif

# Include platform sources in build
LIB_SRCS += $(PLATFORM_SRCS)

# Usage:
# make PLATFORM=m5stack
# make PLATFORM=arduino
```

### 3. 依存関係管理

```makefile
# Automatic dependency generation
DEPDIR = .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

# Compile with dependency generation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@) $(dir $(DEPDIR)/$*.d)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Include dependency files
-include $(patsubst $(SRC_DIR)/%.cpp,$(DEPDIR)/%.d,$(LIB_SRCS))
```

---

## クロスコンパイル

### 1. ARM Cortex-M 向けクロスコンパイル

```makefile
# ARM toolchain
CXX = arm-none-eabi-g++
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy

# ARM-specific flags
CXXFLAGS += -mcpu=cortex-m4
CXXFLAGS += -mthumb
CXXFLAGS += -mfloat-abi=hard
CXXFLAGS += -mfpu=fpv4-sp-d16

# Linker script
LDFLAGS += -T linker_script.ld
```

### 2. ESP32 向けクロスコンパイル

```makefile
# ESP32 toolchain
CXX = xtensa-esp32-elf-g++

# ESP32-specific flags
CXXFLAGS += -mlongcalls
CXXFLAGS += -mtext-section-literals

# ESP32 SDK includes
CXXFLAGS += -I$(IDF_PATH)/components/esp32/include
```

---

## PlatformIO

### 1. platformio.ini 構成

```ini
; Global settings
[platformio]
default_envs = m5stack

; Common settings
[env]
framework = arduino
build_flags =
    -std=c++17
    -Wall
    -Wextra
    -Iinclude
lib_deps =

; M5Stack environment
[env:m5stack]
platform = espressif32
board = m5stack-core-esp32
build_flags =
    ${env.build_flags}
    -DPLATFORM_M5STACK
lib_deps =
    ${env.lib_deps}
    m5stack/M5Stack@^0.4.3

; Arduino Uno environment
[env:uno]
platform = atmelavr
board = uno
build_flags =
    ${env.build_flags}
    -DPLATFORM_ARDUINO

; Raspberry Pi Pico environment
[env:pico]
platform = raspberrypi
board = pico
build_flags =
    ${env.build_flags}
    -DPLATFORM_PICO
```

### 2. ビルドコマンド

```bash
# 環境を指定してビルド
pio run -e m5stack
pio run -e uno
pio run -e pico

# アップロード
pio run -e m5stack --target upload

# シリアルモニタ
pio device monitor

# クリーン
pio run --target clean
```

### 3. テスト設定

```ini
; Test environment
[env:native]
platform = native
build_flags =
    ${env.build_flags}
    -DUNIT_TEST

test_build_src = yes
```

---

## CI/CD

### 1. GitHub Actions 設定

```yaml
# .github/workflows/build.yml
name: Build

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build-host:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Install Clang
        run: sudo apt-get install -y clang

      - name: Build Debug
        run: make BUILD=debug

      - name: Build Release
        run: make BUILD=release

      - name: Run Tests
        run: make test

  build-embedded:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Set up Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.x'

      - name: Install PlatformIO
        run: pip install platformio

      - name: Build M5Stack
        run: pio run -e m5stack

      - name: Build Arduino
        run: pio run -e uno

  lint:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Install clang-tidy
        run: sudo apt-get install -y clang-tidy

      - name: Run Lint
        run: ./scripts/lint.sh
```

### 2. 自動フォーマット

```yaml
# .github/workflows/format.yml
name: Format Check

on: [push, pull_request]

jobs:
  format:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Install clang-format
        run: sudo apt-get install -y clang-format

      - name: Check Formatting
        run: |
          find include src -name "*.h" -o -name "*.hpp" -o -name "*.cpp" | \
          xargs clang-format --dry-run --Werror
```

---

## ベストプラクティス

### 1. ビルド時間の短縮

```makefile
# 並列ビルド
make -j$(nproc)

# ccache でコンパイルキャッシュ
CXX = ccache clang++
```

### 2. ビルドの再現性

```makefile
# コンパイラバージョン固定
CXX_VERSION = $(shell $(CXX) --version)
$(info Using compiler: $(CXX_VERSION))

# フラグを明示的に記録
build-info:
	@echo "Build configuration:"
	@echo "  CXX: $(CXX)"
	@echo "  CXXFLAGS: $(CXXFLAGS)"
	@echo "  LDFLAGS: $(LDFLAGS)"
```

### 3. クリーンビルド

```makefile
# 完全クリーン (依存関係も削除)
distclean: clean
	rm -rf $(DEPDIR)
	rm -rf .pio

.PHONY: distclean
```

---

## トラブルシューティング

### 1. よくあるビルドエラー

**問題: `undefined reference to get_system_context()`**

```bash
# 解決策: プラットフォーム実装をリンク
make PLATFORM=m5stack
```

**問題: `error: 'constexpr' needed for in-class initialization`**

```bash
# 解決策: C++17 モードを確認
make CXXFLAGS="-std=c++17"
```

### 2. PlatformIO エラー

**問題: `Library not found`**

```bash
# 解決策: ライブラリを再インストール
pio lib install
```

**問題: `Serial port not found`**

```bash
# 解決策: シリアルポートを手動指定
pio run --target upload --upload-port /dev/ttyUSB0
```

---

## 関連ドキュメント

- CLAUDE.md - コーディング規約とビルド要件
- [パフォーマンスガイド](performance.md) - 最適化オプション
- [テストガイド](testing.md) - テストビルド設定

---

**Version:** 2.0.1
**Last Updated:** 2025-11-25
