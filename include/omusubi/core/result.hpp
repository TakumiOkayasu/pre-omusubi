#pragma once

#include <omusubi/core/types.h>

#include <cassert>
#include <type_traits>

namespace omusubi {

namespace detail {

/**
 * @brief Result用ストレージ基底クラス（トリビアルデストラクタ版）
 *
 * T と E が両方トリビアルに破壊可能な場合に使用。
 * constexpr 対応のためデストラクタはデフォルト。
 */
template <typename T, typename E, bool TriviallyDestructible>
struct ResultStorage {
    union {
        T value;
        E error;
        char dummy;
    };

    bool is_ok;

    constexpr ResultStorage() noexcept : dummy(0), is_ok(false) {}

    constexpr explicit ResultStorage(const T& v) noexcept : value(v), is_ok(true) {}

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    constexpr explicit ResultStorage(T&& v) noexcept : value(static_cast<T&&>(v)), is_ok(true) {}

    struct ErrorTag {};

    constexpr ResultStorage(ErrorTag /*tag*/, const E& e) noexcept : error(e), is_ok(false) {}

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    constexpr ResultStorage(ErrorTag /*tag*/, E&& e) noexcept : error(static_cast<E&&>(e)), is_ok(false) {}

    // トリビアルデストラクタ（constexpr対応）
    ~ResultStorage() = default;

    // コピー・ムーブ
    constexpr ResultStorage(const ResultStorage& other) noexcept : dummy(0), is_ok(other.is_ok) {
        if (is_ok) {
            value = other.value;
        } else {
            error = other.error;
        }
    }

    constexpr ResultStorage(ResultStorage&& other) noexcept : dummy(0), is_ok(other.is_ok) {
        if (is_ok) {
            value = static_cast<T&&>(other.value);
        } else {
            error = static_cast<E&&>(other.error);
        }
    }

    constexpr ResultStorage& operator=(const ResultStorage& other) noexcept {
        if (this != &other) {
            is_ok = other.is_ok;
            if (is_ok) {
                value = other.value;
            } else {
                error = other.error;
            }
        }
        return *this;
    }

    constexpr ResultStorage& operator=(ResultStorage&& other) noexcept {
        if (this != &other) {
            is_ok = other.is_ok;
            if (is_ok) {
                value = static_cast<T&&>(other.value);
            } else {
                error = static_cast<E&&>(other.error);
            }
        }
        return *this;
    }
};

/**
 * @brief Result用ストレージ基底クラス（非トリビアルデストラクタ版）
 *
 * T または E が非トリビアルデストラクタを持つ場合に使用。
 */
template <typename T, typename E>
struct ResultStorage<T, E, false> {
    union {
        T value;
        E error;
        char dummy;
    };

    bool is_ok;

    constexpr ResultStorage() noexcept : dummy(0), is_ok(false) {}

    constexpr explicit ResultStorage(const T& v) noexcept : value(v), is_ok(true) {}

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    constexpr explicit ResultStorage(T&& v) noexcept : value(static_cast<T&&>(v)), is_ok(true) {}

    struct ErrorTag {};

    constexpr ResultStorage(ErrorTag /*tag*/, const E& e) noexcept : error(e), is_ok(false) {}

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    constexpr ResultStorage(ErrorTag /*tag*/, E&& e) noexcept : error(static_cast<E&&>(e)), is_ok(false) {}

    // 非トリビアルデストラクタ
    ~ResultStorage() {
        if (is_ok) {
            value.~T();
        } else {
            error.~E();
        }
    }

    // コピー・ムーブ（デストラクタ呼び出しが必要）
    ResultStorage(const ResultStorage& other) noexcept : dummy(0), is_ok(other.is_ok) {
        if (is_ok) {
            new (&value) T(other.value);
        } else {
            new (&error) E(other.error);
        }
    }

    ResultStorage(ResultStorage&& other) noexcept : dummy(0), is_ok(other.is_ok) {
        if (is_ok) {
            new (&value) T(static_cast<T&&>(other.value));
        } else {
            new (&error) E(static_cast<E&&>(other.error));
        }
    }

    ResultStorage& operator=(const ResultStorage& other) noexcept {
        if (this != &other) {
            // 既存の値を破棄
            if (is_ok) {
                value.~T();
            } else {
                error.~E();
            }

            is_ok = other.is_ok;
            if (is_ok) {
                new (&value) T(other.value);
            } else {
                new (&error) E(other.error);
            }
        }
        return *this;
    }

    ResultStorage& operator=(ResultStorage&& other) noexcept {
        if (this != &other) {
            // 既存の値を破棄
            if (is_ok) {
                value.~T();
            } else {
                error.~E();
            }

            is_ok = other.is_ok;
            if (is_ok) {
                new (&value) T(static_cast<T&&>(other.value));
            } else {
                new (&error) E(static_cast<E&&>(other.error));
            }
        }
        return *this;
    }
};

} // namespace detail

/**
 * @brief Rust風の Result<T, E> 実装
 *
 * 成功時の値とエラー時のエラー情報を1つの型で表現します。
 * ヒープアロケーションを行わず、スタック上に値またはエラーを配置します。
 *
 * @tparam T 成功時の値の型
 * @tparam E エラー情報の型（デフォルトは Error）
 *
 * @note constexpr 対応：T と E がリテラル型の場合、コンパイル時評価可能
 */
template <typename T, typename E = Error>
class Result : private detail::ResultStorage<T, E, std::is_trivially_destructible_v<T> && std::is_trivially_destructible_v<E>> {
private:
    using Base = detail::ResultStorage<T, E, std::is_trivially_destructible_v<T> && std::is_trivially_destructible_v<E>>;
    using ErrorTag = typename Base::ErrorTag;

