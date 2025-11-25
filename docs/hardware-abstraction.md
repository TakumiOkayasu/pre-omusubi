# ハードウェア抽象化ガイド

このドキュメントでは、Omusubiフレームワークにおけるハードウェア固有の抽象化戦略と実装ガイドラインを定義します。

**汎用的な抽象化原則については[抽象化原則ガイド](abstraction-principles.md)を参照してください。**

## 目次

1. [組み込み環境の制約](#組み込み環境の制約)
   - ゼロオーバーヘッド抽象化
   - メモリ制約
   - 型の使い分け: `uint32_t` vs `size_t`
2. [ハードウェア分類と抽象化戦略](#ハードウェア分類と抽象化戦略)
3. [レイヤー別の抽象化](#レイヤー別の抽象化)
4. [プラットフォーム実装パターン](#プラットフォーム実装パターン)
5. [ハードウェア固有の考慮事項](#ハードウェア固有の考慮事項)
6. [具体例](#具体例)

---

## 組み込み環境の制約

### 1. ゼロオーバーヘッド抽象化

**Omusubiは組み込みシステムのため、抽象化によるパフォーマンスオーバーヘッドを許容しません。**

#### 許容される抽象化

- ✅ **純粋仮想関数** - vtableのオーバーヘッドは許容(インターフェース層のみ)
- ✅ **constexpr関数** - コンパイル時に解決されるためオーバーヘッドゼロ
- ✅ **インライン関数** - 関数呼び出しオーバーヘッドなし
- ✅ **テンプレート** - コンパイル時特殊化、実行時オーバーヘッドなし

#### 禁止される抽象化

- ❌ **動的メモリ確保** - `new`, `delete`, `malloc`, `free`
- ❌ **例外処理** - コードサイズとスタック使用量が増大
- ❌ **RTTI** - `typeid`, `dynamic_cast`(バイナリサイズ増大)
- ❌ **仮想デストラクタの連鎖** - 不要なvtableエントリ

### 2. メモリ制約

#### スタック割り当ての原則

```cpp
// ✅ 良い例: スタック割り当て
void process_data() {
    FixedString<64> buffer;
    buffer.append("Hello"_sv);
}

// ❌ 悪い例: 動的メモリ確保
void process_data() {
    std::string buffer;  // ヒープ使用
    buffer = "Hello";
}
```

### 3. 型の使い分け: `uint32_t` vs `size_t`

**重要:** Omusubiでは用途によって型を使い分けます。

#### `uint32_t`を使う場合(内部実装)

**コンテナの容量・サイズ、カウンタ、インデックスには`uint32_t`を使用。**

**理由:**
- プラットフォーム間でのバイナリ互換性
- サイズの予測可能性(常に32ビット)
- 組み込み環境での一貫性

```cpp
// ✅ 良い例: 内部実装
template <uint32_t Capacity>
class FixedString {
    char buffer_[Capacity + 1];
    uint32_t length_;  // 常に32ビット
public:
    constexpr uint32_t capacity() const { return Capacity; }
    constexpr uint32_t length() const { return length_; }
};

// ループカウンタも uint32_t
for (uint32_t i = 0; i < count; ++i) {
    // ...
}
```

#### `size_t`を使う場合(外部API互換性)

**標準ライブラリやハードウェアAPIとのインターフェースには`size_t`を使用。**

**理由:**
- POSIX、Arduino、ESP-IDFなどの標準APIとの互換性
- ハードウェアベンダーのライブラリとの統合

```cpp
// ✅ 良い例: 標準APIとの互換性
class ByteReadable {
    virtual size_t read(span<uint8_t> buffer) = 0;     // size_t
    virtual size_t available() const = 0;              // size_t
};

class ByteWritable {
    virtual size_t write(span<const uint8_t> data) = 0;  // size_t
};
```

**対応する標準API:**
```cpp
// POSIX
ssize_t read(int fd, void* buf, size_t count);
size_t write(int fd, const void* buf, size_t count);

// Arduino/ESP32
size_t Serial.readBytes(uint8_t* buffer, size_t length);
size_t Serial.write(const uint8_t* buffer, size_t size);

// C標準ライブラリ
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);
```

#### 使い分けルール(まとめ)

| 用途 | 型 | 理由 |
|------|-----|------|
| コンテナの容量 | `uint32_t` | 固定サイズ、バイナリ互換性 |
| コンテナのサイズ | `uint32_t` | 内部実装、予測可能性 |
| ループカウンタ | `uint32_t` | 一貫性、明示的なサイズ |
| テンプレートパラメータ | `uint32_t` | 固定サイズ |
| I/O操作の戻り値 | `size_t` | 標準API互換性 |
| ハードウェアAPIラッパー | `size_t` | ベンダーAPI互換性 |

**詳細:** [型システムの統一](type-system-unification.md)を参照してください。

#### 静的バッファの活用

```cpp
// ✅ 良い例: 静的バッファ + Placement New
namespace {
    alignas(BluetoothImpl) uint8_t bt_buffer[sizeof(BluetoothImpl)];
    bool bt_initialized = false;
}

BluetoothContext* get_bluetooth() {
    if (!bt_initialized) {
        new (bt_buffer) BluetoothImpl();
        bt_initialized = true;
    }
    return reinterpret_cast<BluetoothContext*>(bt_buffer);
}
```

---

## ハードウェア分類と抽象化戦略

### デバイス分類の決定木

新しいハードウェアを抽象化する際の判断フロー:

```
ハードウェアデバイス
│
├─ 入力デバイス(センサー)
│  ├─ データを取得する操作がある?
│  │  └─ YES → Measurable系インターフェース
│  │
│  └─ 測定データの構造は?
│     ├─ スカラー値 → float, int32_t などの基本型
│     ├─ ベクトル → Vector3 などの具体型
│     └─ 複雑 → 専用の構造体(具体型)
│
└─ 出力デバイス(アクチュエーター)
   ├─ どんな操作をする?
   │  ├─ ON/OFF → Switchable
   │  ├─ 位置制御 → PositionControllable
   │  ├─ 速度制御 → SpeedControllable
   │  ├─ PWM制御 → PWMControllable
   │  └─ 複数の組み合わせ → 多重継承
   │
   └─ 制御パラメータは正規化できる?
      ├─ YES → 0.0 ～ 1.0 で統一
      └─ NO → 具体的な単位(度、rpm等)は実装で変換
```

### 入力と出力の分離原則

```
┌─────────────────────────────────────┐
│  入力デバイス(センサー)              │
│  - 測定する                          │
│  - データを取得する                  │
│  - Measurable系                     │
│  例: 温度センサー、流量計、加速度計  │
└─────────────────────────────────────┘
              ↓
        ユーザーコード
              ↓
┌─────────────────────────────────────┐
│  出力デバイス(アクチュエーター)      │
│  - 制御する                          │
│  - 操作を実行する                    │
│  - Controllable系, Switchable系     │
│  例: モーター、ポンプ、LED、サーボ   │
└─────────────────────────────────────┘
```

**❌ アンチパターン: 入出力の混在**
```cpp
// ❌ 悪い例: ポンプに流量計を混入
class SmartPumpContext : public Switchable, public FlowMonitorable {
    // センサー(入力)とアクチュエーター(出力)を混同
};

// ✅ 良い例: 分離
class PumpContext : public Switchable { };        // 出力
class FlowSensorContext { };                      // 入力
```

---

## レイヤー別の抽象化

Omusubiは5層アーキテクチャを採用しています。各層で適切な抽象化レベルを選択します。

### Layer 1: Interface Layer(完全抽象化)

**目的:** プラットフォーム非依存のAPI定義

```cpp
// include/omusubi/interface/readable.h
class ByteReadable {
public:
    ByteReadable() = default;
    virtual ~ByteReadable() = default;

    // 純粋仮想関数 - 操作を抽象化
    virtual size_t read(span<uint8_t> buffer) = 0;
    virtual size_t available() const = 0;
};
```

**特徴:**
- 純粋仮想関数のみ
- 具体的な実装を含まない
- Java/C#のインターフェースに相当

### Layer 2: Context Layer(DIコンテナ)

**目的:** デバイス管理と依存性注入

```cpp
// include/omusubi/context/connectable_context.h
class ConnectableContext {
public:
    virtual ~ConnectableContext() = default;

    // デバイス取得(抽象型を返す)
    virtual SerialContext* get_serial_context(uint8_t port) const = 0;
    virtual uint8_t get_serial_count() const = 0;
    virtual BluetoothContext* get_bluetooth_context() const = 0;
    virtual WiFiContext* get_wifi_context() const = 0;
};
```

**特徴:**
- ファクトリーパターン
- デバイスのライフタイム管理
- モック置き換え可能

### Layer 3: Device Layer(部分抽象化)

**目的:** デバイス固有の操作定義

```cpp
// include/omusubi/device/serial_context.h
class SerialContext : public ByteReadable, public TextReadable,
                      public ByteWritable, public TextWritable,
                      public Connectable {
public:
    SerialContext() = default;
    virtual ~SerialContext() = default;

    // Interface Layer の操作を実装(抽象のまま)
    size_t read(span<uint8_t> buffer) override = 0;
    size_t write(span<const uint8_t> data) override = 0;
    bool connect() override = 0;
    void disconnect() override = 0;
    bool is_connected() const override = 0;
};
```

**特徴:**
- 複数のインターフェースを組み合わせ
- デバイス固有のメソッドを追加可能
- プラットフォーム実装は次の層

### Layer 4: Platform Layer(具体実装)

**目的:** プラットフォーム固有の実装

```cpp
// include/omusubi/platform/m5stack/m5stack_serial_context.hpp
class M5StackSerialContext : public SerialContext {
private:
    uint8_t port_;

public:
    explicit M5StackSerialContext(uint8_t port) : port_(port) {}

    // 具体的な実装
    size_t read(span<uint8_t> buffer) override;
    size_t write(span<const uint8_t> data) override;
    bool connect() override;
    void disconnect() override;
    bool is_connected() const override;
};
```

---

## プラットフォーム実装パターン

### Pattern A: Context as DI Container

**Contextはデバイスの所有と管理を行う。**

```cpp
// ヘッダー
class M5StackConnectableContext : public ConnectableContext {
private:
    mutable M5StackSerialContext serials_[3];
    mutable M5StackBluetoothContext bluetooth_;

public:
    M5StackConnectableContext();
    SerialContext* get_serial_context(uint8_t port) const override;
    BluetoothContext* get_bluetooth_context() const override;
};
```

**実装:**
```cpp
// .cpp
M5StackConnectableContext::M5StackConnectableContext()
    : serials_{
        M5StackSerialContext(0),
        M5StackSerialContext(1),
        M5StackSerialContext(2)
      },
      bluetooth_() {}

SerialContext* M5StackConnectableContext::get_serial_context(uint8_t port) const {
    return (port < 3) ? &serials_[port] : nullptr;
}

BluetoothContext* M5StackConnectableContext::get_bluetooth_context() const {
    return &bluetooth_;
}
```

### Pattern B: 実装の完全隠蔽

**プラットフォーム固有の型をヘッダーに含めない。**

```cpp
// ✅ ヘッダー: プラットフォーム非依存
class M5StackBluetoothContext : public BluetoothContext {
private:
    // プラットフォーム固有型は一切含めない
public:
    M5StackBluetoothContext();
    ~M5StackBluetoothContext() override;

    size_t write(span<const uint8_t> data) override;
    bool connect() override;
};
```

```cpp
// ✅ 実装ファイル: 完全隠蔽
#include <BluetoothSerial.h>  // M5Stack固有

namespace {
    struct BluetoothImpl {
        BluetoothSerial bt;
        bool connected = false;
    };

    static BluetoothImpl impl;  // 静的変数
}

size_t M5StackBluetoothContext::write(span<const uint8_t> data) {
    return impl.bt.write(data.data(), data.size());
}
```

**利点:**
- ヘッダーの再コンパイル不要
- プラットフォーム切り替えが容易
- 依存関係の最小化

**重要な原則:**
- ✅ ヘッダーにプラットフォーム固有型を含めない
- ✅ 実装詳細は`.cpp`の匿名名前空間に隠蔽
- ✅ `void*`ポインタや前方宣言も不要
- ✅ 静的変数には`static`キーワードを明示

---

## ハードウェア固有の考慮事項

### 1. 割り込み処理

**割り込みハンドラは最小限の処理に留める。**

```cpp
// ❌ 悪い例: 割り込み内で複雑な処理
void IRAM_ATTR button_isr() {
    auto* ctx = get_system_context();
    auto* serial = ctx->get_connectable_context()->get_serial_context(0);
    serial->write("Button pressed\n"_sv);  // 割り込み内でI/O
}

// ✅ 良い例: フラグのみ設定
volatile bool button_pressed = false;

void IRAM_ATTR button_isr() {
    button_pressed = true;  // フラグのみ
}

void loop() {
    if (button_pressed) {
        button_pressed = false;
        handle_button_press();  // メインループで処理
    }
}
```

**原則:**
- 割り込み内ではフラグ設定やカウンタ更新のみ
- 複雑な処理はメインループに委譲
- I/O操作は割り込み内で行わない

### 2. タイミング制約

**リアルタイム制約のある処理を考慮。**

```cpp
// センサー読み取り周期の保証
class AccelerometerContext {
private:
    uint32_t last_read_ms_ = 0;
    static constexpr uint32_t MIN_READ_INTERVAL_MS = 10;  // 100Hz

public:
    Vector3 get_values() const {
        uint32_t now = millis();
        if (now - last_read_ms_ < MIN_READ_INTERVAL_MS) {
            // 前回の値を返す(キャッシュ)
            return cached_values_;
        }
        // 実際に読み取り
        last_read_ms_ = now;
        cached_values_ = read_from_hardware();
        return cached_values_;
    }
};
```

**考慮点:**
- センサーのサンプリングレート制限
- モーターのPWM周波数
- 通信のボーレート

### 3. ハードウェアレジスタアクセス

**volatile修飾子の適切な使用。**

```cpp
// ハードウェアレジスタの定義
namespace hardware {
    struct GPIORegisters {
        volatile uint32_t data;
        volatile uint32_t direction;
        volatile uint32_t interrupt_enable;
    };

    // レジスタベースアドレス
    constexpr GPIORegisters* GPIO0 = reinterpret_cast<GPIORegisters*>(0x40010000);
}

// レジスタアクセス
class GPIOContext {
    void set_pin_high(uint8_t pin) {
        hardware::GPIO0->data |= (1U << pin);  // volatileなのでキャッシュされない
    }
};
```

**重要:**
- ハードウェアレジスタは必ず`volatile`
- ビット操作はアトミックでない場合がある
- クリティカルセクションの保護を検討

### 4. 電源管理

**低消費電力モードとの兼ね合い。**

```cpp
class PowerContext {
public:
    virtual ~PowerContext() = default;

    // 電源状態の制御
    virtual bool enter_sleep_mode() = 0;
    virtual void wake_up() = 0;

    // デバイスの電源管理
    virtual bool enable_peripheral(PeripheralID id) = 0;
    virtual void disable_peripheral(PeripheralID id) = 0;
};

// 使用例: 不要なデバイスの電源オフ
void optimize_power() {
    auto* power = ctx->get_power_context();

    // WiFi未使用時はオフ
    if (!wifi_needed) {
        power->disable_peripheral(PeripheralID::WIFI);
    }

    // スリープモード移行
    power->enter_sleep_mode();
}
```

### 5. ハードウェアエラー処理

**ハードウェアの異常状態を検出。**

```cpp
class SensorContext {
public:
    // センサー状態の確認
    virtual bool is_ready() const = 0;
    virtual SensorError get_last_error() const = 0;
};

enum class SensorError : uint8_t {
    NONE = 0,
    NOT_CONNECTED,
    COMMUNICATION_FAILED,
    OUT_OF_RANGE,
    CALIBRATION_ERROR
};

// 使用例
auto* sensor = ctx->get_sensor_context()->get_accelerometer();
if (!sensor->is_ready()) {
    auto error = sensor->get_last_error();
    // エラー処理
}
```

### 6. デバイス初期化とクリーンアップ

**ハードウェアの初期化順序と終了処理。**

```cpp
class SystemContext {
public:
    // 初期化(依存関係を考慮した順序で実行)
    virtual void begin() = 0;

    // 更新(定期的に呼び出し)
    virtual void update() = 0;

    // 終了処理
    virtual void shutdown() = 0;
};

// M5Stack実装例
void M5StackSystemContext::begin() {
    // 1. 低レベルハードウェア初期化
    init_clock();
    init_gpio();

    // 2. 周辺機器初期化
    init_serial();
    init_i2c();
    init_spi();

    // 3. センサー初期化
    init_imu();
    init_sensors();

    // 4. 通信デバイス初期化
    init_wifi();
    init_bluetooth();
}

void M5StackSystemContext::shutdown() {
    // 逆順で終了処理
    shutdown_bluetooth();
    shutdown_wifi();
    shutdown_sensors();
    shutdown_peripherals();
}
```

### 7. 複数インスタンスの制約

**ハードウェアリソースの制限を考慮。**

```cpp
// シングルトンデバイス(1つだけ)
class WiFiContext {
    // WiFiモジュールは通常1つだけ
};

// 複数インスタンス対応(制限あり)
class SerialContext {
    // UARTポートは3つまで(ハードウェア制約)
};

class ConnectableContext {
public:
    // 最大数を返すメソッド
    virtual uint8_t get_serial_count() const = 0;  // 例: 3
    virtual SerialContext* get_serial_context(uint8_t port) const = 0;
};
```

---

## 具体例

### 例1: シリアル通信デバイス

#### インターフェース定義

```cpp
// include/omusubi/interface/readable.h
class ByteReadable {
public:
    ByteReadable() = default;
    virtual ~ByteReadable() = default;

    virtual size_t read(span<uint8_t> buffer) = 0;
    virtual size_t available() const = 0;
};

// include/omusubi/interface/writable.h
class ByteWritable {
public:
    ByteWritable() = default;
    virtual ~ByteWritable() = default;

    virtual size_t write(span<const uint8_t> data) = 0;
};
```

#### デバイス定義

```cpp
// include/omusubi/device/serial_context.h
class SerialContext : public ByteReadable, public ByteWritable,
                      public Connectable {
public:
    SerialContext() = default;
    ~SerialContext() override = default;

    // インターフェースの実装(抽象のまま)
    size_t read(span<uint8_t> buffer) override = 0;
    size_t write(span<const uint8_t> data) override = 0;
    bool connect() override = 0;
    void disconnect() override = 0;
    bool is_connected() const override = 0;
};
```

#### プラットフォーム実装

```cpp
// include/omusubi/platform/m5stack/m5stack_serial_context.hpp
class M5StackSerialContext : public SerialContext {
private:
    uint8_t port_;

public:
    explicit M5StackSerialContext(uint8_t port);

    size_t read(span<uint8_t> buffer) override;
    size_t write(span<const uint8_t> data) override;
    bool connect() override;
    void disconnect() override;
    bool is_connected() const override;
};
```

```cpp
// src/platform/m5stack/m5stack_serial_context.cpp
#include <HardwareSerial.h>

namespace {
    struct SerialImpl {
        HardwareSerial* hw;
        bool connected;
    };

    static SerialImpl impls[] = {
        {&Serial, false},
        {&Serial1, false},
        {&Serial2, false}
    };
}

M5StackSerialContext::M5StackSerialContext(uint8_t port)
    : port_(port) {}

size_t M5StackSerialContext::read(span<uint8_t> buffer) {
    auto& impl = impls[port_];
    if (!impl.connected) return 0;
    return impl.hw->readBytes(buffer.data(), buffer.size());
}

bool M5StackSerialContext::connect() {
    auto& impl = impls[port_];
    impl.hw->begin(115200);
    impl.connected = true;
    return true;
}
```

### 例2: 加速度センサー

#### インターフェース

```cpp
// include/omusubi/interface/measurable.h
class Measurable3D {
public:
    virtual ~Measurable3D() = default;
    virtual Vector3 get_values() const = 0;  // 操作を抽象化
};
```

**重要:** `Vector3`は具体型(構造の具体化)

```cpp
// include/omusubi/core/vector3.h
struct Vector3 {
    float x;
    float y;
    float z;

    constexpr Vector3() : x(0), y(0), z(0) {}
    constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
};
```

#### デバイス実装

```cpp
// include/omusubi/device/accelerometer_context.h
class AccelerometerContext : public Measurable3D {
public:
    Vector3 get_values() const override = 0;
};

// include/omusubi/platform/m5stack/m5stack_accelerometer_context.hpp
class M5StackAccelerometerContext : public AccelerometerContext {
public:
    Vector3 get_values() const override;
};
```

```cpp
// src/platform/m5stack/m5stack_accelerometer_context.cpp
#include <M5Stack.h>

Vector3 M5StackAccelerometerContext::get_values() const {
    float x, y, z;
    M5.IMU.getAccelData(&x, &y, &z);
    return Vector3(x, y, z);  // 具体型を返す
}
```

### 例3: サーボモーター(アクチュエーター)

#### インターフェース

```cpp
// 位置制御インターフェース
class PositionControllable {
public:
    virtual ~PositionControllable() = default;

    virtual bool set_position(float position) = 0;  // 0.0 ～ 1.0
    virtual float get_position() const = 0;
    virtual PositionRange get_range() const = 0;
};

// 位置範囲(具体型)
struct PositionRange {
    float min;
    float max;

    constexpr PositionRange(float min, float max)
        : min(min), max(max) {}
};
```

#### デバイス実装

```cpp
// include/omusubi/device/servo_context.h
class ServoContext : public PositionControllable {
public:
    bool set_position(float position) override = 0;
    float get_position() const override = 0;
    PositionRange get_range() const override = 0;
};
```

```cpp
// include/omusubi/platform/m5stack/m5stack_servo_context.hpp
class M5StackServoContext : public ServoContext {
private:
    uint8_t pin_;

public:
    explicit M5StackServoContext(uint8_t pin);

    bool set_position(float position) override;
    float get_position() const override;
    PositionRange get_range() const override;
};
```

```cpp
// src/platform/m5stack/m5stack_servo_context.cpp
#include <ESP32Servo.h>

namespace {
    struct ServoImpl {
        Servo servo;
        uint8_t pin;
        float current_position;
        bool attached;
    };

    // 最大8個のサーボ対応
    static ServoImpl servos[8];
    static uint8_t servo_count = 0;

    ServoImpl* get_servo(uint8_t pin) {
        for (uint8_t i = 0; i < servo_count; ++i) {
            if (servos[i].pin == pin) {
                return &servos[i];
            }
        }

        if (servo_count < 8) {
            servos[servo_count].pin = pin;
            servos[servo_count].current_position = 0.0f;
            servos[servo_count].attached = false;
            return &servos[servo_count++];
        }

        return nullptr;
    }
}

M5StackServoContext::M5StackServoContext(uint8_t pin)
    : pin_(pin) {}

bool M5StackServoContext::set_position(float position) {
    auto* impl = get_servo(pin_);
    if (!impl) return false;

    if (!impl->attached) {
        impl->servo.attach(impl->pin);
        impl->attached = true;
    }

    // 0.0 ～ 1.0 を 0° ～ 180° に変換
    auto angle = static_cast<int>(position * 180.0f);
    impl->servo.write(angle);
    impl->current_position = position;

    return true;
}

float M5StackServoContext::get_position() const {
    auto* impl = get_servo(pin_);
    return impl ? impl->current_position : 0.0f;
}

PositionRange M5StackServoContext::get_range() const {
    return PositionRange(0.0f, 1.0f);  // 0% ～ 100%
}
```

### 例4: 給水ポンプ(操作と結果の分離)

#### ✅ 正しいアプローチ: PWM制御

```cpp
// ポンプはPWM制御アクチュエーター
class WaterPumpContext : public Switchable, public PWMControllable {
public:
    // Switchable
    bool turn_on() override = 0;
    bool turn_off() override = 0;
    bool is_on() const override = 0;

    // PWMControllable
    bool set_duty_cycle(float duty) override = 0;  // 0.0 ～ 1.0
    float get_duty_cycle() const override = 0;
};
```

**使用例:**
```cpp
auto* pump = ctx->get_actuator_context()->get_pwm_pump(0);

// PWM 50%で動作(結果的に流量も約50%になる)
pump->set_duty_cycle(0.5f);

// PWM 80%に増加
pump->set_duty_cycle(0.8f);

// 停止
pump->turn_off();
```

**重要:**
- `set_duty_cycle(0.5f)` = PWM 50%(**操作**)
- 流量が約50%になる = **結果**
- 操作と結果を混同しない

#### 流量測定が必要な場合: デバイスを分離

```cpp
// 流量計は別デバイス(センサー)として実装
struct FlowMeasurement {
    float flow_rate;      // L/min
    float total_volume;   // L

    constexpr FlowMeasurement(float rate, float total)
        : flow_rate(rate), total_volume(total) {}
};

class FlowSensorContext {
public:
    virtual ~FlowSensorContext() = default;

    // 操作: 測定値を取得
    virtual FlowMeasurement get_measurement() const = 0;

    // 操作: 累積量をリセット
    virtual void reset_total() = 0;
};
```

**ポンプと流量計を組み合わせた使用例:**

```cpp
// ポンプ(アクチュエーター)
auto* pump = ctx->get_actuator_context()->get_pwm_pump(0);

// 流量計(センサー)
auto* flow_sensor = ctx->get_sensor_context()->get_flow_sensor(0);

// ポンプをPWM 60%で起動
pump->set_duty_cycle(0.6f);

// 1秒待機
delay(1000);

// 流量を測定(別デバイス)
auto measurement = flow_sensor->get_measurement();
auto current_flow = measurement.flow_rate;      // 例: 2.5 L/min
auto total_volume = measurement.total_volume;   // 例: 0.042 L

// 必要に応じてポンプを調整
if (current_flow < 2.0f) {
    pump->set_duty_cycle(0.8f);  // PWMを上げる
}
```

#### 教訓

**1. 操作と結果を分離する**
- ✅ 操作: `set_duty_cycle()`, `set_speed()`
- ❌ 結果: 流量(直接制御できない)

**2. アクチュエーターとセンサーを分離する**
- ✅ ポンプ = 出力デバイス(操作)
- ✅ 流量計 = 入力デバイス(測定)
- ❌ 両者を混在させない

**3. 既存インターフェースを活用する**
- `Switchable`, `PWMControllable`, `SpeedControllable`
- 専用インターフェースを作らない

**4. 物理的な結果を抽象化しない**
- 流量、温度、圧力 → これらは測定対象(センサー)
- PWM、速度、位置 → これらは制御対象(アクチュエーター)

---

## まとめ

### ハードウェア抽象化のチェックリスト

新しいハードウェア抽象化を追加する際のチェックリスト:

- [ ] デバイスは入力(センサー)か出力(アクチュエーター)か明確か?
- [ ] 入出力を混在させていないか?
- [ ] 操作と結果を分離しているか?
- [ ] ハードウェアリソースの制約を考慮しているか?
- [ ] 割り込み処理は最小限か?
- [ ] タイミング制約を満たしているか?
- [ ] ハードウェアレジスタにvolatileを使用しているか?
- [ ] 電源管理を考慮しているか?
- [ ] エラー処理を実装しているか?
- [ ] 初期化とクリーンアップの順序は正しいか?
- [ ] 複数インスタンスの制限を文書化しているか?
- [ ] ヘッダーにプラットフォーム固有型を含んでいないか?
- [ ] 実装は`.cpp`の匿名名前空間に隠蔽しているか?
- [ ] 静的変数にstaticキーワードを明示しているか?

---

## 関連ドキュメント

- [抽象化原則ガイド](abstraction-principles.md) - 汎用的な抽象化原則とSOLID原則
- [アーキテクチャガイド](architecture.md) - 5層アーキテクチャの詳細
- [C++17機能ガイド](cpp17-features.md) - 使用可能なC++機能
- CLAUDE.md - コーディング規約全般

---

**Version:** 2.1.0
**Last Updated:** 2025-11-17
