# パフォーマンスガイド

このドキュメントでは、Omusubiフレームワークにおけるパフォーマンス最適化の方針と実装手法を定義します。

## 目次

1. [基本方針](#基本方針)
2. [メモリ最適化](#メモリ最適化)
3. [CPU最適化](#CPU最適化)
4. [コンパイル時計算](#コンパイル時計算)
5. [キャッシング戦略](#キャッシング戦略)
6. [ビルド最適化](#ビルド最適化)
7. [計測とプロファイリング](#計測とプロファイリング)
8. [アンチパターン](#アンチパターン)

---

## 基本方針

### 1. ゼロオーバーヘッド抽象化

**抽象化のコストは実行時にゼロであるべき。**

```cpp
// ✅ 良い例: インライン展開されて直接アクセスと同等
class Vector3 {
    float x, y, z;
public:
    constexpr float get_x() const { return x; }  // インライン化
};

// コンパイル後は以下と同等:
// float value = vec.x;

// ❌ 悪い例: 仮想関数テーブル経由のアクセス (不要なオーバーヘッド)
class Vector3Base {
public:
    virtual float get_x() const = 0;  // 仮想関数呼び出しコスト
};
```

**ゼロオーバーヘッドの実現手段:**
- `constexpr` 関数 (コンパイル時計算)
- `inline` 関数 (関数呼び出しコスト削減)
- テンプレート (型に応じた最適化)
- 参照・ポインタ (コピーコスト削減)

### 2. 最適化の優先順位

**最適化は以下の順序で実施:**

1. **アルゴリズム選択** - O(n²) → O(n log n) の改善が最大の効果
2. **データ構造** - キャッシュ効率の良い配置
3. **メモリアロケーション** - ヒープ割り当て削減
4. **コンパイル時計算** - 実行時計算の削減
5. **マイクロ最適化** - ループ展開、分岐予測など

### 3. 計測駆動の最適化

**推測せず、計測してから最適化する。**

```cpp
// 計測なしの最適化は時間の無駄
// 1. プロファイラで計測
// 2. ボトルネックを特定
// 3. 最適化実施
// 4. 再計測して効果検証
```

---

## メモリ最適化

### 1. ヒープアロケーション禁止

**すべてのオブジェクトはスタックまたは静的領域に配置。**

```cpp
// ✅ 良い例: スタック割り当て
void process_data() {
    FixedString<256> buffer;  // スタック
    buffer.append("Hello"sv);
    // 関数終了時に自動解放
}

// ✅ 良い例: 静的割り当て
static FixedBuffer<1024> global_buffer;

// ❌ 悪い例: ヒープ割り当て
void process_data() {
    auto* buffer = new char[256];  // 禁止
    // ...
    delete[] buffer;
}
```

**理由:**
- ヒープアロケーションは遅い (システムコール)
- メモリフラグメンテーション
- メモリリーク リスク
- リアルタイム性の損失

### 2. 固定サイズコンテナ

**実行時サイズ変更が不要ならテンプレートパラメータで容量指定。**

```cpp
// ✅ 良い例: コンパイル時にサイズ確定
FixedString<64> device_name;
FixedBuffer<256> packet_buffer;

// サイズはコンパイル時に決定、実行時オーバーヘッドなし
static_assert(sizeof(FixedString<64>) == 64 + sizeof(uint32_t), "");

// ❌ 悪い例: 動的サイズ変更 (組み込みでは不要)
std::vector<char> buffer;  // ヒープ割り当て
buffer.resize(256);         // 実行時サイズ変更
```

**利点:**
- コンパイル時にメモリ配置決定
- キャッシュ効率が良い
- メモリ使用量が予測可能

### 3. アラインメント最適化

**構造体メンバーはサイズ順に配置してパディングを削減。**

```cpp
// ❌ 悪い例: パディングが発生 (12バイト)
struct BadLayout {
    uint8_t a;   // 1バイト
    // 3バイトのパディング
    uint32_t b;  // 4バイト
    uint8_t c;   // 1バイト
    // 3バイトのパディング
};
static_assert(sizeof(BadLayout) == 12, "");

// ✅ 良い例: パディング最小化 (8バイト)
struct GoodLayout {
    uint32_t b;  // 4バイト
    uint8_t a;   // 1バイト
    uint8_t c;   // 1バイト
    // 2バイトのパディング
};
static_assert(sizeof(GoodLayout) == 8, "");
```

**パディング削減のルール:**
1. 大きい型から順に配置 (uint64_t → uint32_t → uint16_t → uint8_t)
2. 同じサイズの型をまとめる
3. `#pragma pack` は使わない (パフォーマンス劣化)

### 4. ローカル変数の最小化

**スタック使用量を抑える。**

```cpp
// ✅ 良い例: 必要な時だけスコープ内で定義
void process() {
    // 処理1
    {
        FixedString<256> temp;
        temp.append("data"sv);
        // スコープ終了でスタック解放
    }

    // 処理2
    {
        FixedBuffer<512> buffer;
        // 別のスコープで再利用
    }
}

// ❌ 悪い例: 関数全体で大きなバッファを保持
void process() {
    FixedString<256> temp;     // 常に256バイト消費
    FixedBuffer<512> buffer;   // 常に512バイト消費
    // ... 長い処理
}
```

---

## CPU最適化

### 1. 関数インライン化

**小さな関数はインライン展開してコールオーバーヘッド削減。**

```cpp
// ✅ 自動的にインライン化される (ヘッダーオンリー)
class FixedString {
public:
    constexpr uint32_t byte_length() const {
        return length_;  // 1命令に展開
    }
};

// コンパイル後:
// uint32_t len = str.length_;  // 関数呼び出しなし

// ❌ インライン化されない (大きすぎる関数)
void complex_processing() {
    // 100行以上のコード
    // コンパイラはインライン化しない
}
```

**インライン化のガイドライン:**
- 1〜5行程度の関数
- ループを含まない
- constexpr 関数は積極的にインライン化される

### 2. 分岐予測の最適化

**条件分岐は予測可能なパターンにする。**

```cpp
// ✅ 良い例: 予測可能な分岐
for (uint32_t i = 0; i < count; ++i) {
    if (buffer[i] != 0) {  // データ依存だが、ループ内で一貫性がある
        process(buffer[i]);
    }
}

// ❌ 悪い例: 予測不可能な分岐
for (uint32_t i = 0; i < count; ++i) {
    if (random() % 2 == 0) {  // ランダム → ミスプレディクション多発
        process(buffer[i]);
    }
}

// ✅ 良い例: likely/unlikely ヒント (C++20以降、C++14では使えない)
// C++14では分岐を減らす設計で対応
if (error_condition) {  // エラーは稀
    handle_error();
}
// 通常処理
```

**分岐削減のテクニック:**

```cpp
// ❌ 条件分岐あり
int abs_value(int x) {
    if (x < 0) {
        return -x;
    }
    return x;
}

// ✅ 分岐なし (ビット演算)
int abs_value(int x) {
    int mask = x >> 31;
    return (x + mask) ^ mask;
}
```

### 3. ループ最適化

**ループカウントを減らし、ループ展開を促進。**

```cpp
// ❌ 悪い例: 毎回サイズ取得
for (uint32_t i = 0; i < buffer.size(); ++i) {
    process(buffer[i]);
}

// ✅ 良い例: サイズをキャッシュ
uint32_t count = buffer.size();
for (uint32_t i = 0; i < count; ++i) {
    process(buffer[i]);
}

// ✅ 最適例: 範囲for文 (イテレータ最適化)
for (auto& item : buffer) {
    process(item);
}
```

**ループアンローリング:**

```cpp
// コンパイラが自動展開
for (uint32_t i = 0; i < 4; ++i) {
    buffer[i] = 0;
}

// 展開後 (コンパイラが生成):
// buffer[0] = 0;
// buffer[1] = 0;
// buffer[2] = 0;
// buffer[3] = 0;
```

### 4. 仮想関数呼び出しの回避

**ホットパス (頻繁に実行される部分) では仮想関数を避ける。**

```cpp
// ❌ 悪い例: ループ内で仮想関数呼び出し
for (uint32_t i = 0; i < 10000; ++i) {
    device->write(data);  // 仮想関数 → 間接呼び出し
}

// ✅ 良い例: ループ外で型確定
SerialContext* serial = static_cast<SerialContext*>(device);
for (uint32_t i = 0; i < 10000; ++i) {
    serial->write(data);  // 直接呼び出し (インライン化可能)
}

// ✅ 最適例: テンプレート特殊化
template<typename DeviceT>
void bulk_write(DeviceT* device, const uint8_t* data, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        device->write(data[i]);  // 型が確定 → インライン化
    }
}
```

---

## コンパイル時計算

### 1. constexpr の活用

**実行時計算をコンパイル時に移動。**

```cpp
// ✅ コンパイル時に計算
constexpr uint32_t calculate_buffer_size(uint32_t elements) {
    return elements * sizeof(uint32_t) + sizeof(uint32_t);
}

constexpr uint32_t BUFFER_SIZE = calculate_buffer_size(256);
// BUFFER_SIZE = 1028 (コンパイル時に確定)

// ❌ 実行時計算
uint32_t buffer_size = elements * sizeof(uint32_t) + sizeof(uint32_t);
```

**constexpr の利点:**
- 実行時オーバーヘッドゼロ
- 即値として埋め込まれる
- テンプレート引数に使用可能

### 2. コンパイル時文字列処理

**文字列の長さやハッシュ値をコンパイル時計算。**

```cpp
// ✅ コンパイル時文字列長計算
constexpr uint32_t string_length(const char* str) {
    uint32_t len = 0;
    while (str[len] != '\0') {
        ++len;
    }
    return len;
}

constexpr auto NAME = "DeviceName";
constexpr uint32_t NAME_LENGTH = string_length(NAME);  // = 10

// 実行時に長さ計算不要
FixedString<NAME_LENGTH> device_name(NAME);
```

### 3. static_assert による検証

**不正な値をコンパイル時に検出。**

```cpp
template<uint32_t N>
class FixedString {
    static_assert(N > 0, "容量は1以上である必要がある");
    static_assert(N <= 4096, "容量が大きすぎる (スタックオーバーフロー)");

    char buffer_[N + 1];
};

// コンパイルエラー:
// FixedString<0> invalid;      // 容量は1以上である必要がある
// FixedString<10000> huge;     // 容量が大きすぎる
```

---

## キャッシング戦略

### 1. Context ポインタのキャッシュ

**頻繁にアクセスするデバイスポインタはグローバル変数にキャッシュ。**

```cpp
// ❌ 悪い例: 毎回メソッドチェーンで取得
void loop() {
    auto* serial = ctx.get_connectable_context()->get_serial_context(0);
    serial->write("Hello"sv);  // 毎回ポインタ取得
}

// ✅ 良い例: setup() で一度だけ取得
SerialContext* serial = nullptr;

void setup() {
    serial = ctx.get_connectable_context()->get_serial_context(0);
}

void loop() {
    serial->write("Hello"sv);  // キャッシュされたポインタを使用
}
```

**パフォーマンス改善:**
- メソッド呼び出し削減: 2回 → 0回
- ポインタ取得: 毎回 → 1回のみ

### 2. 頻繁に使う値のキャッシュ

**計算結果を再利用。**

```cpp
// ❌ 悪い例: 毎回計算
for (uint32_t i = 0; i < buffer.size(); ++i) {
    if (i < buffer.size() / 2) {  // 毎回除算
        process(buffer[i]);
    }
}

// ✅ 良い例: 計算結果をキャッシュ
uint32_t half = buffer.size() / 2;
for (uint32_t i = 0; i < buffer.size(); ++i) {
    if (i < half) {
        process(buffer[i]);
    }
}
```

### 3. データ局所性の向上

**キャッシュヒット率を高める配置。**

```cpp
// ❌ 悪い例: データが分散
struct Device {
    uint8_t* data;      // 別の場所を参照
    uint32_t size;
};

Device devices[10];
for (auto& dev : devices) {
    process(dev.data);  // キャッシュミス多発
}

// ✅ 良い例: データを埋め込み
struct Device {
    uint8_t data[256];  // データを構造体内に配置
    uint32_t size;
};

Device devices[10];  // 連続したメモリ領域
for (auto& dev : devices) {
    process(dev.data);  // キャッシュヒット率向上
}
```

---

## ビルド最適化

### 1. 最適化レベル

**リリースビルドは `-O2` または `-Os` を使用。**

```makefile
# デバッグビルド
CXXFLAGS_DEBUG = -std=c++17 -g -O0

# リリースビルド (速度優先)
CXXFLAGS_RELEASE = -std=c++17 -O2 -DNDEBUG

# リリースビルド (サイズ優先、組み込みシステム推奨)
CXXFLAGS_RELEASE_SIZE = -std=c++17 -Os -DNDEBUG
```

**最適化レベルの選択:**
- `-O0`: デバッグ用 (最適化なし)
- `-O1`: 基本的な最適化
- `-O2`: 速度優先の最適化 (推奨)
- `-O3`: 積極的な最適化 (コードサイズ増大)
- `-Os`: サイズ優先の最適化 (組み込み推奨)

### 2. LTO (Link Time Optimization)

**リンク時最適化でさらなる最適化。**

```makefile
# LTO有効化
CXXFLAGS_RELEASE = -std=c++17 -O2 -flto -DNDEBUG
LDFLAGS_RELEASE = -flto
```

**効果:**
- インライン展開の範囲拡大
- 未使用コードの削除
- 関数の統合

### 3. 不要な機能の無効化

**組み込み環境では不要な機能を無効化。**

```makefile
# RTTI 無効化 (type_info 不要)
CXXFLAGS += -fno-rtti

# 例外無効化 (組み込みでは使わない)
CXXFLAGS += -fno-exceptions
```

**効果:**
- コードサイズ削減: 5〜20%
- 実行速度向上: 1〜5%

---

## 計測とプロファイリング

### 1. 実行時間計測

**マイクロ秒単位で計測。**

```cpp
// ホスト環境での計測
#include <chrono>

void benchmark() {
    auto start = std::chrono::high_resolution_clock::now();

    // 計測対象の処理
    for (uint32_t i = 0; i < 10000; ++i) {
        process_data();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    printf("実行時間: %lld μs\n", duration.count());
}
```

**実機での計測:**

```cpp
// M5Stack での計測 (millis() 使用)
void benchmark() {
    uint32_t start = millis();

    for (uint32_t i = 0; i < 1000; ++i) {
        process_data();
    }

    uint32_t elapsed = millis() - start;
    serial->write("実行時間: "sv);
    // elapsed を出力
    serial->write(" ms\n"sv);
}
```

### 2. メモリ使用量計測

**スタックとヒープの使用量を確認。**

```cpp
// スタック使用量推定
void estimate_stack_usage() {
    FixedString<256> str;
    FixedBuffer<1024> buffer;
    Vector3 vec;

    // 合計スタック使用量
    uint32_t stack_usage = sizeof(str) + sizeof(buffer) + sizeof(vec);
    printf("スタック使用量: %u バイト\n", stack_usage);
}

// M5Stack でのヒープ使用量
void check_heap() {
    uint32_t free_heap = ESP.getFreeHeap();
    serial->write("空きヒープ: "sv);
    // free_heap を出力
}
```

### 3. プロファイリングツール

**ホスト環境でのプロファイリング:**

```bash
# gprof によるプロファイリング
clang++ -std=c++17 -pg -O2 main.cpp -o main
./main
gprof main gmon.out > profile.txt

# Valgrind Callgrind
valgrind --tool=callgrind ./main
kcachegrind callgrind.out.*

# perf (Linux)
perf record ./main
perf report
```

---

## アンチパターン

### 1. 不要なコピー

```cpp
// ❌ 悪い例: 毎回コピー
FixedString<256> get_device_name() {
    FixedString<256> name("Device");
    return name;  // コピーコンストラクタ呼び出し
}

// ✅ 良い例: std::string_view で参照を返す
std::string_view get_device_name() {
    static constexpr char NAME[] = "Device";
    return std::string_view(NAME);  // コピーなし
}
```

### 2. 小さな文字列への大きなバッファ

```cpp
// ❌ 悪い例: メモリ無駄
FixedString<1024> device_name("M5Stack");  // 1024バイト確保、7バイトしか使わない

// ✅ 良い例: 適切なサイズ
FixedString<64> device_name("M5Stack");  // 64バイトで十分
```

### 3. ループ内での重い処理

```cpp
// ❌ 悪い例: ループ内でメモリアロケーション
void process() {
    for (uint32_t i = 0; i < 1000; ++i) {
        FixedString<256> temp;  // 毎回構築
        temp.append("data"sv);
        use(temp);
    }
}

// ✅ 良い例: ループ外で一度だけ確保
void process() {
    FixedString<256> temp;
    for (uint32_t i = 0; i < 1000; ++i) {
        temp.clear();
        temp.append("data"sv);
        use(temp);
    }
}
```

### 4. 文字列連結の非効率

```cpp
// ❌ 悪い例: 複数回の append
FixedString<256> message;
message.append("Error: "sv);
message.append("code="sv);
message.append("123"sv);

// ✅ 良い例: まとめて append
FixedString<256> message;
message.append("Error: code=123"sv);  // 1回のみ
```

### 5. 不要な仮想関数

```cpp
// ❌ 悪い例: 仮想関数が不要
class Config {
public:
    virtual uint32_t get_timeout() const { return 1000; }  // 継承しないのに仮想関数
};

// ✅ 良い例: 非仮想関数
class Config {
public:
    constexpr uint32_t get_timeout() const { return 1000; }  // インライン化可能
};
```

---

## パフォーマンスチェックリスト

コード最適化時のチェックリスト:

- [ ] ヒープアロケーションを使用していない
- [ ] 固定サイズコンテナを使用している
- [ ] constexpr でコンパイル時計算を活用している
- [ ] Context ポインタをキャッシュしている
- [ ] ループ内で不要な計算をしていない
- [ ] 仮想関数呼び出しを最小化している
- [ ] 構造体のパディングを最適化している
- [ ] 不要なコピーを避けている
- [ ] リリースビルドで `-O2` または `-Os` を使用している
- [ ] 計測してボトルネックを特定している

---

## 関連ドキュメント

- [CLAUDE.md](../CLAUDE.md) - ゼロオーバーヘッド抽象化の設計原則
- [テストガイド](testing.md) - パフォーマンステストの書き方
- [ビルドシステムガイド](build-system.md) - 最適化オプションの設定

---

**Version:** 1.1.0
**Last Updated:** 2025-11-25
