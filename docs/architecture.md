# Omusubiアーキテクチャ

このドキュメントでは、Omusubiフレームワークの設計思想とアーキテクチャの詳細を説明します。

## 設計哲学

### なぜContext Patternを採用したか

Omusubiは組み込みシステム特有の制約に対応するため、Context Patternを採用しています。

**背景となる制約:**
- 静的書き込み領域をリンクできない環境が存在（BREW、Android Application Contextなど）
- グローバル変数やシングルトンに依存した設計は移植性が低い
- 実行時のメモリ確保が制限される組み込み環境

**Context Patternの利点:**
- データの配置場所を実装側で自由に決定できる（静的領域、動的領域、外部メモリ等）
- インターフェースを通じたアクセスにより、実装の詳細を隠蔽
- DIコンテナとして機能し、テスト時のモック差し替えが容易
- 階層構造により関心事を分離

**参考デザインパターン:**
- Abstract Factory Pattern
- Dependency Injection Pattern
- Strategy Pattern

### メモリ制約への対応方針

**ゼロオーバーヘッド抽象化:**
- すべてのオブジェクトはスタック割り当て
- ヒープ確保禁止（`new`, `delete`, `malloc`, `free`使用不可）
- 例外禁止（例外テーブルのメモリオーバーヘッド回避）
- RTTI禁止（型情報テーブルのメモリ削減）

**コンパイル時計算の推進:**
- `constexpr`を積極的に使用して実行時計算を削減
- テンプレートによるコード生成で型安全性を保証
- `span<T>`による非所有参照で不要なコピーを回避

**固定サイズバッファの使用:**
- `FixedString<N>`, `FixedBuffer<N>`で容量を明示
- スタック上に確保されることを保証
- サイズ超過は実行時エラーではなく、コンパイル時チェック可能

### インターフェース分離原則（ISP）の適用

**単一責任のインターフェース設計:**

各`*able`インターフェースは単一の責任を持ちます：

```cpp
// ✅ 良い例：単一責任
class Readable {
    virtual size_t read(span<uint8_t> buffer) = 0;
    virtual size_t available() const = 0;
    // 読み取りに関連するメソッドのみ
};

// ❌ 悪い例：複数責任の混在
class NetworkDevice {
    virtual bool connect() = 0;      // 接続責任
    virtual uint8_t scan() = 0;      // スキャン責任
    virtual size_t read() = 0;       // 読み取り責任
};
```

**インターフェースの組み合わせ:**

複数の機能を持つデバイスは、複数のインターフェースを実装します：

```cpp
class SerialContext : public TextReadable, public TextWritable, public Connectable {
    // Serial通信には読み書き＋接続管理が必要
};
```

**抽象化のポリシー:**

- **操作（動詞）は抽象化可能** - `connect()`, `read()`, `write()`等のアクションはインターフェース化
- **構造（データ）は抽象化しない** - `FixedString<N>`, `Vector3`等の具象型を直接使用
- 構造を抽象化すると必ず抽象化漏れが発生するため、具象型を使用することで明確化

## レイヤー構造

Omusubiは5つのレイヤーで構成されています。

### 1. Interface Layer (`include/omusubi/interface/`)

**責務:** 単一責任の契約定義

**`*able`インターフェース:**
- `ByteReadable` / `TextReadable` - データ読み取り
- `ByteWritable` / `TextWritable` - データ書き込み
- `Connectable` - 接続管理
- `Scannable` - デバイススキャン
- `Pressable` - ボタン状態
- `Measurable` / `Measurable3D` - センサー測定
- `Displayable` - ディスプレイ出力

**設計ルール:**
- 各インターフェースは単一責任
- 関連するメソッドは同一インターフェースに含めてOK（例: `start_scan()` + `stop_scan()`）
- 純粋仮想関数のみ（実装を含まない）
- コピー・ムーブコンストラクタは削除

