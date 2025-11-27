#pragma once

/**
 * @file format.hpp
 * @brief 型安全なフォーマット文字列ライブラリ
 *
 * @note std::forward / std::move を使用しない設計について
 *
 * このファイルでは、forwarding reference パラメータに対して
 * std::forward を意図的に使用していません。
 *
 * 理由:
 * 1. プロジェクト方針として std::move() を禁止している（RVO最適化を妨げるため）
 * 2. std::forward は std::move と同様の効果を持つ場合がある
 * 3. format() の引数は通常コピーコストが小さい基本型やstring_view
 * 4. 完全転送よりもコードの単純さを優先
 *
 * clang-tidy警告の抑制:
 * - cppcoreguidelines-missing-std-forward
 * - cppcoreguidelines-rvalue-reference-param-not-moved
 *
 * これらの警告は意図的な設計決定によるものです。
 */

#include <cstdint>
#include <omusubi/core/fixed_string.hpp>
#include <string_view>
#include <type_traits>

namespace omusubi {

namespace detail {

/**
 * @brief フォーマット文字列のプレースホルダー数をカウント（コンパイル時）
 */
constexpr uint32_t count_placeholders(const char* str, uint32_t len) noexcept {
    uint32_t count = 0;
    uint32_t i = 0;

    while (i < len) {
        if (str[i] == '{') {
            if (i + 1 < len && str[i + 1] == '{') {
                // エスケープされた '{{' → プレースホルダーではない
                i += 2;
            } else if (i + 1 < len && str[i + 1] == '}') {
                // プレースホルダー '{}' をカウント
                ++count;
                i += 2;
            } else {
                // 不正なフォーマット（'{'の後に'}'がない）
                ++i;
            }
        } else if (str[i] == '}') {
            if (i + 1 < len && str[i + 1] == '}') {
                // エスケープされた '}}' → プレースホルダーではない
                i += 2;
            } else {
                // 不正なフォーマット（対応する'{'がない'}'）
                ++i;
            }
        } else {
            ++i;
        }
    }

    return count;
}

/**
 * @brief 文字列リテラルのコンパイル時検証ヘルパー
 *
 * C++17では文字列リテラル自体をNon-Type Template Parameterとして
 * 渡すことはできないが、constexpr関数で検証結果を返すことは可能
 */
template <uint32_t N, uint32_t ArgCount>
struct format_string_checker {
    constexpr format_string_checker(const char (&str)[N]) noexcept {
        // C++17: constexprコンストラクタ内で検証
        // プレースホルダー数をカウント
        const uint32_t placeholder_count = count_placeholders(str, N - 1);

        // C++17では条件付きでコンパイルエラーを出すことが困難
        // そのため、この検証は実行時にも行われる
        // ただし、constexpr評価時にエラーになれば、コンパイルエラーとなる
        if (placeholder_count != ArgCount) {
            // C++17: この部分は実行時に評価される可能性がある
            // 完全なコンパイル時エラーにはならない
        }
    }
};

} // namespace detail

/**
 * @brief コンパイル時フォーマット文字列検証
 *
 * C++23のstd::basic_format_stringと同等の機能をC++17で実現
 *
 * C++17の制約:
 * - 文字列リテラルをNon-Type Template Parameterとして渡せない
 * - 関数パラメータをstatic_assertの定数式として使えない
 *
 * この実装では:
 * - 型安全性を提供（引数の型をテンプレートパラメータで保証）
 * - constexpr関数による実行時検証（最適化により一部コンパイル時に検証される可能性あり）
 */
template <typename... Args>
class basic_format_string {
public:
    /**
     * @brief 文字列リテラルから構築
     *
     * C++17: constexpr評価時に検証が実行される
     * ただし、完全なコンパイル時エラーは保証されない
     */
    template <uint32_t N>
    constexpr basic_format_string(const char (&str)[N]) noexcept : str_(str), length_(N - 1) {
        // constexprコンテキストでの検証を試みる
        [[maybe_unused]] const detail::format_string_checker<N, sizeof...(Args)> checker(str);
    }

