#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <new>
#include <omusubi/core/span.hpp>
#include <type_traits>

namespace omusubi {

/**
 * @brief 固定容量の可変長配列（std::vectorのヒープレス版）
 *
 * 動的メモリ確保なしで可変長配列を実現。
 * 要素数はCapacityまで動的に増減可能。
 *
 * @tparam T 要素の型
 * @tparam Capacity 最大要素数
 */
template <typename T, size_t Capacity>
class StaticVector {
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;

private:
    // アライメントを保証したストレージ
    alignas(T) unsigned char storage_[sizeof(T) * Capacity];
    size_type size_;

    // ストレージへのポインタ取得
    [[nodiscard]] pointer storage_ptr() noexcept { return reinterpret_cast<pointer>(storage_); }

    [[nodiscard]] const_pointer storage_ptr() const noexcept { return reinterpret_cast<const_pointer>(storage_); }

public:
    /**
     * @brief デフォルトコンストラクタ
     */
    constexpr StaticVector() noexcept : storage_ {}, size_(0) {}

    /**
     * @brief 初期化リストからの構築
     */
    StaticVector(std::initializer_list<T> init) noexcept : storage_ {}, size_(0) {
        for (const auto& item : init) {
            if (size_ >= Capacity) {
                break;
            }
            push_back(item);
        }
    }

    /**
     * @brief コピーコンストラクタ
     */
    StaticVector(const StaticVector& other) noexcept : storage_ {}, size_(0) {
        for (size_type i = 0; i < other.size_; ++i) {
            push_back(other[i]);
        }
    }

    /**
     * @brief ムーブコンストラクタ
     */
    StaticVector(StaticVector&& other) noexcept : storage_ {}, size_(0) {
        for (size_type i = 0; i < other.size_; ++i) {
            push_back(static_cast<T&&>(other[i]));
        }
        other.clear();
    }

    /**
     * @brief コピー代入演算子
     */
    StaticVector& operator=(const StaticVector& other) noexcept {
        if (this != &other) {
            clear();
            for (size_type i = 0; i < other.size_; ++i) {
                push_back(other[i]);
            }
        }
        return *this;
    }

    /**
     * @brief ムーブ代入演算子
     */
    StaticVector& operator=(StaticVector&& other) noexcept {
        if (this != &other) {
            clear();
            for (size_type i = 0; i < other.size_; ++i) {
                push_back(static_cast<T&&>(other[i]));
            }
            other.clear();
        }
        return *this;
    }

    /**
     * @brief デストラクタ
     */
    ~StaticVector() noexcept { clear(); }

    // === 容量関連 ===

    /**
     * @brief 現在の要素数を取得
     */
    [[nodiscard]] constexpr size_type size() const noexcept { return size_; }

    /**
     * @brief 最大容量を取得
     */
    [[nodiscard]] constexpr size_type capacity() const noexcept { return Capacity; }

    /**
     * @brief 空か判定
     */
    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }

    /**
     * @brief 満杯か判定
     */
    [[nodiscard]] constexpr bool full() const noexcept { return size_ >= Capacity; }

    /**
     * @brief 残り容量を取得
     */
    [[nodiscard]] constexpr size_type remaining() const noexcept { return Capacity - size_; }

    // === 要素アクセス ===

    /**
     * @brief インデックスアクセス（境界チェックなし）
     */
    [[nodiscard]] reference operator[](size_type index) noexcept { return storage_ptr()[index]; }

    /**
     * @brief インデックスアクセス（境界チェックなし、const版）
     */
    [[nodiscard]] const_reference operator[](size_type index) const noexcept { return storage_ptr()[index]; }

    /**
     * @brief 先頭要素への参照
     */
    [[nodiscard]] reference front() noexcept { return storage_ptr()[0]; }

    /**
     * @brief 先頭要素への参照（const版）
     */
    [[nodiscard]] const_reference front() const noexcept { return storage_ptr()[0]; }

    /**
     * @brief 末尾要素への参照
     */
    [[nodiscard]] reference back() noexcept { return storage_ptr()[size_ - 1]; }

    /**
     * @brief 末尾要素への参照（const版）
     */
    [[nodiscard]] const_reference back() const noexcept { return storage_ptr()[size_ - 1]; }

    /**
     * @brief データへのポインタを取得
     */
    [[nodiscard]] pointer data() noexcept { return storage_ptr(); }

