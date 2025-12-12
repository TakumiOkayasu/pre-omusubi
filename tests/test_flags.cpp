// flags.hpp の単体テスト

#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <omusubi/core/flags.hpp>

#include "doctest.h"

using namespace omusubi;

// テスト用の列挙型
enum class Permission : uint8_t {
    READ = 0,    // ビット0
    WRITE = 1,   // ビット1
    EXECUTE = 2, // ビット2
    DELETE = 3   // ビット3
};

enum class Status : uint16_t { CONNECTED = 0, AUTHENTICATED = 1, ENCRYPTED = 2, COMPRESSED = 3 };

TEST_CASE("Flags - 基本機能") {
    SUBCASE("デフォルト構築") {
        Flags<Permission> flags;
        CHECK(flags.none());
        CHECK_FALSE(flags.any());
        CHECK_EQ(flags.count(), 0);
        CHECK_EQ(flags.value(), 0);
    }

    SUBCASE("単一フラグで構築") {
        Flags<Permission> flags(Permission::READ);
        CHECK(flags.test(Permission::READ));
        CHECK_FALSE(flags.test(Permission::WRITE));
        CHECK_EQ(flags.count(), 1);
    }

    SUBCASE("生の値で構築") {
        Flags<Permission> flags(static_cast<uint8_t>(0b1010)); // WRITE と DELETE
        CHECK_FALSE(flags.test(Permission::READ));
        CHECK(flags.test(Permission::WRITE));
        CHECK_FALSE(flags.test(Permission::EXECUTE));
        CHECK(flags.test(Permission::DELETE));
    }
}

TEST_CASE("Flags - フラグ操作") {
    SUBCASE("set(flag)") {
        Flags<Permission> flags;
        flags.set(Permission::READ).set(Permission::EXECUTE);
        CHECK(flags.test(Permission::READ));
        CHECK_FALSE(flags.test(Permission::WRITE));
        CHECK(flags.test(Permission::EXECUTE));
        CHECK_EQ(flags.count(), 2);
    }

    SUBCASE("set(flag, value)") {
        Flags<Permission> flags;
        flags.set(Permission::READ, true);
        flags.set(Permission::WRITE, false);
        flags.set(Permission::EXECUTE, true);
        CHECK(flags.test(Permission::READ));
        CHECK_FALSE(flags.test(Permission::WRITE));
        CHECK(flags.test(Permission::EXECUTE));
    }

    SUBCASE("set(flags)") {
        Flags<Permission> flags;
        Flags<Permission> to_set = Permission::READ | Permission::WRITE;
        flags.set(to_set);
        CHECK(flags.test(Permission::READ));
        CHECK(flags.test(Permission::WRITE));
        CHECK_FALSE(flags.test(Permission::EXECUTE));
    }

    SUBCASE("reset(flag)") {
        Flags<Permission> flags = Permission::READ | Permission::WRITE | Permission::EXECUTE;
        flags.reset(Permission::WRITE);
        CHECK(flags.test(Permission::READ));
        CHECK_FALSE(flags.test(Permission::WRITE));
        CHECK(flags.test(Permission::EXECUTE));
    }

    SUBCASE("reset(flags)") {
        Flags<Permission> flags = Permission::READ | Permission::WRITE | Permission::EXECUTE;
        flags.reset(Permission::READ | Permission::EXECUTE);
        CHECK_FALSE(flags.test(Permission::READ));
        CHECK(flags.test(Permission::WRITE));
        CHECK_FALSE(flags.test(Permission::EXECUTE));
    }

    SUBCASE("clear()") {
        Flags<Permission> flags = Permission::READ | Permission::WRITE;
        flags.clear();
        CHECK(flags.none());
        CHECK_EQ(flags.value(), 0);
    }

    SUBCASE("toggle(flag)") {
        Flags<Permission> flags(Permission::READ);
        flags.toggle(Permission::READ);  // ON -> OFF
        flags.toggle(Permission::WRITE); // OFF -> ON
        CHECK_FALSE(flags.test(Permission::READ));
        CHECK(flags.test(Permission::WRITE));
    }

    SUBCASE("toggle(flags)") {
        Flags<Permission> flags = Permission::READ | Permission::WRITE;
        flags.toggle(Permission::WRITE | Permission::EXECUTE);
        CHECK(flags.test(Permission::READ));        // そのまま
        CHECK_FALSE(flags.test(Permission::WRITE)); // ON -> OFF
        CHECK(flags.test(Permission::EXECUTE));     // OFF -> ON
    }
}