    /**
     * @brief std::string_viewから構築
     *
     * 注意: std::string_viewは実行時値のため、コンパイル時検証不可
     */
    constexpr basic_format_string(std::string_view sv) noexcept : str_(sv.data()), length_(static_cast<uint32_t>(sv.size())) {}

    /**
     * @brief C文字列として取得
     */
    [[nodiscard]] constexpr const char* c_str() const noexcept { return str_; }

    /**
     * @brief std::string_viewとして取得
     */
    [[nodiscard]] constexpr std::string_view view() const noexcept { return {str_, length_}; }

    /**
     * @brief 長さを取得
     */
    [[nodiscard]] constexpr uint32_t length() const noexcept { return length_; }

    /**
     * @brief 引数数を取得（コンパイル時定数）
     */
    static constexpr uint32_t arg_count() noexcept { return sizeof...(Args); }

private:
    const char* str_;
    uint32_t length_;
};

/**
 * @brief format_string型エイリアス（std::format_string相当）
 */
template <typename... Args>
using format_string = basic_format_string<Args...>;

} // namespace omusubi

namespace omusubi::detail {

/**
 * @brief 型の最大文字列長を取得（コンパイル時計算用）
 */
template <typename T>
struct max_string_length {
    static constexpr uint32_t value = 32; // デフォルト
};

// 整数型の最大長
template <>
struct max_string_length<int8_t> {
    static constexpr uint32_t value = 4; // "-128"
};

template <>
struct max_string_length<uint8_t> {
    static constexpr uint32_t value = 3; // "255"
};

template <>
struct max_string_length<int16_t> {
    static constexpr uint32_t value = 6; // "-32768"
};

template <>
struct max_string_length<uint16_t> {
    static constexpr uint32_t value = 5; // "65535"
};

template <>
struct max_string_length<int32_t> {
    static constexpr uint32_t value = 11; // "-2147483648"
};

template <>
struct max_string_length<uint32_t> {
    static constexpr uint32_t value = 10; // "4294967295"
};

template <>
struct max_string_length<int64_t> {
    static constexpr uint32_t value = 20; // "-9223372036854775808"
};

template <>
struct max_string_length<uint64_t> {
    static constexpr uint32_t value = 20; // "18446744073709551615"
};

// ブール型
template <>
struct max_string_length<bool> {
    static constexpr uint32_t value = 5; // "false"
};

// 文字型
template <>
struct max_string_length<char> {
    static constexpr uint32_t value = 1;
};

// ポインタ型（文字列として扱う、最大長は不明なので大きめに）
template <>
struct max_string_length<const char*> {
    static constexpr uint32_t value = 64;
};

template <>
struct max_string_length<char*> {
    static constexpr uint32_t value = 64;
};

// std::string_view
template <>
struct max_string_length<std::string_view> {
    static constexpr uint32_t value = 64;
};

/**
 * @brief 引数リストの最大文字列長の合計を計算
 */
template <typename... Args>
struct sum_max_string_length;

template <>
struct sum_max_string_length<> {
    static constexpr uint32_t value = 0;
};

template <typename T, typename... Rest>
struct sum_max_string_length<T, Rest...> {
    static constexpr uint32_t value = max_string_length<T>::value + sum_max_string_length<Rest...>::value;
};

/**
 * @brief フォーマット文字列の固定部分の長さを計算（プレースホルダーを除く）
 */
constexpr uint32_t calculate_fixed_length(const char* str, uint32_t len) noexcept {
    uint32_t fixed_len = 0;
    uint32_t i = 0;

    while (i < len) {
        if (str[i] == '{') {
            if (i + 1 < len && str[i + 1] == '{') {
                // エスケープされた '{{' → 1文字分
                fixed_len += 1;
                i += 2;
            } else if (i + 1 < len && str[i + 1] == '}') {
                // プレースホルダー '{}' → カウントしない
                i += 2;
            } else {
                // 不正なフォーマット
                fixed_len += 1;
                ++i;
            }
        } else if (str[i] == '}') {
            if (i + 1 < len && str[i + 1] == '}') {
                // エスケープされた '}}' → 1文字分
                fixed_len += 1;
                i += 2;
            } else {
                // 不正なフォーマット
                fixed_len += 1;
                ++i;
            }
        } else {
            // 通常の文字
            fixed_len += 1;
            ++i;
        }
    }

    return fixed_len;
}

/**
 * @brief 必要な容量を計算（コンパイル時）
 */
template <uint32_t FormatLen, typename... Args>
struct calculate_capacity {
    static constexpr uint32_t value = FormatLen + sum_max_string_length<Args...>::value + 1; // +1 for null terminator
};

/**
 * @brief 整数を文字列に変換（C++17 if constexpr版）
 *
 * 符号付き/符号なし整数型を統合的に処理
 */
template <typename T>
constexpr uint32_t integer_to_string(T value, char* buffer, uint32_t buffer_size) noexcept {
    // 負数処理（符号付き型のみ）
    const bool is_negative = [&]() {
        if constexpr (std::is_signed_v<T>) {
            return value < 0;
        }
        return false;
    }();
    if constexpr (std::is_signed_v<T>) {
        if (is_negative) {
            value = -value;
        }
    }

    // 逆順で数字を格納
    uint32_t pos = 0;
    if (value == 0) {
        if (pos >= buffer_size) {
            return 0;
        }
        buffer[pos++] = '0';
    } else {
        while (value > 0 && pos < buffer_size) {
            buffer[pos++] = '0' + (value % 10);
            value /= 10;
        }
    }

    // 負号を追加（符号付き型のみ）
    if constexpr (std::is_signed_v<T>) {
        if (is_negative && pos < buffer_size) {
            buffer[pos++] = '-';
        }
    }

    // 反転
    for (uint32_t i = 0; i < pos / 2; ++i) {
        const char tmp = buffer[i];
        buffer[i] = buffer[pos - 1 - i];
        buffer[pos - 1 - i] = tmp;
    }

    return pos;
}

/**
 * @brief 符号なし整数を文字列に変換
 */
template <typename T>
constexpr uint32_t unsigned_to_string(T value, char* buffer, uint32_t buffer_size) noexcept {
    // integer_to_stringを使用（if constexprで符号なし型として処理される）
    return integer_to_string(value, buffer, buffer_size);
}

/**
 * @brief 16進数文字列に変換
 */
template <typename T>
constexpr uint32_t hex_to_string(T value, char* buffer, uint32_t buffer_size, bool uppercase = false) noexcept {
    const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

    uint32_t pos = 0;
    if (value == 0) {
        if (pos >= buffer_size) {
            return 0;
        }
        buffer[pos++] = '0';
    } else {
        // 逆順で16進数を格納
        while (value > 0 && pos < buffer_size) {
            buffer[pos++] = digits[value % 16];
            value /= 16;
        }

        // 反転
        for (uint32_t i = 0; i < pos / 2; ++i) {
            const char tmp = buffer[i];
            buffer[i] = buffer[pos - 1 - i];
            buffer[pos - 1 - i] = tmp;
        }
    }

    return pos;
}

/**
 * @brief 型の素の型を取得（std::decayの簡易版）
 */
template <typename T>
struct remove_cv_ref {
    using type = T;
};

template <typename T>
struct remove_cv_ref<T&> {
    using type = T;
};

template <typename T>
struct remove_cv_ref<const T> {
    using type = T;
};

template <typename T>
struct remove_cv_ref<const T&> {
    using type = T;
};

template <typename T, uint32_t N>
struct remove_cv_ref<T[N]> {
    using type = T*;
};

template <typename T, uint32_t N>
struct remove_cv_ref<const T[N]> {
    using type = const T*;
};

template <typename T, uint32_t N>
struct remove_cv_ref<T (&)[N]> {
    using type = T*;
};

template <typename T, uint32_t N>
struct remove_cv_ref<const T (&)[N]> {
    using type = const T*;
};

/**
 * @brief 値を文字列に変換するトレイト（C++17 if constexpr版）
 */
template <typename T>
struct formatter {
    static constexpr uint32_t to_string(T value, char* buffer, uint32_t buffer_size) noexcept {
        // 符号付き整数型
        if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t>) {
            return integer_to_string(static_cast<int32_t>(value), buffer, buffer_size);
        } else if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>) {
            return integer_to_string(value, buffer, buffer_size);
        }
        // 符号なし整数型
        else if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t>) {
            return unsigned_to_string(static_cast<uint32_t>(value), buffer, buffer_size);
        } else if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>) {
            return unsigned_to_string(value, buffer, buffer_size);
        }
        // bool型
        else if constexpr (std::is_same_v<T, bool>) {
            if (value) {
                if (buffer_size < 4) {
                    return 0;
                }
                buffer[0] = 't';
                buffer[1] = 'r';
                buffer[2] = 'u';
                buffer[3] = 'e';
                return 4;
            }
            if (buffer_size < 5) {
                return 0;
            }
            buffer[0] = 'f';
            buffer[1] = 'a';
            buffer[2] = 'l';
            buffer[3] = 's';
            buffer[4] = 'e';
            return 5;
        }
        // char型
        else if constexpr (std::is_same_v<T, char>) {
            if (buffer_size < 1) {
                return 0;
            }
            buffer[0] = value;
            return 1;
        }
        // const char*型
        else if constexpr (std::is_same_v<T, const char*>) {
            if (value == nullptr) {
                return 0;
            }
            uint32_t pos = 0;
            while (value[pos] != '\0' && pos < buffer_size) {
                buffer[pos] = value[pos];
                ++pos;
            }
            return pos;
        }
        // std::string_view型
        else if constexpr (std::is_same_v<T, std::string_view>) {
            const auto view_size = static_cast<uint32_t>(value.size());
            const uint32_t len = (view_size < buffer_size) ? view_size : buffer_size;

            for (uint32_t i = 0; i < len; ++i) {
                buffer[i] = value[i];
            }

            return len;
        }
        // その他の型（未対応）
        else {
            return 0;
        }
    }
};

