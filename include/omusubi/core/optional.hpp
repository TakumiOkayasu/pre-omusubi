#pragma once

#include <cassert>
#include <cstdint>
#include <new>

namespace omusubi {

/**
 * @brief std::optional<T> 互換の Optional<T> 実装
 *
 * C++14 環境で std::optional の機能を提供します。
 * ヒープアロケーションを行わず、スタック上に値を配置します。
 *
 * @tparam T 保持する値の型
 *
 * @note C++17 以降では std::optional を使用することを推奨
 */
template <typename T>
class Optional {
private:
    alignas(T) uint8_t storage_[sizeof(T)];
    bool has_value_;

    T* ptr() { return reinterpret_cast<T*>(storage_); }

    const T* ptr() const { return reinterpret_cast<const T*>(storage_); }

public:
    /**
     * @brief デフォルトコンストラクタ（値なし）
     */
    constexpr Optional() noexcept : storage_ {}, has_value_(false) {}

    /**
     * @brief 値を持つコンストラクタ
     * @param value 保持する値
     */
    constexpr Optional(const T& value) noexcept : has_value_(true) { new (storage_) T(value); }

    /**
     * @brief ムーブコンストラクタ
     * @param value 保持する値（ムーブ）
     */
    constexpr Optional(T&& value) noexcept : has_value_(true) { new (storage_) T(static_cast<T&&>(value)); }

    /**
     * @brief コピーコンストラクタ
     * @param other コピー元
     */
    Optional(const Optional& other) noexcept : has_value_(other.has_value_) {
        if (has_value_) {
            new (storage_) T(*other.ptr());
        }
    }

    /**
     * @brief ムーブコンストラクタ
     * @param other ムーブ元
     */
    Optional(Optional&& other) noexcept : has_value_(other.has_value_) {
        if (has_value_) {
            new (storage_) T(static_cast<T&&>(*other.ptr()));
            other.reset();
        }
    }

    /**
     * @brief デストラクタ
     */
    ~Optional() {
        if (has_value_) {
            ptr()->~T();
        }
    }

    /**
     * @brief コピー代入演算子
     * @param other コピー元
     * @return *this
     */
    Optional& operator=(const Optional& other) noexcept {
        if (this != &other) {
            if (has_value_) {
                ptr()->~T();
            }
            has_value_ = other.has_value_;
            if (has_value_) {
                new (storage_) T(*other.ptr());
            }
        }
        return *this;
    }

    /**
     * @brief ムーブ代入演算子
     * @param other ムーブ元
     * @return *this
     */
    Optional& operator=(Optional&& other) noexcept {
        if (this != &other) {
            if (has_value_) {
                ptr()->~T();
            }
            has_value_ = other.has_value_;
            if (has_value_) {
                new (storage_) T(static_cast<T&&>(*other.ptr()));
                other.reset();
            }
        }
        return *this;
    }

    /**
     * @brief 値を持つかどうかを判定
     * @return 値を持つ場合true
     */
    constexpr bool has_value() const noexcept { return has_value_; }

    /**
     * @brief bool への暗黙変換
     * @return 値を持つ場合true
     */
    constexpr explicit operator bool() const noexcept { return has_value_; }

    /**
     * @brief 値を取得（参照）
     * @return 保持している値への参照
     * @warning 値を持たない場合はアサーション失敗
     */
    constexpr T& value() {
        assert(has_value_);
        return *ptr();
    }

    /**
     * @brief 値を取得（const参照）
     * @return 保持している値へのconst参照
     * @warning 値を持たない場合はアサーション失敗
     */
    constexpr const T& value() const {
        assert(has_value_);
        return *ptr();
    }

    /**
     * @brief 値を取得、値がない場合はデフォルト値を返す
     * @param default_value デフォルト値
     * @return 保持している値、または default_value
     */
    constexpr T value_or(const T& default_value) const { return has_value_ ? *ptr() : default_value; }

    /**
     * @brief ポインタ風アクセス演算子
     * @return 保持している値へのポインタ
     * @warning 値を持たない場合はアサーション失敗
     */
    constexpr T* operator->() {
        assert(has_value_);
        return ptr();
    }

    /**
     * @brief ポインタ風アクセス演算子（const）
     * @return 保持している値へのconstポインタ
     * @warning 値を持たない場合はアサーション失敗
     */
    constexpr const T* operator->() const {
        assert(has_value_);
        return ptr();
    }

    /**
     * @brief 間接参照演算子
     * @return 保持している値への参照
     * @warning 値を持たない場合はアサーション失敗
     */
    constexpr T& operator*() {
        assert(has_value_);
        return *ptr();
    }

    /**
     * @brief 間接参照演算子（const）
     * @return 保持している値へのconst参照
     * @warning 値を持たない場合はアサーション失敗
     */
    constexpr const T& operator*() const {
        assert(has_value_);
        return *ptr();
    }

    /**
     * @brief 値をリセット（値なし状態にする）
     */
    void reset() noexcept {
        if (has_value_) {
            ptr()->~T();
            has_value_ = false;
        }
    }

    /**
     * @brief 新しい値を設定
     * @param value 設定する値
     */
    void emplace(const T& value) noexcept {
        reset();
        new (storage_) T(value);
        has_value_ = true;
    }

    /**
     * @brief 新しい値を設定（ムーブ）
     * @param value 設定する値（ムーブ）
     */
    void emplace(T&& value) noexcept {
        reset();
        new (storage_) T(static_cast<T&&>(value));
        has_value_ = true;
    }
};

/**
 * @brief 等価比較演算子
 */
template <typename T>
constexpr bool operator==(const Optional<T>& lhs, const Optional<T>& rhs) {
    if (lhs.has_value() != rhs.has_value()) {
        return false;
    }
    if (!lhs.has_value()) {
        return true; // 両方とも値なし
    }
    return lhs.value() == rhs.value();
}

/**
 * @brief 非等価比較演算子
 */
template <typename T>
constexpr bool operator!=(const Optional<T>& lhs, const Optional<T>& rhs) {
    return !(lhs == rhs);
}

/**
 * @brief 値との等価比較演算子
 */
template <typename T>
constexpr bool operator==(const Optional<T>& opt, const T& value) {
    return opt.has_value() && opt.value() == value;
}

/**
 * @brief 値との非等価比較演算子
 */
template <typename T>
constexpr bool operator!=(const Optional<T>& opt, const T& value) {
    return !(opt == value);
}

} // namespace omusubi
