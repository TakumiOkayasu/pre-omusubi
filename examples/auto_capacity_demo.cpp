// Capacity自動計算のデモ

#include <cstdio>
#include <omusubi/core/format.hpp>

using namespace omusubi;

int main() {
    printf("=== Auto Capacity Demo ===\n\n");

    printf("--- 基本的な使用法 ---\n");
    {
        // Capacity指定なし - 自動計算
        auto str1 = format("Value: {}", 42);
        printf("%s (capacity=%u)\n", str1.c_str(), str1.capacity());

        auto str2 = format("Hello, {}!", "World");
        printf("%s (capacity=%u)\n", str2.c_str(), str2.capacity());

        auto str3 = format("x={}, y={}, z={}", 1, 2, 3);
        printf("%s (capacity=%u)\n", str3.c_str(), str3.capacity());
    }

    printf("\n--- 明示的指定との比較 ---\n");
    {
        // 明示的にCapacityを指定
        auto str1 = format<128>("Value: {}", 42);
        printf("Explicit: %s (capacity=%u)\n", str1.c_str(), str1.capacity());

        // 自動計算
        auto str2 = format("Value: {}", 42);
        printf("Auto:     %s (capacity=%u)\n", str2.c_str(), str2.capacity());

        printf("\n自動計算により、必要最小限のメモリで済む\n");
    }

    printf("\n--- 様々な型での自動計算 ---\n");
    {
        auto str1 = format("Bool: {}", true);
        printf("%s (capacity=%u)\n", str1.c_str(), str1.capacity());

        auto str2 = format("Char: {}", 'A');
        printf("%s (capacity=%u)\n", str2.c_str(), str2.capacity());

        auto str3 = format("String: {}, Int: {}, Bool: {}", "test", 123, false);
        printf("%s (capacity=%u)\n", str3.c_str(), str3.capacity());
    }

    printf("\n--- 容量計算の仕組み ---\n");
    printf("必要容量 = フォーマット文字列の長さ + 各引数の最大文字列長 + null終端\n\n");

    {
        // "Value: {}" = 8文字（{}は除く）
        // int32_tの最大長 = 11文字（"-2147483648"）
        // 合計 = 8 + 11 + 1 = 20
        auto str = format("Value: {}", 42);
        printf("例: \"Value: {}\" + int32_t\n");
        printf("  フォーマット文字列: 8文字（'Value: '）\n");
        printf("  int32_t最大長: 11文字\n");
        printf("  null終端: 1文字\n");
        printf("  合計容量: %u\n", str.capacity());
        printf("  実際の長さ: %u ('%s')\n", str.byte_length(), str.c_str());
    }

    printf("\n--- format_toでの自動計算 ---\n");
    {
        // 出力先バッファを指定する場合
        FixedString<128> buffer;
        format_to(buffer, "Value: {}", 42);
        printf("Buffer指定: %s\n", buffer.c_str());

        // 自動計算版（新しいFixedStringを返す）
        auto result = format_to("Value: {}", 42);
        printf("Auto: %s (capacity=%u)\n", result.c_str(), result.capacity());
    }

    printf("\n--- 使い分けのガイドライン ---\n");
    printf("1. 自動計算 format(...)\n");
    printf("   - 最小限のメモリ使用\n");
    printf("   - 容量を気にする必要なし\n");
    printf("   - 推奨される使用法\n");
    printf("\n");
    printf("2. 明示的指定 format<N>(...)\n");
    printf("   - 大きめのバッファを確保したい場合\n");
    printf("   - 容量を厳密に制御したい場合\n");
    printf("\n");
    printf("3. fmt(...)\n");
    printf("   - デフォルト容量256\n");
    printf("   - 簡潔な記述\n");
    printf("   - 一般的な用途向け\n");

    printf("\n--- メモリ効率の比較 ---\n");
    {
        // 短い文字列の場合
        auto auto_str = format("ID: {}", 1);
        auto fixed_str = format<256>("ID: {}", 1);
        auto fmt_str = fmt("ID: {}", 1);

        printf("短い文字列 'ID: 1' のメモリ使用量:\n");
        printf("  自動計算:     %u bytes\n", auto_str.capacity());
        printf("  明示的(256):  %u bytes\n", fixed_str.capacity());
        printf("  fmt(256):     %u bytes\n", fmt_str.capacity());
        printf("  → 自動計算が最も効率的\n");
    }

    printf("\n--- 制限事項 ---\n");
    printf("・const char*やstd::string_viewは最大64文字と仮定\n");
    printf("・実際の文字列長が64文字を超える場合、切り詰められる可能性\n");
    printf("・その場合は明示的にCapacityを指定することを推奨\n");

    return 0;
}