### 2. Context Layer (`include/omusubi/context/`)

**責務:** デバイスのグループ化とDIコンテナ

**カテゴリ別Context:**
- `ConnectableContext` - すべての接続可能デバイス（主要なデバイスアクセスポイント）
  - SerialContext、BluetoothContextはByteReadable/TextReadable/ByteWritableを実装
- `ScannableContext` - すべてのスキャン可能デバイス
- `SensorContext` - すべてのセンサーデバイス
- `InputContext` - すべての入力デバイス
- `OutputContext` - すべての出力デバイス
- `SystemInfoContext` - システム情報（読み取り専用）
- `PowerContext` - 電源管理

**Contextの役割:**
- デバイスの生成と所有権管理
- テスト時のモック差し替え
- プラットフォーム切り替えの抽象化

**Getter命名規則（厳格）:**

```cpp
// ✅ 必須パターン
DeviceContext* get_device_context() const;

// ✅ ランタイムパラメータ版
SerialContext* get_serial_context(uint8_t port) const;

// ✅ コンパイル時パラメータ版（オプション）
template<uint8_t Port>
SerialContext* get_serial_context() const;

// ❌ 禁止パターン
SerialContext* serial_context() const;        // get_プレフィックスなし
SerialContext* get_serial() const;            // _context サフィックスなし
SerialContext* get_serial_1() const;          // 同一型の複数getter禁止（パラメータ化すべき）
```

### 3. Device Layer (`include/omusubi/device/`)

**責務:** 具体的なデバイスクラス

**デバイスContext:**
- `SerialContext` - シリアル通信（`TextReadable + TextWritable + Connectable`）
- `BluetoothContext` - Bluetooth通信（`TextReadable + TextWritable + Connectable + Scannable`）
- `WiFiContext` - WiFi接続（`Connectable + Scannable`）
- `BLEContext` - BLE通信（`Connectable + Scannable`）

**複数インスタンスを持つデバイスの扱い:**

```cpp
// ✅ パラメータで識別
virtual SerialContext* get_serial_context(uint8_t port) const = 0;
virtual uint8_t get_serial_count() const = 0;

// ✅ テンプレート版（コンパイル時最適化）
template<uint8_t Port>
SerialContext* get_serial_context() const {
    return get_serial_context(Port);
}

// ❌ 禁止：同一型の複数getter
virtual SerialContext* get_serial_1_context() const = 0;
virtual SerialContext* get_serial_2_context() const = 0;
```

### 4. Component Layer（オプション、複雑なデバイス用）

**責務:** デバイス内部のサブコンポーネント表現

**使用する場合:**
- デバイスが動的に作成可能なサブコンポーネントを持つ
- サブコンポーネントが独自の状態とメソッドを持つ
- サブコンポーネントが独立したドメイン概念を表す

**例: BLE階層構造**

```cpp
// 基本（3層）: SystemContext → CategoryContext → DeviceContext
BLEContext* ble = ctx->get_connectable_context()->get_ble_context();

// 階層的（4層+）: → Component（Service, Characteristic）
BLEService* service = ble->add_service(service_uuid);
BLECharacteristic* ch = service->add_characteristic(char_uuid, properties);
ch->write("data"sv);
```

**命名規則（重要）:**

```cpp
// ✅ ドメイン標準用語を使用
BLECharacteristic, BLEService    // BLE仕様の用語
HTTPRequest, HTTPResponse        // Web標準の用語
File, Directory                  // OS標準の用語

// ❌ *Context サフィックスは禁止（Componentは概念が異なる）
BLECharacteristicContext         // 間違い
BLEServiceContext                // 間違い
```

### 5. Platform Layer (`include/omusubi/platform/`, `src/platform/`) ※将来実装予定

**責務:** プラットフォーム固有の実装

> **注意:** 現時点では Platform Layer は未実装です。以下はアーキテクチャ設計のガイドラインです。

