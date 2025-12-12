#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <new>
#include <omusubi/core/span.hpp>
#include <type_traits>

namespace omusubi {

/**
 * @brief 固定長リングバッファ（FIFO）
 *
 * 動的メモリ確保なしのFIFOバッファ。
 * センサーデータの蓄積、シリアル通信バッファ等に使用。
 * 満杯時の push は最古の要素を上書き。
 *
 * @tparam T 要素の型
 * @tparam Capacity 最大要素数
 */
template <typename T, size_t Capacity>
class RingBuffer {
public:
    using value_type = T;
    using size_type = size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

private:
    // アライメントを保証したストレージ
    alignas(T) unsigned char storage_[sizeof(T) * Capacity];
    size_type head_; // 次に読み出す位置
    size_type tail_; // 次に書き込む位置
    size_type size_; // 現在の要素数

    // ストレージへのポインタ取得
    [[nodiscard]] pointer storage_ptr() noexcept { return reinterpret_cast<pointer>(storage_); }

    [[nodiscard]] const_pointer storage_ptr() const noexcept { return reinterpret_cast<const_pointer>(storage_); }

    // インデックスをラップ
    [[nodiscard]] static constexpr size_type wrap(size_type index) noexcept { return index % Capacity; }

public:
    /**
     * @brief デフォルトコンストラクタ
     */
    constexpr RingBuffer() noexcept : storage_ {}, head_(0), tail_(0), size_(0) {}

    /**
     * @brief デストラクタ
     */
    ~RingBuffer() noexcept { clear(); }

    // コピー・ムーブは複雑になるため禁止
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;

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
     * @brief 先頭要素への参照（最古の要素）
     */
    [[nodiscard]] reference front() noexcept { return storage_ptr()[head_]; }

    /**
     * @brief 先頭要素への参照（const版）
     */
    [[nodiscard]] const_reference front() const noexcept { return storage_ptr()[head_]; }

    /**
     * @brief 末尾要素への参照（最新の要素）
     */
    [[nodiscard]] reference back() noexcept {
        size_type index = (tail_ == 0) ? Capacity - 1 : tail_ - 1;
        return storage_ptr()[index];
    }

    /**
     * @brief 末尾要素への参照（const版）
     */
    [[nodiscard]] const_reference back() const noexcept {
        size_type index = (tail_ == 0) ? Capacity - 1 : tail_ - 1;
        return storage_ptr()[index];
    }

    /**
     * @brief インデックスアクセス（0が最古、size()-1が最新）
     */
    [[nodiscard]] reference operator[](size_type index) noexcept { return storage_ptr()[wrap(head_ + index)]; }

    /**
     * @brief インデックスアクセス（const版）
     */
    [[nodiscard]] const_reference operator[](size_type index) const noexcept { return storage_ptr()[wrap(head_ + index)]; }

    // === 変更操作 ===

    /**
     * @brief 末尾に要素を追加
     *
     * 満杯の場合は最古の要素を上書きする。
     */
    void push(const T& value) noexcept {
        if (full()) {
            // 最古の要素を破棄
            storage_ptr()[head_].~T();
            head_ = wrap(head_ + 1);
            --size_;
        }
        new (storage_ptr() + tail_) T(value);
        tail_ = wrap(tail_ + 1);
        ++size_;
    }

    /**
     * @brief 末尾に要素を追加（ムーブ版）
     */
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved) - プロジェクト方針によりstd::move未使用
    void push(T&& value) noexcept {
        if (full()) {
            // 最古の要素を破棄
            storage_ptr()[head_].~T();
            head_ = wrap(head_ + 1);
            --size_;
        }
        new (storage_ptr() + tail_) T(static_cast<T&&>(value));
        tail_ = wrap(tail_ + 1);
        ++size_;
    }

    /**
     * @brief 末尾に要素を追加（上書きなし）
     * @return 成功時true、満杯時false
     */
    bool try_push(const T& value) noexcept {
        if (full()) {
            return false;
        }
        new (storage_ptr() + tail_) T(value);
        tail_ = wrap(tail_ + 1);
        ++size_;
        return true;
    }

