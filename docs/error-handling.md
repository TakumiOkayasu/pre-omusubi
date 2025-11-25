# エラーハンドリングガイド

---
**Version:** 1.2.0
**Last Updated:** 2025-11-25
---

このドキュメントでは、Omusubiフレームワークにおけるエラーハンドリングの方針と実装パターンを定義します。

**変更履歴:**
- 1.2.0 (2025-11-25): Optional<T>エイリアスを削除し、std::optionalを直接使用
- 1.1.0 (2025-11-25): Optional<T>をC++17標準std::optionalへ移行
- 1.0.0: 初版作成

## 目次

1. [基本方針](#基本方針)
2. [エラー通知パターン](#エラー通知パターン)
3. [エラーコード定義](#エラーコード定義)
4. [エラー伝播](#エラー伝播)
5. [アサーションの使用](#アサーションの使用)
6. [デバッグビルドとリリースビルド](#デバッグビルドとリリースビルド)

---

## 基本方針

### 1. 例外を使用しない

**Omusubiは組み込みシステムのため、C++例外を使用しません。**

**理由:**
- コードサイズの増大
- スタック使用量の増加
- 実行時オーバーヘッド
- リアルタイム性への影響

### 2. エラーは戻り値で通知

**すべてのエラーは関数の戻り値で通知します。**

```cpp
// ✅ 良い例: 戻り値でエラー通知
bool connect() {
    if (!initialize_hardware()) {
        return false;  // エラー
    }
    return true;  // 成功
}

// ❌ 悪い例: 例外を投げる
void connect() {
    if (!initialize_hardware()) {
        throw ConnectionError();  // 禁止
    }
}
```

### 3. エラー状態は明示的に確認

**呼び出し側は必ずエラーを確認する責任があります。**

```cpp
// ✅ 良い例: エラーチェック
auto* serial = ctx->get_connectable_context()->get_serial_context(0);
if (!serial->connect()) {
    // エラー処理
    return;
}

// ❌ 悪い例: エラーを無視
serial->connect();  // 戻り値を確認しない
```

---

## エラー通知パターン

Omusubiでは以下の5つのエラー通知パターンを提供します:

1. **bool型** - 成功/失敗のみ
2. **enum class** - エラーコードで詳細を通知
3. **ErrorInfo構造体** - エラーコード + デバッグ情報
4. **std::optional** - 値の有無を表現 (C++17標準)
5. **Result<T, E>** - 値とエラー情報を両方返す (Rust風)

### std::optionalとResult<T, E>の使い分け

**std::optionalを使用すべき場合:**
- 「値が存在するかもしれないし、しないかもしれない」という状況
- エラーの「理由」は重要ではなく、「値がない」という事実だけが重要
- 検索結果、設定値取得、キャッシュ参照など、値がないことが正常な動作

**Result<T, E>を使用すべき場合:**
- 「失敗した理由を知る必要がある」という状況
- エラーの「理由」が重要で、エラー種別により処理を分岐する必要がある
- ファイルI/O、通信処理、パース処理など、エラー理由により対処が異なる

**判断基準:**
「失敗したとき、なぜ失敗したか知る必要がある？」
- NO → `std::optional`
- YES → `Result<T, E>`

詳細な使い分け例は各パターンの説明を参照してください。

### Pattern 1: bool型による成功/失敗

**最もシンプルなパターン。成功/失敗のみを通知。**

```cpp
// インターフェース定義
class Connectable {
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
};

// 使用例
if (!device->connect()) {
    // 接続失敗
    return;
}
```

**適用場面:**
- 成功/失敗の2値で十分な場合
- エラーの詳細が不要な場合
- シンプルな操作

### Pattern 2: enum classによるエラーコード

**詳細なエラー理由を返す必要がある場合。**

```cpp
// エラーコード定義
enum class Error : uint8_t {
    OK = 0,
    INVALID_PARAMETER,
    NOT_CONNECTED,
    TIMEOUT,
    BUFFER_FULL,
    HARDWARE_ERROR
};

// インターフェース定義
class Connectable {
    virtual Error connect() = 0;
    virtual Error disconnect() = 0;
};

// 使用例
auto error = device->connect();
if (error != Error::OK) {
    switch (error) {
        case Error::TIMEOUT:
            // タイムアウト処理
            break;
        case Error::HARDWARE_ERROR:
            // ハードウェアエラー処理
            break;
        default:
            // その他のエラー
            break;
    }
}
```

**適用場面:**
- エラーの種類を区別する必要がある場合
- エラーに応じた処理を分岐したい場合
- デバッグ情報として詳細が必要な場合

### Pattern 3: 構造体による詳細情報

**エラーコードと追加情報を返す場合。**

```cpp
// エラー情報構造体
struct ErrorInfo {
    Error code;
    uint32_t line;        // エラー発生行
    const char* message;  // エラーメッセージ

    constexpr ErrorInfo() noexcept
        : code(Error::OK), line(0), message(nullptr) {}

    constexpr ErrorInfo(Error code, uint32_t line, const char* msg) noexcept
        : code(code), line(line), message(msg) {}

    constexpr bool is_ok() const noexcept { return code == Error::OK; }
};

// マクロで行番号を自動取得
#define MAKE_ERROR(code, msg) ErrorInfo(code, __LINE__, msg)

// 使用例
ErrorInfo connect_with_retry() {
    for (uint8_t i = 0; i < 3; ++i) {
        auto error = device->connect();
        if (error == Error::OK) {
            return ErrorInfo();  // 成功
        }
        delay(100);
    }
    return MAKE_ERROR(Error::TIMEOUT, "Connection failed after 3 retries");
}
```

**適用場面:**
- デバッグ情報が必要な場合
- エラーログに詳細を記録したい場合

### Pattern 4: std::optional による値の有無表現

**C++17標準の std::optional を直接使用します。**

```cpp
#include <optional>

// 使用例
std::optional<int> opt1;         // 空
std::optional<int> opt2(42);     // 値あり
std::optional<int> opt3 = {};    // 空（std::nullopt相当）

// 主要なAPI
constexpr bool has_value() const noexcept;
constexpr explicit operator bool() const noexcept;

constexpr T& value();
constexpr const T& value() const;
constexpr T value_or(const T& default_value) const;

// ポインタ風アクセス（has_value() == falseの場合は未定義動作）
constexpr T* operator->();
constexpr const T* operator->() const;
constexpr T& operator*();
constexpr const T& operator*() const;

void reset() noexcept;
void emplace(const T& value);
void emplace(T&& value);
```

**使用例:**
```cpp
std::optional<uint32_t> read_sensor() {
    if (!sensor_ready()) {
        return {};  // 値なし（std::nullopt相当）
    }
    return read_from_hardware();  // 値あり
}

// 呼び出し側
auto result = read_sensor();
if (result.has_value()) {
    // または if (result)
    process_value(result.value());
} else {
    handle_error();
}

// value_or() でデフォルト値指定
uint32_t value = read_sensor().value_or(0);
```

**適用場面:**
- 値が存在するかもしれないし、しないかもしれない状況
- エラーの「理由」は重要ではなく、「値がない」という事実だけが重要
- 検索結果、設定値取得、キャッシュ参照など

**具体例:**

```cpp
// ✅ std::optional が適切な例

// 検索結果（見つからない場合がある）
std::optional<uint32_t> find_device_id(StringView name) {
    for (uint32_t i = 0; i < device_count; i++) {
        if (devices[i].name == name) {
            return i;  // 暗黙的に std::optional<uint32_t> に変換
        }
    }
    return {};  // 見つからなかった（std::nullopt相当）
}

// 設定値の取得（設定されていない場合がある）
std::optional<uint32_t> get_timeout() {
    if (timeout_configured) {
        return timeout_value;
    }
    return {};  // 未設定
}

// キャッシュ参照（キャッシュミスは正常な動作）
std::optional<SensorData> get_cached_data(uint32_t id) {
    if (cache_contains(id)) {
        return cache[id];
    }
    return {};  // キャッシュミス
}
```

### Pattern 5: Result<T, E> による値とエラーの分離

**値とエラー情報を両方返す場合。**

```cpp
// Result<T, E> パターン（Rust風）
template <typename T, typename E = Error>
class Result {
private:
    union {
        T value_;
        E error_;
    };
    bool is_ok_;

public:
    // 成功
    static constexpr Result ok(const T& value) noexcept {
        Result r;
        r.is_ok_ = true;
        new (&r.value_) T(value);
        return r;
    }

    // エラー
    static constexpr Result err(const E& error) noexcept {
        Result r;
        r.is_ok_ = false;
        new (&r.error_) E(error);
        return r;
    }

    ~Result() {
        if (is_ok_) {
            value_.~T();
        } else {
            error_.~E();
        }
    }

    constexpr bool is_ok() const noexcept { return is_ok_; }
    constexpr bool is_err() const noexcept { return !is_ok_; }

    constexpr T& value() {
        assert(is_ok_);
        return value_;
    }

    constexpr const T& value() const {
        assert(is_ok_);
        return value_;
    }

    constexpr E& error() {
        assert(!is_ok_);
        return error_;
    }

    constexpr const E& error() const {
        assert(!is_ok_);
        return error_;
    }

    constexpr T value_or(const T& default_value) const {
        return is_ok_ ? value_ : default_value;
    }

private:
    Result() = default;
};

// 使用例
Result<uint32_t, Error> read_sensor() {
    if (!sensor_ready()) {
        return Result<uint32_t, Error>::err(Error::NOT_CONNECTED);
    }
    uint32_t value = read_from_hardware();
    return Result<uint32_t, Error>::ok(value);
}

// 呼び出し側
auto result = read_sensor();
if (result.is_ok()) {
    process_value(result.value());
} else {
    handle_error(result.error());
}
```

**適用場面:**
- 失敗した理由を知る必要がある状況
- エラーの「理由」が重要で、エラー種別により処理を分岐する必要がある
- ファイルI/O、通信処理、パース処理など

**具体例:**

```cpp
// ✅ Result<T, E> が適切な例

// ファイル読み込み（失敗理由が複数ある）
Result<FixedBuffer<256>, Error> read_file(StringView path) {
    if (!file_exists(path)) {
        return Result<FixedBuffer<256>, Error>::err(Error::FILE_NOT_FOUND);
    }
    if (!has_permission(path)) {
        return Result<FixedBuffer<256>, Error>::err(Error::PERMISSION_DENIED);
    }
    auto data = read_data(path);
    if (data.is_empty()) {
        return Result<FixedBuffer<256>, Error>::err(Error::READ_FAILED);
    }
    return Result<FixedBuffer<256>, Error>::ok(data);
}

// 通信処理（エラー種別により対処が異なる）
Result<uint32_t, Error> send_command(uint8_t cmd) {
    if (!is_connected()) {
        return Result<uint32_t, Error>::err(Error::NOT_CONNECTED);
    }
    if (buffer_full()) {
        return Result<uint32_t, Error>::err(Error::BUFFER_FULL);
    }
    if (!write_success(cmd)) {
        return Result<uint32_t, Error>::err(Error::WRITE_FAILED);
    }
    auto response = read_response();
    return Result<uint32_t, Error>::ok(response);
}

// 呼び出し側でエラー種別により処理を分岐
auto result = send_command(CMD_STATUS);
if (result.is_err()) {
    switch (result.error()) {
        case Error::NOT_CONNECTED:
            reconnect();
            break;
        case Error::BUFFER_FULL:
            wait_and_retry();
            break;
        case Error::WRITE_FAILED:
            log_error("Write failed");
            break;
    }
}

// パース処理（構文エラーの詳細が必要）
Result<uint32_t, Error> parse_number(StringView text) {
    if (text.is_empty()) {
        return Result<uint32_t, Error>::err(Error::INVALID_PARAMETER);
    }
    if (!is_valid_format(text)) {
        return Result<uint32_t, Error>::err(Error::INVALID_DATA);
    }
    uint32_t value = convert_to_number(text);
    return Result<uint32_t, Error>::ok(value);
}
```

---

## エラーコード定義

### 共通エラーコード

**プロジェクト全体で共通のエラーコードを定義。**

```cpp
// include/omusubi/core/error.h
namespace omusubi {

enum class Error : uint8_t {
    // 成功
    OK = 0,

    // 一般的なエラー (1-19)
    UNKNOWN_ERROR = 1,
    NOT_IMPLEMENTED = 2,
    INVALID_PARAMETER = 3,
    OUT_OF_RANGE = 4,
    BUFFER_FULL = 5,
    BUFFER_EMPTY = 6,

    // 接続エラー (20-39)
    NOT_CONNECTED = 20,
    ALREADY_CONNECTED = 21,
    CONNECTION_FAILED = 22,
    DISCONNECTED = 23,
    TIMEOUT = 24,

    // ハードウェアエラー (40-59)
    HARDWARE_ERROR = 40,
    HARDWARE_NOT_READY = 41,
    HARDWARE_BUSY = 42,
    SENSOR_ERROR = 43,

    // リソースエラー (60-79)
    OUT_OF_MEMORY = 60,
    RESOURCE_BUSY = 61,
    RESOURCE_UNAVAILABLE = 62,

    // I/Oエラー (80-99)
    READ_ERROR = 80,
    WRITE_ERROR = 81,
    IO_ERROR = 82,

    // 設定エラー (100-119)
    INVALID_CONFIG = 100,
    CONFIG_NOT_FOUND = 101
};

// エラーコードを文字列に変換
constexpr const char* error_to_string(Error error) noexcept {
    switch (error) {
        case Error::OK: return "OK";
        case Error::UNKNOWN_ERROR: return "Unknown error";
        case Error::NOT_IMPLEMENTED: return "Not implemented";
        case Error::INVALID_PARAMETER: return "Invalid parameter";
        case Error::NOT_CONNECTED: return "Not connected";
        case Error::TIMEOUT: return "Timeout";
        case Error::HARDWARE_ERROR: return "Hardware error";
        case Error::OUT_OF_MEMORY: return "Out of memory";
        case Error::READ_ERROR: return "Read error";
        case Error::WRITE_ERROR: return "Write error";
        default: return "Unknown error code";
    }
}

} // namespace omusubi
```

### デバイス固有エラーコード

**デバイス固有のエラーは拡張enumで定義。**

```cpp
// WiFi固有エラー
enum class WiFiError : uint8_t {
    SSID_NOT_FOUND = 120,
    INVALID_PASSWORD = 121,
    DHCP_FAILED = 122,
    DNS_FAILED = 123
};

// Errorに変換
constexpr Error to_error(WiFiError err) noexcept {
    return static_cast<Error>(err);
}
```

---

## エラー伝播

### 1. 早期リターンパターン

**エラーが発生したら即座に返す。**

```cpp
bool setup_device() {
    if (!initialize_hardware()) {
        return false;  // 早期リターン
    }

    if (!configure_settings()) {
        cleanup_hardware();
        return false;  // 早期リターン
    }

    if (!start_operation()) {
        cleanup_hardware();
        return false;  // 早期リターン
    }

    return true;
}
```

### 2. エラーチェーンパターン

**複数のエラーを伝播させる。**

```cpp
Error connect_and_send() {
    auto error = device->connect();
    if (error != Error::OK) {
        return error;  // エラーを伝播
    }

    error = device->write(data);
    if (error != Error::OK) {
        device->disconnect();
        return error;  // エラーを伝播
    }

    return Error::OK;
}
```

### 3. リソース管理とエラー

**RAIIパターンでリソースを確実に解放。**

```cpp
class ScopedConnection {
private:
    Connectable* device_;
    bool connected_;

public:
    explicit ScopedConnection(Connectable* device)
        : device_(device), connected_(false) {
        if (device_) {
            connected_ = device_->connect();
        }
    }

    ~ScopedConnection() {
        if (connected_ && device_) {
            device_->disconnect();
        }
    }

    bool is_connected() const { return connected_; }
};

// 使用例
void send_data() {
    ScopedConnection conn(device);
    if (!conn.is_connected()) {
        return;  // エラー: 自動的にdisconnectされる
    }

    device->write(data);
    // スコープ終了時に自動的にdisconnect
}
```

---

## アサーションの使用

### 1. アサーションの基本方針

**プログラマエラー(バグ)の検出にのみ使用。**

```cpp
#include <cassert>

// ✅ 良い例: プログラマエラーの検出
void set_buffer_size(uint32_t size) {
    assert(size > 0);           // サイズは必ず正
    assert(size <= MAX_SIZE);   // 最大値を超えない
    buffer_size_ = size;
}

// ❌ 悪い例: 実行時エラーのチェック
bool connect() {
    assert(is_hardware_present());  // ハードウェアは外部要因で変わる
    return true;
}
```

### 2. static_assertの使用

**コンパイル時にチェック可能なものはstatic_assert。**

```cpp
template <uint32_t N>
class FixedString {
    static_assert(N > 0, "String capacity must be greater than 0");
    static_assert(N <= 1024, "String capacity too large");

    char buffer_[N + 1];
};
```

### 3. デバッグビルドのみのチェック

**パフォーマンスに影響する場合はデバッグビルドのみ。**

```cpp
#ifdef NDEBUG
    #define DEBUG_ASSERT(expr) ((void)0)
#else
    #define DEBUG_ASSERT(expr) assert(expr)
#endif

void process_array(const uint8_t* data, uint32_t length) {
    DEBUG_ASSERT(data != nullptr);  // リリースビルドでは削除される
    DEBUG_ASSERT(length > 0);

    for (uint32_t i = 0; i < length; ++i) {
        process_byte(data[i]);
    }
}
```

---

## デバッグビルドとリリースビルド

### デバッグビルド

**開発時に使用。エラーチェックを最大化。**

```cpp
#ifndef NDEBUG
    // デバッグビルド
    #define DEBUG_MODE 1
    #define ENABLE_ASSERTIONS 1
    #define ENABLE_LOGGING 1
#else
    // リリースビルド
    #define DEBUG_MODE 0
    #define ENABLE_ASSERTIONS 0
    #define ENABLE_LOGGING 0
#endif
```

**デバッグビルドの特徴:**
- すべてのアサーションが有効
- 詳細なログ出力
- 最適化なし(`-O0`)
- デバッグシンボル付き(`-g`)

### リリースビルド

**本番環境で使用。パフォーマンス最優先。**

**リリースビルドの特徴:**
- アサーションは無効(`assert`は削除される)
- ログ出力は最小限
- 最適化有効(`-O2`または`-Os`)
- デバッグシンボルなし

### エラーチェックの使い分け

```cpp
void critical_operation(uint32_t value) {
    // リリースビルドでも必須のチェック
    if (value > MAX_VALUE) {
        return;  // エラーハンドリング
    }

    // デバッグビルドのみのチェック
    DEBUG_ASSERT(is_initialized());
    DEBUG_ASSERT(buffer_ != nullptr);

    // 処理本体
    process(value);
}
```

---

## ベストプラクティス

### 1. エラーを無視しない

```cpp
// ❌ 悪い例
device->connect();  // 戻り値を確認しない

// ✅ 良い例
if (!device->connect()) {
    // エラー処理
}
```

### 2. エラーメッセージは明確に

```cpp
// ❌ 悪い例
return Error::ERROR;  // 何のエラー?

// ✅ 良い例
return Error::CONNECTION_TIMEOUT;  // 明確
```

### 3. エラー時のリソース解放

```cpp
// ✅ 良い例
bool initialize() {
    buffer_ = allocate_buffer();
    if (!buffer_) {
        return false;
    }

    if (!setup_hardware()) {
        free_buffer(buffer_);  // リソース解放
        buffer_ = nullptr;
        return false;
    }

    return true;
}
```

### 4. エラーログの記録

```cpp
bool connect() {
    auto error = device->connect();
    if (error != Error::OK) {
        // エラーログを記録
        log_error("Connection failed: %s", error_to_string(error));
        return false;
    }
    return true;
}
```

---

## チェックリスト

エラーハンドリングのチェックリスト:

- [ ] すべての関数でエラーを適切に返している
- [ ] 呼び出し側でエラーをチェックしている
- [ ] エラー時のリソース解放を実装している
- [ ] アサーションはプログラマエラーにのみ使用
- [ ] エラーコードは明確で一貫性がある
- [ ] デバッグビルドで十分なエラーチェック
- [ ] リリースビルドでパフォーマンスを損なわない

---

## 関連ドキュメント

- [デバッグガイド](debug.md) - ログ出力とデバッグ手法
- [テストガイド](testing.md) - エラーケースのテスト方法
- [C++17機能ガイド](cpp17_features.md) - std::optionalの使用
- [CLAUDE.md](../CLAUDE.md) - コーディング規約全般

---

**Version:** 1.2.0
**Last Updated:** 2025-11-17