**実装パターン:**

```cpp
// Pattern A: Context owns devices（推奨）
class M5StackConnectableContext : public ConnectableContext {
private:
    mutable M5StackSerialContext serial0_ {0};
    mutable M5StackSerialContext serial1_ {1};
    mutable M5StackSerialContext serial2_ {2};
    mutable M5StackBluetoothContext bluetooth_;
    mutable M5StackWiFiContext wifi_;
    mutable M5StackBLEContext ble_;

public:
    SerialContext* get_serial_context(uint8_t port) const override {
        switch (port) {
        case 0: return &serial0_;
        case 1: return &serial1_;
        case 2: return &serial2_;
        default: return nullptr;
        }
    }
};
```

**プラットフォーム固有実装の隠蔽:**

```cpp
// ❌ ヘッダーに実装詳細を露出
class M5StackBluetoothContext {
private:
    void* impl_;           // 実装ポインタが見える
    struct Impl;           // 型名が見える
};

// ✅ .cppファイルに完全隠蔽
namespace {
struct BluetoothImpl {
    BluetoothSerial bt;    // M5Stack固有型
    bool connected;
};

static BluetoothImpl impl;  // staticによる単一インスタンス
}

void M5StackBluetoothContext::write(std::string_view text) {
    impl.bt.write(text.data(), text.size());
}
```

## SystemContextアクセスパターン

### Free Function Pattern（必須）

```cpp
// グローバル関数（唯一のアクセス方法）
SystemContext& get_system_context();

// ✅ 使用例
SystemContext& ctx = get_system_context();

// ❌ 禁止（存在しない）
SystemContext& ctx = SystemContext::get_instance();
```

**実装パターン（各プラットフォーム）:**

```cpp
// src/platform/m5stack/system_context.cpp
namespace omusubi {
    SystemContext& get_system_context() {
        static platform::m5stack::M5StackSystemContext inst;
        return inst;
    }
}
```

**利点:**
- ユーザーコードはプラットフォーム非依存
- ビルドシステムでリンクするファイルを切り替えるだけ
- Meyers Singletonによるスレッドセーフな初期化

## Method Chain Design

### 基本パターン（3層）

```cpp
SystemContext → CategoryContext → DeviceContext → method()
```

**使用例:**

```cpp
// シリアル通信
ctx->get_connectable_context()->get_serial_context(0)->write("Hello"sv);

// WiFi接続
ctx->get_connectable_context()->get_wifi_context()->connect_to(ssid, password);

// センサー読み取り
ctx->get_sensor_context()->get_accelerometer_context()->get_values();
```

### 階層パターン（4層+）

```cpp
SystemContext → CategoryContext → DeviceContext → Component → method()
```

**使用例:**

```cpp
// BLE: Service と Characteristic
BLEContext* ble = ctx->get_connectable_context()->get_ble_context();
BLEService* service = ble->add_service(uuid);
BLECharacteristic* ch = service->add_characteristic(uuid, props);
ch->write("Hello"sv);
```

### パフォーマンス最適化

**Contextポインタのキャッシュ（推奨）:**

```cpp
// ❌ 毎回チェーンを辿る（低速）
void loop() {
    ctx.get_connectable_context()->get_serial_context(0)->write("data"sv);
}

// ✅ setup()で一度取得してキャッシュ（高速）
SerialContext* serial = nullptr;

void setup() {
    serial = ctx.get_connectable_context()->get_serial_context(0);
}

void loop() {
    serial->write("data"sv);  // 直接アクセス（オーバーヘッドなし）
}
```

**理由:**
- Method chainは可読性と発見可能性のため
- パフォーマンスが必要な箇所ではポインタをキャッシュ
- Contextポインタは`SystemContext`の生存期間中有効

## 新しいプラットフォームの追加

### 必要な実装ステップ

**1. プラットフォーム固有のSystemContext実装**

