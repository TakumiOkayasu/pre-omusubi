// Capacity自動計算のテスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/format.hpp>
#include <string_view>

#include "doctest.h"

using namespace omusubi;
using namespace std::literals;

TEST_CASE("Auto Capacity - 基本機能") {
    SUBCASE("整数フォーマット") {
        auto str = format("Value: {}", 42);
        CHECK_EQ(str.view(), "Value: 42"sv);
    }

    SUBCASE("文字列フォーマット") {
        auto str = format("Hello, {}!", "World");
        CHECK_EQ(str.view(), "Hello, World!"sv);
    }

    SUBCASE("複数引数フォーマット") {
        auto str = format("x={}, y={}", 10, 20);
        CHECK_EQ(str.view(), "x=10, y=20"sv);
    }
}

TEST_CASE("Auto Capacity - 様々な型") {
    SUBCASE("Bool型") {
        auto str = format("Bool: {}", true);
        CHECK_EQ(str.view(), "Bool: true"sv);
    }

    SUBCASE("Char型") {
        auto str = format("Char: {}", 'A');
        CHECK_EQ(str.view(), "Char: A"sv);
    }

    SUBCASE("混合型") {
        auto str = format("Int: {}, Bool: {}, Char: {}", 123, false, 'X');
        CHECK_EQ(str.view(), "Int: 123, Bool: false, Char: X"sv);
    }
}

TEST_CASE("Auto Capacity - format_to") {
    SUBCASE("整数フォーマット") {
        auto str = format_to("Value: {}", 42);
        CHECK_EQ(str.view(), "Value: 42"sv);
    }

    SUBCASE("複数引数フォーマット") {
        auto str = format_to("Name: {}, Age: {}", "Alice", 25);
        CHECK_EQ(str.view(), "Name: Alice, Age: 25"sv);
    }
}

TEST_CASE("Auto Capacity - 容量計算") {
    SUBCASE("int32_tの容量") {
        auto str = format("Value: {}", 42);
        // 実際の出力は "Value: 42" = 9文字だが、容量は最大を想定
        CHECK(str.capacity() >= str.byte_length());
    }

    SUBCASE("複数int32_tの容量") {
        auto str = format("x={}, y={}", 10, 20);
        CHECK(str.capacity() >= str.byte_length());
    }

    SUBCASE("boolの容量") {
        auto str = format("Bool: {}", true);
        CHECK(str.capacity() >= str.byte_length());
    }
}

TEST_CASE("Auto Capacity - 明示的指定との比較") {
    // 明示的指定
    auto str1 = format<128>("Value: {}", 42);

    // 自動計算
    auto str2 = format("Value: {}", 42);

    CHECK_EQ(str1.view(), str2.view());
}

TEST_CASE("Auto Capacity - 最大文字列長") {
    SUBCASE("int8_t") {
        CHECK_EQ(detail::max_string_length<int8_t>::value, 4U);
    }

    SUBCASE("uint8_t") {
        CHECK_EQ(detail::max_string_length<uint8_t>::value, 3U);
    }

    SUBCASE("int16_t") {
        CHECK_EQ(detail::max_string_length<int16_t>::value, 6U);
    }

    SUBCASE("uint16_t") {
        CHECK_EQ(detail::max_string_length<uint16_t>::value, 5U);
    }

    SUBCASE("int32_t") {
        CHECK_EQ(detail::max_string_length<int32_t>::value, 11U);
    }

    SUBCASE("uint32_t") {
        CHECK_EQ(detail::max_string_length<uint32_t>::value, 10U);
    }

    SUBCASE("int64_t") {
        CHECK_EQ(detail::max_string_length<int64_t>::value, 20U);
    }

    SUBCASE("uint64_t") {
        CHECK_EQ(detail::max_string_length<uint64_t>::value, 20U);
    }

    SUBCASE("bool") {
        CHECK_EQ(detail::max_string_length<bool>::value, 5U);
    }

    SUBCASE("char") {
        CHECK_EQ(detail::max_string_length<char>::value, 1U);
    }

    SUBCASE("const char*") {
        CHECK_EQ(detail::max_string_length<const char*>::value, 64U);
    }

    SUBCASE("std::string_view") {
        CHECK_EQ(detail::max_string_length<std::string_view>::value, 64U);
    }
}
