// bit_set.hpp の単体テスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/bit_set.hpp>

#include "doctest.h"

using namespace omusubi;

TEST_CASE("BitSet - 基本機能") {
    SUBCASE("デフォルト構築") {
        BitSet<8> bits;
        CHECK_FALSE(bits.any());
        CHECK(bits.none());
        CHECK_EQ(bits.count(), 0);
    }

    SUBCASE("初期値付き構築") {
        BitSet<8> bits(0b10101010);
        CHECK(bits.test(1));
        CHECK_FALSE(bits.test(0));
        CHECK(bits.test(3));
        CHECK_FALSE(bits.test(2));
        CHECK_EQ(bits.count(), 4);
    }

    SUBCASE("size()") {
        CHECK_EQ(BitSet<8>::size(), 8);
        CHECK_EQ(BitSet<32>::size(), 32);
        CHECK_EQ(BitSet<64>::size(), 64);
        CHECK_EQ(BitSet<100>::size(), 100);
    }
}

TEST_CASE("BitSet - ビット操作") {
    SUBCASE("set(pos)") {
        BitSet<8> bits;
        bits.set(0).set(3).set(7);
        CHECK(bits.test(0));
        CHECK(bits.test(3));
        CHECK(bits.test(7));
        CHECK_FALSE(bits.test(1));
        CHECK_EQ(bits.count(), 3);
    }

    SUBCASE("set(pos, value)") {
        BitSet<8> bits;
        bits.set(0, true);
        bits.set(1, false);
        bits.set(2, true);
        CHECK(bits.test(0));
        CHECK_FALSE(bits.test(1));
        CHECK(bits.test(2));
    }

    SUBCASE("set() - すべて1") {
        BitSet<8> bits;
        bits.set();
        CHECK(bits.all());
        CHECK_EQ(bits.count(), 8);
    }

    SUBCASE("reset(pos)") {
        BitSet<8> bits(0xFF);
        bits.reset(0).reset(4);
        CHECK_FALSE(bits.test(0));
        CHECK_FALSE(bits.test(4));
        CHECK(bits.test(1));
        CHECK_EQ(bits.count(), 6);
    }

    SUBCASE("reset() - すべて0") {
        BitSet<8> bits(0xFF);
        bits.reset();
        CHECK(bits.none());
        CHECK_EQ(bits.count(), 0);
    }

    SUBCASE("flip(pos)") {
        BitSet<8> bits(0b00001111);
        bits.flip(0); // 1 -> 0
        bits.flip(4); // 0 -> 1
        CHECK_FALSE(bits.test(0));
        CHECK(bits.test(4));
    }

    SUBCASE("flip() - すべて反転") {
        BitSet<8> bits(0b00001111);
        bits.flip();
        CHECK_EQ(bits.to_uint32(), 0b11110000);
    }
}

TEST_CASE("BitSet - 添字演算子") {
    BitSet<8> bits(0b10101010);
    CHECK_FALSE(bits[0]);
    CHECK(bits[1]);
    CHECK_FALSE(bits[2]);
    CHECK(bits[3]);
}

TEST_CASE("BitSet - クエリ関数") {
    SUBCASE("all()") {
        BitSet<8> bits;
        CHECK_FALSE(bits.all());
        bits.set();
        CHECK(bits.all());
        bits.reset(0);
        CHECK_FALSE(bits.all());
    }

    SUBCASE("any()") {
        BitSet<8> bits;
        CHECK_FALSE(bits.any());
        bits.set(3);
        CHECK(bits.any());
    }

    SUBCASE("none()") {
        BitSet<8> bits;
        CHECK(bits.none());
        bits.set(0);
        CHECK_FALSE(bits.none());
    }

    SUBCASE("count()") {
        BitSet<8> bits(0b10101010);
        CHECK_EQ(bits.count(), 4);

        BitSet<8> all_set;
        all_set.set();
        CHECK_EQ(all_set.count(), 8);
    }
}

TEST_CASE("BitSet - 変換") {
    SUBCASE("to_uint32()") {
        BitSet<8> bits(0xAB);
        CHECK_EQ(bits.to_uint32(), 0xAB);
    }

    SUBCASE("to_uint64()") {
        BitSet<64> bits(0x123456789ABCDEF0ULL);
        CHECK_EQ(bits.to_uint64(), 0x123456789ABCDEF0ULL);
    }
}

