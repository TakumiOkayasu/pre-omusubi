// parse.hpp の単体テスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/parse.hpp>

#include "doctest.h"

using namespace omusubi;
using namespace std::literals;

TEST_CASE("parse_uint - 基本機能") {
    SUBCASE("10進数") {
        auto result = parse_uint<uint32_t>("123"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 123U);
    }

    SUBCASE("0") {
        auto result = parse_uint<uint32_t>("0"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 0U);
    }

    SUBCASE("最大値付近") {
        auto result = parse_uint<uint8_t>("255"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 255U);
    }
}

TEST_CASE("parse_uint - 基数指定") {
    SUBCASE("2進数") {
        auto result = parse_uint<uint32_t>("1010"sv, 2);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 10U);
    }

    SUBCASE("8進数") {
        auto result = parse_uint<uint32_t>("77"sv, 8);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 63U);
    }

    SUBCASE("16進数（小文字）") {
        auto result = parse_uint<uint32_t>("ff"sv, 16);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 255U);
    }

    SUBCASE("16進数（大文字）") {
        auto result = parse_uint<uint32_t>("FF"sv, 16);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 255U);
    }

    SUBCASE("16進数（混合）") {
        auto result = parse_uint<uint32_t>("DeadBeef"sv, 16);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 0xDEADBEEFU);
    }
}

TEST_CASE("parse_uint - エラーケース") {
    SUBCASE("空文字列") {
        auto result = parse_uint<uint32_t>(""sv);
        CHECK_FALSE(result.is_ok());
        CHECK_EQ(result.error(), Error::INVALID_DATA);
    }

    SUBCASE("無効な文字") {
        auto result = parse_uint<uint32_t>("12x34"sv);
        CHECK_FALSE(result.is_ok());
    }

    SUBCASE("オーバーフロー") {
        auto result = parse_uint<uint8_t>("256"sv);
        CHECK_FALSE(result.is_ok());
    }

    SUBCASE("無効な基数") {
        auto result = parse_uint<uint32_t>("123"sv, 1);
        CHECK_FALSE(result.is_ok());
        CHECK_EQ(result.error(), Error::INVALID_PARAMETER);
    }

    SUBCASE("基数に対して無効な桁") {
        auto result = parse_uint<uint32_t>("29"sv, 8); // 8進数で9は無効
        CHECK_FALSE(result.is_ok());
    }
}

TEST_CASE("parse_int - 基本機能") {
    SUBCASE("正の数") {
        auto result = parse_int<int32_t>("123"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 123);
    }

    SUBCASE("負の数") {
        auto result = parse_int<int32_t>("-456"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), -456);
    }

    SUBCASE("+符号付き") {
        auto result = parse_int<int32_t>("+789"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 789);
    }

    SUBCASE("0") {
        auto result = parse_int<int32_t>("0"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 0);
    }

    SUBCASE("-0") {
        auto result = parse_int<int32_t>("-0"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 0);
    }
}

TEST_CASE("parse_int - 境界値") {
    SUBCASE("int8_t 最大値") {
        auto result = parse_int<int8_t>("127"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 127);
    }

    SUBCASE("int8_t 最小値") {
        auto result = parse_int<int8_t>("-128"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), -128);
    }

    SUBCASE("int8_t オーバーフロー（正）") {
        auto result = parse_int<int8_t>("128"sv);
        CHECK_FALSE(result.is_ok());
    }

    SUBCASE("int8_t オーバーフロー（負）") {
        auto result = parse_int<int8_t>("-129"sv);
        CHECK_FALSE(result.is_ok());
    }
}

TEST_CASE("parse_int - エラーケース") {
    SUBCASE("空文字列") {
        auto result = parse_int<int32_t>(""sv);
        CHECK_FALSE(result.is_ok());
    }

    SUBCASE("符号のみ") {
        auto result = parse_int<int32_t>("-"sv);
        CHECK_FALSE(result.is_ok());
    }

    SUBCASE("無効な文字") {
        auto result = parse_int<int32_t>("12a34"sv);
        CHECK_FALSE(result.is_ok());
    }
}