    /**
     * @brief データへのポインタを取得（const版）
     */
    [[nodiscard]] const_pointer data() const noexcept { return storage_ptr(); }

    // === イテレータ ===

    [[nodiscard]] iterator begin() noexcept { return storage_ptr(); }

    [[nodiscard]] const_iterator begin() const noexcept { return storage_ptr(); }

    [[nodiscard]] const_iterator cbegin() const noexcept { return storage_ptr(); }

    [[nodiscard]] iterator end() noexcept { return storage_ptr() + size_; }

    [[nodiscard]] const_iterator end() const noexcept { return storage_ptr() + size_; }

    [[nodiscard]] const_iterator cend() const noexcept { return storage_ptr() + size_; }

    // === 変更操作 ===

    /**
     * @brief 末尾に要素を追加
     * @return 成功時true、容量不足時false
     */
    bool push_back(const T& value) noexcept {
        if (size_ >= Capacity) {
            return false;
        }
        new (storage_ptr() + size_) T(value);
        ++size_;
        return true;
    }

    /**
     * @brief 末尾に要素を追加（ムーブ版）
     * @return 成功時true、容量不足時false
     */
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved) - プロジェクト方針によりstd::move未使用
    bool push_back(T&& value) noexcept {
        if (size_ >= Capacity) {
            return false;
        }
        new (storage_ptr() + size_) T(static_cast<T&&>(value));
        ++size_;
        return true;
    }

    /**
     * @brief 末尾に要素を直接構築
     * @return 成功時true、容量不足時false
     */
    template <typename... Args>
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward) - プロジェクト方針によりstd::forward未使用
    bool emplace_back(Args&&... args) noexcept {
        if (size_ >= Capacity) {
            return false;
        }
        new (storage_ptr() + size_) T(static_cast<Args&&>(args)...);
        ++size_;
        return true;
    }

    /**
     * @brief 末尾の要素を削除
     */
    void pop_back() noexcept {
        if (size_ > 0) {
            --size_;
            storage_ptr()[size_].~T();
        }
    }

    /**
     * @brief 全要素をクリア
     */
    void clear() noexcept {
        for (size_type i = 0; i < size_; ++i) {
            storage_ptr()[i].~T();
        }
        size_ = 0;
    }

    /**
     * @brief 指定位置の要素を削除（順序維持）
     * @return 成功時true、範囲外時false
     */
    bool erase(size_type index) noexcept {
        if (index >= size_) {
            return false;
        }
        // 要素をシフト
        for (size_type i = index; i < size_ - 1; ++i) {
            storage_ptr()[i] = static_cast<T&&>(storage_ptr()[i + 1]);
        }
        pop_back();
        return true;
    }

    /**
     * @brief 指定位置の要素を削除（順序不維持、高速）
     * @return 成功時true、範囲外時false
     */
    bool erase_unordered(size_type index) noexcept {
        if (index >= size_) {
            return false;
        }
        if (index != size_ - 1) {
            storage_ptr()[index] = static_cast<T&&>(storage_ptr()[size_ - 1]);
        }
        pop_back();
        return true;
    }

    /**
     * @brief サイズを変更
     * @param new_size 新しいサイズ
     * @return 成功時true、容量超過時false
     */
    bool resize(size_type new_size) noexcept {
        if (new_size > Capacity) {
            return false;
        }
        // 縮小の場合、余分な要素を破棄
        while (size_ > new_size) {
            pop_back();
        }
        // 拡大の場合、デフォルト構築
        while (size_ < new_size) {
            if (!emplace_back()) {
                return false;
            }
        }
        return true;
    }

    // === span変換 ===

    /**
     * @brief spanとして取得
     */
    [[nodiscard]] span<T> as_span() noexcept { return span<T>(storage_ptr(), size_); }

    /**
     * @brief spanとして取得（const版）
     */
    [[nodiscard]] span<const T> as_span() const noexcept { return span<const T>(storage_ptr(), size_); }

    // === 比較演算子 ===

    [[nodiscard]] bool operator==(const StaticVector& other) const noexcept {
        if (size_ != other.size_) {
            return false;
        }
        for (size_type i = 0; i < size_; ++i) {
            if (!(storage_ptr()[i] == other.storage_ptr()[i])) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool operator!=(const StaticVector& other) const noexcept { return !(*this == other); }
};

} // namespace omusubi