```cpp
// include/omusubi/platform/mydevice/mydevice_system_context.hpp
class MyDeviceSystemContext : public SystemContext {
private:
    mutable MyDeviceConnectableContext connectable_;
    mutable MyDeviceScannableContext scannable_;
    mutable MyDeviceSensorContext sensor_;
    // ... 他のContext

public:
    MyDeviceSystemContext();
    ~MyDeviceSystemContext() override = default;

    void begin() override;
    void update() override;
    void delay(uint32_t ms) override;
    void reset() override;

    ConnectableContext* get_connectable_context() const override {
        return &connectable_;
    }
    ScannableContext* get_scannable_context() const override {
        return &scannable_;
    }
    // ... 他のgetter
};
```

**2. 各CategoryContextの実装**

```cpp
// include/omusubi/platform/mydevice/mydevice_connectable_context.hpp
class MyDeviceConnectableContext : public ConnectableContext {
private:
    mutable MyDeviceSerialContext serial0_ {0};
    mutable MyDeviceSerialContext serial1_ {1};
    mutable MyDeviceBluetoothContext bluetooth_;
    // ... 必要なデバイス

public:
    SerialContext* get_serial_context(uint8_t port) const override;
    uint8_t get_serial_count() const override;
    BluetoothContext* get_bluetooth_context() const override;
    // ... 他のgetter
};
```

**3. 各DeviceContextの実装**

```cpp
// include/omusubi/platform/mydevice/mydevice_serial_context.hpp
class MyDeviceSerialContext : public SerialContext {
private:
    uint8_t port_;

public:
    explicit MyDeviceSerialContext(uint8_t port);
    ~MyDeviceSerialContext() override;

    // インターフェース実装
    size_t read(span<uint8_t> buffer) override;
    size_t available() const override;
    size_t read_line(span<char> buffer) override;
    size_t write(span<const uint8_t> data) override;
    size_t write_text(span<const char> text) override;
    bool connect() override;
    void disconnect() override;
    bool is_connected() const override;
};
```

**4. get_system_context()の実装**

```cpp
// src/platform/mydevice/system_context.cpp
namespace omusubi {
    SystemContext& get_system_context() {
        static platform::mydevice::MyDeviceSystemContext inst;
        return inst;
    }
}
```

**5. ビルド設定**

PlatformIO/Arduino IDEで該当プラットフォームのファイルのみをビルド対象にします。

### チェックリスト

- [ ] SystemContext実装
- [ ] 全CategoryContext実装
- [ ] 必要な全DeviceContext実装
- [ ] get_system_context()の実装
- [ ] プラットフォーム固有の初期化処理
- [ ] サンプルコード作成
- [ ] ドキュメント更新

## 新しいデバイスの追加

### インターフェース選択基準

**1. デバイスの機能を分析**

例: 温度センサー
- データ読み取り → `Readable`
- 数値測定 → `Measurable`

**2. 適切なインターフェースを選択**

```cpp
class TemperatureSensorContext : public Measurable {
public:
    float get_value() const override;
};
```

**3. 新しいインターフェースが必要な場合**

既存のインターフェースで表現できない機能は、新しいインターフェースを定義：

```cpp
// include/omusubi/interface/configurable.h
class Configurable {
public:
    virtual bool set_parameter(std::string_view key, std::string_view value) = 0;
    virtual std::string_view get_parameter(std::string_view key) const = 0;
};
```

### Context追加手順

**1. DeviceContextの定義**

```cpp
// include/omusubi/device/temperature_sensor_context.h
class TemperatureSensorContext : public Measurable {
public:
    TemperatureSensorContext() = default;
    ~TemperatureSensorContext() override = default;

    TemperatureSensorContext(const TemperatureSensorContext&) = delete;
    TemperatureSensorContext& operator=(const TemperatureSensorContext&) = delete;

    float get_value() const override = 0;
};
```

**2. CategoryContextへの追加**