TEST_CASE("BitSet - ビット演算") {
    SUBCASE("AND") {
        BitSet<8> a(0b11110000);
        BitSet<8> b(0b10101010);
        auto result = a & b;
        CHECK_EQ(result.to_uint32(), 0b10100000);
    }

    SUBCASE("OR") {
        BitSet<8> a(0b11110000);
        BitSet<8> b(0b00001111);
        auto result = a | b;
        CHECK_EQ(result.to_uint32(), 0b11111111);
    }

    SUBCASE("XOR") {
        BitSet<8> a(0b11110000);
        BitSet<8> b(0b10101010);
        auto result = a ^ b;
        CHECK_EQ(result.to_uint32(), 0b01011010);
    }

    SUBCASE("NOT") {
        BitSet<8> bits(0b00001111);
        auto result = ~bits;
        CHECK_EQ(result.to_uint32(), 0b11110000);
    }

    SUBCASE("複合代入演算子") {
        BitSet<8> a(0b11110000);
        BitSet<8> b(0b10101010);

        a &= b;
        CHECK_EQ(a.to_uint32(), 0b10100000);

        a = BitSet<8>(0b11110000);
        a |= b;
        CHECK_EQ(a.to_uint32(), 0b11111010);

        a = BitSet<8>(0b11110000);
        a ^= b;
        CHECK_EQ(a.to_uint32(), 0b01011010);
    }
}

TEST_CASE("BitSet - シフト演算") {
    SUBCASE("左シフト") {
        BitSet<8> bits(0b00000011);
        auto result = bits << 2;
        CHECK_EQ(result.to_uint32(), 0b00001100);
    }

    SUBCASE("右シフト") {
        BitSet<8> bits(0b11000000);
        auto result = bits >> 2;
        CHECK_EQ(result.to_uint32(), 0b00110000);
    }

    SUBCASE("複合代入シフト") {
        BitSet<8> bits(0b00001111);
        bits <<= 4;
        CHECK_EQ(bits.to_uint32(), 0b11110000);

        bits >>= 2;
        CHECK_EQ(bits.to_uint32(), 0b00111100);
    }

    SUBCASE("オーバーフローシフト") {
        BitSet<8> bits(0xFF);
        bits <<= 10; // サイズを超えるシフト
        CHECK(bits.none());

        bits = BitSet<8>(0xFF);
        bits >>= 10;
        CHECK(bits.none());
    }
}

TEST_CASE("BitSet - 比較演算") {
    BitSet<8> a(0b10101010);
    BitSet<8> b(0b10101010);
    BitSet<8> c(0b01010101);

    CHECK(a == b);
    CHECK_FALSE(a == c);
    CHECK(a != c);
    CHECK_FALSE(a != b);
}

TEST_CASE("BitSet - 大きなサイズ") {
    SUBCASE("64ビット") {
        BitSet<64> bits;
        bits.set(0);
        bits.set(63);
        CHECK(bits.test(0));
        CHECK(bits.test(63));
        CHECK_FALSE(bits.test(32));
        CHECK_EQ(bits.count(), 2);
    }

    SUBCASE("100ビット") {
        BitSet<100> bits;
        bits.set(0);
        bits.set(50);
        bits.set(99);
        CHECK(bits.test(0));
        CHECK(bits.test(50));
        CHECK(bits.test(99));
        CHECK_EQ(bits.count(), 3);
    }

    SUBCASE("128ビット - all/none") {
        BitSet<128> bits;
        CHECK(bits.none());

        bits.set();
        CHECK(bits.all());
        CHECK_EQ(bits.count(), 128);
    }
}

TEST_CASE("BitSet - 境界外アクセス") {
    BitSet<8> bits;
    // 境界外の set は無視される
    bits.set(100);
    CHECK(bits.none());

    // 境界外の test は false を返す
    CHECK_FALSE(bits.test(100));

    // 境界外の reset は無視される
    bits.set(0);
    bits.reset(100);
    CHECK(bits.test(0));

    // 境界外の flip は無視される
    bits.flip(100);
    CHECK(bits.test(0));
}

TEST_CASE("BitSet - constexpr 対応") {
    // コンパイル時評価
    constexpr BitSet<8> bits(0b10101010);
    static_assert(bits.test(1), "bit 1 should be set");
    static_assert(!bits.test(0), "bit 0 should not be set");
    static_assert(bits.count() == 4, "count should be 4");
    static_assert(bits.size() == 8, "size should be 8");

    constexpr auto flipped = ~bits;
    static_assert(flipped.to_uint32() == 0b01010101, "flipped should be 0x55");

    constexpr BitSet<8> a(0b11110000);
    constexpr BitSet<8> b(0b00001111);
    constexpr auto combined = a | b;
    static_assert(combined.all(), "combined should have all bits set");

    // ランタイムでも確認
    CHECK_EQ(bits.count(), 4);
    CHECK(combined.all());
}

TEST_CASE("BitSet - メソッドチェーン") {
    BitSet<8> bits;
    bits.set(0).set(2).set(4).flip(2).reset(0);

    CHECK_FALSE(bits.test(0)); // reset
    CHECK_FALSE(bits.test(2)); // flip
    CHECK(bits.test(4));       // set
    CHECK_EQ(bits.count(), 1);
}