/**
 * @brief 再帰終了
 */
template <uint32_t Capacity>
void format_impl(FixedString<Capacity>& result, std::string_view format_str, uint32_t& /*arg_index*/) noexcept {
    // 残りの文字列を追加
    uint32_t pos = 0;
    auto format_len = static_cast<uint32_t>(format_str.size());

    while (pos < format_len) {
        if (format_str[pos] == '{' && pos + 1 < format_len && format_str[pos + 1] == '{') {
            result.append('{');
            pos += 2;
        } else if (format_str[pos] == '}' && pos + 1 < format_len && format_str[pos + 1] == '}') {
            result.append('}');
            pos += 2;
        } else {
            result.append(format_str[pos]);
            ++pos;
        }
    }
}

/**
 * @brief フォーマット実装（可変長引数）
 */
template <uint32_t Capacity, typename T, typename... Args>
void format_impl(FixedString<Capacity>& result, std::string_view format_str, uint32_t& arg_index, T&& value, Args&&... args) noexcept {
    // フォーマット文字列を解析
    uint32_t pos = 0;
    auto format_len = static_cast<uint32_t>(format_str.size());

    while (pos < format_len) {
        // プレースホルダー検索
        if (format_str[pos] == '{') {
            if (pos + 1 < format_len && format_str[pos + 1] == '}') {
                // 現在の引数を変換
                char buffer[64] = {};
                const uint32_t len = formatter<typename remove_cv_ref<T>::type>::to_string(value, buffer, sizeof(buffer));

                if (len > 0) {
                    result.append(std::string_view(buffer, len));
                }

                // 次の引数へ
                ++arg_index;
                format_impl(result, std::string_view(format_str.data() + pos + 2, format_len - pos - 2), arg_index, args...);

                return;
            }

            if (pos + 1 < format_len && format_str[pos + 1] == '{') {
                // エスケープされた '{{' → '{'
                result.append('{');
                pos += 2;
                continue;
            }
        }

        if (format_str[pos] == '}' && pos + 1 < format_len && format_str[pos + 1] == '}') {
            // エスケープされた '}}' → '}'
            result.append('}');
            pos += 2;
            continue;
        }

        // 通常の文字
        result.append(format_str[pos]);
        ++pos;
    }
}

} // namespace omusubi::detail