```cpp
// include/omusubi/context/sensor_context.h
class SensorContext {
public:
    virtual TemperatureSensorContext* get_temperature_sensor_context() const = 0;
};
```

**3. プラットフォーム実装**

```cpp
// include/omusubi/platform/m5stack/m5stack_temperature_sensor_context.hpp
class M5StackTemperatureSensorContext : public TemperatureSensorContext {
public:
    float get_value() const override;
};

// include/omusubi/platform/m5stack/m5stack_sensor_context.hpp
class M5StackSensorContext : public SensorContext {
private:
    mutable M5StackTemperatureSensorContext temp_sensor_;

public:
    TemperatureSensorContext* get_temperature_sensor_context() const override {
        return &temp_sensor_;
    }
};
```

### 命名規則

**DeviceContext:**
- パターン: `<Device名>Context`
- 例: `SerialContext`, `WiFiContext`, `TemperatureSensorContext`

**Platform実装:**
- パターン: `<Platform名><Device名>Context`
- 例: `M5StackSerialContext`, `ArduinoWiFiContext`

**Getter:**
- パターン: `get_<device名>_context()`
- 例: `get_serial_context()`, `get_temperature_sensor_context()`

## 設計上の禁止事項

### 1. ヒープ割り当て禁止

```cpp
// ❌ 禁止
auto* obj = new MyObject();
char* buf = (char*)malloc(256);
std::unique_ptr<Device> device;
std::vector<int> data;

// ✅ 許可
MyObject obj;                    // スタック
FixedString<256> str;           // 固定サイズ
static MyObject global_obj;     // 静的領域
```

### 2. 例外禁止

```cpp
// ❌ 禁止
throw std::runtime_error("error");
try { } catch { }

// ✅ 許可（エラーハンドリング）
bool connect() {
    if (error) {
        return false;  // エラーを返り値で返す
    }
    return true;
}
```

### 3. RTTI禁止

```cpp
// ❌ 禁止
typeid(obj)
dynamic_cast<Derived*>(base)

// ✅ 許可
static_cast<Derived*>(base)  // 型が確実な場合のみ
```

### 4. マクロ禁止

```cpp
// ❌ 禁止
#define MAX_SIZE 256
#define SQUARE(x) ((x) * (x))

// ✅ 許可
constexpr uint32_t MAX_SIZE = 256;
constexpr int square(int x) { return x * x; }
```

### 5. グローバル変数の制限

```cpp
// ❌ 避けるべき
MyDevice device;  // グローバル変数

// ✅ 推奨
SystemContext& ctx = get_system_context();
MyDeviceContext* device = ctx.get_device_context();
```

## パフォーマンス最適化戦略

### 1. Contextポインタのキャッシュ

```cpp
// グローバルキャッシュ（推奨）
SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void setup() {
    serial = ctx.get_connectable_context()->get_serial_context(0);
}

void loop() {
    serial->write("data"sv);  // 高速アクセス
}
```

### 2. constexprの活用

```cpp
// コンパイル時計算
constexpr auto msg = static_string("Hello");
constexpr size_t len = msg.size();  // 実行時コストゼロ
```

### 3. spanによるゼロコピー

```cpp
// ❌ コピーが発生
void process(FixedString<256> str);

// ✅ ゼロコピー
void process(span<const char> str);
void process(std::string_view str);
```

### 4. テンプレートによるコンパイル時最適化

```cpp
// ランタイム
SerialContext* serial = ctx.get_serial_context(0);

// コンパイル時（port番号が定数の場合）
SerialContext* serial = ctx.get_serial_context<0>();
```

## 参考資料

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Embedded C++ Coding Standard](https://www.barr-group.com/embedded-systems/books/embedded-c-coding-standard)
- Design Patterns: Abstract Factory, Dependency Injection
- Android Application Context design

---

**Version:** 2.1.0
**Last Updated:** 2025-11-25
