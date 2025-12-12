#pragma once

#include <cstdint>
#include <type_traits>

namespace omusubi {

/**
 * @brief 型安全なビットフラグ
 *
 * 列挙型をベースとした型安全なフラグ操作を提供する。
 * 各列挙値はビット位置として扱われ、複数のフラグを組み合わせて使用できる。
 *
 * @tparam Enum 列挙型（値は0から始まるビット位置を表す）
 *
 * @note ヒープアロケーションなし、constexpr 対応
 *
 * @example
 * ```cpp
 * enum class Permission : uint8_t {
 *     READ = 0,    // ビット0
 *     WRITE = 1,   // ビット1
 *     EXECUTE = 2  // ビット2
 * };
 *
 * Flags<Permission> perms;
 * perms.set(Permission::READ).set(Permission::WRITE);
 * if (perms.test(Permission::READ)) { ... }
 * ```
 */
template <typename Enum>
class Flags {
    static_assert(std::is_enum_v<Enum>, "Flags requires an enum type");

public:
    // 基底型（enumの基底型を使用）
    using underlying_type = std::underlying_type_t<Enum>;

private:
    underlying_type value_ = 0;

    // enumの値からビットマスクを計算
    [[nodiscard]] static constexpr underlying_type to_mask(Enum flag) noexcept { return underlying_type(1) << static_cast<underlying_type>(flag); }

public:
    /**
     * @brief デフォルトコンストラクタ（すべてのフラグがオフ）
     */
    constexpr Flags() noexcept = default;

    /**
     * @brief 単一フラグで初期化
     * @param flag 初期フラグ
     */
    constexpr Flags(Enum flag) noexcept : value_(to_mask(flag)) {} // NOLINT(google-explicit-constructor)

    /**
     * @brief 生の値で初期化
     * @param value 生の値
     */
    constexpr explicit Flags(underlying_type value) noexcept : value_(value) {}

    /**
     * @brief 指定フラグがセットされているか確認
     * @param flag 確認するフラグ
     * @return フラグがセットされていればtrue
     */
    [[nodiscard]] constexpr bool test(Enum flag) const noexcept { return (value_ & to_mask(flag)) != 0; }

    /**
     * @brief 指定フラグがセットされているか確認（test の別名）
     * @param flag 確認するフラグ
     * @return フラグがセットされていればtrue
     */
    [[nodiscard]] constexpr bool has(Enum flag) const noexcept { return test(flag); }

    /**
     * @brief すべての指定フラグがセットされているか確認
     * @param flags 確認するフラグの組み合わせ
     * @return すべてのフラグがセットされていればtrue
     */
    [[nodiscard]] constexpr bool has_all(Flags flags) const noexcept { return (value_ & flags.value_) == flags.value_; }

    /**
     * @brief いずれかの指定フラグがセットされているか確認
     * @param flags 確認するフラグの組み合わせ
     * @return いずれかのフラグがセットされていればtrue
     */
    [[nodiscard]] constexpr bool has_any(Flags flags) const noexcept { return (value_ & flags.value_) != 0; }

    /**
     * @brief フラグをセット
     * @param flag セットするフラグ
     * @return 自身への参照
     */
    constexpr Flags& set(Enum flag) noexcept {
        value_ |= to_mask(flag);
        return *this;
    }

    /**
     * @brief フラグを指定値にセット
     * @param flag 操作するフラグ
     * @param value セットする値
     * @return 自身への参照
     */
    constexpr Flags& set(Enum flag, bool value) noexcept {
        if (value) {
            return set(flag);
        }
        return reset(flag);
    }

    /**
     * @brief 複数フラグをセット
     * @param flags セットするフラグの組み合わせ
     * @return 自身への参照
     */
    constexpr Flags& set(Flags flags) noexcept {
        value_ |= flags.value_;
        return *this;
    }

    /**
     * @brief フラグをリセット（クリア）
     * @param flag リセットするフラグ
     * @return 自身への参照
     */
    constexpr Flags& reset(Enum flag) noexcept {
        value_ &= ~to_mask(flag);
        return *this;
    }

    /**
     * @brief 複数フラグをリセット
     * @param flags リセットするフラグの組み合わせ
     * @return 自身への参照
     */
    constexpr Flags& reset(Flags flags) noexcept {
        value_ &= ~flags.value_;
        return *this;
    }

    /**
     * @brief すべてのフラグをリセット
     * @return 自身への参照
     */
    constexpr Flags& clear() noexcept {
        value_ = 0;
        return *this;
    }

    /**
     * @brief フラグを反転
     * @param flag 反転するフラグ
     * @return 自身への参照
     */
    constexpr Flags& toggle(Enum flag) noexcept {
        value_ ^= to_mask(flag);
        return *this;
    }

