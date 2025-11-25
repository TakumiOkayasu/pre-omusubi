#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cstring>
#include <omusubi/core/format.hpp>

#include "doctest.h"

using namespace omusubi;

TEST_CASE("Format - 基本的なフォーマット") {
    SUBCASE("基本的な文字列フォーマット") {
        auto result = format<128>("Hello, {}!", "World");
        CHECK_EQ(strcmp(result.c_str(), "Hello, World!"), 0);
    }

    SUBCASE("複数の引数") {
        auto result = format<128>("Name: {}, Age: {}", "Alice", 25);
        CHECK_EQ(strcmp(result.c_str(), "Name: Alice, Age: 25"), 0);
    }

    SUBCASE("引数なし") {
        auto result = format<128>("No placeholders");
        CHECK_EQ(strcmp(result.c_str(), "No placeholders"), 0);
    }
}

TEST_CASE("Format - 整数型") {
    SUBCASE("整数型") {
        auto result = format<128>("Value: {}", 42);
        CHECK_EQ(strcmp(result.c_str(), "Value: 42"), 0);
    }

    SUBCASE("負数") {
        auto result = format<128>("Negative: {}", -123);
        CHECK_EQ(strcmp(result.c_str(), "Negative: -123"), 0);
    }

    SUBCASE("0") {
        auto result = format<128>("Zero: {}", 0);
        CHECK_EQ(strcmp(result.c_str(), "Zero: 0"), 0);
    }

    SUBCASE("uint32_t") {
        auto result = format<128>("Unsigned: {}", static_cast<uint32_t>(4294967295U));
        CHECK_EQ(strcmp(result.c_str(), "Unsigned: 4294967295"), 0);
    }

    SUBCASE("int64_t") {
        auto result = format<128>("Large: {}", static_cast<int64_t>(1234567890123LL));
        CHECK_EQ(strcmp(result.c_str(), "Large: 1234567890123"), 0);
    }
}

TEST_CASE("Format - 各種型") {
    SUBCASE("bool") {
        auto result = format<128>("Bool: {} and {}", true, false);
        CHECK_EQ(strcmp(result.c_str(), "Bool: true and false"), 0);
    }

    SUBCASE("char") {
        auto result = format<128>("Char: {}", 'A');
        CHECK_EQ(strcmp(result.c_str(), "Char: A"), 0);
    }

    SUBCASE("std::string_view") {
        auto result = format<128>("View: {}", std::string_view("test"));
        CHECK_EQ(strcmp(result.c_str(), "View: test"), 0);
    }
}

TEST_CASE("Format - エスケープ") {
    SUBCASE("エスケープされた括弧") {
        auto result = format<128>("Escaped: {{}}");
        CHECK_EQ(strcmp(result.c_str(), "Escaped: {}"), 0);
    }

    SUBCASE("混在") {
        auto result = format<128>("{{Value: {}}}", 42);
        CHECK_EQ(strcmp(result.c_str(), "{Value: 42}"), 0);
    }
}

TEST_CASE("Format - 複数プレースホルダー") {
    SUBCASE("複数のプレースホルダー") {
        auto result = format<128>("{} + {} = {}", 1, 2, 3);
        CHECK_EQ(strcmp(result.c_str(), "1 + 2 = 3"), 0);
    }

    SUBCASE("異なる型の混在") {
        auto result = format<128>("String: {}, Int: {}, Bool: {}", "test", 100, true);
        CHECK_EQ(strcmp(result.c_str(), "String: test, Int: 100, Bool: true"), 0);
    }
}

TEST_CASE("Format - 16進数") {
    SUBCASE("16進数（小文字）") {
        auto result = format_hex<128>(255);
        CHECK_EQ(strcmp(result.c_str(), "0xff"), 0);
    }

    SUBCASE("16進数（大文字）") {
        auto result = format_hex<128>(255, true);
        CHECK_EQ(strcmp(result.c_str(), "0xFF"), 0);
    }

    SUBCASE("より大きな値") {
        auto result = format_hex<128>(0xDEADBEEF);
        CHECK_EQ(strcmp(result.c_str(), "0xdeadbeef"), 0);
    }
}

TEST_CASE("Format - エッジケース") {
    SUBCASE("小さいバッファ") {
        auto result = format<16>("Short");
        CHECK_EQ(strcmp(result.c_str(), "Short"), 0);
    }

    SUBCASE("最小サイズ") {
        auto result = format<1>("{}", 'X');
        CHECK_EQ(strcmp(result.c_str(), "X"), 0);
    }
}

TEST_CASE("Format - 実行時") {
    auto result = format<128>("Runtime: {}", 42);
    CHECK_EQ(strcmp(result.c_str(), "Runtime: 42"), 0);
    CHECK_EQ(result.capacity(), 128);
}

TEST_CASE("Format - format_to()") {
    FixedString<128> str;

    SUBCASE("基本的な使用") {
        format_to(str, "Hello, {}!", "World");
        CHECK_EQ(strcmp(str.c_str(), "Hello, World!"), 0);
    }

    SUBCASE("複数の引数") {
        format_to(str, "Name: {}, Age: {}", "Bob", 30);
        CHECK_EQ(strcmp(str.c_str(), "Name: Bob, Age: 30"), 0);
    }

    SUBCASE("型の混在") {
        format_to(str, "Int: {}, Bool: {}", 123, false);
        CHECK_EQ(strcmp(str.c_str(), "Int: 123, Bool: false"), 0);
    }
}

TEST_CASE("Format - format_hex_to()") {
    FixedString<64> str;

    SUBCASE("16進数フォーマット（小文字）") {
        format_hex_to(str, 255);
        CHECK_EQ(strcmp(str.c_str(), "0xff"), 0);
    }

    SUBCASE("16進数フォーマット（大文字）") {
        format_hex_to(str, 0xABCD, true);
        CHECK_EQ(strcmp(str.c_str(), "0xABCD"), 0);
    }
}

TEST_CASE("Format - fmt()ラッパー") {
    SUBCASE("デフォルト容量でのフォーマット") {
        auto result = fmt("Hello, {}!", "World");
        CHECK_EQ(strcmp(result.c_str(), "Hello, World!"), 0);
        CHECK_EQ(result.capacity(), 256);
    }

    SUBCASE("複数の引数") {
        auto result = fmt("x={}, y={}, z={}", 10, 20, 30);
        CHECK_EQ(strcmp(result.c_str(), "x=10, y=20, z=30"), 0);
    }

    SUBCASE("型の混在") {
        auto result = fmt("String: {}, Int: {}, Bool: {}", "test", 42, true);
        CHECK_EQ(strcmp(result.c_str(), "String: test, Int: 42, Bool: true"), 0);
    }
}

TEST_CASE("Format - fmt_hex()ラッパー") {
    SUBCASE("デフォルト容量での16進数フォーマット") {
        auto result = fmt_hex(255);
        CHECK_EQ(strcmp(result.c_str(), "0xff"), 0);
        CHECK_EQ(result.capacity(), 256);
    }

    SUBCASE("大文字") {
        auto result = fmt_hex(0xDEADBEEF, true);
        CHECK_EQ(strcmp(result.c_str(), "0xDEADBEEF"), 0);
    }
}
