# API Reference

Omusubi APIの完全なリファレンスドキュメントです。

## 目次

- [Core Types](#core-types)
- [Interfaces](#interfaces)
- [Contexts](#contexts)
- [SystemContext](#systemcontext)

## Core Types

### StringView

非所有のUTF-8文字列ビュー。

```cpp
class StringView {
public:
    // コンストラクタ
    constexpr StringView() noexcept;
    constexpr StringView(const char* data, uint32_t byte_length) noexcept;

    // 静的ファクトリ
    static StringView from_c_string(const char* str);

    // アクセサ
    constexpr const char* data() const noexcept;
    constexpr uint32_t byte_length() const noexcept;
    uint32_t char_count() const noexcept;

    // 文字アクセス
    StringView get_char(uint32_t char_index) const noexcept;

    // 比較
    bool equals(StringView other) const noexcept;
    bool operator==(StringView other) const noexcept;
    bool operator!=(StringView other) const noexcept;

    // イテレータ
    const char* begin() const noexcept;
    const char* end() const noexcept;
};
```

**使用例:**

```cpp
// 文字列リテラル
using namespace omusubi::literals;
StringView msg = "Hello"_sv;

// C文字列から
const char* str = "World";
StringView view = StringView::from_c_string(str);

// 比較
if (msg == "Hello"_sv) {
    // ...
}
```

### FixedString<N>

固定長のUTF-8文字列バッファ。

```cpp
template <uint32_t Capacity>
class FixedString {
public:
    // コンストラクタ
    FixedString() noexcept;
    explicit FixedString(const char* str) noexcept;
    explicit FixedString(StringView view) noexcept;

    // 容量・サイズ
    constexpr uint32_t capacity() const noexcept;
    uint32_t byte_length() const noexcept;

    // アクセサ
    const char* data() const noexcept;
    const char* c_str() const noexcept;
    StringView view() const noexcept;

    // 変更
    bool append(StringView view) noexcept;
    bool append(const char* str) noexcept;
    bool append(char c) noexcept;
    void clear() noexcept;

    // 比較
    bool operator==(StringView other) const noexcept;
    bool operator!=(StringView other) const noexcept;

    // 文字アクセス
    StringView get_char(uint32_t char_index) const noexcept;

    // spanサポート
    span<char> as_span() noexcept;
    span<const char> as_span() const noexcept;
    void from_span(span<const char> s) noexcept;
};
```

**使用例:**

```cpp
// 容量256バイトの文字列
FixedString<256> str;
str.append("Hello"_sv);
str.append(" World"_sv);

// C文字列として取得
const char* cstr = str.c_str();

// StringViewに変換
StringView view = str.view();
```

### FixedBuffer<N>

固定長のバイトバッファ。

```cpp
template <uint32_t Capacity>
class FixedBuffer {
public:
    // コンストラクタ
    FixedBuffer() noexcept;

    // 容量・サイズ
    constexpr uint32_t capacity() const noexcept;
    uint32_t length() const noexcept;

    // アクセサ
    const uint8_t* data() const noexcept;
    uint8_t* data() noexcept;

    // 変更
    bool append(const uint8_t* data, uint32_t length) noexcept;
    bool append(uint8_t byte) noexcept;
    void clear() noexcept;

    // 添字アクセス
    uint8_t operator[](uint32_t index) const noexcept;
    uint8_t& operator[](uint32_t index) noexcept;

    // spanサポート
    span<uint8_t> as_span() noexcept;
    span<const uint8_t> as_span() const noexcept;
    void from_span(span<const uint8_t> s) noexcept;
};
```

**使用例:**

```cpp
// 容量1024バイトのバッファ
FixedBuffer<1024> buffer;
buffer.append(0x01);
buffer.append(0x02);

// データ取得
const uint8_t* data = buffer.data();
uint32_t len = buffer.length();
```

### span<T>

非所有のメモリビュー（C++20 std::span相当）。

```cpp
template <typename T>
class span {
public:
    using element_type = T;
    using size_type = size_t;

    // コンストラクタ
    constexpr span() noexcept;
    constexpr span(T* data, size_type size) noexcept;
    template <size_t N>
    constexpr span(T (&arr)[N]) noexcept;

    // アクセサ
    constexpr T* data() const noexcept;
    constexpr size_type size() const noexcept;
    constexpr bool empty() const noexcept;

    // 添字アクセス
    constexpr T& operator[](size_type index) const noexcept;

    // イテレータ
    constexpr T* begin() const noexcept;
    constexpr T* end() const noexcept;

    // サブスパン
    constexpr span<T> subspan(size_type offset, size_type count) const noexcept;
};
```

**使用例:**

```cpp
// 配列からspan作成
uint8_t buffer[256];
span<uint8_t> s(buffer, 256);

// ゼロコピーで関数に渡す
void process(span<const uint8_t> data) {
    for (uint8_t byte : data) {
        // ...
    }
}
process(s);
```

### Vector3

3次元ベクトル（センサーデータ用）。

```cpp
struct Vector3 {
    float x;
    float y;
    float z;

    constexpr Vector3() noexcept;
    constexpr Vector3(float x_val, float y_val, float z_val) noexcept;
};
```

**使用例:**

```cpp
Vector3 accel = sensor->get_values();
float magnitude = sqrt(accel.x * accel.x + accel.y * accel.y + accel.z * accel.z);
```

### PowerState

電源状態の列挙型。

```cpp
enum class PowerState : uint8_t {
    BATTERY,   // バッテリー駆動
    USB,       // USB給電
    EXTERNAL,  // 外部電源
    UNKNOWN    // 不明
};
```

### ButtonState

ボタン状態の列挙型。

```cpp
enum class ButtonState : uint8_t {
    PRESSED,   // 押されている
    RELEASED   // 離されている
};
```

## Interfaces

### ByteReadable

バイト列読み取りインターフェース。

```cpp
class ByteReadable {
public:
    virtual size_t read(span<uint8_t> buffer) = 0;
    virtual size_t available() const = 0;
};
```

**使用例:**

```cpp
uint8_t buffer[256];
size_t n = device->read(span<uint8_t>(buffer, 256));
```

### TextReadable

テキスト読み取りインターフェース（ByteReadableを継承）。

```cpp
class TextReadable : public ByteReadable {
public:
    virtual size_t read_line(span<char> buffer) = 0;
};
```

**使用例:**

```cpp
char line_buffer[256];
size_t n = device->read_line(span<char>(line_buffer, 256));
StringView line(line_buffer, n);
```

### ByteWritable

バイト列書き込みインターフェース。

```cpp
class ByteWritable {
public:
    virtual size_t write(span<const uint8_t> data) = 0;
};
```

**使用例:**

```cpp
uint8_t data[] = {0x01, 0x02, 0x03};
device->write(span<const uint8_t>(data, 3));
```

### TextWritable

テキスト書き込みインターフェース（ByteWritableを継承）。

```cpp
class TextWritable : public ByteWritable {
public:
    virtual size_t write_text(span<const char> text) = 0;
};
```

**使用例:**

```cpp
device->write_text("Hello"_sv);
```

### Connectable

接続管理インターフェース。

```cpp
class Connectable {
public:
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
};
```

**使用例:**

```cpp
if (!device->is_connected()) {
    device->connect();
}

// 使用後
device->disconnect();
```

### Scannable

ネットワーク/デバイススキャンインターフェース。

```cpp
class Scannable {
public:
    virtual void start_scan() = 0;
    virtual void stop_scan() = 0;
    virtual uint8_t get_found_count() const = 0;
    virtual FixedString<64> get_found_name(uint8_t index) const = 0;
    virtual int32_t get_found_signal_strength(uint8_t index) const = 0;
};
```

**使用例:**

```cpp
wifi->start_scan();
ctx.delay(3000);  // スキャン待機
wifi->stop_scan();

uint8_t count = wifi->get_found_count();
for (uint8_t i = 0; i < count; ++i) {
    FixedString<64> ssid = wifi->get_found_name(i);
    int32_t rssi = wifi->get_found_signal_strength(i);
    // ...
}
```

### Pressable

ボタン状態インターフェース。

```cpp
class Pressable {
public:
    virtual ButtonState get_state() const = 0;
};
```

**使用例:**

```cpp
if (button->get_state() == ButtonState::PRESSED) {
    // ボタン押下時の処理
}
```

### Measurable

単一値測定インターフェース。

```cpp
class Measurable {
public:
    virtual float get_value() const = 0;
};
```

**使用例:**

```cpp
float temp = temperature_sensor->get_value();
```

### Measurable3D

3次元測定インターフェース。

```cpp
class Measurable3D {
public:
    virtual Vector3 get_values() const = 0;
};
```

**使用例:**

```cpp
Vector3 accel = accelerometer->get_values();
```

### Displayable

ディスプレイ出力インターフェース。

```cpp
class Displayable {
public:
    virtual void clear() = 0;
    virtual void set_text_color(uint16_t color) = 0;
    virtual void set_text_size(uint8_t size) = 0;
    virtual void draw_text(int16_t x, int16_t y, StringView text) = 0;
    virtual void fill_screen(uint16_t color) = 0;
};
```

**使用例:**

```cpp
display->clear();
display->set_text_color(0xFFFF);
display->set_text_size(2);
display->draw_text(0, 0, "Hello"_sv);
```

## Contexts

### ConnectableContext

接続可能デバイスのコンテキスト。

```cpp
class ConnectableContext {
public:
    // シリアルポート（複数インスタンス）
    virtual SerialContext* get_serial_context(uint8_t port) const = 0;
    virtual uint8_t get_serial_count() const = 0;

    template<uint8_t Port>
    SerialContext* get_serial_context() const;

    // Bluetooth（単一インスタンス）
    virtual BluetoothContext* get_bluetooth_context() const = 0;

    // WiFi（単一インスタンス）
    virtual WiFiContext* get_wifi_context() const = 0;

    // BLE（単一インスタンス）
    virtual BLEContext* get_ble_context() const = 0;
};
```

**使用例:**

```cpp
ConnectableContext* conn = ctx.get_connectable_context();

// ランタイムパラメータ
SerialContext* serial = conn->get_serial_context(0);

// コンパイル時パラメータ
SerialContext* serial = conn->get_serial_context<0>();

// 単一インスタンスデバイス
WiFiContext* wifi = conn->get_wifi_context();
```

### ReadableContext

読み取り可能デバイスのコンテキスト。

```cpp
class ReadableContext {
public:
    virtual SerialContext* get_serial_context(uint8_t port) const = 0;
    virtual uint8_t get_serial_count() const = 0;
    virtual BluetoothContext* get_bluetooth_context() const = 0;
};
```

### WritableContext

書き込み可能デバイスのコンテキスト。

```cpp
class WritableContext {
public:
    virtual SerialContext* get_serial_context(uint8_t port) const = 0;
    virtual uint8_t get_serial_count() const = 0;
    virtual BluetoothContext* get_bluetooth_context() const = 0;
};
```

### ScannableContext

スキャン可能デバイスのコンテキスト。

```cpp
class ScannableContext {
public:
    virtual BluetoothContext* get_bluetooth_context() const = 0;
    virtual WiFiContext* get_wifi_context() const = 0;
    virtual BLEContext* get_ble_context() const = 0;
};
```

### SensorContext

センサーデバイスのコンテキスト。

```cpp
class SensorContext {
public:
    virtual AccelerometerContext* get_accelerometer_context() const = 0;
    virtual GyroscopeContext* get_gyroscope_context() const = 0;
    // ... 他のセンサー
};
```

### InputContext

入力デバイスのコンテキスト。

```cpp
class InputContext {
public:
    virtual ButtonContext* get_button_context(uint8_t index) const = 0;
    virtual uint8_t get_button_count() const = 0;
    // ... 他の入力デバイス
};
```

### OutputContext

出力デバイスのコンテキスト。

```cpp
class OutputContext {
public:
    virtual DisplayContext* get_display_context() const = 0;
    // ... 他の出力デバイス
};
```

### SystemInfoContext

システム情報コンテキスト（読み取り専用）。

```cpp
class SystemInfoContext {
public:
    virtual FixedString<64> get_device_name() const = 0;
    virtual FixedString<32> get_firmware_version() const = 0;
    virtual uint64_t get_chip_id() const = 0;
    virtual uint32_t get_uptime_ms() const = 0;
    virtual uint32_t get_free_memory() const = 0;
};
```

**使用例:**

```cpp
SystemInfoContext* info = ctx.get_system_info_context();
FixedString<64> name = info->get_device_name();
uint32_t uptime = info->get_uptime_ms();
uint32_t free_mem = info->get_free_memory();
```

### PowerContext

電源管理コンテキスト。

```cpp
class PowerContext {
public:
    virtual PowerState get_power_state() const = 0;
    virtual uint8_t get_battery_level() const = 0;
    virtual bool is_charging() const = 0;
};
```

**使用例:**

```cpp
PowerContext* power = ctx.get_power_context();
PowerState state = power->get_power_state();
uint8_t level = power->get_battery_level();

if (state == PowerState::BATTERY && level < 20) {
    // バッテリー残量低下警告
}
```

## SystemContext

システム全体のコンテキスト。

```cpp
class SystemContext {
public:
    // システム制御
    virtual void begin() = 0;
    virtual void update() = 0;
    virtual void delay(uint32_t ms) = 0;
    virtual void reset() = 0;

    // カテゴリ別コンテキストアクセス
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

// グローバル関数（唯一のアクセス方法）
SystemContext& get_system_context();
```

### システム制御メソッド

**begin()**

システムを初期化します。`setup()`内で最初に呼び出す必要があります。

```cpp
void setup() {
    ctx.begin();  // 必須
    // ... 他の初期化
}
```

**update()**

システムを更新します。`loop()`内で定期的に呼び出す必要があります。

```cpp
void loop() {
    ctx.update();  // 推奨
    // ... メインロジック
}
```

**delay(uint32_t ms)**

指定されたミリ秒数だけ待機します。

```cpp
ctx.delay(1000);  // 1秒待機
```

**reset()**

システムをリセットします。

```cpp
ctx.reset();  // システム再起動
```

### 使用例

**基本的な使用方法:**

```cpp
#include <omusubi/omusubi.h>

using namespace omusubi;
using namespace omusubi::literals;

// グローバルでContextを取得
SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void setup() {
    // システム初期化
    ctx.begin();

    // デバイス取得（一度だけ）
    serial = ctx.get_connectable_context()->get_serial_context(0);
}

void loop() {
    // システム更新
    ctx.update();

    // デバイス使用
    serial->write_text("Hello"_sv);

    ctx.delay(1000);
}
```

**複数デバイスの使用:**

```cpp
SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;
WiFiContext* wifi = nullptr;
DisplayContext* display = nullptr;

void setup() {
    ctx.begin();

    // 各デバイス取得
    serial = ctx.get_connectable_context()->get_serial_context(0);
    wifi = ctx.get_connectable_context()->get_wifi_context();
    display = ctx.get_output_context()->get_display_context();

    // WiFi接続
    wifi->connect_to("SSID"_sv, "password"_sv);
}

void loop() {
    ctx.update();

    // 各デバイス使用
    if (wifi->is_connected()) {
        display->draw_text(0, 0, "WiFi Connected"_sv);
    }

    ctx.delay(100);
}
```

## 型変換とヘルパー関数

### StringView変換

```cpp
// C文字列 → StringView
StringView view = StringView::from_c_string("Hello");

// FixedString → StringView
FixedString<256> str("World");
StringView view = str.view();

// リテラル → StringView
using namespace omusubi::literals;
StringView view = "Hello"_sv;
```

### span変換

```cpp
// FixedBuffer → span
FixedBuffer<256> buffer;
span<uint8_t> s = buffer.as_span();

// FixedString → span
FixedString<256> str;
span<char> s = str.as_span();

// 配列 → span
uint8_t arr[256];
span<uint8_t> s(arr, 256);
```

## エラーハンドリング

Omusubiは例外を使用しません。エラーは返り値で通知されます。

```cpp
// bool返り値（成功/失敗）
if (!wifi->connect_to("SSID"_sv, "password"_sv)) {
    // 接続失敗
}

// size_t返り値（読み書きしたバイト数）
size_t n = serial->read(buffer);
if (n == 0) {
    // 読み取りデータなし
}

// ポインタ返り値（nullptr = エラー）
SerialContext* serial = ctx.get_serial_context(999);
if (serial == nullptr) {
    // 無効なポート番号
}
```

## パフォーマンス最適化

### Contextポインタのキャッシュ

```cpp
// ❌ 遅い（毎回チェーンを辿る）
void loop() {
    ctx.get_connectable_context()->get_serial_context(0)->write("data"_sv);
}

// ✅ 速い（ポインタをキャッシュ）
SerialContext* serial = nullptr;

void setup() {
    serial = ctx.get_connectable_context()->get_serial_context(0);
}

void loop() {
    serial->write("data"_sv);
}
```

### spanによるゼロコピー

```cpp
// ❌ コピーが発生
void process(FixedString<256> str) { }

// ✅ ゼロコピー
void process(StringView str) { }
void process(span<const char> str) { }
```

### constexprの使用

```cpp
// コンパイル時計算
constexpr auto msg = static_string("Hello");
constexpr size_t len = msg.size();
```