    /**
     * @brief 複数フラグを反転
     * @param flags 反転するフラグの組み合わせ
     * @return 自身への参照
     */
    constexpr Flags& toggle(Flags flags) noexcept {
        value_ ^= flags.value_;
        return *this;
    }

    /**
     * @brief いずれかのフラグがセットされているか
     * @return 1つでもフラグがセットされていればtrue
     */
    [[nodiscard]] constexpr bool any() const noexcept { return value_ != 0; }

    /**
     * @brief すべてのフラグがオフか
     * @return すべてのフラグがオフならtrue
     */
    [[nodiscard]] constexpr bool none() const noexcept { return value_ == 0; }

    /**
     * @brief セットされているフラグの数を数える
     * @return セットされているフラグの数
     */
    [[nodiscard]] constexpr uint32_t count() const noexcept {
        underlying_type v = value_;
        uint32_t result = 0;
        while (v != 0) {
            v &= v - 1;
            ++result;
        }
        return result;
    }

    /**
     * @brief 生の値を取得
     * @return 内部の値
     */
    [[nodiscard]] constexpr underlying_type value() const noexcept { return value_; }

    /**
     * @brief bool への変換（いずれかのフラグがセットされているか）
     */
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return any(); }

    /**
     * @brief 生の値への変換
     */
    [[nodiscard]] constexpr explicit operator underlying_type() const noexcept { return value_; }

    // ビット演算子

    /**
     * @brief ビットOR代入
     */
    constexpr Flags& operator|=(Flags other) noexcept {
        value_ |= other.value_;
        return *this;
    }

    /**
     * @brief ビットOR代入（単一フラグ）
     */
    constexpr Flags& operator|=(Enum flag) noexcept {
        value_ |= to_mask(flag);
        return *this;
    }

    /**
     * @brief ビットAND代入
     */
    constexpr Flags& operator&=(Flags other) noexcept {
        value_ &= other.value_;
        return *this;
    }

    /**
     * @brief ビットXOR代入
     */
    constexpr Flags& operator^=(Flags other) noexcept {
        value_ ^= other.value_;
        return *this;
    }

    /**
     * @brief ビットXOR代入（単一フラグ）
     */
    constexpr Flags& operator^=(Enum flag) noexcept {
        value_ ^= to_mask(flag);
        return *this;
    }

    /**
     * @brief ビットNOT
     */
    [[nodiscard]] constexpr Flags operator~() const noexcept { return Flags(~value_); }

    // 比較演算子

    [[nodiscard]] constexpr bool operator==(Flags other) const noexcept { return value_ == other.value_; }

    [[nodiscard]] constexpr bool operator!=(Flags other) const noexcept { return value_ != other.value_; }
};

// 二項演算子

template <typename Enum>
[[nodiscard]] constexpr Flags<Enum> operator|(Flags<Enum> lhs, Flags<Enum> rhs) noexcept {
    return Flags<Enum>(lhs.value() | rhs.value());
}

template <typename Enum>
[[nodiscard]] constexpr Flags<Enum> operator|(Flags<Enum> lhs, Enum rhs) noexcept {
    return lhs | Flags<Enum>(rhs);
}

template <typename Enum>
[[nodiscard]] constexpr Flags<Enum> operator|(Enum lhs, Flags<Enum> rhs) noexcept {
    return Flags<Enum>(lhs) | rhs;
}

template <typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
[[nodiscard]] constexpr Flags<Enum> operator|(Enum lhs, Enum rhs) noexcept {
    return Flags<Enum>(lhs) | Flags<Enum>(rhs);
}

template <typename Enum>
[[nodiscard]] constexpr Flags<Enum> operator&(Flags<Enum> lhs, Flags<Enum> rhs) noexcept {
    return Flags<Enum>(lhs.value() & rhs.value());
}

template <typename Enum>
[[nodiscard]] constexpr Flags<Enum> operator^(Flags<Enum> lhs, Flags<Enum> rhs) noexcept {
    return Flags<Enum>(lhs.value() ^ rhs.value());
}

/**
 * @brief 単一フラグからFlagsを作成するヘルパー
 * @param flag フラグ
 * @return Flagsオブジェクト
 */
template <typename Enum>
[[nodiscard]] constexpr Flags<Enum> make_flags(Enum flag) noexcept {
    return Flags<Enum>(flag);
}

/**
 * @brief 複数フラグからFlagsを作成するヘルパー（可変長引数）
 * @param first 最初のフラグ
 * @param rest 残りのフラグ
 * @return Flagsオブジェクト
 */
template <typename Enum, typename... Enums>
[[nodiscard]] constexpr Flags<Enum> make_flags(Enum first, Enums... rest) noexcept {
    return Flags<Enum>(first) | make_flags(rest...);
}

} // namespace omusubi
