# ハードウェア抽象化ガイドライン

このドキュメントでは、I2Cマルチプレクサー（PaHub等）やその他のハードウェア拡張をOmusubiで扱う際の設計指針を説明します。

## 設計方針

### 基本原則

Omusubiでは、ハードウェア拡張（I2C Mux、GPIO Expander等）を**Platform層で個別実装**することを推奨します。

**理由:**

1. **ハードウェア構成は固定的** - プロジェクトごとにセンサー構成は決まっており、動的に変わることはほぼない
2. **明示的なコードの方が分かりやすい** - 抽象化層が増えるとデバッグが困難
3. **YAGNI** - 複雑なケース（カスケード接続等）は、実際に必要になってから検討すべき

### 非推奨: 過度な抽象化

```cpp
// ❌ 過度な抽象化 - 何が繋がってるか分からない
auto* sensor = ctx->get_sensor_by_path({0, 2, 1});
auto* sensor = ctx->get_i2c_channel(0)->get_sub_channel(2)->get_sensor(1);
```

### 推奨: 明示的な実装

```cpp
// ✅ 明示的 - 一目瞭然
auto* dlight = ctx->get_dlight_sensor();   // PaHub CH0
auto* color = ctx->get_color_sensor();      // PaHub CH1
```

## I2Cマルチプレクサー（PaHub）対応

### 実装パターン

PaHubなどのI2Cマルチプレクサーは、**センサークラス内部でチャンネル選択を隠蔽**します。

#### 1. PaHubラッパークラス

```cpp
// PaHubの薄いラッパー
class PaHub {
    TwoWire& wire_;
    uint8_t address_;

public:
    PaHub(TwoWire& wire, uint8_t address = 0x70)
        : wire_(wire), address_(address) {}

    void select(uint8_t channel) const {
        wire_.beginTransmission(address_);
        wire_.write(1 << channel);
        wire_.endTransmission();
    }

    void deselect() const {
        wire_.beginTransmission(address_);
        wire_.write(0);
        wire_.endTransmission();
    }
};
```

#### 2. PaHub経由のセンサークラス

```cpp
// DLightセンサー（PaHub経由）
class M5DLightOnPaHub : public Measurable1D {
    PaHub& hub_;
    uint8_t channel_;
    // DLightの内部状態...

public:
    M5DLightOnPaHub(PaHub& hub, uint8_t channel)
        : hub_(hub), channel_(channel) {}

    [[nodiscard]] float get_value() const override {
        hub_.select(channel_);
        float value = read_sensor_value();
        hub_.deselect();
        return value;
    }

private:
    float read_sensor_value() const {
        // センサー固有の読み取り処理
    }
};
```

#### 3. Platform Context実装

```cpp
// M5Stack向けSensorContext実装
class M5StackSensorContext : public SensorContext {
    PaHub pahub_{Wire, 0x70};

    // センサーインスタンス（チャンネル固定）
    mutable M5DLightOnPaHub dlight_{pahub_, 0};      // CH0: DLight
    mutable M5ColorSensorOnPaHub color_{pahub_, 1};  // CH1: カラーセンサー

public:
    [[nodiscard]] Measurable1D* get_dlight_sensor() const {
        return &dlight_;
    }

    [[nodiscard]] Measurable1D* get_color_sensor() const {
        return &color_;
    }
};
```

### 使用例

```cpp
#include <omusubi/omusubi.h>

using namespace omusubi;

void setup() {
    auto& ctx = get_system_context();
    ctx.begin();
}

void loop() {
    auto& ctx = get_system_context();
    auto* sensor_ctx = ctx.get_sensor_context();

    // PaHubの存在を意識せずにアクセス
    float lux = sensor_ctx->get_dlight_sensor()->get_value();
    float color = sensor_ctx->get_color_sensor()->get_value();

    ctx.delay(100);
}
```

## 設計判断のフローチャート

新しいハードウェア拡張を追加する際の判断基準：

```
ハードウェア拡張が必要
        │
        ▼
┌───────────────────────────┐
│ 構成は固定的か？          │
│ （プロジェクト内で変わらない）│
└───────────────────────────┘
        │
    Yes │              No
        ▼              ▼
┌──────────────┐  ┌──────────────────────┐
│ Platform層で  │  │ 本当に動的な構成が    │
│ 個別実装      │  │ 必要か再検討          │
│ （推奨）      │  │ （ほとんどの場合不要）  │
└──────────────┘  └──────────────────────┘
```

## 複雑なケースへの対応

### カスケード接続（Mux→Mux）

カスケード接続が必要な場合も、**Platform層での明示的な実装**を推奨します。

```cpp
// カスケード接続の例
class CascadedSensor : public Measurable1D {
    PaHub& hub1_;
    PaHub& hub2_;
    uint8_t ch1_;
    uint8_t ch2_;

public:
    CascadedSensor(PaHub& hub1, uint8_t ch1, PaHub& hub2, uint8_t ch2)
        : hub1_(hub1), ch1_(ch1), hub2_(hub2), ch2_(ch2) {}

    [[nodiscard]] float get_value() const override {
        hub1_.select(ch1_);   // 親Mux選択
        hub2_.select(ch2_);   // 子Mux選択
        float value = read_sensor();
        hub2_.deselect();
        hub1_.deselect();
        return value;
    }
};
```

### 排他制御が必要な場合

マルチタスク環境で複数のセンサーが同時にアクセスする場合は、Platform層でミューテックスを実装してください。

```cpp
class ThreadSafePaHub {
    PaHub hub_;
    mutable SemaphoreHandle_t mutex_;

public:
    void select(uint8_t channel) const {
        xSemaphoreTake(mutex_, portMAX_DELAY);
        hub_.select(channel);
    }

    void deselect() const {
        hub_.deselect();
        xSemaphoreGive(mutex_);
    }
};
```

## まとめ

| 方針 | 説明 |
|------|------|
| **抽象化しない** | core/interfaceにハブ関連のインターフェースは追加しない |
| **Platform層で実装** | 各プラットフォームで具体的なセンサークラスを作成 |
| **チャンネルは固定** | コンストラクタでチャンネルを指定し、実行時に変更しない |
| **明示的なAPI** | `get_dlight_sensor()`のように、何が取得できるか明確に |

---

**Version:** 1.0.0
**Last Updated:** 2025-12-13