TEST_CASE("parse_float - 基本機能") {
    SUBCASE("整数部のみ") {
        auto result = parse_float<float>("123"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), doctest::Approx(123.0f));
    }

    SUBCASE("小数部あり") {
        auto result = parse_float<float>("3.14"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), doctest::Approx(3.14f).epsilon(0.001));
    }

    SUBCASE("負の小数") {
        auto result = parse_float<float>("-2.5"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), doctest::Approx(-2.5f));
    }

    SUBCASE("先頭が小数点") {
        auto result = parse_float<float>(".5"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), doctest::Approx(0.5f));
    }

    SUBCASE("末尾が小数点") {
        auto result = parse_float<float>("5."sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), doctest::Approx(5.0f));
    }

    SUBCASE("0.0") {
        auto result = parse_float<float>("0.0"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), doctest::Approx(0.0f));
    }
}

TEST_CASE("parse_float - double") {
    SUBCASE("高精度") {
        auto result = parse_float<double>("3.141592653589793"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), doctest::Approx(3.141592653589793).epsilon(0.0000001));
    }
}

TEST_CASE("parse_float - エラーケース") {
    SUBCASE("空文字列") {
        auto result = parse_float<float>(""sv);
        CHECK_FALSE(result.is_ok());
    }

    SUBCASE("符号のみ") {
        auto result = parse_float<float>("-"sv);
        CHECK_FALSE(result.is_ok());
    }

    SUBCASE("小数点のみ") {
        auto result = parse_float<float>("."sv);
        CHECK_FALSE(result.is_ok());
    }

    SUBCASE("複数の小数点") {
        auto result = parse_float<float>("1.2.3"sv);
        CHECK_FALSE(result.is_ok());
    }

    SUBCASE("無効な文字") {
        auto result = parse_float<float>("1.2x"sv);
        CHECK_FALSE(result.is_ok());
    }
}

TEST_CASE("便利関数") {
    SUBCASE("parse_i32") {
        auto result = parse_i32("-12345"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), -12345);
    }

    SUBCASE("parse_u32") {
        auto result = parse_u32("12345"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 12345U);
    }

    SUBCASE("parse_i64") {
        auto result = parse_i64("-9223372036854775807"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), -9223372036854775807LL);
    }

    SUBCASE("parse_u64") {
        auto result = parse_u64("18446744073709551615"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), 18446744073709551615ULL);
    }

    SUBCASE("parse_f32") {
        auto result = parse_f32("1.5"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), doctest::Approx(1.5f));
    }

    SUBCASE("parse_f64") {
        auto result = parse_f64("1.5"sv);
        CHECK(result.is_ok());
        CHECK_EQ(result.value(), doctest::Approx(1.5));
    }
}

TEST_CASE("constexpr 対応") {
    // コンパイル時に評価可能
    constexpr auto result = parse_uint<uint32_t>("42", 10);
    static_assert(result.is_ok(), "Should be ok");
    static_assert(result.value() == 42, "Value should be 42");

    constexpr auto int_result = parse_int<int32_t>("-100", 10);
    static_assert(int_result.is_ok(), "Should be ok");
    static_assert(int_result.value() == -100, "Value should be -100");

    constexpr auto hex_result = parse_uint<uint32_t>("ff", 16);
    static_assert(hex_result.is_ok(), "Should be ok");
    static_assert(hex_result.value() == 255, "Value should be 255");

    constexpr auto err_result = parse_uint<uint8_t>("256", 10);
    static_assert(err_result.is_err(), "Should be error (overflow)");

    // ランタイムでもテスト（static_assertが通れば基本的にOK）
    CHECK(result.is_ok());
    CHECK_EQ(result.value(), 42U);
}
