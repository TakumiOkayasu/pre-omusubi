// RingBuffer<T, N> の単体テスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/ring_buffer.hpp>

#include "doctest.h"

using namespace omusubi;

TEST_CASE("RingBuffer - 基本機能") {
    SUBCASE("デフォルトコンストラクタ") {
        RingBuffer<int, 5> buf;
        CHECK(buf.empty());
        CHECK_EQ(buf.size(), 0U);
        CHECK_EQ(buf.capacity(), 5U);
        CHECK_FALSE(buf.full());
    }
}

TEST_CASE("RingBuffer - push/pop操作") {
    RingBuffer<int, 3> buf;

    SUBCASE("基本的なpush/pop") {
        buf.push(1);
        buf.push(2);
        buf.push(3);

        CHECK_EQ(buf.size(), 3U);
        CHECK(buf.full());

        CHECK_EQ(buf.front(), 1);
        CHECK_EQ(buf.back(), 3);

        buf.pop();
        CHECK_EQ(buf.size(), 2U);
        CHECK_EQ(buf.front(), 2);
    }

    SUBCASE("満杯時の上書き動作") {
        buf.push(1);
        buf.push(2);
        buf.push(3);
        buf.push(4); // 1を上書き

        CHECK_EQ(buf.size(), 3U);
        CHECK_EQ(buf.front(), 2);
        CHECK_EQ(buf.back(), 4);

        buf.push(5); // 2を上書き
        CHECK_EQ(buf.front(), 3);
        CHECK_EQ(buf.back(), 5);
    }

    SUBCASE("pop with output") {
        buf.push(10);
        buf.push(20);

        int value = 0;
        CHECK(buf.pop(value));
        CHECK_EQ(value, 10);

        CHECK(buf.pop(value));
        CHECK_EQ(value, 20);

        CHECK_FALSE(buf.pop(value)); // 空
    }
}

TEST_CASE("RingBuffer - try_push") {
    RingBuffer<int, 3> buf;

    SUBCASE("容量内では成功") {
        CHECK(buf.try_push(1));
        CHECK(buf.try_push(2));
        CHECK(buf.try_push(3));
        CHECK_EQ(buf.size(), 3U);
    }

    SUBCASE("満杯時は失敗") {
        buf.try_push(1);
        buf.try_push(2);
        buf.try_push(3);

        CHECK_FALSE(buf.try_push(4));
        CHECK_EQ(buf.size(), 3U);
        CHECK_EQ(buf.back(), 3); // 上書きされていない
    }
}

TEST_CASE("RingBuffer - インデックスアクセス") {
    RingBuffer<int, 5> buf;
    buf.push(10);
    buf.push(20);
    buf.push(30);

    SUBCASE("operator[]") {
        CHECK_EQ(buf[0], 10); // 最古
        CHECK_EQ(buf[1], 20);
        CHECK_EQ(buf[2], 30); // 最新
    }

    SUBCASE("ラップアラウンド後のアクセス") {
        buf.pop(); // 10を削除
        buf.push(40);
        buf.push(50); // ラップアラウンド発生

        CHECK_EQ(buf[0], 20);
        CHECK_EQ(buf[1], 30);
        CHECK_EQ(buf[2], 40);
        CHECK_EQ(buf[3], 50);
    }
}

TEST_CASE("RingBuffer - clear") {
    RingBuffer<int, 5> buf;
    buf.push(1);
    buf.push(2);
    buf.push(3);

    buf.clear();
    CHECK(buf.empty());
    CHECK_EQ(buf.size(), 0U);

    // クリア後も正常に使用可能
    buf.push(100);
    CHECK_EQ(buf.front(), 100);
}

TEST_CASE("RingBuffer - remaining") {
    RingBuffer<int, 5> buf;

    CHECK_EQ(buf.remaining(), 5U);

    buf.push(1);
    CHECK_EQ(buf.remaining(), 4U);

    buf.push(2);
    buf.push(3);
    CHECK_EQ(buf.remaining(), 2U);

    buf.push(4);
    buf.push(5);
    CHECK_EQ(buf.remaining(), 0U);
}

