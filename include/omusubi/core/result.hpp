#pragma once

#include <cassert>
#include <cstdint>
#include <new>

namespace omusubi {

// Forward declaration
enum class Error : uint8_t;

/**
 * @brief Rust風の Result<T, E> 実装
 *
 * 成功時の値とエラー時のエラー情報を1つの型で表現します。
 * ヒープアロケーションを行わず、スタック上に値またはエラーを配置します。
 *
 * @tparam T 成功時の値の型
 * @tparam E エラー情報の型（デフォルトは Error）
 *
 * @note union を使用するため、T と E は同時に存在しません
 */
template <typename T, typename E = Error>
class Result {
private:
    union Storage {
        T value;
        E error;

        Storage() noexcept {}

        ~Storage() {}
    };

    Storage storage_;
    bool is_ok_;

    // プライベートコンストラクタ
    Result() noexcept : storage_ {}, is_ok_(false) {}

public:
    /**
     * @brief 成功を表す Result を作成
     * @param value 成功時の値
     * @return 成功を表す Result
     */
    static Result ok(const T& value) noexcept {
        Result r;
        r.is_ok_ = true;
        new (&r.storage_.value) T(value);
        return r;
    }

    /**
     * @brief 成功を表す Result を作成（ムーブ）
     * @param value 成功時の値（ムーブ）
     * @return 成功を表す Result
     */
    static Result ok(T&& value) noexcept {
        Result r;
        r.is_ok_ = true;
        new (&r.storage_.value) T(static_cast<T&&>(value));
        return r;
    }

    /**
     * @brief エラーを表す Result を作成
     * @param error エラー情報
     * @return エラーを表す Result
     */
    static Result err(const E& error) noexcept {
        Result r;
        r.is_ok_ = false;
        new (&r.storage_.error) E(error);
        return r;
    }

    /**
     * @brief エラーを表す Result を作成（ムーブ）
     * @param error エラー情報（ムーブ）
     * @return エラーを表す Result
     */
    static Result err(E&& error) noexcept {
        Result r;
        r.is_ok_ = false;
        new (&r.storage_.error) E(static_cast<E&&>(error));
        return r;
    }

    /**
     * @brief コピーコンストラクタ
     * @param other コピー元
     */
    Result(const Result& other) noexcept : is_ok_(other.is_ok_) {
        if (is_ok_) {
            new (&storage_.value) T(other.storage_.value);
        } else {
            new (&storage_.error) E(other.storage_.error);
        }
    }

    /**
     * @brief ムーブコンストラクタ
     * @param other ムーブ元
     */
    Result(Result&& other) noexcept : is_ok_(other.is_ok_) {
        if (is_ok_) {
            new (&storage_.value) T(static_cast<T&&>(other.storage_.value));
        } else {
            new (&storage_.error) E(static_cast<E&&>(other.storage_.error));
        }
    }

    /**
     * @brief デストラクタ
     */
    ~Result() {
        if (is_ok_) {
            storage_.value.~T();
        } else {
            storage_.error.~E();
        }
    }

    /**
     * @brief コピー代入演算子
     * @param other コピー元
     * @return *this
     */
    Result& operator=(const Result& other) noexcept {
        if (this != &other) {
            // 既存の値を破棄
            if (is_ok_) {
                storage_.value.~T();
            } else {
                storage_.error.~E();
            }

            // 新しい値をコピー
            is_ok_ = other.is_ok_;
            if (is_ok_) {
                new (&storage_.value) T(other.storage_.value);
            } else {
                new (&storage_.error) E(other.storage_.error);
            }
        }
        return *this;
    }

    /**
     * @brief ムーブ代入演算子
     * @param other ムーブ元
     * @return *this
     */
    Result& operator=(Result&& other) noexcept {
        if (this != &other) {
            // 既存の値を破棄
            if (is_ok_) {
                storage_.value.~T();
            } else {
                storage_.error.~E();
            }

            // 新しい値をムーブ
            is_ok_ = other.is_ok_;
            if (is_ok_) {
                new (&storage_.value) T(static_cast<T&&>(other.storage_.value));
            } else {
                new (&storage_.error) E(static_cast<E&&>(other.storage_.error));
            }
        }
        return *this;
    }

    /**
     * @brief 成功かどうかを判定
     * @return 成功の場合true
     */
    constexpr bool is_ok() const noexcept { return is_ok_; }

    /**
     * @brief エラーかどうかを判定
     * @return エラーの場合true
     */
    constexpr bool is_err() const noexcept { return !is_ok_; }

    /**
     * @brief 成功時の値を取得（参照）
     * @return 成功時の値への参照
     * @warning エラー状態の場合はアサーション失敗
     */
    constexpr T& value() {
        assert(is_ok_);
        return storage_.value;
    }

    /**
     * @brief 成功時の値を取得（const参照）
     * @return 成功時の値へのconst参照
     * @warning エラー状態の場合はアサーション失敗
     */
    constexpr const T& value() const {
        assert(is_ok_);
        return storage_.value;
    }

    /**
     * @brief エラー情報を取得（参照）
     * @return エラー情報への参照
     * @warning 成功状態の場合はアサーション失敗
     */
    constexpr E& error() {
        assert(!is_ok_);
        return storage_.error;
    }

    /**
     * @brief エラー情報を取得（const参照）
     * @return エラー情報へのconst参照
     * @warning 成功状態の場合はアサーション失敗
     */
    constexpr const E& error() const {
        assert(!is_ok_);
        return storage_.error;
    }

    /**
     * @brief 値を取得、エラーの場合はデフォルト値を返す
     * @param default_value デフォルト値
     * @return 成功時の値、またはdefault_value
     */
    constexpr T value_or(const T& default_value) const { return is_ok_ ? storage_.value : default_value; }

    /**
     * @brief bool への暗黙変換（成功かどうか）
     * @return 成功の場合true
     */
    constexpr explicit operator bool() const noexcept { return is_ok_; }

    /**
     * @brief 間接参照演算子（値へのアクセス）
     * @return 成功時の値への参照
     * @warning エラー状態の場合はアサーション失敗
     */
    constexpr T& operator*() {
        assert(is_ok_);
        return storage_.value;
    }

    /**
     * @brief 間接参照演算子（値へのアクセス、const）
     * @return 成功時の値へのconst参照
     * @warning エラー状態の場合はアサーション失敗
     */
    constexpr const T& operator*() const {
        assert(is_ok_);
        return storage_.value;
    }

    /**
     * @brief ポインタ風アクセス演算子
     * @return 成功時の値へのポインタ
     * @warning エラー状態の場合はアサーション失敗
     */
    constexpr T* operator->() {
        assert(is_ok_);
        return &storage_.value;
    }

    /**
     * @brief ポインタ風アクセス演算子（const）
     * @return 成功時の値へのconstポインタ
     * @warning エラー状態の場合はアサーション失敗
     */
    constexpr const T* operator->() const {
        assert(is_ok_);
        return &storage_.value;
    }
};

} // namespace omusubi
