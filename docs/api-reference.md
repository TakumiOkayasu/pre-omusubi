# APIリファレンス

Omusubi APIの完全なリファレンスドキュメントです。

## 目次

- [Core Types](#core-types)
- [Interfaces](#interfaces)
- [Contexts](#contexts)
- [SystemContext](#systemcontext)

## Core Types

### std::string_view（UTF-8ヘルパー付き）

標準ライブラリの`std::string_view`を使用し、UTF-8ヘルパー関数を提供。

```cpp
// std::string_viewを直接使用
#include <string_view>
#include <omusubi/core/string_view.h>  // UTF-8ヘルパー

// omusubiが提供するヘルパー関数
namespace omusubi {
    // UTF-8文字数を取得
    constexpr uint32_t char_length(std::string_view sv) noexcept;

    // UTF-8文字インデックスからバイト位置を取得
    constexpr uint32_t get_char_position(std::string_view sv, uint32_t char_index) noexcept;

    // 空かどうか判定
    constexpr bool is_empty(std::string_view sv) noexcept;

    // 等価判定
    constexpr bool equals(std::string_view a, std::string_view b) noexcept;

    // C文字列からstd::string_viewを構築
    std::string_view from_c_string(const char* str) noexcept;
}

```

**使用例:**

```cpp
// 文字列リテラル（標準ライブラリの_svを使用）
using namespace std::literals;
std::string_view msg = "Hello"sv;

// C文字列から
const char* str = "World";
std::string_view view = omusubi::from_c_string(str);

// 比較
if (msg == "Hello"sv) {
    // ...
}

// UTF-8文字数（日本語など）
std::string_view japanese = "こんにちは"sv;
uint32_t char_count = omusubi::char_length(japanese);  // 5
```

### FixedString<N>

固定長のUTF-8文字列バッファ。ヒープを使わずにスタック上に確保。

```cpp
// 容量を指定して作成
FixedString<256> str;
str.append("Hello"sv);
str.append(" World"sv);

// ヘルパー関数でサイズ自動推論
auto str2 = fixed_string("Hello");  // FixedString<5>

// std::string_view / C文字列として取得
std::string_view view = str.view();
const char* cstr = str.c_str();
```

### FixedBuffer<N>

固定長のバイトバッファ。

```cpp
FixedBuffer<1024> buffer;
buffer.append(0x01);
buffer.append(0x02);

const uint8_t* data = buffer.data();
uint32_t len = buffer.length();
```

### span<T>

非所有のメモリビュー（C++20 std::span相当）。ゼロコピーでデータを渡す。

```cpp
uint8_t buffer[256];
span<uint8_t> s(buffer, 256);

// 関数に渡す
void process(span<const uint8_t> data) {
    for (uint8_t byte : data) { /* ... */ }
}
process(s);
```

### StaticVector<T, N>

固定容量の可変長配列。`std::vector` のヒープレス代替。

```cpp
// 最大10要素の配列
StaticVector<int, 10> vec;
vec.push_back(1);
vec.push_back(2);
vec.emplace_back(3);

// 初期化子リストで構築
StaticVector<int, 5> vec2 = {1, 2, 3};

// イテレーション
for (int v : vec) {
    // ...
}

// アクセス
vec[0];           // 添字アクセス
vec.front();      // 先頭要素
vec.back();       // 末尾要素
vec.size();       // 現在の要素数
vec.capacity();   // 最大容量
vec.empty();      // 空かどうか
vec.full();       // 満杯かどうか

// 削除
vec.pop_back();              // 末尾を削除
vec.erase(vec.begin());      // 順序を維持して削除
vec.erase_unordered(0);      // 順序を維持しない高速削除
vec.clear();                 // 全削除
```

### RingBuffer<T, N>

固定長のリングバッファ（FIFO）。古いデータを自動的に上書き。

```cpp
// 最大5要素のリングバッファ
RingBuffer<int, 5> ring;

// 要素追加（満杯時は古いデータを上書き）
ring.push(1);
ring.push(2);

// 要素追加（満杯時は失敗）
if (!ring.try_push(3)) {
    // バッファ満杯
}

// 取り出し（FIFO順）
if (auto val = ring.pop()) {
    int v = *val;
}

// 先頭・末尾参照
ring.front();  // 最も古い要素
ring.back();   // 最も新しい要素

// 状態確認
ring.size();      // 現在の要素数
ring.empty();     // 空かどうか
ring.full();      // 満杯かどうか
ring.clear();     // 全削除

// イテレーション（FIFO順）
for (int v : ring) {
    // ...
}
```

### Function<Sig, Size>

ヒープレスの型消去コールバックラッパー。

```cpp
// 基本的な使い方
Function<void()> callback = []() { /* ... */ };
callback();

// 引数と戻り値
Function<int(int, int)> add = [](int a, int b) { return a + b; };
int result = add(1, 2);  // 3

// キャプチャ付きラムダ
int multiplier = 10;
Function<int(int)> mul = [multiplier](int x) { return x * multiplier; };

// 状態確認
if (callback) {          // 有効かどうか
    callback();
}

// ストレージサイズ指定（デフォルト32バイト）
Function<void(), 64> large_fn;  // 64バイトまでのキャプチャに対応
```

**注意:** キャプチャのサイズが `Size` を超えるとコンパイルエラーになります。

### Vector3

3次元ベクトル（センサーデータ用）。`float x, y, z` メンバーを持つ。

```cpp
Vector3 accel = sensor->get_values();
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

### format()

型安全な文字列フォーマット関数。`{}` でプレースホルダーを指定。

```cpp
auto msg = format("Hello, {}!", "World");          // "Hello, World!"
auto hex = format("Value: 0x{:X}", 255);           // "Value: 0xFF"
auto log = format("[{}] {}", "INFO", "started");   // "[INFO] started"
```

**フォーマット指定子:** `{}`, `{:d}`, `{:x}`, `{:X}`, `{:b}`, `{:f}`, `{:s}`

### Result<T, E>

Rust風のエラーハンドリング型。例外を使わずにエラーを返す。
**constexpr対応:** T と E がトリビアルに破壊可能な場合、コンパイル時評価が可能。

```cpp
Result<uint32_t, Error> read_sensor() {
    if (!ready) return Result<uint32_t, Error>::err(Error::NOT_CONNECTED);
    return Result<uint32_t, Error>::ok(value);
}

auto result = read_sensor();
if (result.is_ok()) {
    process(result.value());
}

// デフォルト値付き
uint32_t value = read_sensor().value_or(0);

// constexpr での使用
constexpr auto ok_result = Result<int, Error>::ok(42);
static_assert(ok_result.is_ok());
static_assert(ok_result.value() == 42);
```

### Logger

シングルトンパターンによるグローバルログ機能。

```cpp
// 初期化（setup()で1回）
static SerialLogOutput log_output(serial);
get_logger().set_output(&log_output);

// グローバル関数でログ出力
log<LogLevel::INFO>("System running"sv);
log<LogLevel::DEBUG>("Debug info"sv);
log<LogLevel::WARNING>("Low memory"sv);
log<LogLevel::ERROR>("Connection failed"sv);

// format()と組み合わせ
auto msg = format("Sensor: {}", value);
log<LogLevel::DEBUG>(msg.view());

```

**ポイント:** リリースビルド（`NDEBUG`）ではDEBUGログは完全削除される。

### parse関数

文字列から数値への変換関数。オーバーフロー検出付き。

```cpp
// 符号なし整数パース
auto result = parse_uint<uint32_t>("12345"sv);
if (result.is_ok()) {
    uint32_t value = result.value();
}

// 符号付き整数パース
auto result = parse_int<int32_t>("-42"sv);
if (result.is_ok()) {
    int32_t value = result.value();
}

// 浮動小数点パース
auto result = parse_float<float>("3.14"sv);
if (result.is_ok()) {
    float value = result.value();
}

// 便利なエイリアス
parse_u8("255"sv);     // Result<uint8_t, Error>
parse_u16("65535"sv);  // Result<uint16_t, Error>
parse_u32("123"sv);    // Result<uint32_t, Error>
parse_i8("-128"sv);    // Result<int8_t, Error>
parse_i16("-1000"sv);  // Result<int16_t, Error>
parse_i32("-12345"sv); // Result<int32_t, Error>
parse_f32("1.5"sv);    // Result<float, Error>
parse_f64("3.14159"sv);// Result<double, Error>
```

**対応フォーマット:**
- 整数: `"123"`, `"-456"`, `"+789"`
- 浮動小数点: `"3.14"`, `"-1.5"`, `".5"`, `"2."`

### 数学ユーティリティ

組み込み向けの数学関数群。すべて constexpr 対応。

```cpp
// 値を範囲内に制限
int clamped = clamp(15, 0, 10);  // 10

// 値を別のレンジにマッピング（Arduino map()相当）
int pwm = map_range(512, 0, 1023, 0, 255);  // 127

// マッピング + クランプ
int safe = map_range_clamped(1500, 0, 1023, 0, 255);  // 255

// 線形補間
float mid = lerp(0.0f, 10.0f, 0.5f);  // 5.0

// 逆線形補間（位置を求める）
float t = inverse_lerp(0.0f, 10.0f, 5.0f);  // 0.5

// 絶対値
int a = omusubi::abs(-5);  // 5

// 符号
int s = sign(-3);  // -1

// 最小値・最大値
int m = min(3, 5);  // 3
int M = max(3, 5);  // 5

// コンパイル時評価
constexpr int c = clamp(15, 0, 10);
static_assert(c == 10);
```

## Interfaces

インターフェースはヘッダーファイル（`include/omusubi/interface/`）を参照。

| インターフェース | 機能 |
|-----------------|------|
| `ByteReadable` | バイト読み取り（`read(span<uint8_t>)`） |
| `TextReadable` | テキスト読み取り（`read_line(span<char>)`） |
| `ByteWritable` | バイト書き込み（`write(span<const uint8_t>)`） |
| `TextWritable` | テキスト書き込み（`write("text"sv)`） |
| `Connectable` | 接続管理（`connect()`, `disconnect()`） |
| `Scannable` | スキャン（`start_scan()`, `get_found_count()`） |
| `Pressable` | ボタン状態（`get_state()`） |
| `Measurable` | センサー値（`get_value()`） |
| `Measurable3D` | 3軸センサー（`get_values()` → `Vector3`） |
| `Displayable` | 表示（`clear()`, `draw_pixel()`） |

## Contexts

Contextはヘッダーファイル（`include/omusubi/context/`）を参照。

| Context | 取得メソッド |
|---------|-------------|
| `ConnectableContext` | `get_serial_context(port)`, `get_wifi_context()`, `get_ble_context()` |
| `SensorContext` | `get_accelerometer_context()`, `get_gyroscope_context()` |
| `InputContext` | `get_button_context(index)` |
| `OutputContext` | `get_display_context()`, `get_led_context()` |
| `PowerContext` | `get_battery_level()`, `get_power_state()` |
| `SystemInfoContext` | `get_device_name()`, `get_firmware_version()` |

**使用例:**

```cpp
ConnectableContext* conn = ctx.get_connectable_context();
SerialContext* serial = conn->get_serial_context(0);
WiFiContext* wifi = conn->get_wifi_context();
```

## SystemContext

`get_system_context()` でシステム全体のContextを取得。

```cpp
#include <omusubi/omusubi.h>
using namespace omusubi;

SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void setup() {
    ctx.begin();
    serial = ctx.get_connectable_context()->get_serial_context(0);
}

void loop() {
    ctx.update();
    serial->write("Hello"sv);
    ctx.delay(1000);
}
```

## Tips

**Contextポインタのキャッシュ:**
```cpp
// ✅ setup()で一度だけ取得してキャッシュ
SerialContext* serial = ctx.get_connectable_context()->get_serial_context(0);
```

**ゼロコピー:**
```cpp
// ✅ std::string_view / span を使用
void process(std::string_view str) { }
```

---

**Version:** 3.1.0
**Last Updated:** 2025-12-12
