# 開発ガイド

## コード品質管理

このプロジェクトでは、コードの品質を保つために自動フォーマットとLintを使用しています。

### 自動フォーマットの仕組み

**2段階のフォーマット構成:**

1. **保存時（VS Code）**: ファイル保存時にclangdが自動的にclang-formatを実行
   - 即座にフォーマット適用
   - 開発体験の向上

2. **コミット時（pre-commit hook）**: ステージされたファイルに対してフォーマット+Lintを実行
   - 最終的な品質保証
   - エラーがあればコミットをブロック

### 利用可能なスクリプト

#### 1. フォーマット（自動修正）

すべてのC++ファイルを自動的にフォーマットします：

```bash
./scripts/format.sh
```

#### 2. Lint（警告・エラーチェック）

コードの問題をチェックします：

```bash
./scripts/lint.sh
```

エラーがある場合は終了コード1で終了します。

#### 3. フォーマットチェック（CI/CD用）

フォーマットが正しいかチェックします（修正はしません）：

```bash
./scripts/check-format.sh
```

### VS Code設定（推奨）

`.vscode/settings.json`で保存時の自動フォーマットが有効になっています：

```json
{
  "editor.formatOnSave": true,
  "[cpp]": {
    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd"
  }
}
```

**必要な拡張機能:**
- `llvm-vs-code-extensions.vscode-clangd` (推奨)

Dev Containerを使用している場合は自動的にインストールされます。

### Git Commit時の自動チェック

コミット時に自動的に以下が実行されます：

1. ステージされたC++ファイルを自動フォーマット（clang-format）
2. Lintチェック（clang-tidy）
3. エラーがあればコミットをブロック

**フロー:**
```
ファイル編集 → 保存（自動フォーマット） → git add → git commit（フォーマット+Lint） → git push（CI/CD）
```

#### バイパス方法

緊急時にチェックをスキップする場合：

```bash
git commit --no-verify
```

⚠️ 推奨されません。CI/CDで品質チェックが失敗します。

### CI/CDパイプライン

GitHub Actionsが自動的に以下をチェックします：

- ✅ フォーマットチェック
- ✅ Lintチェック

**注意:** CI環境ではM5Stack固有のヘッダーがないため、実機コードのビルドは行いません。実機でのビルドはPlatformIOやArduino IDEで行ってください。

プッシュ前に以下を実行することを推奨：

```bash
# フォーマット適用
./scripts/format.sh

# Lintチェック
./scripts/lint.sh

# ステージング
git add .

# コミット（自動チェック実行）
git commit -m "your message"
```

## コーディング規約

### フォーマット規則

`.clang-format`ファイルに定義されています。主な規則：

- **行の長さ**: 最大180文字
- **インデント**: スペース4つ
- **中括弧**: 開き中括弧は同じ行、省略禁止
- **テンプレート**: `template<>`の後ろは改行
- **初期化子**: `{}` の前にスペース1つ（例: `StringView {}`）
- **制御文**: if/for/while文の開き中括弧の前の行には空行
- **閉じ中括弧**: 閉じ中括弧の後ろが閉じ中括弧でなければ空行

### Lint規則

`.clang-tidy`ファイルに定義されています。主なチェック：

- バグの可能性がある箇所の検出
- モダンC++14のベストプラクティス
- パフォーマンスの問題
- 可読性の問題
- 命名規則

## トラブルシューティング

### clang-formatが見つからない

```bash
sudo apt-get install clang-format
```

### clang-tidyが見つからない

```bash
sudo apt-get install clang-tidy
```

### pre-commit hookが動作しない

```bash
chmod +x .git/hooks/pre-commit
```

### フォーマットが適用されない

1. `.clang-format`ファイルが存在することを確認
2. 手動実行: `./scripts/format.sh`
3. ファイルを再度ステージング: `git add <file>`