TEST_CASE("RingBuffer - イテレータ") {
    RingBuffer<int, 5> buf;
    buf.push(10);
    buf.push(20);
    buf.push(30);

    SUBCASE("範囲for文") {
        int sum = 0;
        for (int val : buf) {
            sum += val;
        }
        CHECK_EQ(sum, 60);
    }

    SUBCASE("begin/end") {
        auto it = buf.begin();
        CHECK_EQ(*it, 10);
        ++it;
        CHECK_EQ(*it, 20);
        ++it;
        CHECK_EQ(*it, 30);
        ++it;
        CHECK(it == buf.end());
    }

    SUBCASE("ラップアラウンド後のイテレーション") {
        buf.pop();
        buf.pop();
        buf.push(40);
        buf.push(50);

        int expected[] = {30, 40, 50};
        int i = 0;
        for (int val : buf) {
            CHECK_EQ(val, expected[i++]);
        }
    }
}

TEST_CASE("RingBuffer - const イテレータ") {
    RingBuffer<int, 5> buf;
    buf.push(1);
    buf.push(2);
    buf.push(3);

    const auto& cbuf = buf;

    int sum = 0;
    for (int val : cbuf) {
        sum += val;
    }
    CHECK_EQ(sum, 6);
}

TEST_CASE("RingBuffer - emplace") {
    struct Point {
        int x;
        int y;

        Point() : x(0), y(0) {}

        Point(int px, int py) : x(px), y(py) {}
    };

    RingBuffer<Point, 3> buf;

    buf.emplace(1, 2);
    buf.emplace(3, 4);

    CHECK_EQ(buf[0].x, 1);
    CHECK_EQ(buf[0].y, 2);
    CHECK_EQ(buf[1].x, 3);
    CHECK_EQ(buf[1].y, 4);
}

// 非トリビアル型でのテスト（デストラクタが呼ばれることを確認）
struct Counter {
    static int instances;
    int value;

    Counter() : value(0) { ++instances; }

    explicit Counter(int v) : value(v) { ++instances; }

    Counter(const Counter& other) : value(other.value) { ++instances; }

    Counter(Counter&& other) noexcept : value(other.value) {
        ++instances;
        other.value = -1;
    }

    ~Counter() { --instances; }

    Counter& operator=(const Counter& other) {
        value = other.value;
        return *this;
    }

    Counter& operator=(Counter&& other) noexcept {
        value = other.value;
        other.value = -1;
        return *this;
    }
};

int Counter::instances = 0;

TEST_CASE("RingBuffer - 非トリビアル型") {
    Counter::instances = 0;

    SUBCASE("構築と破棄") {
        {
            RingBuffer<Counter, 5> buf;
            buf.emplace(1);
            buf.emplace(2);
            buf.emplace(3);
            CHECK_EQ(Counter::instances, 3);
        }
        CHECK_EQ(Counter::instances, 0);
    }

    SUBCASE("popでデストラクタが呼ばれる") {
        {
            RingBuffer<Counter, 5> buf;
            buf.emplace(1);
            buf.emplace(2);
            CHECK_EQ(Counter::instances, 2);
            buf.pop();
            CHECK_EQ(Counter::instances, 1);
        }
        CHECK_EQ(Counter::instances, 0);
    }

    SUBCASE("上書き時にデストラクタが呼ばれる") {
        {
            RingBuffer<Counter, 3> buf;
            buf.emplace(1);
            buf.emplace(2);
            buf.emplace(3);
            CHECK_EQ(Counter::instances, 3);

            buf.emplace(4);                  // 1を上書き
            CHECK_EQ(Counter::instances, 3); // 数は変わらない

            buf.emplace(5); // 2を上書き
            CHECK_EQ(Counter::instances, 3);
        }
        CHECK_EQ(Counter::instances, 0);
    }

    SUBCASE("clearでデストラクタが呼ばれる") {
        {
            RingBuffer<Counter, 5> buf;
            buf.emplace(1);
            buf.emplace(2);
            buf.emplace(3);
            CHECK_EQ(Counter::instances, 3);
            buf.clear();
            CHECK_EQ(Counter::instances, 0);
        }
    }
}
