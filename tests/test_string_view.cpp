// std::string_view ヘルパー関数の単体テスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/string_view.h>

#include <string_view>

#include "doctest.h"

using namespace omusubi;
using namespace std::literals;

TEST_CASE("std::string_view - 基本機能") {
    SUBCASE("デフォルトコンストラクタ") {
        std::string_view empty;
        CHECK_EQ(empty.size(), 0U);
        CHECK(empty.empty());
    }

    SUBCASE("文字列リテラルからの構築") {
        std::string_view sv = "Hello"sv;
        CHECK_EQ(sv.size(), 5U);
        CHECK(sv == "Hello"sv);
    }

    SUBCASE("C文字列からの構築") {
        std::string_view sv2 = from_c_string("World");
        CHECK_EQ(sv2.size(), 5U);
    }
}

TEST_CASE("std::string_view - UTF-8処理") {
    SUBCASE("日本語文字列") {
        std::string_view japanese = "こんにちは"sv;
        CHECK_EQ(japanese.size(), 15U);
        CHECK_EQ(char_length(japanese), 5U);
    }

    SUBCASE("混合文字列") {
        std::string_view mixed = "Hello世界"sv;
        CHECK_EQ(mixed.size(), 11U);
        CHECK_EQ(char_length(mixed), 7U);
    }

    SUBCASE("絵文字") {
        std::string_view emoji = "😀"sv;
        CHECK_EQ(emoji.size(), 4U);
        CHECK_EQ(char_length(emoji), 1U);
    }
}

TEST_CASE("std::string_view - 部分文字列") {
    std::string_view sv = "Hello World"sv;

    SUBCASE("substr") {
        auto sub = sv.substr(0, 5);
        CHECK_EQ(sub.size(), 5U);
        CHECK(sub == "Hello"sv);
    }

    SUBCASE("範囲外アクセス - 超過する長さ") {
        auto sub3 = sv.substr(6, 100);
        CHECK_EQ(sub3.size(), 5U); // "World"
    }
}

TEST_CASE("std::string_view - 文字列比較") {
    std::string_view s1 = "Hello"sv;
    std::string_view s2 = "Hello"sv;
    std::string_view s3 = "World"sv;

    CHECK(s1 == s2);
    CHECK(s1 != s3);
}

TEST_CASE("std::string_view - 検索機能") {
    std::string_view sv = "Hello World Hello"sv;

    SUBCASE("starts_with (手動実装)") {
        // C++20の starts_with がない場合の代替
        auto starts_with = [](std::string_view str, std::string_view prefix) { return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix; };

        CHECK(starts_with(sv, "Hello"sv));
        CHECK_FALSE(starts_with(sv, "World"sv));
    }

    SUBCASE("ends_with (手動実装)") {
        // C++20の ends_with がない場合の代替
        auto ends_with = [](std::string_view str, std::string_view suffix) { return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix; };

        CHECK(ends_with(sv, "Hello"sv));
        CHECK_FALSE(ends_with(sv, "World"sv));
    }

    SUBCASE("find") {
        CHECK(sv.find("World"sv) != std::string_view::npos);
        CHECK(sv.find("Goodbye"sv) == std::string_view::npos);
    }
}

TEST_CASE("std::string_view - イテレータ") {
    std::string_view sv = "ABC"sv;

    int count = 0;

    for (char c : sv) {
        CHECK((c == 'A' || c == 'B' || c == 'C'));
        count++;
    }

    CHECK_EQ(count, 3);
}

TEST_CASE("omusubi::utf8 - UTF-8ヘルパー関数") {
    SUBCASE("1バイト文字（ASCII）") {
        CHECK_EQ(utf8::get_char_byte_length('A'), 1U);
    }

    SUBCASE("2バイト文字") {
        CHECK_EQ(utf8::get_char_byte_length(0xC0), 2U);
    }

    SUBCASE("3バイト文字") {
        CHECK_EQ(utf8::get_char_byte_length(0xE0), 3U);
    }

    SUBCASE("4バイト文字") {
        CHECK_EQ(utf8::get_char_byte_length(0xF0), 4U);
    }

    SUBCASE("文字数カウント") {
        const char* str = "Aあ😀"; // 1 + 3 + 4 = 8バイト, 3文字
        uint32_t char_count = utf8::count_chars(str, 8);
        CHECK_EQ(char_count, 3U);
    }
}

TEST_CASE("omusubi - ヘルパー関数") {
    SUBCASE("char_length") {
        std::string_view ascii = "Hello"sv;
        CHECK_EQ(char_length(ascii), 5U);

        std::string_view japanese = "日本語"sv;
        CHECK_EQ(char_length(japanese), 3U);
    }

    SUBCASE("get_char_position") {
        std::string_view mixed = "A日B"sv; // 1 + 3 + 1 = 5 bytes

        CHECK_EQ(get_char_position(mixed, 0), 0U); // 'A' at byte 0
        CHECK_EQ(get_char_position(mixed, 1), 1U); // '日' at byte 1
        CHECK_EQ(get_char_position(mixed, 2), 4U); // 'B' at byte 4
    }

    SUBCASE("is_empty") {
        std::string_view empty;
        std::string_view non_empty = "Hello"sv;

        CHECK(is_empty(empty));
        CHECK_FALSE(is_empty(non_empty));
    }

    SUBCASE("equals") {
        std::string_view a = "Hello"sv;
        std::string_view b = "Hello"sv;
        std::string_view c = "World"sv;

        CHECK(equals(a, b));
        CHECK_FALSE(equals(a, c));
    }

    SUBCASE("from_c_string") {
        auto sv = from_c_string("Test");
        CHECK_EQ(sv.size(), 4U);
        CHECK(sv == "Test"sv);

        auto null_sv = from_c_string(nullptr);
        CHECK(null_sv.empty());
    }
}
