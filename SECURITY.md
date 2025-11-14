# Security Policy

## サポートされているバージョン

現在、セキュリティ更新が提供されているバージョン：

| Version | Supported          |
| ------- | ------------------ |
| 0.1.x   | :white_check_mark: |
| < 0.1   | :x:                |

## セキュリティ脆弱性の報告

Omusubiプロジェクトのセキュリティを重視しています。セキュリティ脆弱性を発見した場合は、以下の手順に従って報告してください。

### 報告方法

**公開Issueでは報告しないでください。** セキュリティ脆弱性は、悪用されるリスクを最小限に抑えるため、非公開で報告してください。

#### 推奨：GitHub Security Advisories

1. GitHubリポジトリの「Security」タブを開く
2. 「Report a vulnerability」をクリック
3. 脆弱性の詳細を記入して送信

#### 代替：メール

GitHub Security Advisoriesが利用できない場合は、メールで報告してください：

**Email:** security@your-domain.com

**件名:** [SECURITY] Omusubi Security Vulnerability Report

### 報告に含めるべき情報

脆弱性を迅速に評価・修正するため、以下の情報を可能な限り提供してください：

1. **脆弱性の種類**
   - バッファオーバーフロー
   - メモリリーク
   - 不正なポインタアクセス
   - その他

2. **影響を受けるコンポーネント**
   - ファイルパスと行番号
   - 影響を受けるプラットフォーム（M5Stack, Arduino等）

3. **再現手順**
   - 脆弱性を再現するための詳細な手順
   - 再現可能なコード例

4. **影響範囲**
   - どのような攻撃が可能か
   - 想定される被害（情報漏洩、DoS、コード実行等）

5. **発見したバージョン**
   - Omusubiのバージョンまたはコミットハッシュ

6. **環境情報**
   - プラットフォーム（M5Stack, Arduino等）
   - ビルド環境（PlatformIO, Arduino IDE等）

### レポートの例

```
タイトル: [SECURITY] Buffer overflow in FixedString::append

概要:
FixedString<N>::append()メソッドで容量チェックが不十分なため、
バッファオーバーフローが発生する可能性があります。

再現手順:
1. FixedString<8> str; を作成
2. str.append("12345678"); を実行（容量ちょうど）
3. str.append("9"); を実行
4. バッファオーバーフローが発生

影響:
スタックメモリの破壊、予期しない動作、クラッシュの可能性

バージョン: v0.1.0
プラットフォーム: M5Stack
```

## セキュリティ脆弱性の対応プロセス

### 1. 受領確認（24時間以内）

報告を受け取ったことを確認するメールを送信します。

### 2. 初期評価（3営業日以内）

- 脆弱性の妥当性を評価
- 影響範囲と深刻度を判断
- 対応優先度を決定

### 3. 修正開発

**深刻度による対応時間:**

| 深刻度 | 対応時間目安 | 説明 |
|--------|-------------|------|
| Critical | 7日以内 | リモートコード実行、任意メモリアクセス等 |
| High | 30日以内 | 権限昇格、DoS、情報漏洩等 |
| Medium | 90日以内 | 限定的な影響 |
| Low | 次回リリース | 軽微な影響 |

### 4. 修正のリリース

- セキュリティパッチをリリース
- CHANGELOGに記載
- GitHub Security Advisoryを公開
- 報告者にクレジットを付与（希望する場合）

### 5. 公開通知

修正版リリース後、以下を実施：
- セキュリティアドバイザリの公開
- READMEでの告知
- 影響を受けるユーザーへの通知（重大な場合）

## セキュリティのベストプラクティス

Omusubiを使用する際のセキュリティベストプラクティス：

### 1. 入力検証

外部からの入力は必ず検証してください：

