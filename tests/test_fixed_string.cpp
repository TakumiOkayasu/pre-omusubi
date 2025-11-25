// FixedString<N> の単体テスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/fixed_string.hpp>
#include <string_view>

#include "doctest.h"

using namespace omusubi;
using namespace std::literals;
using std::string_view;

TEST_CASE("FixedString<N> - 基本機能") {
    SUBCASE("デフォルトコンストラクタ") {
        FixedString<32> s1;
        CHECK_EQ(s1.byte_length(), 0U);
        CHECK_EQ(s1.view(), ""sv);
    }

    SUBCASE("C文字列からの構築") {
        FixedString<32> s2("Hello");
        CHECK_EQ(s2.byte_length(), 5U);
        CHECK_EQ(s2.view(), "Hello"sv);
    }

    SUBCASE("std::string_viewからの構築") {
        FixedString<32> s3("World"sv);
        CHECK_EQ(s3.byte_length(), 5U);
        CHECK_EQ(s3.view(), "World"sv);
    }
}

TEST_CASE("FixedString<N> - 追加操作") {
    FixedString<32> s;

    SUBCASE("std::string_view追加") {
        CHECK(s.append("Hello"sv));
        CHECK_EQ(s.byte_length(), 5U);
    }

    SUBCASE("C文字列追加") {
        s.append("Hello"sv);
        CHECK(s.append(" World"));
        CHECK_EQ(s.view(), "Hello World"sv);
    }

    SUBCASE("1文字追加") {
        s.append("Hello World"sv);
        CHECK(s.append('!'));
        CHECK_EQ(s.view(), "Hello World!"sv);
    }
}

TEST_CASE("FixedString<N> - 容量制限") {
    FixedString<10> s;

    SUBCASE("容量内の追加") {
        CHECK(s.append("12345"));
        CHECK(s.append("67890"));
        CHECK_EQ(s.byte_length(), 10U);
    }

    SUBCASE("容量超過") {
        s.append("1234567890");
        CHECK_FALSE(s.append("X"));
        CHECK_EQ(s.byte_length(), 10U);
    }
}

TEST_CASE("FixedString<N> - UTF-8処理") {
    SUBCASE("日本語追加") {
        FixedString<64> s;
        s.append("こんにちは"sv);
        CHECK_EQ(s.byte_length(), 15U);
        CHECK_EQ(s.char_length(), 5U);
    }

    SUBCASE("混合文字列") {
        FixedString<64> s2("Hello世界"sv);
        CHECK_EQ(s2.byte_length(), 11U);
        CHECK_EQ(s2.char_length(), 7U);
    }
}

TEST_CASE("FixedString<N> - クリア操作") {
    FixedString<32> s("Hello World");
    CHECK_EQ(s.byte_length(), 11U);

    s.clear();
    CHECK_EQ(s.byte_length(), 0U);
    CHECK_EQ(s.view(), ""sv);

    SUBCASE("クリア後の再利用") {
        CHECK(s.append("New"));
        CHECK_EQ(s.view(), "New"sv);
    }
}

TEST_CASE("FixedString<N> - 比較操作") {
    FixedString<32> s1("Hello");
    FixedString<32> s2("Hello");
    FixedString<32> s3("World");

    CHECK(s1 == "Hello"sv);
    CHECK(s1 != "World"sv);
    CHECK(s1.view() == s2.view());
}

TEST_CASE("FixedString<N> - std::string_view変換") {
    FixedString<32> s("Test String");
    std::string_view view = s.view();

    CHECK_EQ(view.size(), 11U);
    CHECK(view == "Test String"sv);
    CHECK(view.data() == s.data());
}

TEST_CASE("FixedString<N> - イテレータ") {
    FixedString<32> s("ABC");

    int count = 0;
    for (char c : s) {
        CHECK((c == 'A' || c == 'B' || c == 'C'));
        count++;
    }
    CHECK_EQ(count, 3);
}

TEST_CASE("FixedString<N> - null終端保証") {
    FixedString<32> s;
    s.append("Hello");

    CHECK_EQ(s.data()[5], '\0');
    CHECK_EQ(s.c_str()[5], '\0');
}

TEST_CASE("FixedString<N> - constexpr対応") {
    SUBCASE("コンパイル時文字列構築") {
        constexpr auto str1 = fixed_string("Hello");
        static_assert(str1.byte_length() == 5, "constexpr byte_length()");
        static_assert(str1.capacity() == 5, "constexpr capacity()");
        CHECK_EQ(str1.byte_length(), 5U);
        CHECK_EQ(str1.view(), "Hello"sv);
    }

    SUBCASE("コンパイル時UTF-8処理") {
        constexpr auto utf8_str = fixed_string("こんにちは");
        static_assert(utf8_str.byte_length() == 15, "constexpr UTF-8バイト長");
        static_assert(utf8_str.char_length() == 5, "constexpr UTF-8文字数");
        CHECK_EQ(utf8_str.byte_length(), 15U);
        CHECK_EQ(utf8_str.char_length(), 5U);
    }

    SUBCASE("コンパイル時std::string_view") {
        constexpr std::string_view view("Test");
        static_assert(view.size() == 4, "constexpr std::string_view");
        CHECK_EQ(view.size(), 4U);
    }

    SUBCASE("実行時にconstexpr関数を使用") {
        FixedString<32> s1;
        s1.append("Hello");
        s1.append(" World");
        CHECK_EQ(s1.byte_length(), 11U);
        CHECK_EQ(s1.view(), "Hello World"sv);
    }
}
