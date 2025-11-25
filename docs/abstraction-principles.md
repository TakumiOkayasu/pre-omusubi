# 抽象化原則ガイド

このドキュメントでは、Omusubiフレームワークにおける汎用的な抽象化の原則とSOLID原則の適用方法を定義します。

## 目次

1. [核心原則: 操作と構造の分離](#核心原則-操作と構造の分離)
2. [SOLID原則の適用](#solid原則の適用)
3. [Javaデザインパターンとの対応](#javaデザインパターンとの対応)
4. [抽象化の判断基準](#抽象化の判断基準)
5. [実装パターン](#実装パターン)
6. [アンチパターン](#アンチパターン)

---

## 核心原則: 操作と構造の分離

### Operations (verbs) can be abstracted; Structures (data, task definitions) should NOT be abstracted.

この原則は、Omusubiフレームワーク全体の抽象化戦略の基礎となります。

### ✅ 操作の抽象化(推奨)

**操作(動作・アクション)は抽象化すべき対象です。**

#### 理由

- 操作は安定した契約(contract)を持つ
- インターフェースの変更頻度が低い
- 実装の詳細を隠蔽できる

#### 例

```cpp
// ✅ 良い例: 操作の抽象化
class Writable {
public:
    virtual ~Writable() = default;
    virtual size_t write(span<const uint8_t> data) = 0;
};

class Connectable {
public:
    virtual ~Connectable() = default;
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
};

// 具体的な実装
class SerialContext : public Writable, public Connectable {
    size_t write(span<const uint8_t> data) override;
    bool connect() override;
    void disconnect() override;
    bool is_connected() const override;
};
```

**抽象化すべき操作の例:**
- `read()`, `write()` - I/O操作
- `connect()`, `disconnect()` - 接続管理
- `start_scan()`, `stop_scan()` - スキャン制御
- `get_values()` - データ取得操作

### ❌ 構造の抽象化(非推奨)

**データ構造やタスク定義は抽象化すべきではありません。**

#### 理由

- 抽象化が具体的な実装の詳細を漏らす(Leaky Abstraction)
- オーバーヘッドが高い
- 型安全性が低下する

#### アンチパターン例

```cpp
// ❌ 悪い例: データ構造の抽象化
class Container {
public:
    virtual void* get_data() = 0;        // 型安全性が失われる
    virtual size_t get_capacity() = 0;   // 具体的な詳細が漏れる
    virtual size_t get_size() = 0;
};

// ❌ 悪い例: タスク定義の抽象化
class TaskConfig {
public:
    virtual void set_parameter(const char* key, const char* value) = 0;
    // 具体的なタスクの要件が抽象化を破る
};
```

#### 正しいアプローチ

```cpp
// ✅ 良い例: 具体的なデータ構造
template <uint32_t Capacity>
class FixedString {
    char buffer_[Capacity + 1];
    uint32_t length_;
public:
    constexpr uint32_t capacity() const { return Capacity; }
    constexpr uint32_t length() const { return length_; }
    constexpr const char* data() const { return buffer_; }
};

// ✅ 良い例: 具体的なデータ型
struct Vector3 {
    float x;
    float y;
    float z;
};
```

**具体化すべき構造の例:**
- `FixedString<N>`, `FixedBuffer<N>` - 固定長コンテナ
- `Vector3` - 3次元ベクトル
- `PowerState` - 電源状態(enum)
- 設定構造体(Configuration structs)

---

## SOLID原則の適用

### 1. Single Responsibility Principle (SRP)

**各インターフェースは単一の責務を持つ。**

```cpp
// ✅ 良い例: 単一責務
class ByteReadable {
    virtual size_t read(span<uint8_t> buffer) = 0;
    virtual size_t available() const = 0;
};

class Connectable {
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
};

// ❌ 悪い例: 複数責務
class SerialDevice {
    virtual size_t read(span<uint8_t> buffer) = 0;  // 読み取り責務
    virtual bool connect() = 0;                     // 接続責務
    virtual void start_scan() = 0;                  // スキャン責務(Serialには不要)
};
```

### 2. Interface Segregation Principle (ISP)

**クライアントは使用しないメソッドに依存すべきでない。**

```cpp
// ✅ 良い例: 独立したインターフェース
class TextReadable {
    virtual size_t read_line(span<char> buffer) = 0;
};

class ByteReadable {
    virtual size_t read(span<uint8_t> buffer) = 0;
    virtual size_t available() const = 0;
};

// デバイスは必要なインターフェースのみ実装
class SerialContext : public ByteReadable, public TextReadable {
    // 両方必要
};

class SensorContext : public ByteReadable {
    // バイト読み取りのみ
};

// ❌ 悪い例: 継承の強制
class TextReadable : public ByteReadable {
    // TextReadableを使うだけでByteReadableも必要になる
};
```

### 3. Dependency Inversion Principle (DIP)

**上位モジュールは抽象に依存し、具象に依存すべきでない。**

```cpp
// ✅ 良い例: 抽象型を返す
class Scannable {
    virtual std::string_view get_found_name(uint8_t index) const = 0;
    //      ↑ std::string_view は抽象型
};

class SystemInfoContext {
    virtual std::string_view get_device_name() const = 0;
    //      ↑ 実装詳細(FixedString<N>)を隠蔽
};

// ❌ 悪い例: 具象型を返す
class Scannable {
    virtual FixedString<64> get_found_name(uint8_t index) const = 0;
    //      ↑ 具体的な実装に依存
};
```

---

## Javaデザインパターンとの対応

Omusubiのインターフェース設計はJava標準ライブラリのパターンを参考にしています。

| Omusubiインターフェース | Java対応 | 目的 |
|----------------------|---------|------|
| `ByteReadable` | `InputStream` | バイトストリーム読み取り |
| `TextReadable` | `Reader` | テキストストリーム読み取り |
| `ByteWritable` | `OutputStream` | バイトストリーム書き込み |
| `TextWritable` | `Writer` | テキストストリーム書き込み |
| `Scannable` | `Scanner` | デバイス/ネットワークスキャン |
| `Connectable` | - | 接続管理(組み込み特有) |
| `std::string_view` | `CharSequence` | 文字列の抽象参照 |

**設計思想の借用:**
- Java: 完全な抽象化(仮想関数テーブルのオーバーヘッドは許容)
- C++ Omusubi: ゼロオーバーヘッド抽象化(組み込み制約)

**Javaパターンの利点:**
- 実績のある設計
- 直感的なAPI
- 明確な責務分離

---

## 抽象化の判断基準

### ✅ 抽象化すべき対象

#### 1. 操作(動詞)

```cpp
// ✅ 操作の抽象化
virtual bool turn_on() = 0;
virtual bool set_position(float pos) = 0;
virtual Vector3 get_values() const = 0;
virtual void start_scan() = 0;
```

**判断基準:**
- ハードウェアに「何かをさせる」ならば抽象化
- 動詞で表現できるならば抽象化

#### 2. プラットフォーム間で変わる実装詳細

```cpp
// ✅ 実装詳細を隠蔽
virtual bool connect() = 0;  // シリアル、Bluetooth、WiFiで実装が異なる
```

### ❌ 抽象化すべきでない対象

#### 1. データ構造(名詞)

```cpp
// ❌ 構造を抽象化しない
struct Vector3 {  // 具体型
    float x, y, z;
};

// ❌ 間違った抽象化
class VectorData {
    virtual float get_x() const = 0;  // 不要な抽象化
    virtual float get_y() const = 0;
};
```

**判断基準:**
- データの「形」は抽象化しない
- 構造体で定義できるものは構造体を使う

#### 2. 設定や状態(結果)

```cpp
// ❌ 結果を抽象化しない
class FlowControllable {
    virtual float get_flow_rate() const = 0;  // 流量は結果
};

// ✅ 操作を抽象化
class PWMControllable {
    virtual bool set_duty_cycle(float duty) = 0;  // PWMは操作
};
```

**判断基準:**
- 「制御の結果」として決まる値は抽象化しない
- 「直接制御できる」値のみ抽象化

#### 3. タスクの定義や手順

```cpp
// ❌ タスク定義を抽象化しない
class TaskConfig {
    virtual void set_parameter(const char* key, const char* value) = 0;
};

// ✅ 具体的な設定構造体
struct WateringConfig {
    uint32_t duration_ms;
    float pump_speed;
    uint32_t interval_ms;
};
```

---

## 実装パターン

### Pattern 1: 状態を持たないインターフェース

**純粋な操作のみを定義する。**

```cpp
// ✅ 良い例: 純粋インターフェース
class Writable {
public:
    Writable() = default;
    virtual ~Writable() = default;
    Writable(const Writable&) = delete;
    Writable& operator=(const Writable&) = delete;

    virtual size_t write(span<const uint8_t> data) = 0;
};
```

**特徴:**
- メンバ変数なし
- 純粋仮想関数のみ
- コピー・ムーブ禁止

### Pattern 2: 多重継承による機能組み合わせ

**必要な操作インターフェースを組み合わせる。**

```cpp
// シリアル通信: 読み取り + 書き込み + 接続
class SerialContext : public ByteReadable, public TextReadable,
                      public ByteWritable, public TextWritable,
                      public Connectable {
    // 各インターフェースのメソッドを実装
};

// LED: ON/OFF + PWM調光
class LEDContext : public Switchable, public PWMControllable {
    // 両方のインターフェースを実装
};
```

### Pattern 3: 抽象型を返すゲッター

**実装の詳細を隠蔽する。**

```cpp
// ✅ 良い例: 抽象型を返す
class SystemInfoContext {
    virtual std::string_view get_device_name() const = 0;
    //      ↑ 実装がFixedString<N>であることを隠蔽
};

// 実装側
class M5StackSystemInfoContext : public SystemInfoContext {
private:
    FixedString<32> device_name_;
public:
    std::string_view get_device_name() const override {
        return device_name_.view();  // FixedString → std::string_view 変換
    }
};
```

### Pattern 4: 具体型の構造体

**データ構造は具体的に定義する。**

```cpp
// ✅ センサーデータ: 具体的な構造体
struct Vector3 {
    float x;
    float y;
    float z;

    constexpr Vector3() : x(0), y(0), z(0) {}
    constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
};

// ✅ 設定: 具体的な構造体
struct ServoConfig {
    float min_angle;
    float max_angle;
    uint32_t pulse_min_us;
    uint32_t pulse_max_us;

    constexpr ServoConfig()
        : min_angle(0.0f), max_angle(180.0f),
          pulse_min_us(500), pulse_max_us(2500) {}
};
```

---

## アンチパターン

### ❌ Anti-Pattern 1: 仮想関数の過剰使用

```cpp
// ❌ 悪い例: 不要な仮想関数
class FixedString {
public:
    virtual ~FixedString() = default;  // 不要なvtable
    virtual uint32_t length() const { return length_; }  // constexprで十分
};

// ✅ 良い例: constexpr
template <uint32_t N>
class FixedString {
public:
    constexpr uint32_t length() const { return length_; }  // vtableなし
};
```

### ❌ Anti-Pattern 2: 抽象化の漏洩

```cpp
// ❌ 悪い例: 実装詳細が漏れる
class Container {
public:
    virtual void* get_internal_buffer() = 0;  // 内部バッファを公開
    virtual void set_allocator(Allocator* alloc) = 0;  // メモリ管理の詳細
};

// ✅ 良い例: 操作のみ公開
class Container {
public:
    virtual size_t size() const = 0;
    virtual bool push(const T& value) = 0;
};
```

### ❌ Anti-Pattern 3: 不要なラッパー関数

```cpp
// ❌ 悪い例: 型変換のためだけのラッパー
template <std::size_t N>
auto make_static_string(const char (&str)[N]) {
    return static_string<N - 1>(str);  // std::size_t → uint32_t 変換のみ
}

// ✅ 良い例: 直接使用
template <uint32_t N>
constexpr auto static_string(const char (&str)[N]) {
    return StaticString<N - 1>(str);
}
```

### ❌ Anti-Pattern 4: 構造の抽象化

```cpp
// ❌ 悪い例: データ構造を抽象化
class Point {
public:
    virtual float get_x() const = 0;
    virtual float get_y() const = 0;
    virtual void set_x(float x) = 0;
    virtual void set_y(float y) = 0;
};

// ✅ 良い例: 具体的な構造体
struct Point {
    float x;
    float y;

    constexpr Point() : x(0), y(0) {}
    constexpr Point(float x, float y) : x(x), y(y) {}
};
```

### ❌ Anti-Pattern 5: Same-Type Getter

```cpp
// ❌ 悪い例: 同じ型を返すゲッターが複数
class SensorContext {
public:
    virtual TemperatureSensor* get_temp_sensor_1() const = 0;
    virtual TemperatureSensor* get_temp_sensor_2() const = 0;
    virtual TemperatureSensor* get_temp_sensor_3() const = 0;
};

// ✅ 良い例: パラメータで区別
class SensorContext {
public:
    virtual TemperatureSensor* get_temp_sensor(uint8_t index) const = 0;
    virtual uint8_t get_temp_sensor_count() const = 0;
};
```

---

## まとめ

### 抽象化の黄金律

1. **操作は抽象化、構造は具体化**
   - `read()`, `write()`, `connect()` → 抽象化
   - `FixedString<N>`, `Vector3` → 具体化

2. **SOLID原則の遵守**
   - SRP: 単一責務
   - ISP: インターフェース分離
   - DIP: 抽象型を返す

3. **Javaパターンの参考**
   - InputStream/OutputStream パターン
   - Reader/Writer パターン
   - CharSequence パターン

4. **実装詳細の隠蔽**
   - インターフェースは契約のみ定義
   - 実装詳細は公開しない

### チェックリスト

新しいインターフェースを設計する際のチェックリスト:

- [ ] インターフェースは操作のみ定義しているか?
- [ ] データ構造は具体型で定義しているか?
- [ ] 仮想関数は必要最小限か?
- [ ] SOLID原則に違反していないか?
- [ ] 既存のインターフェースで代用できないか?
- [ ] 複数のデバイスで共通の操作か?
- [ ] データ構造を返す場合、具体型か?
- [ ] 抽象化の漏洩がないか?

---

## 関連ドキュメント

- [ハードウェア抽象化ガイド](hardware-abstraction.md) - ハードウェア固有の抽象化戦略
- [アーキテクチャガイド](architecture.md) - 5層アーキテクチャの詳細
- [C++17機能ガイド](cpp17-features.md) - 使用可能なC++機能
- CLAUDE.md - コーディング規約全般

---

**Version:** 1.0.0
**Last Updated:** 2025-11-17
