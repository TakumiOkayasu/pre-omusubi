// Function<Sig, Size> の単体テスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/function.hpp>

#include "doctest.h"

using namespace omusubi;

// テスト用の自由関数
int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

void increment(int& value) {
    ++value;
}

TEST_CASE("Function - 基本機能") {
    SUBCASE("デフォルトコンストラクタ") {
        Function<int(int, int)> f;
        CHECK(f.empty());
        CHECK_FALSE(static_cast<bool>(f));
        CHECK(f == nullptr);
    }

    SUBCASE("nullptr からの構築") {
        Function<int(int, int)> f(nullptr);
        CHECK(f.empty());
    }
}

TEST_CASE("Function - 関数ポインタ") {
    SUBCASE("関数ポインタからの構築") {
        Function<int(int, int)> f(add);
        CHECK_FALSE(f.empty());
        CHECK(static_cast<bool>(f));
        CHECK_EQ(f(2, 3), 5);
    }

    SUBCASE("別の関数に変更") {
        Function<int(int, int)> f(add);
        CHECK_EQ(f(2, 3), 5);

        f = Function<int(int, int)>(multiply);
        CHECK_EQ(f(2, 3), 6);
    }

    SUBCASE("void戻り値の関数") {
        Function<void(int&)> f(increment);
        int value = 10;
        f(value);
        CHECK_EQ(value, 11);
    }
}

TEST_CASE("Function - ラムダ") {
    SUBCASE("キャプチャなしラムダ") {
        Function<int(int, int)> f([](int a, int b) { return a - b; });
        CHECK_EQ(f(10, 3), 7);
    }

    SUBCASE("値キャプチャラムダ") {
        int offset = 100;
        Function<int(int)> f([offset](int x) { return x + offset; });
        CHECK_EQ(f(5), 105);
    }

    SUBCASE("参照キャプチャラムダ") {
        int counter = 0;
        Function<void()> f([&counter]() { ++counter; });

        f();
        CHECK_EQ(counter, 1);
        f();
        CHECK_EQ(counter, 2);
    }

    SUBCASE("複数キャプチャ") {
        int a = 10;
        int b = 20;
        Function<int()> f([a, b]() { return a + b; });
        CHECK_EQ(f(), 30);
    }
}

TEST_CASE("Function - ファンクタ") {
    struct Adder {
        int base;

        explicit Adder(int b) : base(b) {}

        int operator()(int x) const { return base + x; }
    };

    SUBCASE("ファンクタからの構築") {
        Function<int(int)> f(Adder(100));
        CHECK_EQ(f(5), 105);
    }
}

TEST_CASE("Function - ムーブ") {
    SUBCASE("ムーブコンストラクタ") {
        int captured = 42;
        Function<int()> f1([captured]() { return captured; });
        CHECK_EQ(f1(), 42);

        Function<int()> f2(static_cast<Function<int()>&&>(f1));
        CHECK(f1.empty()); // ムーブ後は空
        CHECK_EQ(f2(), 42);
    }

    SUBCASE("ムーブ代入") {
        Function<int(int)> f1([](int x) { return x * 2; });
        Function<int(int)> f2([](int x) { return x * 3; });

        f2 = static_cast<Function<int(int)>&&>(f1);
        CHECK(f1.empty());
        CHECK_EQ(f2(5), 10);
    }
}

TEST_CASE("Function - クリア") {
    SUBCASE("clear()") {
        Function<int()> f([]() { return 42; });
        CHECK_FALSE(f.empty());

        f.clear();
        CHECK(f.empty());
    }

    SUBCASE("nullptr 代入") {
        Function<int()> f([]() { return 42; });
        f = nullptr;
        CHECK(f.empty());
    }
}

TEST_CASE("Function - 比較演算子") {
    Function<int()> empty;
    Function<int()> filled([]() { return 1; });

    CHECK(empty == nullptr);
    CHECK(nullptr == empty);
    CHECK_FALSE(empty != nullptr);

    CHECK(filled != nullptr);
    CHECK(nullptr != filled);
    CHECK_FALSE(filled == nullptr);
}

TEST_CASE("Function - storage_size") {
    CHECK_EQ(Function<int()>::storage_size(), 32U);
    CHECK_EQ((Function<int(), 64>::storage_size()), 64U);
    CHECK_EQ((Function<int(), 16>::storage_size()), 16U);
}

TEST_CASE("Function - 大きなキャプチャ") {
    // 大きなストレージを使用
    struct LargeData {
        int values[10];
    };

    LargeData data {};
    data.values[0] = 100;
    data.values[9] = 999;

    Function<int(), 64> f([data]() { return data.values[0] + data.values[9]; });
    CHECK_EQ(f(), 1099);
}

// デストラクタが呼ばれることを確認
struct Counter {
    static int instances;

    Counter() { ++instances; }

    Counter(const Counter&) { ++instances; }

    Counter(Counter&&) noexcept { ++instances; }

    ~Counter() { --instances; }

    Counter& operator=(const Counter&) = default;
    Counter& operator=(Counter&&) = default;

    int operator()() const { return instances; }
};

int Counter::instances = 0;

TEST_CASE("Function - デストラクタ呼び出し") {
    Counter::instances = 0;

    SUBCASE("スコープを抜けるとデストラクタが呼ばれる") {
        {
            Function<int()> f(Counter {});
            CHECK_EQ(Counter::instances, 1);
        }
        CHECK_EQ(Counter::instances, 0);
    }

    SUBCASE("clear()でデストラクタが呼ばれる") {
        Function<int()> f(Counter {});
        CHECK_EQ(Counter::instances, 1);
        f.clear();
        CHECK_EQ(Counter::instances, 0);
    }

    SUBCASE("再代入でデストラクタが呼ばれる") {
        Function<int()> f(Counter {});
        CHECK_EQ(Counter::instances, 1);
        f = nullptr;
        CHECK_EQ(Counter::instances, 0);
    }
}

TEST_CASE("Function - 引数の転送") {
    SUBCASE("値渡し") {
        Function<int(int)> f([](int x) { return x * 2; });
        CHECK_EQ(f(21), 42);
    }

    SUBCASE("参照渡し") {
        Function<void(int&)> f([](int& x) { x *= 2; });
        int value = 21;
        f(value);
        CHECK_EQ(value, 42);
    }

    SUBCASE("const参照渡し") {
        Function<int(const int&)> f([](const int& x) { return x + 1; });
        int value = 41;
        CHECK_EQ(f(value), 42);
    }
}
