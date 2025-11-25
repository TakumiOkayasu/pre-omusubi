# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- 包括的なドキュメント整備
  - Architecture design document
  - Contributing guide
  - Platform support documentation
  - API reference
  - Testing guide
- GitHub Issue templates (Bug report, Feature request, Platform support)
- Pull Request template
- Code quality tools integration (clang-format, clang-tidy)
- CI/CD pipeline with GitHub Actions

### Changed
- Interface hierarchy redesign
  - Split `Readable` into `ByteReadable` and `TextReadable`
  - Split `Writable` into `ByteWritable` and `TextWritable`
- `Scannable` interface改善
  - `scan()` → `start_scan()` / `stop_scan()`
  - スキャン結果取得メソッド追加
- `Connectable` interface拡張
  - `disconnect()` メソッド追加
  - `is_connected()` メソッド追加

### Fixed
- M5Stack Serial context copy constructor error
- Header dependency issues (missing `<cstdint>` includes)
- Code formatting consistency across all files

## [0.1.0] - 2025-11-25 (Initial Release)

### Added
- Core types
  - `std::string_view` UTF-8ヘルパー関数
  - `FixedString<N>` - 固定長文字列
  - `FixedBuffer<N>` - 固定長バッファ
  - `span<T>` - メモリビュー
  - `Vector3` - 3次元ベクトル
- Interface layer
  - `Readable` / `Writable` - 読み書きインターフェース
  - `Connectable` - 接続管理
  - `Scannable` - スキャン機能
  - `Pressable` - ボタン入力
  - `Measurable` / `Measurable3D` - 測定
  - `Displayable` - ディスプレイ出力
- Context layer
  - `ConnectableContext` - 接続可能デバイス
  - `ReadableContext` / `WritableContext` - 読み書き可能デバイス
  - `ScannableContext` - スキャン可能デバイス
  - `SensorContext` - センサー
  - `InputContext` / `OutputContext` - 入出力
  - `SystemInfoContext` - システム情報
  - `PowerContext` - 電源管理
- Device layer
  - `SerialContext` - シリアル通信
  - `BluetoothContext` - Bluetooth通信
  - `WiFiContext` - WiFi接続
  - `BLEContext` - BLE通信
- M5Stack platform implementation
  - SystemContext実装
  - Serial/Bluetooth/WiFi/BLE実装
  - センサー実装（加速度計、ジャイロ）
  - 入出力実装（ボタン、ディスプレイ）
  - 電源管理実装

### Documentation
- README.md
- DEVELOPMENT.md - 開発ガイド
- CLAUDE.md - AI開発支援向けガイドライン

[Unreleased]: https://github.com/TakumiOkayasu/omusubi/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/TakumiOkayasu/omusubi/releases/tag/v0.1.0