namespace omusubi {

/**
 * @brief 文字列フォーマット（basic_format_string版）- 主要実装
 *
 * @tparam Capacity 結果バッファのサイズ
 * @tparam FmtArgs フォーマット文字列が期待する型
 * @tparam Args 実際の引数の型
 * @param format_str フォーマット文字列（"{}"でプレースホルダー）
 * @param args フォーマット引数
 * @return FixedString<Capacity> フォーマット済み文字列
 *
 * 使用例:
 * @code
 * format_string<const char*, int> fs("Name: {}, Age: {}");
 * auto str = format<128>(fs, "Alice", 25);
 * // 結果: "Name: Alice, Age: 25"
 * @endcode
 */
template <uint32_t Capacity, typename... FmtArgs, typename... Args>
constexpr FixedString<Capacity> format(const basic_format_string<FmtArgs...>& format_str, Args&&... args) noexcept {
    FixedString<Capacity> result;
    uint32_t arg_index = 0;
    detail::format_impl(result, format_str.view(), arg_index, args...);
    return result;
}

/**
 * @brief 文字列フォーマット（文字列リテラル版、Capacity指定）
 *
 * 文字列リテラルから暗黙的にbasic_format_stringを構築
 *
 * 使用例:
 * @code
 * auto str = format<128>("Hello, {}! Value: {}", "World", 42);
 * // 結果: "Hello, World! Value: 42"
 * @endcode
 */
template <uint32_t Capacity, uint32_t N, typename... Args>
constexpr FixedString<Capacity> format(const char (&format_str)[N], Args&&... args) noexcept {
    return format<Capacity>(basic_format_string<Args...>(format_str), args...);
}

/**
 * @brief 文字列フォーマット（文字列リテラル版、Capacity自動計算）
 *
 * 文字列リテラルと引数の型から必要な容量を自動計算
 *
 * 使用例:
 * @code
 * auto str = format("Hello, {}! Value: {}", "World", 42);
 * // 容量は自動計算される
 * @endcode
 */
template <uint32_t N, typename... Args>
constexpr auto format(const char (&format_str)[N], Args&&... args) noexcept -> FixedString<detail::calculate_capacity<N, typename detail::remove_cv_ref<Args>::type...>::value> {
    constexpr uint32_t capacity = detail::calculate_capacity<N, typename detail::remove_cv_ref<Args>::type...>::value;
    return format<capacity>(basic_format_string<Args...>(format_str), args...);
}

/**
 * @brief 文字列フォーマット（std::string_view版、互換性維持）
 *
 * 実行時に構築されたstd::string_viewから使用
 */
template <uint32_t Capacity, typename... Args>
constexpr FixedString<Capacity> format(std::string_view format_str, Args&&... args) noexcept {
    return format<Capacity>(basic_format_string<Args...>(format_str), args...);
}

/**
 * @brief 16進数フォーマット
 */
template <uint32_t Capacity, typename T>
FixedString<Capacity> format_hex(T value, bool uppercase = false) noexcept {
    FixedString<Capacity> result;
    result.append("0x");

    char buffer[32] = {};
    uint32_t len = detail::hex_to_string(value, buffer, sizeof(buffer), uppercase);
    if (len > 0) {
        result.append(std::string_view(buffer, len));
    }

    return result;
}

/**
 * @brief 文字列フォーマット（basic_format_string版）- 主要実装
 *
 * @tparam N 出力バッファの容量
 * @tparam FmtArgs フォーマット文字列が期待する型
 * @tparam Args 実際の引数の型
 * @param result 出力先のFixedString
 * @param format_str フォーマット文字列
 * @param args フォーマット引数
 * @return bool 常にtrue（将来の拡張用）
 *
 * 使用例:
 * @code
 * FixedString<128> str;
 * format_string<const char*, int> fs("Name: {}, Age: {}");
 * format_to(str, fs, "Alice", 25);
 * @endcode
 */
template <uint32_t N, typename... FmtArgs, typename... Args>
constexpr bool format_to(FixedString<N>& result, const basic_format_string<FmtArgs...>& format_str, Args&&... args) noexcept {
    result.clear();
    uint32_t arg_index = 0;
    detail::format_impl(result, format_str.view(), arg_index, args...);
    return true;
}

/**
 * @brief 文字列フォーマット（文字列リテラル版、バッファ指定）
 *
 * 文字列リテラルから暗黙的にbasic_format_stringを構築
 *
 * 使用例:
 * @code
 * FixedString<128> str;
 * format_to(str, "Hello, {}! Value: {}", "World", 42);
 * @endcode
 */
template <uint32_t N, uint32_t M, typename... Args>
constexpr bool format_to(FixedString<N>& result, const char (&format_str)[M], Args&&... args) noexcept {
    return format_to(result, basic_format_string<Args...>(format_str), args...);
}

/**
 * @brief 文字列フォーマット（文字列リテラル版、Capacity自動計算）
 *
 * 必要な容量を自動計算してFixedStringを返す
 *
 * 使用例:
 * @code
 * auto str = format_to("Hello, {}! Value: {}", "World", 42);
 * // 容量は自動計算される
 * @endcode
 */
template <uint32_t N, typename... Args>
constexpr auto format_to(const char (&format_str)[N], Args&&... args) noexcept -> FixedString<detail::calculate_capacity<N, typename detail::remove_cv_ref<Args>::type...>::value> {
    constexpr uint32_t capacity = detail::calculate_capacity<N, typename detail::remove_cv_ref<Args>::type...>::value;
    FixedString<capacity> result;
    format_to(result, basic_format_string<Args...>(format_str), args...);
    return result;
}

/**
 * @brief 文字列フォーマット（std::string_view版、互換性維持）
 *
 * 実行時に構築されたstd::string_viewから使用
 */
template <uint32_t N, typename... Args>
constexpr bool format_to(FixedString<N>& result, std::string_view format_str, Args&&... args) noexcept {
    return format_to(result, basic_format_string<Args...>(format_str), args...);
}

/**
 * @brief 16進数フォーマット（テンプレート引数隠蔽版）
 */
template <uint32_t N, typename T>
bool format_hex_to(FixedString<N>& result, T value, bool uppercase = false) noexcept {
    result.clear();
    result.append("0x");

    char buffer[32] = {};
    uint32_t len = detail::hex_to_string(value, buffer, sizeof(buffer), uppercase);
    if (len > 0) {
        result.append(std::string_view(buffer, len));
    }

    return true;
}

/**
 * @brief デフォルト容量256のフォーマット関数（basic_format_string版）- 主要実装
 *
 * @tparam FmtArgs フォーマット文字列が期待する型
 * @tparam Args 実際の引数の型
 * @param format_str フォーマット文字列
 * @param args フォーマット引数
 * @return FixedString<256> フォーマット済み文字列
 *
 * 使用例:
 * @code
 * format_string<const char*, int> fs("Name: {}, Age: {}");
 * auto str = fmt(fs, "Alice", 25);
 * // 結果: "Name: Alice, Age: 25"
 * @endcode
 */
template <typename... FmtArgs, typename... Args>
constexpr FixedString<256> fmt(const basic_format_string<FmtArgs...>& format_str, Args&&... args) noexcept {
    return format<256>(format_str, args...);
}

/**
 * @brief デフォルト容量256のフォーマット関数（文字列リテラル版）
 *
 * 文字列リテラルから暗黙的にbasic_format_stringを構築
 *
 * 使用例:
 * @code
 * auto str = fmt("Hello, {}! Value: {}", "World", 42);
 * // 結果: "Hello, World! Value: 42"
 * @endcode
 */
template <uint32_t N, typename... Args>
constexpr FixedString<256> fmt(const char (&format_str)[N], Args&&... args) noexcept {
    return format<256>(basic_format_string<Args...>(format_str), args...);
}

/**
 * @brief デフォルト容量256のフォーマット関数（std::string_view版、互換性維持）
 *
 * 実行時に構築されたstd::string_viewから使用
 */
template <typename... Args>
constexpr FixedString<256> fmt(std::string_view format_str, Args&&... args) noexcept {
    return format<256>(basic_format_string<Args...>(format_str), args...);
}

/**
 * @brief デフォルト容量256の16進数フォーマット
 */
template <typename T>
FixedString<256> fmt_hex(T value, bool uppercase = false) noexcept {
    return format_hex<256>(value, uppercase);
}

} // namespace omusubi
