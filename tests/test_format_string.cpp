// basic_format_stringのテスト（コンパイル時検証）

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cstring>
#include <omusubi/core/format.hpp>

#include "doctest.h"

using namespace omusubi;

TEST_CASE("FormatString - コンパイル時検証") {
    SUBCASE("単一引数") {
        format_string<int> fs("Value: {}");
        CHECK_EQ(strcmp(fs.c_str(), "Value: {}"), 0);
        CHECK_EQ(fs.length(), 9);
    }

    SUBCASE("複数引数") {
        format_string<int, int> fs("{} + {} = ?");
        CHECK_EQ(strcmp(fs.c_str(), "{} + {} = ?"), 0);
    }

    SUBCASE("異なる型の混在") {
        format_string<const char*, int, bool> fs("String: {}, Int: {}, Bool: {}");
        CHECK_EQ(strcmp(fs.c_str(), "String: {}, Int: {}, Bool: {}"), 0);
    }

    SUBCASE("プレースホルダーなし") {
        format_string<> fs("No placeholders");
        CHECK_EQ(strcmp(fs.c_str(), "No placeholders"), 0);
    }

    SUBCASE("エスケープされた括弧") {
        format_string<int> fs("{{Value: {}}}");
        CHECK_EQ(strcmp(fs.c_str(), "{{Value: {}}}"), 0);
    }
}

TEST_CASE("FormatString - view/c_str/arg_count") {
    format_string<int, int> fs("x={}, y={}");

    auto view = fs.view();
    CHECK_EQ(view.size(), 10U);
    CHECK_EQ(view[0], 'x');

    auto c_str = fs.c_str();
    CHECK_EQ(strcmp(c_str, "x={}, y={}"), 0);

    CHECK_EQ(fs.arg_count(), 2);
}

TEST_CASE("FormatString - format()との組み合わせ") {
    SUBCASE("単一引数") {
        format_string<const char*> fs("Hello, {}!");
        auto result = format<128>(fs, "World");
        CHECK_EQ(strcmp(result.c_str(), "Hello, World!"), 0);
    }

    SUBCASE("複数の引数") {
        format_string<int, int, int> fs("{} + {} = {}");
        auto result = format<128>(fs, 1, 2, 3);
        CHECK_EQ(strcmp(result.c_str(), "1 + 2 = 3"), 0);
    }

    SUBCASE("型の混在") {
        format_string<const char*, int, bool> fs("String: {}, Int: {}, Bool: {}");
        auto result = format<128>(fs, "test", 42, true);
        CHECK_EQ(strcmp(result.c_str(), "String: test, Int: 42, Bool: true"), 0);
    }
}

TEST_CASE("FormatString - format_to()との組み合わせ") {
    FixedString<128> str;

    SUBCASE("基本的な使用") {
        format_string<const char*, int> fs("Name: {}, Age: {}");
        format_to(str, fs, "Alice", 25);
        CHECK_EQ(strcmp(str.c_str(), "Name: Alice, Age: 25"), 0);
    }

    SUBCASE("複数の引数") {
        format_string<int, int> fs("x={}, y={}");
        format_to(str, fs, 10, 20);
        CHECK_EQ(strcmp(str.c_str(), "x=10, y=20"), 0);
    }
}

TEST_CASE("FormatString - fmt()との組み合わせ") {
    SUBCASE("単一引数") {
        format_string<const char*> fs("Hello, {}!");
        auto result = fmt(fs, "World");
        CHECK_EQ(strcmp(result.c_str(), "Hello, World!"), 0);
    }

    SUBCASE("複数引数") {
        format_string<int, int, int> fs("x={}, y={}, z={}");
        auto result = fmt(fs, 1, 2, 3);
        CHECK_EQ(strcmp(result.c_str(), "x=1, y=2, z=3"), 0);
    }
}

TEST_CASE("FormatString - プレースホルダーカウント") {
    SUBCASE("エスケープされた括弧1") {
        format_string<int> fs("{{}} {}");
        auto result = format<64>(fs, 42);
        CHECK_EQ(strcmp(result.c_str(), "{} 42"), 0);
    }

    SUBCASE("エスケープされた括弧2") {
        format_string<int, int> fs("{{{}}} and {}");
        auto result = format<64>(fs, 1, 2);
        CHECK_EQ(strcmp(result.c_str(), "{1} and 2"), 0);
    }
}

TEST_CASE("FormatString - コンパイル時検証（説明のみ）") {
    // 以下はコンパイルエラーになるべきケース（実行時テストでは検証不可）
    // format_string<int, int> fs1("{}");  // エラー: 2引数だが1プレースホルダー
    // format_string<int> fs2("{} {}");    // エラー: 1引数だが2プレースホルダー
    CHECK(true);
}