```cpp
// ❌ 危険：検証なし
void process_input(SerialContext* serial) {
    char buffer[256];
    size_t n = serial->read_line(span<char>(buffer, 256));
    // bufferを直接使用
}

// ✅ 安全：検証あり
void process_input(SerialContext* serial) {
    char buffer[256];
    size_t n = serial->read_line(span<char>(buffer, 256));

    // 長さチェック
    if (n == 0 || n >= 256) {
        return;  // 不正な入力
    }

    // null終端を保証
    buffer[n] = '\0';

    // 使用
}
```

### 2. バッファサイズの適切な管理

```cpp
// ❌ 危険：容量超過の可能性
FixedString<64> str;
for (int i = 0; i < 100; ++i) {
    str.append('x');  // 64バイト超過
}

// ✅ 安全：容量チェック
FixedString<64> str;
for (int i = 0; i < 100; ++i) {
    if (!str.append('x')) {
        break;  // 容量不足で停止
    }
}
```

### 3. ポインタの妥当性チェック

```cpp
// ❌ 危険：nullptrチェックなし
SerialContext* serial = ctx.get_serial_context(999);
serial->write("data"_sv);  // クラッシュの可能性

// ✅ 安全：nullptrチェック
SerialContext* serial = ctx.get_serial_context(0);
if (serial != nullptr) {
    serial->write("data"_sv);
}
```

### 4. WiFi/Bluetoothのセキュリティ

```cpp
// ✅ WiFi接続時の推奨設定
WiFiContext* wifi = ctx.get_wifi_context();

// WPA2以上を使用（WEP/WPA1は避ける）
wifi->connect_to("SSID"_sv, "strong_password"_sv);

// 接続確認
if (!wifi->is_connected()) {
    // エラー処理
}
```

### 5. センシティブ情報の扱い

```cpp
// ❌ 危険：パスワードをハードコード
wifi->connect_to("MyWiFi"_sv, "password123"_sv);

// ✅ 推奨：設定ファイルや環境変数から読み込む
// （実装はプラットフォーム依存）
FixedString<64> ssid = load_from_config("wifi_ssid");
FixedString<64> password = load_from_config("wifi_password");
wifi->connect_to(ssid.view(), password.view());
```

## 既知の制限事項

Omusubiは組み込みシステム向けに設計されており、以下の制約があります：

### メモリ安全性

- **スタックオーバーフロー:** 大きなバッファをスタックに確保する場合、スタックサイズに注意
- **バッファオーバーフロー:** `FixedString`/`FixedBuffer`は容量チェックを行うが、手動でのインデックスアクセスは保護されない

### ネットワークセキュリティ

- **暗号化:** WiFi/Bluetoothの暗号化はプラットフォーム依存
- **認証:** デバイス認証はアプリケーション層で実装する必要がある
- **TLS/SSL:** 現時点では未サポート（将来的に検討）

### 物理的セキュリティ

- **デバッグポート:** シリアルポートは常に有効
- **ファームウェア保護:** コード保護はプラットフォーム依存

## セキュリティ更新の通知

セキュリティ更新の通知を受け取る方法：

1. **GitHub Watch:** リポジトリを"Watch" → "Security alerts only"
2. **GitHub Security Advisories:** セキュリティタブでアドバイザリを確認
3. **CHANGELOG.md:** セキュリティ修正はCHANGELOGに記載

## 連絡先

セキュリティに関する質問や懸念事項：

- **GitHub Security Advisories:** [推奨] 脆弱性報告用
- **Email:** security@your-domain.com
- **GitHub Discussions:** 一般的なセキュリティ質問用

## クレジット

セキュリティ脆弱性を報告してくださった方々に感謝します：

<!-- 報告者のリストをここに追加 -->

## 参考資料

- [CERT Secure Coding Standards](https://wiki.sei.cmu.edu/confluence/display/seccode)
- [OWASP Embedded Application Security](https://owasp.org/www-project-embedded-application-security/)
- [CWE - Common Weakness Enumeration](https://cwe.mitre.org/)
