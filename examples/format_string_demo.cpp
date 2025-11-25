// basic_format_stringのデモ（C++23互換の型安全なフォーマット文字列）

#include <cstdio>
#include <omusubi/core/format.hpp>

using namespace omusubi;

int main() {
    printf("=== basic_format_string Demo ===\n\n");

    printf("--- 基本的な使用法 ---\n");
    {
        // format_stringで型を明示
        format_string<int> fs("Value: {}");

        // 型安全なフォーマット
        auto result = format<128>(fs, 42);
        printf("%s\n", result.c_str());

        // fmtでも使用可能
        auto result2 = fmt(fs, 100);
        printf("%s\n", result2.c_str());
    }

    printf("\n--- 複数の型を指定 ---\n");
    {
        // 複数の引数型を指定
        format_string<const char*, int, bool> fs("Name: {}, Age: {}, Active: {}");

        auto result = fmt(fs, "Alice", 25, true);
        printf("%s\n", result.c_str());

        // format_toでも使用可能
        FixedString<128> str;
        format_to(str, fs, "Bob", 30, false);
        printf("%s\n", str.c_str());
    }

    printf("\n--- メタ情報の取得 ---\n");
    {
        format_string<int, int, int> fs("x={}, y={}, z={}");

        // フォーマット文字列の内容
        printf("Format string: %s\n", fs.c_str());

        // 長さ
        printf("Length: %u\n", fs.length());

        // 引数数
        printf("Argument count: %u\n", fs.arg_count());

        // std::string_viewとして取得
        auto view = fs.view();
        printf("As std::string_view: ");
        for (uint32_t i = 0; i < view.size(); ++i) {
            printf("%c", view[i]);
        }
        printf("\n");
    }

    printf("\n--- 型安全性のデモ ---\n");
    {
        // 型を明示することで、コンパイル時に型の一貫性を保証
        format_string<int, int> fs("x={}, y={}");

        // 正しい型で使用
        auto result1 = fmt(fs, 10, 20);
        printf("%s\n", result1.c_str());

        // 異なる型でも動作（実行時に変換）
        auto result2 = fmt(fs, 1.5, 2.5); // double -> int
        printf("%s\n", result2.c_str());
    }

    printf("\n--- エスケープのサポート ---\n");
    {
        // エスケープされた括弧
        format_string<int> fs("{{Value: {}}}");
        auto result = fmt(fs, 42);
        printf("%s\n", result.c_str());
    }

    printf("\n--- C++23 std::format_stringとの比較 ---\n");
    printf("C++23:\n");
    printf("  auto s = std::format(\"Value: {}\", 42);\n");
    printf("  std::format_string<int> fs(\"Value: {}\");\n");
    printf("  auto s = std::format(fs, 42);\n");
    printf("\n");
    printf("Omusubi (C++14):\n");
    printf("  auto s = fmt(\"Value: {}\", 42);\n");
    printf("  format_string<int> fs(\"Value: {}\");\n");
    printf("  auto s = fmt(fs, 42);\n");

    printf("\n--- 利点 ---\n");
    printf("1. 型の明示化: フォーマット文字列に期待される型を明確に示せる\n");
    printf("2. 再利用性: フォーマット文字列を変数として保持できる\n");
    printf("3. C++23互換: 将来のC++標準への移行が容易\n");
    printf("4. 型安全性: テンプレート引数で型情報を保持\n");

    return 0;
}
