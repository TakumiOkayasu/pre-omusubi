#pragma once

#include <cstddef>
#include <cstdint>

namespace omusubi {

/**
 * @brief 非所有メモリビュー（C++20 std::spanの簡易版）
 *
 * データを所有せず参照のみ。参照先の生存期間に注意。
 */
template <typename T>
class span {
public:
    using element_type = T;
    using value_type = T;
    using size_type = size_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = T*;
    using const_iterator = const T*;

private:
    T* data_;
    size_type size_;

public:
    constexpr span() noexcept : data_(nullptr), size_(0) {}

    constexpr span(T* data, size_type size) noexcept : data_(data), size_(size) {}

    template <size_t N>
    constexpr span(T (&arr)[N]) noexcept : data_(arr), size_(N) {}

    constexpr span(const span&) noexcept = default;
    constexpr span& operator=(const span&) noexcept = default;
    ~span() noexcept = default;
    constexpr span(span&&) noexcept = default;
    constexpr span& operator=(span&&) noexcept = default;

    constexpr pointer data() const noexcept { return data_; }

    constexpr size_type size() const noexcept { return size_; }

    constexpr size_type size_bytes() const noexcept { return size_ * sizeof(T); }

    constexpr bool empty() const noexcept { return size_ == 0; }

    constexpr reference operator[](size_type index) const noexcept { return data_[index]; }

    constexpr reference front() const noexcept { return data_[0]; }

    constexpr reference back() const noexcept { return data_[size_ - 1]; }

    constexpr iterator begin() const noexcept { return data_; }

    constexpr iterator end() const noexcept { return data_ + size_; }

    constexpr const_iterator cbegin() const noexcept { return data_; }

    constexpr const_iterator cend() const noexcept { return data_ + size_; }

    constexpr span<T> subspan(size_type offset, size_type count) const noexcept { return span<T>(data_ + offset, count); }

    constexpr span<T> first(size_type count) const noexcept { return span<T>(data_, count); }

    constexpr span<T> last(size_type count) const noexcept { return span<T>(data_ + size_ - count, count); }
};

constexpr span<uint8_t> as_bytes(span<char> s) noexcept {
    // NOLINT(bugprone-casting-through-void)
    return {static_cast<uint8_t*>(static_cast<void*>(s.data())), s.size()};
}

constexpr span<const uint8_t> as_bytes(span<const char> s) noexcept {
    // NOLINT(bugprone-casting-through-void)
    return {static_cast<const uint8_t*>(static_cast<const void*>(s.data())), s.size()};
}

constexpr span<char> as_chars(span<uint8_t> s) noexcept {
    // NOLINT(bugprone-casting-through-void)
    return {static_cast<char*>(static_cast<void*>(s.data())), s.size()};
}

constexpr span<const char> as_chars(span<const uint8_t> s) noexcept {
    // NOLINT(bugprone-casting-through-void)
    return {static_cast<const char*>(static_cast<const void*>(s.data())), s.size()};
}

} // namespace omusubi
