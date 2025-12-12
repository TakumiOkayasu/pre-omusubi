// math.hpp の単体テスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/math.hpp>

#include "doctest.h"

using namespace omusubi;

TEST_CASE("clamp - 基本機能") {
    SUBCASE("範囲内の値") {
        CHECK_EQ(clamp(5, 0, 10), 5);
        CHECK_EQ(clamp(0, 0, 10), 0);
        CHECK_EQ(clamp(10, 0, 10), 10);
    }

    SUBCASE("範囲外の値（下限）") {
        CHECK_EQ(clamp(-5, 0, 10), 0);
    }

    SUBCASE("範囲外の値（上限）") {
        CHECK_EQ(clamp(15, 0, 10), 10);
    }

    SUBCASE("浮動小数点") {
        CHECK_EQ(clamp(0.5f, 0.0f, 1.0f), doctest::Approx(0.5f));
        CHECK_EQ(clamp(-0.5f, 0.0f, 1.0f), doctest::Approx(0.0f));
        CHECK_EQ(clamp(1.5f, 0.0f, 1.0f), doctest::Approx(1.0f));
    }

    SUBCASE("負の範囲") {
        CHECK_EQ(clamp(-5, -10, -1), -5);
        CHECK_EQ(clamp(-15, -10, -1), -10);
        CHECK_EQ(clamp(0, -10, -1), -1);
    }
}

TEST_CASE("map_range - 基本機能") {
    SUBCASE("0-10 から 0-100 へのマッピング") {
        CHECK_EQ(map_range(0, 0, 10, 0, 100), 0);
        CHECK_EQ(map_range(5, 0, 10, 0, 100), 50);
        CHECK_EQ(map_range(10, 0, 10, 0, 100), 100);
    }

    SUBCASE("0-1023 から 0-255 へのマッピング（ADC→PWM）") {
        CHECK_EQ(map_range(0, 0, 1023, 0, 255), 0);
        CHECK_EQ(map_range(512, 0, 1023, 0, 255), 127); // 近似
        CHECK_EQ(map_range(1023, 0, 1023, 0, 255), 255);
    }

    SUBCASE("浮動小数点") {
        CHECK_EQ(map_range(0.5f, 0.0f, 1.0f, 0.0f, 100.0f), doctest::Approx(50.0f));
    }

    SUBCASE("逆方向マッピング") {
        CHECK_EQ(map_range(0, 0, 10, 100, 0), 100);
        CHECK_EQ(map_range(10, 0, 10, 100, 0), 0);
        CHECK_EQ(map_range(5, 0, 10, 100, 0), 50);
    }

    SUBCASE("範囲外の値（外挿）") {
        CHECK_EQ(map_range(15, 0, 10, 0, 100), 150); // 外挿される
        CHECK_EQ(map_range(-5, 0, 10, 0, 100), -50);
    }

    SUBCASE("ゼロ除算防止") {
        CHECK_EQ(map_range(5, 10, 10, 0, 100), 0); // in_min == in_max の場合
    }
}

TEST_CASE("map_range_clamped - 基本機能") {
    SUBCASE("範囲内") {
        CHECK_EQ(map_range_clamped(5, 0, 10, 0, 100), 50);
    }

    SUBCASE("範囲外でもクランプされる") {
        CHECK_EQ(map_range_clamped(15, 0, 10, 0, 100), 100);
        CHECK_EQ(map_range_clamped(-5, 0, 10, 0, 100), 0);
    }

    SUBCASE("逆方向でもクランプ") {
        CHECK_EQ(map_range_clamped(15, 0, 10, 100, 0), 0); // out_max < out_min でも動作
        CHECK_EQ(map_range_clamped(-5, 0, 10, 100, 0), 100);
    }
}

