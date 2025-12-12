// StaticVector<T, N> の単体テスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/static_vector.hpp>

#include "doctest.h"

using namespace omusubi;

TEST_CASE("StaticVector - 基本機能") {
    SUBCASE("デフォルトコンストラクタ") {
        StaticVector<int, 10> vec;
        CHECK(vec.empty());
        CHECK_EQ(vec.size(), 0U);
        CHECK_EQ(vec.capacity(), 10U);
        CHECK_FALSE(vec.full());
    }

    SUBCASE("初期化リストからの構築") {
        StaticVector<int, 5> vec = {1, 2, 3};
        CHECK_EQ(vec.size(), 3U);
        CHECK_EQ(vec[0], 1);
        CHECK_EQ(vec[1], 2);
        CHECK_EQ(vec[2], 3);
    }

    SUBCASE("初期化リストが容量を超える場合") {
        StaticVector<int, 3> vec = {1, 2, 3, 4, 5};
        CHECK_EQ(vec.size(), 3U); // 容量までしか入らない
        CHECK(vec.full());
    }
}

TEST_CASE("StaticVector - 要素アクセス") {
    StaticVector<int, 5> vec = {10, 20, 30};

    SUBCASE("operator[]") {
        CHECK_EQ(vec[0], 10);
        CHECK_EQ(vec[1], 20);
        CHECK_EQ(vec[2], 30);

        vec[1] = 25;
        CHECK_EQ(vec[1], 25);
    }

    SUBCASE("front/back") {
        CHECK_EQ(vec.front(), 10);
        CHECK_EQ(vec.back(), 30);
    }

    SUBCASE("data") {
        const int* ptr = vec.data();
        CHECK_EQ(ptr[0], 10);
        CHECK_EQ(ptr[1], 20);
    }
}

TEST_CASE("StaticVector - 追加操作") {
    StaticVector<int, 3> vec;

    SUBCASE("push_back") {
        CHECK(vec.push_back(1));
        CHECK(vec.push_back(2));
        CHECK(vec.push_back(3));
        CHECK_FALSE(vec.push_back(4)); // 容量超過

        CHECK_EQ(vec.size(), 3U);
        CHECK(vec.full());
    }

    SUBCASE("emplace_back") {
        CHECK(vec.emplace_back(100));
        CHECK_EQ(vec[0], 100);
    }

    SUBCASE("remaining") {
        vec.push_back(1);
        CHECK_EQ(vec.remaining(), 2U);
        vec.push_back(2);
        CHECK_EQ(vec.remaining(), 1U);
    }
}

TEST_CASE("StaticVector - 削除操作") {
    SUBCASE("pop_back") {
        StaticVector<int, 5> vec = {1, 2, 3};
        vec.pop_back();
        CHECK_EQ(vec.size(), 2U);
        CHECK_EQ(vec.back(), 2);
    }

    SUBCASE("clear") {
        StaticVector<int, 5> vec = {1, 2, 3};
        vec.clear();
        CHECK(vec.empty());
        CHECK_EQ(vec.size(), 0U);
    }

    SUBCASE("erase（順序維持）") {
        StaticVector<int, 5> vec = {1, 2, 3, 4, 5};
        CHECK(vec.erase(1)); // 2を削除
        CHECK_EQ(vec.size(), 4U);
        CHECK_EQ(vec[0], 1);
        CHECK_EQ(vec[1], 3);
        CHECK_EQ(vec[2], 4);
        CHECK_EQ(vec[3], 5);
    }

    SUBCASE("erase_unordered（高速削除）") {
        StaticVector<int, 5> vec = {1, 2, 3, 4, 5};
        CHECK(vec.erase_unordered(1)); // 2を削除、末尾の5が移動
        CHECK_EQ(vec.size(), 4U);
        CHECK_EQ(vec[0], 1);
        CHECK_EQ(vec[1], 5); // 末尾要素が移動
        CHECK_EQ(vec[2], 3);
        CHECK_EQ(vec[3], 4);
    }

    SUBCASE("erase 範囲外") {
        StaticVector<int, 5> vec = {1, 2, 3};
        CHECK_FALSE(vec.erase(10));
    }
}

TEST_CASE("StaticVector - resize") {
    SUBCASE("縮小") {
        StaticVector<int, 10> vec = {1, 2, 3, 4, 5};
        CHECK(vec.resize(3));
        CHECK_EQ(vec.size(), 3U);
        CHECK_EQ(vec.back(), 3);
    }

    SUBCASE("拡大") {
        StaticVector<int, 10> vec = {1, 2};
        CHECK(vec.resize(5));
        CHECK_EQ(vec.size(), 5U);
    }

    SUBCASE("容量超過") {
        StaticVector<int, 5> vec;
        CHECK_FALSE(vec.resize(10));
    }
}