TEST_CASE("Flags - クエリ関数") {
    SUBCASE("test() / has()") {
        Flags<Permission> flags = Permission::READ | Permission::WRITE;
        CHECK(flags.test(Permission::READ));
        CHECK(flags.has(Permission::READ));
        CHECK(flags.test(Permission::WRITE));
        CHECK(flags.has(Permission::WRITE));
        CHECK_FALSE(flags.test(Permission::EXECUTE));
        CHECK_FALSE(flags.has(Permission::EXECUTE));
    }

    SUBCASE("has_all()") {
        Flags<Permission> flags = Permission::READ | Permission::WRITE | Permission::EXECUTE;

        CHECK(flags.has_all(Permission::READ | Permission::WRITE));
        CHECK(flags.has_all(Permission::READ | Permission::WRITE | Permission::EXECUTE));
        CHECK_FALSE(flags.has_all(Permission::READ | Permission::DELETE));
    }

    SUBCASE("has_any()") {
        Flags<Permission> flags = Permission::READ | Permission::WRITE;

        CHECK(flags.has_any(Permission::READ | Permission::EXECUTE));
        CHECK(flags.has_any(Permission::WRITE | Permission::DELETE));
        CHECK_FALSE(flags.has_any(Permission::EXECUTE | Permission::DELETE));
    }

    SUBCASE("any() / none()") {
        Flags<Permission> empty;
        CHECK_FALSE(empty.any());
        CHECK(empty.none());

        Flags<Permission> has_flags(Permission::READ);
        CHECK(has_flags.any());
        CHECK_FALSE(has_flags.none());
    }

    SUBCASE("count()") {
        Flags<Permission> flags;
        CHECK_EQ(flags.count(), 0);

        flags.set(Permission::READ);
        CHECK_EQ(flags.count(), 1);

        flags.set(Permission::WRITE).set(Permission::EXECUTE);
        CHECK_EQ(flags.count(), 3);
    }
}

TEST_CASE("Flags - 変換") {
    SUBCASE("value()") {
        Flags<Permission> flags = Permission::READ | Permission::EXECUTE;
        CHECK_EQ(flags.value(), 0b0101);
    }

    SUBCASE("operator bool()") {
        Flags<Permission> empty;
        CHECK_FALSE(static_cast<bool>(empty));

        Flags<Permission> has_flags(Permission::READ);
        CHECK(static_cast<bool>(has_flags));
    }

    SUBCASE("operator underlying_type()") {
        Flags<Permission> flags = Permission::READ | Permission::WRITE;
        uint8_t raw = static_cast<uint8_t>(flags);
        CHECK_EQ(raw, 0b0011);
    }
}

TEST_CASE("Flags - ビット演算子") {
    SUBCASE("operator|") {
        Flags<Permission> a(Permission::READ);
        Flags<Permission> b(Permission::WRITE);
        auto result = a | b;
        CHECK(result.test(Permission::READ));
        CHECK(result.test(Permission::WRITE));
    }

    SUBCASE("operator| with Enum") {
        Flags<Permission> flags(Permission::READ);
        auto result = flags | Permission::WRITE;
        CHECK(result.test(Permission::READ));
        CHECK(result.test(Permission::WRITE));

        result = Permission::EXECUTE | flags;
        CHECK(result.test(Permission::READ));
        CHECK(result.test(Permission::EXECUTE));
    }

    SUBCASE("Enum | Enum") {
        auto flags = Permission::READ | Permission::WRITE;
        CHECK(flags.test(Permission::READ));
        CHECK(flags.test(Permission::WRITE));
        CHECK_FALSE(flags.test(Permission::EXECUTE));
    }

    SUBCASE("operator&") {
        Flags<Permission> a = Permission::READ | Permission::WRITE;
        Flags<Permission> b = Permission::WRITE | Permission::EXECUTE;
        auto result = a & b;
        CHECK_FALSE(result.test(Permission::READ));
        CHECK(result.test(Permission::WRITE));
        CHECK_FALSE(result.test(Permission::EXECUTE));
    }

    SUBCASE("operator^") {
        Flags<Permission> a = Permission::READ | Permission::WRITE;
        Flags<Permission> b = Permission::WRITE | Permission::EXECUTE;
        auto result = a ^ b;
        CHECK(result.test(Permission::READ));
        CHECK_FALSE(result.test(Permission::WRITE));
        CHECK(result.test(Permission::EXECUTE));
    }

    SUBCASE("operator~") {
        Flags<Permission> flags(Permission::READ);
        auto inverted = ~flags;
        CHECK_FALSE(inverted.test(Permission::READ));
        // 注: ~は全ビット反転なので、他のビットはすべて1になる
    }

    SUBCASE("複合代入演算子") {
        Flags<Permission> flags(Permission::READ);

        flags |= Permission::WRITE;
        CHECK(flags.test(Permission::READ));
        CHECK(flags.test(Permission::WRITE));

        flags ^= Permission::READ;
        CHECK_FALSE(flags.test(Permission::READ));
        CHECK(flags.test(Permission::WRITE));

        flags &= Permission::WRITE | Permission::EXECUTE;
        CHECK_FALSE(flags.test(Permission::READ));
        CHECK(flags.test(Permission::WRITE));
    }
}