TEST_CASE("lerp - 線形補間") {
    SUBCASE("基本的な補間") {
        CHECK_EQ(lerp(0.0f, 10.0f, 0.0f), doctest::Approx(0.0f));
        CHECK_EQ(lerp(0.0f, 10.0f, 0.5f), doctest::Approx(5.0f));
        CHECK_EQ(lerp(0.0f, 10.0f, 1.0f), doctest::Approx(10.0f));
    }

    SUBCASE("負の値") {
        CHECK_EQ(lerp(-10.0f, 10.0f, 0.5f), doctest::Approx(0.0f));
    }

    SUBCASE("範囲外の t") {
        CHECK_EQ(lerp(0.0f, 10.0f, 1.5f), doctest::Approx(15.0f)); // 外挿
        CHECK_EQ(lerp(0.0f, 10.0f, -0.5f), doctest::Approx(-5.0f));
    }

    SUBCASE("double") {
        CHECK_EQ(lerp(0.0, 100.0, 0.25), doctest::Approx(25.0));
    }
}

TEST_CASE("inverse_lerp - 逆線形補間") {
    SUBCASE("基本") {
        CHECK_EQ(inverse_lerp(0.0f, 10.0f, 0.0f), doctest::Approx(0.0f));
        CHECK_EQ(inverse_lerp(0.0f, 10.0f, 5.0f), doctest::Approx(0.5f));
        CHECK_EQ(inverse_lerp(0.0f, 10.0f, 10.0f), doctest::Approx(1.0f));
    }

    SUBCASE("範囲外の値") {
        CHECK_EQ(inverse_lerp(0.0f, 10.0f, 15.0f), doctest::Approx(1.5f));
    }

    SUBCASE("ゼロ除算防止") {
        CHECK_EQ(inverse_lerp(5.0f, 5.0f, 5.0f), doctest::Approx(0.0f));
    }
}

TEST_CASE("abs - 絶対値") {
    SUBCASE("整数") {
        CHECK_EQ(abs(5), 5);
        CHECK_EQ(abs(-5), 5);
        CHECK_EQ(abs(0), 0);
    }

    SUBCASE("浮動小数点") {
        CHECK_EQ(abs(3.14f), doctest::Approx(3.14f));
        CHECK_EQ(abs(-3.14f), doctest::Approx(3.14f));
    }
}

TEST_CASE("sign - 符号") {
    SUBCASE("正") {
        CHECK_EQ(sign(5), 1);
        CHECK_EQ(sign(0.001f), 1);
    }

    SUBCASE("負") {
        CHECK_EQ(sign(-5), -1);
        CHECK_EQ(sign(-0.001f), -1);
    }

    SUBCASE("ゼロ") {
        CHECK_EQ(sign(0), 0);
        CHECK_EQ(sign(0.0f), 0);
    }
}

TEST_CASE("min/max") {
    SUBCASE("min") {
        CHECK_EQ(min(3, 5), 3);
        CHECK_EQ(min(5, 3), 3);
        CHECK_EQ(min(3, 3), 3);
    }

    SUBCASE("max") {
        CHECK_EQ(max(3, 5), 5);
        CHECK_EQ(max(5, 3), 5);
        CHECK_EQ(max(3, 3), 3);
    }

    SUBCASE("浮動小数点") {
        CHECK_EQ(min(1.5f, 2.5f), doctest::Approx(1.5f));
        CHECK_EQ(max(1.5f, 2.5f), doctest::Approx(2.5f));
    }
}

TEST_CASE("constexpr 対応") {
    // コンパイル時評価
    constexpr int clamped = clamp(15, 0, 10);
    static_assert(clamped == 10, "clamp should work at compile time");

    constexpr int mapped = map_range(5, 0, 10, 0, 100);
    static_assert(mapped == 50, "map_range should work at compile time");

    constexpr float lerped = lerp(0.0f, 10.0f, 0.5f);
    static_assert(lerped == 5.0f, "lerp should work at compile time");

    constexpr int abs_val = omusubi::abs(-42);
    static_assert(abs_val == 42, "abs should work at compile time");

    constexpr int sign_val = sign(-5);
    static_assert(sign_val == -1, "sign should work at compile time");

    constexpr int min_val = min(3, 5);
    static_assert(min_val == 3, "min should work at compile time");

    constexpr int max_val = max(3, 5);
    static_assert(max_val == 5, "max should work at compile time");

    // ランタイムでも確認
    CHECK_EQ(clamped, 10);
    CHECK_EQ(mapped, 50);
}