    // プライベートコンストラクタ（ok用）
    constexpr explicit Result(const T& value) noexcept : Base(value) {}

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    constexpr explicit Result(T&& value) noexcept : Base(static_cast<T&&>(value)) {}

    // プライベートコンストラクタ（err用）
    constexpr Result(ErrorTag tag, const E& error) noexcept : Base(tag, error) {}

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    constexpr Result(ErrorTag tag, E&& error) noexcept : Base(tag, static_cast<E&&>(error)) {}

public:
    /**
     * @brief 成功を表す Result を作成
     * @param value 成功時の値
     * @return 成功を表す Result
     */
    [[nodiscard]] static constexpr Result ok(const T& value) noexcept { return Result(value); }

    /**
     * @brief 成功を表す Result を作成（ムーブ）
     * @param value 成功時の値（ムーブ）
     * @return 成功を表す Result
     */
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    [[nodiscard]] static constexpr Result ok(T&& value) noexcept { return Result(static_cast<T&&>(value)); }

    /**
     * @brief エラーを表す Result を作成
     * @param error エラー情報
     * @return エラーを表す Result
     */
    [[nodiscard]] static constexpr Result err(const E& error) noexcept { return Result(ErrorTag {}, error); }

    /**
     * @brief エラーを表す Result を作成（ムーブ）
     * @param error エラー情報（ムーブ）
     * @return エラーを表す Result
     */
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    [[nodiscard]] static constexpr Result err(E&& error) noexcept { return Result(ErrorTag {}, static_cast<E&&>(error)); }

    // コピー・ムーブは基底クラスから継承
    constexpr Result(const Result&) noexcept = default;
    constexpr Result(Result&&) noexcept = default;
    constexpr Result& operator=(const Result&) noexcept = default;
    constexpr Result& operator=(Result&&) noexcept = default;
    ~Result() = default;

    /**
     * @brief 成功かどうかを判定
     * @return 成功の場合true
     */
    [[nodiscard]] constexpr bool is_ok() const noexcept { return Base::is_ok; }

    /**
     * @brief エラーかどうかを判定
     * @return エラーの場合true
     */
    [[nodiscard]] constexpr bool is_err() const noexcept { return !Base::is_ok; }

    /**
     * @brief 成功時の値を取得（参照）
     * @return 成功時の値への参照
     * @warning エラー状態の場合はアサーション失敗
     */
    [[nodiscard]] constexpr T& value() {
        assert(Base::is_ok);
        return Base::value;
    }

    /**
     * @brief 成功時の値を取得（const参照）
     * @return 成功時の値へのconst参照
     * @warning エラー状態の場合はアサーション失敗
     */
    [[nodiscard]] constexpr const T& value() const {
        assert(Base::is_ok);
        return Base::value;
    }

    /**
     * @brief エラー情報を取得（参照）
     * @return エラー情報への参照
     * @warning 成功状態の場合はアサーション失敗
     */
    [[nodiscard]] constexpr E& error() {
        assert(!Base::is_ok);
        return Base::error;
    }

    /**
     * @brief エラー情報を取得（const参照）
     * @return エラー情報へのconst参照
     * @warning 成功状態の場合はアサーション失敗
     */
    [[nodiscard]] constexpr const E& error() const {
        assert(!Base::is_ok);
        return Base::error;
    }

    /**
     * @brief 値を取得、エラーの場合はデフォルト値を返す
     * @param default_value デフォルト値
     * @return 成功時の値、またはdefault_value
     */
    [[nodiscard]] constexpr T value_or(const T& default_value) const { return Base::is_ok ? Base::value : default_value; }

    /**
     * @brief bool への暗黙変換（成功かどうか）
     * @return 成功の場合true
     */
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return Base::is_ok; }

    /**
     * @brief 間接参照演算子（値へのアクセス）
     * @return 成功時の値への参照
     * @warning エラー状態の場合はアサーション失敗
     */
    [[nodiscard]] constexpr T& operator*() {
        assert(Base::is_ok);
        return Base::value;
    }

    /**
     * @brief 間接参照演算子（値へのアクセス、const）
     * @return 成功時の値へのconst参照
     * @warning エラー状態の場合はアサーション失敗
     */
    [[nodiscard]] constexpr const T& operator*() const {
        assert(Base::is_ok);
        return Base::value;
    }

    /**
     * @brief ポインタ風アクセス演算子
     * @return 成功時の値へのポインタ
     * @warning エラー状態の場合はアサーション失敗
     */
    [[nodiscard]] constexpr T* operator->() {
        assert(Base::is_ok);
        return &(static_cast<Base*>(this)->value);
    }

    /**
     * @brief ポインタ風アクセス演算子（const）
     * @return 成功時の値へのconstポインタ
     * @warning エラー状態の場合はアサーション失敗
     */
    [[nodiscard]] constexpr const T* operator->() const {
        assert(Base::is_ok);
        return &(static_cast<const Base*>(this)->value);
    }
};

} // namespace omusubi