    /**
     * @brief 末尾に要素を追加（上書きなし、ムーブ版）
     * @return 成功時true、満杯時false
     */
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved) - プロジェクト方針によりstd::move未使用
    bool try_push(T&& value) noexcept {
        if (full()) {
            return false;
        }
        new (storage_ptr() + tail_) T(static_cast<T&&>(value));
        tail_ = wrap(tail_ + 1);
        ++size_;
        return true;
    }

    /**
     * @brief 末尾に要素を直接構築
     *
     * 満杯の場合は最古の要素を上書きする。
     */
    template <typename... Args>
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward) - プロジェクト方針によりstd::forward未使用
    void emplace(Args&&... args) noexcept {
        if (full()) {
            // 最古の要素を破棄
            storage_ptr()[head_].~T();
            head_ = wrap(head_ + 1);
            --size_;
        }
        new (storage_ptr() + tail_) T(static_cast<Args&&>(args)...);
        tail_ = wrap(tail_ + 1);
        ++size_;
    }

    /**
     * @brief 先頭要素を削除（最古の要素）
     */
    void pop() noexcept {
        if (!empty()) {
            storage_ptr()[head_].~T();
            head_ = wrap(head_ + 1);
            --size_;
        }
    }

    /**
     * @brief 先頭要素を取得して削除
     * @param out 取得した値の格納先
     * @return 成功時true、空の場合false
     */
    bool pop(T& out) noexcept {
        if (empty()) {
            return false;
        }
        out = static_cast<T&&>(storage_ptr()[head_]);
        storage_ptr()[head_].~T();
        head_ = wrap(head_ + 1);
        --size_;
        return true;
    }

    /**
     * @brief 全要素をクリア
     */
    void clear() noexcept {
        while (!empty()) {
            pop();
        }
        head_ = 0;
        tail_ = 0;
    }

    // === イテレータ ===

    /**
     * @brief リングバッファ用イテレータ
     */
    class iterator {
    public:
        using difference_type = ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::forward_iterator_tag;

    private:
        RingBuffer* buffer_;
        size_type index_;

    public:
        iterator(RingBuffer* buffer, size_type index) noexcept : buffer_(buffer), index_(index) {}

        [[nodiscard]] reference operator*() const noexcept { return (*buffer_)[index_]; }

        [[nodiscard]] pointer operator->() const noexcept { return &(*buffer_)[index_]; }

        iterator& operator++() noexcept {
            ++index_;
            return *this;
        }

        iterator operator++(int) noexcept {
            iterator tmp = *this;
            ++index_;
            return tmp;
        }

        [[nodiscard]] bool operator==(const iterator& other) const noexcept { return index_ == other.index_; }

        [[nodiscard]] bool operator!=(const iterator& other) const noexcept { return index_ != other.index_; }
    };

    /**
     * @brief const イテレータ
     */
    class const_iterator {
    public:
        using difference_type = ptrdiff_t;
        using value_type = T;
        using pointer = const T*;
        using reference = const T&;
        using iterator_category = std::forward_iterator_tag;

    private:
        const RingBuffer* buffer_;
        size_type index_;

    public:
        const_iterator(const RingBuffer* buffer, size_type index) noexcept : buffer_(buffer), index_(index) {}

        [[nodiscard]] reference operator*() const noexcept { return (*buffer_)[index_]; }

        [[nodiscard]] pointer operator->() const noexcept { return &(*buffer_)[index_]; }

        const_iterator& operator++() noexcept {
            ++index_;
            return *this;
        }

        const_iterator operator++(int) noexcept {
            const_iterator tmp = *this;
            ++index_;
            return tmp;
        }

        [[nodiscard]] bool operator==(const const_iterator& other) const noexcept { return index_ == other.index_; }

        [[nodiscard]] bool operator!=(const const_iterator& other) const noexcept { return index_ != other.index_; }
    };

    [[nodiscard]] iterator begin() noexcept { return iterator(this, 0); }

    [[nodiscard]] iterator end() noexcept { return iterator(this, size_); }

    [[nodiscard]] const_iterator begin() const noexcept { return const_iterator(this, 0); }

    [[nodiscard]] const_iterator end() const noexcept { return const_iterator(this, size_); }

    [[nodiscard]] const_iterator cbegin() const noexcept { return const_iterator(this, 0); }

    [[nodiscard]] const_iterator cend() const noexcept { return const_iterator(this, size_); }
};

} // namespace omusubi