TEST_CASE("Flags - 比較演算子") {
    Flags<Permission> a = Permission::READ | Permission::WRITE;
    Flags<Permission> b = Permission::READ | Permission::WRITE;
    Flags<Permission> c = Permission::READ | Permission::EXECUTE;

    CHECK(a == b);
    CHECK_FALSE(a == c);
    CHECK(a != c);
    CHECK_FALSE(a != b);
}

TEST_CASE("Flags - make_flags ヘルパー") {
    SUBCASE("単一フラグ") {
        auto flags = make_flags(Permission::READ);
        CHECK(flags.test(Permission::READ));
        CHECK_EQ(flags.count(), 1);
    }

    SUBCASE("複数フラグ") {
        auto flags = make_flags(Permission::READ, Permission::WRITE, Permission::EXECUTE);
        CHECK(flags.test(Permission::READ));
        CHECK(flags.test(Permission::WRITE));
        CHECK(flags.test(Permission::EXECUTE));
        CHECK_FALSE(flags.test(Permission::DELETE));
        CHECK_EQ(flags.count(), 3);
    }
}

TEST_CASE("Flags - 異なる基底型") {
    // uint16_t ベースの enum
    Flags<Status> status;
    status.set(Status::CONNECTED).set(Status::ENCRYPTED);

    CHECK(status.test(Status::CONNECTED));
    CHECK_FALSE(status.test(Status::AUTHENTICATED));
    CHECK(status.test(Status::ENCRYPTED));
    CHECK_EQ(status.count(), 2);
}

TEST_CASE("Flags - constexpr 対応") {
    // コンパイル時評価
    constexpr Flags<Permission> flags = Permission::READ | Permission::WRITE;
    static_assert(flags.test(Permission::READ), "READ should be set");
    static_assert(flags.test(Permission::WRITE), "WRITE should be set");
    static_assert(!flags.test(Permission::EXECUTE), "EXECUTE should not be set");
    static_assert(flags.count() == 2, "count should be 2");

    constexpr auto combined = flags | Permission::EXECUTE;
    static_assert(combined.count() == 3, "combined count should be 3");

    constexpr auto created = make_flags(Permission::READ, Permission::EXECUTE);
    static_assert(created.test(Permission::READ), "READ should be set");
    static_assert(created.test(Permission::EXECUTE), "EXECUTE should be set");
    static_assert(!created.test(Permission::WRITE), "WRITE should not be set");

    // ランタイムでも確認
    CHECK_EQ(flags.count(), 2);
    CHECK_EQ(combined.count(), 3);
}

TEST_CASE("Flags - メソッドチェーン") {
    Flags<Permission> flags;
    flags.set(Permission::READ).set(Permission::WRITE).toggle(Permission::READ).set(Permission::EXECUTE);

    CHECK_FALSE(flags.test(Permission::READ));
    CHECK(flags.test(Permission::WRITE));
    CHECK(flags.test(Permission::EXECUTE));
    CHECK_EQ(flags.count(), 2);
}

TEST_CASE("Flags - 実用例: ファイルパーミッション") {
    // 読み書き権限を設定
    auto perms = Permission::READ | Permission::WRITE;

    // 実行権限を追加
    perms |= Permission::EXECUTE;

    // 読み取り権限があるか確認
    CHECK(perms.has(Permission::READ));

    // 読み書き両方あるか確認
    CHECK(perms.has_all(Permission::READ | Permission::WRITE));

    // 削除権限を削除（もともとないが）
    perms.reset(Permission::DELETE);

    // 書き込み権限を取り消し
    perms.reset(Permission::WRITE);
    CHECK_FALSE(perms.has(Permission::WRITE));
}
