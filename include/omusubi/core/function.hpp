#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>

namespace omusubi {

/**
 * @brief 固定サイズの型消去コールバック（std::functionのヒープレス版）
 *
 * 動的メモリ確保なしでCallableオブジェクトを格納。
 * ラムダ、関数ポインタ、ファンクタを統一的に扱える。
 *
 * @tparam Signature 関数シグネチャ（例: void(int), int(float, float)）
 * @tparam StorageSize 内部ストレージのバイトサイズ（デフォルト: 32）
 */
template <typename Signature, size_t StorageSize = 32>
class Function;

// 部分特殊化: R(Args...) 形式のシグネチャ
template <typename R, typename... Args, size_t StorageSize>
class Function<R(Args...), StorageSize> {
public:
    using result_type = R;

private:
    // 内部ストレージ（アライメント保証）
    alignas(alignof(void*)) unsigned char storage_[StorageSize];

    // 関数ポインタ型（invoker と deleter）
    using invoker_type = R (*)(void*, Args...);
    using deleter_type = void (*)(void*);

    invoker_type invoker_;
    deleter_type deleter_;

    // 呼び出しラッパー
    template <typename Callable>
    static R invoke_impl(void* storage, Args... args) {
        return (*static_cast<Callable*>(storage))(static_cast<Args&&>(args)...);
    }

    // 削除ラッパー
    template <typename Callable>
    static void delete_impl(void* storage) {
        static_cast<Callable*>(storage)->~Callable();
    }

    // 空の deleter
    static void null_deleter(void* /*storage*/) {}

public:
    /**
     * @brief デフォルトコンストラクタ（空の状態）
     */
    constexpr Function() noexcept : storage_ {}, invoker_(nullptr), deleter_(null_deleter) {}

    /**
     * @brief nullptr からの構築（空の状態）
     */
    constexpr Function(std::nullptr_t) noexcept : storage_ {}, invoker_(nullptr), deleter_(null_deleter) {}

    /**
     * @brief Callable オブジェクトからの構築
     *
     * @tparam Callable ラムダ、関数ポインタ、ファンクタなど
     */
    template <typename Callable, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Callable>, Function> && std::is_invocable_r_v<R, Callable, Args...>>>
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved) - プロジェクト方針によりstd::move未使用
    Function(Callable&& callable) noexcept : storage_ {}, invoker_(nullptr), deleter_(null_deleter) {
        using CallableType = std::decay_t<Callable>;

        static_assert(sizeof(CallableType) <= StorageSize, "Callable is too large for Function storage. Increase StorageSize.");
        static_assert(alignof(CallableType) <= alignof(void*), "Callable alignment too strict.");

        new (storage_) CallableType(static_cast<Callable&&>(callable));
        invoker_ = &invoke_impl<CallableType>;
        deleter_ = &delete_impl<CallableType>;
    }

    /**
     * @brief 関数ポインタからの構築
     */
    Function(R (*func_ptr)(Args...)) noexcept : storage_ {}, invoker_(nullptr), deleter_(null_deleter) {
        if (func_ptr != nullptr) {
            // 関数ポインタを直接ストレージに格納
            using FuncPtrType = R (*)(Args...);
            static_assert(sizeof(FuncPtrType) <= StorageSize, "Function pointer too large.");

            new (storage_) FuncPtrType(func_ptr);
            invoker_ = &invoke_impl<FuncPtrType>;
            deleter_ = &delete_impl<FuncPtrType>;
        }
    }

    /**
     * @brief デストラクタ
     */
    ~Function() noexcept { deleter_(storage_); }

    // コピー禁止（キャプチャされたオブジェクトのコピーが複雑なため）
    Function(const Function&) = delete;
    Function& operator=(const Function&) = delete;

    /**
     * @brief ムーブコンストラクタ
     */
    Function(Function&& other) noexcept : storage_ {}, invoker_(other.invoker_), deleter_(other.deleter_) {
        // バイト単位でコピー（PODとして扱う）
        for (size_t i = 0; i < StorageSize; ++i) {
            storage_[i] = other.storage_[i];
        }
        other.invoker_ = nullptr;
        other.deleter_ = null_deleter;
    }

    /**
     * @brief ムーブ代入演算子
     */
    Function& operator=(Function&& other) noexcept {
        if (this != &other) {
            deleter_(storage_);

            for (size_t i = 0; i < StorageSize; ++i) {
                storage_[i] = other.storage_[i];
            }
            invoker_ = other.invoker_;
            deleter_ = other.deleter_;

            other.invoker_ = nullptr;
            other.deleter_ = null_deleter;
        }
        return *this;
    }

    /**
     * @brief nullptr 代入（クリア）
     */
    Function& operator=(std::nullptr_t) noexcept {
        deleter_(storage_);
        invoker_ = nullptr;
        deleter_ = null_deleter;
        return *this;
    }

    /**
     * @brief 関数呼び出し
     */
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward) - プロジェクト方針によりstd::forward未使用
    R operator()(Args... args) const {
        // 空の場合の呼び出しは未定義動作（軽量化のためチェックなし）
        return invoker_(const_cast<unsigned char*>(storage_), static_cast<Args&&>(args)...);
    }

    /**
     * @brief 有効な関数が格納されているか判定
     */
    [[nodiscard]] explicit operator bool() const noexcept { return invoker_ != nullptr; }

    /**
     * @brief 空か判定
     */
    [[nodiscard]] bool empty() const noexcept { return invoker_ == nullptr; }

    /**
     * @brief クリア
     */
    void clear() noexcept {
        deleter_(storage_);
        invoker_ = nullptr;
        deleter_ = null_deleter;
    }

    /**
     * @brief ストレージサイズを取得
     */
    [[nodiscard]] static constexpr size_t storage_size() noexcept { return StorageSize; }
};

// nullptr との比較演算子
template <typename Sig, size_t Size>
[[nodiscard]] bool operator==(const Function<Sig, Size>& f, std::nullptr_t) noexcept {
    return f.empty();
}

template <typename Sig, size_t Size>
[[nodiscard]] bool operator==(std::nullptr_t, const Function<Sig, Size>& f) noexcept {
    return f.empty();
}

template <typename Sig, size_t Size>
[[nodiscard]] bool operator!=(const Function<Sig, Size>& f, std::nullptr_t) noexcept {
    return !f.empty();
}

template <typename Sig, size_t Size>
[[nodiscard]] bool operator!=(std::nullptr_t, const Function<Sig, Size>& f) noexcept {
    return !f.empty();
}

} // namespace omusubi
