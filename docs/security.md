# セキュリティガイド

このドキュメントでは、Omusubiフレームワークにおけるセキュリティ方針と安全なコーディング手法を定義します。

## 目次

1. [セキュリティ方針](#セキュリティ方針)
2. [入力検証](#入力検証)
3. [バッファ安全性](#バッファ安全性)
4. [通信セキュリティ](#通信セキュリティ)
5. [認証と認可](#認証と認可)
6. [機密情報の扱い](#機密情報の扱い)
7. [脆弱性対策](#脆弱性対策)

---

## セキュリティ方針

### 1. 安全第一の設計

**セキュリティは最初から組み込む。後付けではない。**

**基本原則:**
- **最小権限の原則** - 必要最小限の権限のみ付与
- **Defense in Depth** - 多層防御
- **Fail Secure** - 失敗時は安全側に倒す
- **入力を信用しない** - すべての入力を検証

### 2. 組み込みシステム特有のリスク

**物理アクセスのリスク:**
- デバイスの盗難
- UART/JTAG 経由の不正アクセス
- ファームウェアの抽出

**ネットワークリスク:**
- WiFi 盗聴
- 中間者攻撃 (MITM)
- DoS 攻撃

### 3. セキュアコーディング基準

**CERT C++ Coding Standard に準拠。**

---

## 入力検証

### 1. すべての入力を検証

**外部からの入力は信用しない。**

```cpp
// ❌ 悪い例: 検証なし
void set_buffer_size(uint32_t size) {
    buffer_size_ = size;  // 任意の値を受け入れる
}

// ✅ 良い例: 境界チェック
bool set_buffer_size(uint32_t size) {
    if (size == 0 || size > MAX_BUFFER_SIZE) {
        return false;  // 不正な値を拒否
    }
    buffer_size_ = size;
    return true;
}
```

### 2. 文字列入力の検証

**長さとフォーマットをチェック。**

```cpp
// ❌ 悪い例: 長さチェックなし
void set_device_name(const char* name) {
    strcpy(device_name_, name);  // バッファオーバーフロー
}

// ✅ 良い例: 長さ制限
bool set_device_name(std::string_view name) {
    if (name.size() > MAX_NAME_LENGTH) {
        return false;  // 長すぎる名前を拒否
    }

    device_name_.clear();
    device_name_.append(name);
    return true;
}
```

### 3. 数値入力の検証

**範囲と型をチェック。**

```cpp
// ❌ 悪い例: 範囲チェックなし
void set_port(uint8_t port) {
    serial_port_ = port;  // 任意の値
}

// ✅ 良い例: 範囲チェック
bool set_port(uint8_t port) {
    if (port >= SERIAL_PORT_COUNT) {
        return false;  // 範囲外を拒否
    }
    serial_port_ = port;
    return true;
}

// ✅ 最適例: enum class で型安全性確保
enum class SerialPort : uint8_t {
    PORT_0 = 0,
    PORT_1 = 1,
    PORT_2 = 2
};

void set_port(SerialPort port) {
    // 型システムが範囲を保証
    serial_port_ = static_cast<uint8_t>(port);
}
```

### 4. パスインジェクション対策

**ファイルパスやコマンドを検証。**

```cpp
// ❌ 悪い例: パストラバーサル脆弱性
void load_config(std::string_view filename) {
    // filename = "../../etc/passwd" など危険
    open_file(filename);
}

// ✅ 良い例: パス正規化と検証
bool load_config(std::string_view filename) {
    // パストラバーサル文字列を拒否
    if (filename.contains(".."sv) || filename.contains("/"sv)) {
        return false;
    }

    // ホワイトリスト方式
    if (!is_valid_config_filename(filename)) {
        return false;
    }

    open_file(filename);
    return true;
}
```

---

## バッファ安全性

### 1. バッファオーバーフロー対策

**常に境界チェック。**

```cpp
// ❌ 悪い例: 境界チェックなし
void copy_data(const uint8_t* src, uint32_t len) {
    for (uint32_t i = 0; i < len; ++i) {
        buffer_[i] = src[i];  // バッファオーバーフロー
    }
}

// ✅ 良い例: 容量チェック
bool copy_data(span<const uint8_t> src) {
    if (src.size() > BUFFER_SIZE) {
        return false;  // 容量超過を拒否
    }

    for (uint32_t i = 0; i < src.size(); ++i) {
        buffer_[i] = src[i];
    }
    return true;
}

// ✅ 最適例: span で安全性保証
bool copy_data(span<const uint8_t> src, span<uint8_t> dst) {
    if (src.size() > dst.size()) {
        return false;
    }

    for (uint32_t i = 0; i < src.size(); ++i) {
        dst[i] = src[i];
    }
    return true;
}
```

### 2. null 終端の保証

**文字列バッファは常に null 終端。**

```cpp
// ✅ 良い例: null 終端を保証
template<uint32_t N>
class FixedString {
private:
    char buffer_[N + 1];  // +1 for null terminator
    uint32_t length_ = 0;

public:
    bool append(std::string_view str) {
        if (length_ + str.byte_length() > N) {
            return false;  // 容量不足
        }

        for (uint32_t i = 0; i < str.byte_length(); ++i) {
            buffer_[length_ + i] = str[i];
        }

        length_ += str.byte_length();
        buffer_[length_] = '\0';  // null 終端を必ず設定
        return true;
    }
};
```

### 3. 整数オーバーフロー対策

**演算前に範囲チェック。**

```cpp
// ❌ 悪い例: オーバーフローチェックなし
uint32_t calculate_total(uint32_t a, uint32_t b) {
    return a + b;  // オーバーフロー可能
}

// ✅ 良い例: オーバーフローチェック
bool calculate_total(uint32_t a, uint32_t b, uint32_t& result) {
    // オーバーフローチェック
    if (a > UINT32_MAX - b) {
        return false;  // オーバーフローを検出
    }

    result = a + b;
    return true;
}
```

---

## 通信セキュリティ

### 1. WiFi セキュリティ

**WPA2 以上を使用。**

```cpp
// ❌ 悪い例: 平文通信
wifi->connect_to("MyNetwork"sv, ""sv);  // パスワードなし

// ✅ 良い例: WPA2 暗号化
wifi->connect_to("MyNetwork"sv, "SecurePassword123"sv);
```

### 2. Bluetooth セキュリティ

**ペアリングと認証を実装。**

```cpp
// ✅ Bluetooth ペアリング
class SecureBluetoothContext : public BluetoothContext {
private:
    bool paired_ = false;

public:
    bool pair(std::string_view pin) {
        // PIN コード検証
        if (pin != EXPECTED_PIN) {
            return false;
        }

        paired_ = true;
        return true;
    }

    size_t write(span<const uint8_t> data) override {
        // ペアリングされていなければ拒否
        if (!paired_) {
            return 0;
        }

        return BluetoothContext::write(data);
    }
};
```

### 3. シリアル通信の保護

**重要なコマンドは認証を要求。**

```cpp
// ✅ コマンド認証
class SecureSerialContext : public SerialContext {
private:
    bool authenticated_ = false;

public:
    bool authenticate(std::string_view password) {
        if (password == ADMIN_PASSWORD) {
            authenticated_ = true;
            return true;
        }
        return false;
    }

    bool execute_command(std::string_view command) {
        // 危険なコマンドは認証必須
        if (is_dangerous_command(command)) {
            if (!authenticated_) {
                return false;  // 未認証は拒否
            }
        }

        // コマンド実行
        return process_command(command);
    }
};
```

### 4. データの暗号化

**機密データは暗号化して送信。**

```cpp
// ✅ データ暗号化 (簡易例)
class EncryptedChannel {
private:
    uint8_t key_[16];  // AES-128 鍵

public:
    bool send_encrypted(span<const uint8_t> data) {
        FixedBuffer<256> encrypted;

        // データを暗号化 (実際には AES ライブラリを使用)
        if (!encrypt_aes128(data, key_, encrypted)) {
            return false;
        }

        // 暗号化データを送信
        return channel_->write(encrypted.as_span()) == encrypted.size();
    }
};
```

---

## 認証と認可

### 1. パスワード管理

**パスワードは平文で保存しない。**

```cpp
// ❌ 悪い例: 平文パスワード
const char* PASSWORD = "admin123";  // ハードコード

bool authenticate(std::string_view password) {
    return password == PASSWORD;  // 平文比較
}

// ✅ 良い例: ハッシュ化パスワード
const uint8_t PASSWORD_HASH[32] = {
    // SHA-256 ハッシュ値
    0x8c, 0x69, 0x76, 0xe5, /* ... */
};

bool authenticate(std::string_view password) {
    uint8_t hash[32];
    sha256(password, hash);

    // ハッシュ比較
    return memcmp(hash, PASSWORD_HASH, 32) == 0;
}
```

### 2. セッション管理

**タイムアウトを設定。**

```cpp
class SecureSession {
private:
    bool authenticated_ = false;
    uint32_t last_activity_ = 0;
    static constexpr uint32_t TIMEOUT_MS = 300000;  // 5分

public:
    bool is_authenticated() const {
        uint32_t now = millis();

        // タイムアウトチェック
        if (authenticated_ && (now - last_activity_) > TIMEOUT_MS) {
            authenticated_ = false;  // タイムアウト
        }

        return authenticated_;
    }

    void update_activity() {
        last_activity_ = millis();
    }
};
```

### 3. レート制限

**ブルートフォース攻撃対策。**

```cpp
class RateLimiter {
private:
    uint32_t attempt_count_ = 0;
    uint32_t last_attempt_ = 0;
    static constexpr uint32_t MAX_ATTEMPTS = 5;
    static constexpr uint32_t LOCKOUT_MS = 60000;  // 1分

public:
    bool is_locked() const {
        if (attempt_count_ >= MAX_ATTEMPTS) {
            uint32_t elapsed = millis() - last_attempt_;
            return elapsed < LOCKOUT_MS;  // ロックアウト中
        }
        return false;
    }

    void record_attempt(bool success) {
        last_attempt_ = millis();

        if (success) {
            attempt_count_ = 0;  // リセット
        } else {
            ++attempt_count_;
        }
    }
};

// 使用例
bool authenticate(std::string_view password) {
    if (rate_limiter.is_locked()) {
        return false;  // ロックアウト中
    }

    bool success = check_password(password);
    rate_limiter.record_attempt(success);

    return success;
}
```

---

## 機密情報の扱い

### 1. ハードコードの禁止

**秘密鍵やパスワードをソースコードに埋め込まない。**

```cpp
// ❌ 悪い例: ハードコードされた秘密情報
const char* WIFI_PASSWORD = "MySecretPassword123";
const uint8_t API_KEY[] = "sk_live_abc123...";

// ✅ 良い例: 外部設定ファイルから読み込み
bool load_credentials() {
    // EEPROM や Flash から読み込み
    if (!read_from_flash("wifi_password", wifi_password_)) {
        return false;
    }

    if (!read_from_flash("api_key", api_key_)) {
        return false;
    }

    return true;
}
```

### 2. メモリのクリア

**機密データ使用後はメモリをゼロクリア。**

```cpp
// ✅ 機密データのクリア
void secure_memzero(void* ptr, size_t len) {
    volatile uint8_t* p = static_cast<volatile uint8_t*>(ptr);
    for (size_t i = 0; i < len; ++i) {
        p[i] = 0;
    }
}

void process_password(std::string_view password) {
    // パスワード処理
    uint8_t hash[32];
    sha256(password, hash);

    // 処理後にメモリをクリア
    secure_memzero(hash, sizeof(hash));
}
```

### 3. ログ出力の注意

**機密情報をログに出力しない。**

```cpp
// ❌ 悪い例: パスワードをログ出力
serial->write("接続中: SSID="sv);
serial->write(ssid);
serial->write(", パスワード="sv);
serial->write(password);  // 機密情報が漏洩

// ✅ 良い例: 機密情報をマスク
serial->write("接続中: SSID="sv);
serial->write(ssid);
serial->write(", パスワード=****"sv);  // マスク
```

---

## 脆弱性対策

### 1. OWASP Top 10 対策

**組み込みシステムに適用可能な項目:**

**A01: アクセス制御の不備**
```cpp
// ✅ アクセス制御
bool execute_admin_command(std::string_view command) {
    if (!is_admin_authenticated()) {
        return false;  // 管理者のみ実行可能
    }

    return execute(command);
}
```

**A02: 暗号化の失敗**
```cpp
// ✅ 適切な暗号化
bool send_sensitive_data(span<const uint8_t> data) {
    // TLS または AES で暗号化
    return send_encrypted(data);
}
```

**A03: インジェクション**
```cpp
// ✅ 入力検証とサニタイズ
bool execute_sql_query(std::string_view query) {
    // SQL インジェクション対策: プレースホルダ使用
    // または入力を厳密に検証
    if (!is_safe_query(query)) {
        return false;
    }

    return execute(query);
}
```

### 2. CWE Top 25 対策

**CWE-119: バッファオーバーフロー**
```cpp
// ✅ span で境界安全性保証
bool copy_safe(span<const uint8_t> src, span<uint8_t> dst) {
    if (src.size() > dst.size()) {
        return false;
    }
    // コピー処理
    return true;
}
```

**CWE-190: 整数オーバーフロー**
```cpp
// ✅ オーバーフローチェック
bool add_safe(uint32_t a, uint32_t b, uint32_t& result) {
    if (a > UINT32_MAX - b) {
        return false;
    }
    result = a + b;
    return true;
}
```

**CWE-476: null ポインタデリファレンス**
```cpp
// ✅ null チェック
bool process(SerialContext* serial) {
    if (serial == nullptr) {
        return false;
    }

    serial->write("data"sv);
    return true;
}
```

### 3. ファジングテスト

**異常入力でのテスト。**

```cpp
// ファジングテスト例
void fuzz_test_parser() {
    // ランダムデータでテスト
    for (uint32_t i = 0; i < 10000; ++i) {
        uint8_t random_data[256];
        fill_random(random_data, 256);

        // クラッシュしないことを確認
        parse_packet(span<uint8_t>(random_data, 256));
    }
}
```

---

## セキュリティチェックリスト

実装時のセキュリティチェックリスト:

- [ ] すべての入力を検証している
- [ ] バッファオーバーフロー対策を実装している
- [ ] 整数オーバーフロー対策を実装している
- [ ] null ポインタチェックを実装している
- [ ] 機密情報をハードコードしていない
- [ ] パスワードをハッシュ化している
- [ ] 通信を暗号化している (必要な場合)
- [ ] 認証とセッション管理を実装している
- [ ] レート制限を実装している (必要な場合)
- [ ] ログに機密情報を出力していない
- [ ] 使用後にメモリをクリアしている
- [ ] ファジングテストを実施している

---

## 関連ドキュメント

- [エラーハンドリングガイド](error-handling.md) - 安全なエラー処理
- [テストガイド](testing.md) - セキュリティテスト
- CLAUDE.md - 型安全性とコーディング規約

---

**Version:** 1.0.0
**Last Updated:** 2025-11-17
