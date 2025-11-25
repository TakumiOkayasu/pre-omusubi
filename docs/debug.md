# デバッグガイド

このドキュメントでは、Omusubiフレームワークにおけるデバッグ手法とトラブルシューティング方法を定義します。

## 目次

1. [デバッグ方針](#デバッグ方針)
2. [ログ出力](#ログ出力)
3. [アサーションとデバッグビルド](#アサーションとデバッグビルド)
4. [デバッガの使用](#デバッガの使用)
5. [組み込みデバッグ](#組み込みデバッグ)
6. [よくある問題と解決策](#よくある問題と解決策)
7. [トラブルシューティング](#トラブルシューティング)

---

## デバッグ方針

### 1. 段階的デバッグ

**問題を段階的に絞り込む。**

1. **症状の特定** - 何が起きているか
2. **再現手順の確立** - いつ起きるか
3. **原因箇所の特定** - どこで起きているか
4. **根本原因の分析** - なぜ起きるか
5. **修正と検証** - どう直すか

### 2. デバッグビルドとリリースビルド

**開発時はデバッグビルド、本番はリリースビルド。**

```makefile
# デバッグビルド
CXXFLAGS_DEBUG = -std=c++17 -g -O0 -DDEBUG

# リリースビルド
CXXFLAGS_RELEASE = -std=c++17 -O2 -DNDEBUG
```

**デバッグビルドの特徴:**
- 最適化なし (`-O0`) - 変数の値が追跡可能
- デバッグシンボル (`-g`) - ソースコードとの対応
- アサーション有効 (`assert()` が実行される)
- ログ出力有効

**リリースビルドの特徴:**
- 最適化有効 (`-O2`/`-Os`)
- デバッグシンボルなし
- アサーション無効 (`assert()` は削除される)
- ログ出力最小限

### 3. 再現性の確保

**バグは再現可能でなければ修正できない。**

```cpp
// ✅ 良い例: 再現手順を記録
// 1. setup() でシリアル接続
// 2. loop() で10回データ送信
// 3. 11回目で必ずクラッシュ

// ❌ 悪い例: ランダムに発生
// たまにクラッシュする (再現できない)
```

---

## ログ出力

### 1. シリアルログ

**最も基本的なデバッグ手法。**

```cpp
// グローバル変数
SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;

void setup() {
    ctx.begin();
    serial = ctx.get_connectable_context()->get_serial_context(0);

    // デバッグログ
    serial->write("[DEBUG] setup() started\n"sv);
}

void loop() {
    ctx.update();

    // 変数の値を出力
    serial->write("[DEBUG] counter="sv);
    // counter の値を出力
    serial->write("\n"sv);

    ctx.delay(1000);
}
```

### 2. Loggerクラスの使用

**組み込み向け軽量Loggerを使用。**

Omusubiは `Logger` クラスを提供しています。ヒープ不使用、例外不使用で動作します。

```cpp
#include <omusubi/omusubi.h>

using namespace omusubi;

// グローバル変数
SystemContext& ctx = get_system_context();
SerialContext* serial = nullptr;
SerialLogOutput* log_output = nullptr;
Logger* logger = nullptr;

void setup() {
    ctx.begin();
    serial = ctx.get_connectable_context()->get_serial_context(0);

    // Logger初期化（スタック変数として保持）
    static SerialLogOutput log_out(serial);
    static Logger log(&log_out, LogLevel::INFO);
    log_output = &log_out;
    logger = &log;

    // ログ出力
    logger->info(std::string_view("System started", 14));
    logger->debug(std::string_view("Debug info", 10));  // min_level=INFOなので出力されない
}

void loop() {
    ctx.update();

    // 各ログレベルでの出力
    logger->debug(std::string_view("Debug message", 13));
    logger->info(std::string_view("Info message", 12));
    logger->warning(std::string_view("Warning message", 15));
    logger->error(std::string_view("Error message", 13));
    logger->critical(std::string_view("Critical message", 16));

    ctx.delay(1000);
}
```

**ログレベル:**
- `DEBUG` - デバッグ情報（開発時のみ）
- `INFO` - 一般情報
- `WARNING` - 警告（エラーではないが注意が必要）
- `ERROR` - エラー（機能の一部が失敗）
- `CRITICAL` - 致命的エラー（システムが継続不能）

**最小ログレベルの設定:**
```cpp
// デバッグビルド: DEBUGレベルから出力
logger->set_min_level(LogLevel::DEBUG);

// リリースビルド: WARNINGレベル以上のみ
logger->set_min_level(LogLevel::WARNING);
```

### 3. コンパイル時ログフィルタリング（マクロなし）

**デバッグビルドのみログ出力。**

Loggerクラスには、テンプレート特殊化によりコンパイル時にDEBUGログを削除する機能があります。
マクロを使用しない、型安全な実装です。

```cpp
// テンプレート関数 log_at<Level> を使用
void process() {
    // リリースビルド（NDEBUG定義時）ではDEBUGログが完全に削除される
    log_at<LogLevel::DEBUG>(*logger, std::string_view("process() called", 16));
    log_at<LogLevel::INFO>(*logger, std::string_view("Processing data", 15));

    // 処理本体
}

// 通常のメソッド呼び出しも可能（実行時フィルタリング）
logger->debug(std::string_view("Debug", 5));
logger->info(std::string_view("Info", 4));
```

**実装の仕組み:**
```cpp
// テンプレート特殊化によりDEBUGログを削除
template <>
struct LogDispatcher<LogLevel::DEBUG, false> {  // false = リリースビルド
    static void dispatch(const Logger&, std::string_view) {
        // 空実装 - コンパイラの最適化により完全に削除される
    }
};
```

**リリースビルドでの動作:**
- `log_at<LogLevel::DEBUG>()` は完全にコードから削除される（バイナリサイズ削減）
- `log_at<LogLevel::INFO>()` 以上は実行時にmin_levelで制御
- マクロ不使用により、型安全でデバッグしやすい

### 4. 関数トレース

**関数の呼び出しと終了を記録。**

```cpp
class FunctionTracer {
private:
    const char* name_;

public:
    explicit FunctionTracer(const char* name) : name_(name) {
        serial->write("[ENTER] "sv);
        serial->write(name_);
        serial->write("\n"sv);
    }

    ~FunctionTracer() {
        serial->write("[EXIT] "sv);
        serial->write(name_);
        serial->write("\n"sv);
    }
};

#ifdef DEBUG
    #define TRACE_FUNCTION() FunctionTracer __tracer__(__FUNCTION__)
#else
    #define TRACE_FUNCTION() ((void)0)
#endif

void connect_device() {
    TRACE_FUNCTION();  // [ENTER] connect_device

    // 処理
    device->connect();

    // スコープ終了時に [EXIT] connect_device
}
```

### 5. 変数ダンプ

**変数の内容を詳細に出力。**

```cpp
void dump_vector3(std::string_view name, const Vector3& v) {
    serial->write(name);
    serial->write(" = { x: "sv);
    // v.x を出力
    serial->write(", y: "sv);
    // v.y を出力
    serial->write(", z: "sv);
    // v.z を出力
    serial->write(" }\n"sv);
}

// 使用例
Vector3 acceleration = sensor->get_values();
dump_vector3("acceleration"sv, acceleration);
// 出力: acceleration = { x: 0.0, y: 0.0, z: 9.8 }
```

---

## アサーションとデバッグビルド

### 1. アサーションの使用

**プログラマエラーを早期検出。**

```cpp
#include <cassert>

void set_buffer_size(uint32_t size) {
    assert(size > 0);           // サイズは正の値
    assert(size <= MAX_SIZE);   // 最大値を超えない

    buffer_size_ = size;
}

// デバッグビルド: assert が失敗したらプログラム停止
// リリースビルド: assert は削除される
```

### 2. カスタムアサーション

**詳細なエラーメッセージ付きアサーション。**

```cpp
#ifdef DEBUG
    #define ASSERT_MSG(condition, message) \
        do { \
            if (!(condition)) { \
                serial->write("[ASSERTION FAILED] "sv); \
                serial->write(__FILE__); \
                serial->write(":"); \
                serial->write(__LINE__); \
                serial->write(" - "sv); \
                serial->write(message); \
                serial->write("\n"sv); \
                while (true) { }  // 停止 \
            } \
        } while (0)
#else
    #define ASSERT_MSG(condition, message) ((void)0)
#endif

// 使用例
ASSERT_MSG(port < 3, "シリアルポート番号が範囲外");
```

### 3. 境界チェック

**配列アクセスの境界チェック。**

```cpp
template<uint32_t N>
class FixedBuffer {
private:
    uint8_t buffer_[N];
    uint32_t size_ = 0;

public:
    uint8_t& operator[](uint32_t index) {
        // デバッグビルドのみ境界チェック
        assert(index < size_);
        return buffer_[index];
    }

    const uint8_t& operator[](uint32_t index) const {
        assert(index < size_);
        return buffer_[index];
    }
};
```

---

## デバッガの使用

### 1. GDB (ホスト環境)

**コマンドラインデバッガ。**

```bash
# デバッグビルド
clang++ -std=c++17 -g -O0 main.cpp -o main

# GDB起動
gdb ./main

# GDB コマンド
(gdb) break main           # main関数にブレークポイント
(gdb) run                  # 実行開始
(gdb) next                 # 次の行へ (ステップオーバー)
(gdb) step                 # 関数内へ (ステップイン)
(gdb) print variable       # 変数の値を表示
(gdb) backtrace            # スタックトレース
(gdb) continue             # 実行再開
(gdb) quit                 # 終了
```

### 2. LLDB (macOS/Linux)

**Clang用デバッガ。**

```bash
# LLDB起動
lldb ./main

# LLDB コマンド
(lldb) breakpoint set --name main
(lldb) run
(lldb) next
(lldb) step
(lldb) print variable
(lldb) bt
(lldb) continue
(lldb) quit
```

### 3. VS Code デバッガ

**統合デバッガ (推奨)。**

`.vscode/launch.json`:
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/main",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "preLaunchTask": "build"
        }
    ]
}
```

**使い方:**
1. ブレークポイント設定 (行番号左クリック)
2. F5 キーでデバッグ開始
3. F10: ステップオーバー
4. F11: ステップイン
5. 変数をホバーで値確認

---

## 組み込みデバッグ

### 1. シリアルモニタ

**リアルタイムログ確認。**

```bash
# PlatformIO
pio device monitor

# Arduino IDE
# ツール → シリアルモニタ

# 直接接続
screen /dev/ttyUSB0 115200
```

### 2. LED デバッグ

**シリアルが使えない場合の視覚的デバッグ。**

```cpp
// LED点滅でステータス表示
void signal_error(uint8_t error_code) {
    for (uint8_t i = 0; i < error_code; ++i) {
        led->turn_on();
        ctx.delay(200);
        led->turn_off();
        ctx.delay(200);
    }
}

// 使用例
if (!wifi->connect()) {
    signal_error(3);  // LED を3回点滅
}
```

### 3. ウォッチドッグタイマー

**ハングアップ検出。**

```cpp
void setup() {
    ctx.begin();

    // ウォッチドッグタイマー有効化 (5秒)
    watchdog_enable(5000);
}

void loop() {
    ctx.update();

    // 正常動作中はウォッチドッグをリセット
    watchdog_reset();

    // 処理
    process_data();

    // もし5秒以内に watchdog_reset() が呼ばれなければ
    // 自動的にリセットされる
}
```

### 4. クラッシュダンプ

**クラッシュ時の情報を保存。**

```cpp
// ESP32 の例
void setup() {
    // 前回のクラッシュ情報を表示
    if (ESP.getResetReason() == "SW_CPU_RESET") {
        serial->write("前回はソフトウェアリセット\n"sv);
    }

    // スタックトレース出力
    esp_backtrace_print(100);
}
```

### 5. リモートデバッグ

**WiFi 経由でデバッグ情報送信。**

```cpp
// WiFi経由でログ送信
void remote_log(std::string_view message) {
    if (wifi->is_connected()) {
        // UDP パケットでログサーバーに送信
        udp->send(LOG_SERVER_IP, LOG_SERVER_PORT, message);
    }

    // シリアルにもバックアップ出力
    serial->write(message);
}
```

---

## よくある問題と解決策

### 1. コンパイルエラー

**問題: テンプレート引数が不正**

```cpp
// ❌ エラー
FixedString<-1> str;  // 負の値

// 解決策: static_assert でコンパイル時チェック
template<uint32_t N>
class FixedString {
    static_assert(N > 0, "容量は1以上");
};
```

**問題: 型の不一致**

```cpp
// ❌ エラー
size_t len = str.byte_length();  // uint32_t を size_t に代入

// 解決策: 型を統一
uint32_t len = str.byte_length();
```

### 2. リンクエラー

**問題: 未定義の参照**

```
undefined reference to `get_system_context()'
```

**解決策: 実装ファイルをリンク**

```makefile
# system_context.cpp を追加
SRCS = main.cpp src/platform/m5stack/system_context.cpp
```

### 3. 実行時エラー

**問題: スタックオーバーフロー**

```cpp
// ❌ 問題: 大きすぎるバッファ
void process() {
    FixedBuffer<10000> huge_buffer;  // スタック不足
}

// ✅ 解決策: 静的領域に配置
static FixedBuffer<10000> huge_buffer;

void process() {
    huge_buffer.clear();
    // 使用
}
```

**問題: null ポインタアクセス**

```cpp
// ❌ 問題
SerialContext* serial = nullptr;
serial->write("Hello"sv);  // クラッシュ

// ✅ 解決策: アサーションで検出
SerialContext* serial = ctx.get_serial_context(0);
assert(serial != nullptr);
serial->write("Hello"sv);
```

### 4. メモリ破壊

**問題: バッファオーバーフロー**

```cpp
// ❌ 問題
FixedString<10> str;
str.append("This is a very long string");  // 容量超過

// ✅ 解決策: 戻り値チェック
if (!str.append("This is a very long string")) {
    log(LogLevel::ERROR, "バッファ容量不足"sv);
}
```

### 5. パフォーマンス問題

**問題: ループが遅い**

```cpp
// ❌ 問題: 毎回ポインタ取得
void loop() {
    auto* serial = ctx.get_connectable_context()->get_serial_context(0);
    serial->write("data"sv);  // 毎回メソッドチェーン
}

// ✅ 解決策: ポインタをキャッシュ
SerialContext* serial = nullptr;

void setup() {
    serial = ctx.get_connectable_context()->get_serial_context(0);
}

void loop() {
    serial->write("data"sv);  // キャッシュされたポインタ
}
```

---

## トラブルシューティング

### 1. デバッグフローチャート

```
問題発生
  ↓
コンパイルエラー? → YES → エラーメッセージ確認 → 型/構文修正
  ↓ NO
リンクエラー? → YES → 実装ファイル確認 → Makefile修正
  ↓ NO
実行時クラッシュ? → YES → GDB/LLDB でデバッグ → null チェック/境界チェック
  ↓ NO
期待と異なる動作? → YES → ログ出力追加 → ロジック確認
  ↓ NO
パフォーマンス問題? → YES → プロファイリング → 最適化
```

### 2. チェックリスト

**コンパイルエラー:**
- [ ] 型は正しいか (uint32_t vs size_t)
- [ ] テンプレート引数は有効か
- [ ] ヘッダーファイルをインクルードしているか
- [ ] 名前空間は正しいか

**リンクエラー:**
- [ ] 実装ファイルがビルドに含まれているか
- [ ] プラットフォーム固有の実装をリンクしているか
- [ ] extern "C" が必要な箇所で使用しているか

**実行時エラー:**
- [ ] null ポインタをチェックしているか
- [ ] 配列の境界を超えていないか
- [ ] スタックサイズは十分か
- [ ] 戻り値をチェックしているか

**ロジックエラー:**
- [ ] エラー処理は適切か
- [ ] 初期化を忘れていないか
- [ ] 条件分岐は正しいか
- [ ] ループの終了条件は正しいか

### 3. デバッグツール一覧

**ホスト環境:**
- GDB/LLDB: コマンドラインデバッガ
- VS Code: 統合デバッガ
- Valgrind: メモリリーク検出
- AddressSanitizer: メモリエラー検出
- gprof: プロファイラ

**組み込み環境:**
- シリアルモニタ: ログ出力
- JTAG デバッガ: ハードウェアデバッグ
- LED デバッグ: 視覚的フィードバック
- ウォッチドッグタイマー: ハングアップ検出

---

## 関連ドキュメント

- [エラーハンドリングガイド](error-handling.md) - エラー処理パターン
- [テストガイド](testing.md) - テストによるバグ検出
- [パフォーマンスガイド](performance.md) - パフォーマンス問題の解決

---

**Version:** 1.3.0
**Last Updated:** 2025-11-25