TEST_CASE("StaticVector - イテレータ") {
    StaticVector<int, 5> vec = {10, 20, 30};

    SUBCASE("範囲for文") {
        int sum = 0;
        for (int val : vec) {
            sum += val;
        }
        CHECK_EQ(sum, 60);
    }

    SUBCASE("begin/end") {
        CHECK_EQ(*vec.begin(), 10);
        CHECK_EQ(*(vec.end() - 1), 30);
    }

    SUBCASE("cbegin/cend") {
        CHECK_EQ(*vec.cbegin(), 10);
    }
}

TEST_CASE("StaticVector - span変換") {
    StaticVector<int, 5> vec = {1, 2, 3};

    SUBCASE("as_span") {
        auto s = vec.as_span();
        CHECK_EQ(s.size(), 3U);
        CHECK_EQ(s[0], 1);
        CHECK_EQ(s[2], 3);
    }

    SUBCASE("const as_span") {
        const auto& cvec = vec;
        auto cs = cvec.as_span();
        CHECK_EQ(cs.size(), 3U);
    }
}

TEST_CASE("StaticVector - コピー・ムーブ") {
    SUBCASE("コピーコンストラクタ") {
        StaticVector<int, 5> vec1 = {1, 2, 3};
        StaticVector<int, 5> vec2(vec1);

        CHECK_EQ(vec2.size(), 3U);
        CHECK_EQ(vec2[0], 1);
        CHECK_EQ(vec2[1], 2);
        CHECK_EQ(vec2[2], 3);
    }

    SUBCASE("コピー代入") {
        StaticVector<int, 5> vec1 = {1, 2, 3};
        StaticVector<int, 5> vec2;
        vec2 = vec1;

        CHECK_EQ(vec2.size(), 3U);
        CHECK_EQ(vec2[0], 1);
    }

    SUBCASE("ムーブコンストラクタ") {
        StaticVector<int, 5> vec1 = {1, 2, 3};
        StaticVector<int, 5> vec2(static_cast<StaticVector<int, 5>&&>(vec1));

        CHECK_EQ(vec2.size(), 3U);
        CHECK(vec1.empty()); // ムーブ後は空
    }

    SUBCASE("ムーブ代入") {
        StaticVector<int, 5> vec1 = {1, 2, 3};
        StaticVector<int, 5> vec2;
        vec2 = static_cast<StaticVector<int, 5>&&>(vec1);

        CHECK_EQ(vec2.size(), 3U);
        CHECK(vec1.empty());
    }
}

TEST_CASE("StaticVector - 比較演算子") {
    StaticVector<int, 5> vec1 = {1, 2, 3};
    StaticVector<int, 5> vec2 = {1, 2, 3};
    StaticVector<int, 5> vec3 = {1, 2, 4};
    StaticVector<int, 5> vec4 = {1, 2};

    SUBCASE("等値比較") {
        CHECK(vec1 == vec2);
        CHECK_FALSE(vec1 == vec3);
        CHECK_FALSE(vec1 == vec4);
    }

    SUBCASE("不等比較") {
        CHECK_FALSE(vec1 != vec2);
        CHECK(vec1 != vec3);
        CHECK(vec1 != vec4);
    }
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

    bool operator==(const Counter& other) const { return value == other.value; }
};

int Counter::instances = 0;

TEST_CASE("StaticVector - 非トリビアル型") {
    Counter::instances = 0;

    SUBCASE("構築と破棄") {
        {
            StaticVector<Counter, 5> vec;
            vec.emplace_back(1);
            vec.emplace_back(2);
            vec.emplace_back(3);
            CHECK_EQ(Counter::instances, 3);
        }
        CHECK_EQ(Counter::instances, 0); // 全て破棄されている
    }

    SUBCASE("pop_backでデストラクタが呼ばれる") {
        {
            StaticVector<Counter, 5> vec;
            vec.emplace_back(1);
            vec.emplace_back(2);
            CHECK_EQ(Counter::instances, 2);
            vec.pop_back();
            CHECK_EQ(Counter::instances, 1);
        }
        CHECK_EQ(Counter::instances, 0);
    }

    SUBCASE("clearでデストラクタが呼ばれる") {
        {
            StaticVector<Counter, 5> vec;
            vec.emplace_back(1);
            vec.emplace_back(2);
            vec.emplace_back(3);
            CHECK_EQ(Counter::instances, 3);
            vec.clear();
            CHECK_EQ(Counter::instances, 0);
        }
    }
}
