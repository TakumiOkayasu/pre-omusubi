#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace omusubi {

/**
 * @brief 固定長ビット配列
 *
 * N ビットを効率的に格納・操作するためのコンテナ。
 * I/Oポート操作、フラグ管理、ビットマスク処理などに使用。
 *
 * @tparam N ビット数（1以上）
 *
 * @note ヒープアロケーションなし、constexpr 対応
 */
template <uint32_t N>
class BitSet {
    static_assert(N > 0, "BitSet size must be greater than 0");

private:
    // 内部ストレージの型（32ビット単位で格納）
    using Word = uint32_t;
    static constexpr uint32_t BITS_PER_WORD = 32;
    static constexpr uint32_t WORD_COUNT = (N + BITS_PER_WORD - 1) / BITS_PER_WORD;

    Word data_[WORD_COUNT] = {};

    // ビット位置からワードインデックスを計算
    [[nodiscard]] static constexpr uint32_t word_index(uint32_t pos) noexcept { return pos / BITS_PER_WORD; }

    // ビット位置からワード内オフセットを計算
    [[nodiscard]] static constexpr uint32_t bit_offset(uint32_t pos) noexcept { return pos % BITS_PER_WORD; }

    // 指定位置のビットマスクを取得
    [[nodiscard]] static constexpr Word bit_mask(uint32_t pos) noexcept { return Word(1) << bit_offset(pos); }

public:
    /**
     * @brief デフォルトコンストラクタ（すべて0で初期化）
     */
    constexpr BitSet() noexcept = default;

    /**
     * @brief 初期値を指定して構築
     * @param value 初期値（下位Nビットが使用される）
     */
    constexpr explicit BitSet(uint64_t value) noexcept {
        if constexpr (N <= 32) {
            data_[0] = static_cast<Word>(value) & ((N == 32) ? ~Word(0) : (Word(1) << N) - 1);
        } else if constexpr (N <= 64) {
            data_[0] = static_cast<Word>(value);
            data_[1] = static_cast<Word>(value >> 32) & ((N == 64) ? ~Word(0) : (Word(1) << (N - 32)) - 1);
        } else {
            data_[0] = static_cast<Word>(value);
            data_[1] = static_cast<Word>(value >> 32);
            // 残りは0で初期化済み
        }
    }

    /**
     * @brief ビット数を取得
     * @return ビット数
     */
    [[nodiscard]] static constexpr uint32_t size() noexcept { return N; }

    /**
     * @brief 指定位置のビットを取得
     * @param pos ビット位置（0-indexed）
     * @return ビット値（true=1, false=0）
     */
    [[nodiscard]] constexpr bool test(uint32_t pos) const noexcept {
        if (pos >= N) {
            return false;
        }
        return (data_[word_index(pos)] & bit_mask(pos)) != 0;
    }

    /**
     * @brief 添字演算子（読み取り専用）
     * @param pos ビット位置
     * @return ビット値
     */
    [[nodiscard]] constexpr bool operator[](uint32_t pos) const noexcept { return test(pos); }

    /**
     * @brief 指定位置のビットを1にセット
     * @param pos ビット位置
     * @return 自身への参照
     */
    constexpr BitSet& set(uint32_t pos) noexcept {
        if (pos < N) {
            data_[word_index(pos)] |= bit_mask(pos);
        }
        return *this;
    }

    /**
     * @brief 指定位置のビットを指定値にセット
     * @param pos ビット位置
     * @param value セットする値
     * @return 自身への参照
     */
    constexpr BitSet& set(uint32_t pos, bool value) noexcept {
        if (value) {
            return set(pos);
        }
        return reset(pos);
    }

    /**
     * @brief すべてのビットを1にセット
     * @return 自身への参照
     */
    constexpr BitSet& set() noexcept {
        for (uint32_t i = 0; i < WORD_COUNT; ++i) {
            data_[i] = ~Word(0);
        }
        // 最後のワードの余分なビットをクリア
        constexpr uint32_t EXTRA_BITS = N % BITS_PER_WORD;
        if constexpr (EXTRA_BITS != 0) {
            data_[WORD_COUNT - 1] &= (Word(1) << EXTRA_BITS) - 1;
        }
        return *this;
    }

    /**
     * @brief 指定位置のビットを0にリセット
     * @param pos ビット位置
     * @return 自身への参照
     */
    constexpr BitSet& reset(uint32_t pos) noexcept {
        if (pos < N) {
            data_[word_index(pos)] &= ~bit_mask(pos);
        }
        return *this;
    }

    /**
     * @brief すべてのビットを0にリセット
     * @return 自身への参照
     */
    constexpr BitSet& reset() noexcept {
        for (uint32_t i = 0; i < WORD_COUNT; ++i) {
            data_[i] = 0;
        }
        return *this;
    }

    /**
     * @brief 指定位置のビットを反転
     * @param pos ビット位置
     * @return 自身への参照
     */
    constexpr BitSet& flip(uint32_t pos) noexcept {
        if (pos < N) {
            data_[word_index(pos)] ^= bit_mask(pos);
        }
        return *this;
    }

    /**
     * @brief すべてのビットを反転
     * @return 自身への参照
     */
    constexpr BitSet& flip() noexcept {
        for (uint32_t i = 0; i < WORD_COUNT; ++i) {
            data_[i] = ~data_[i];
        }
        // 最後のワードの余分なビットをクリア
        constexpr uint32_t EXTRA_BITS = N % BITS_PER_WORD;
        if constexpr (EXTRA_BITS != 0) {
            data_[WORD_COUNT - 1] &= (Word(1) << EXTRA_BITS) - 1;
        }
        return *this;
    }

    /**
     * @brief すべてのビットが1かどうか
     * @return すべて1ならtrue
     */
    [[nodiscard]] constexpr bool all() const noexcept {
        // 最後以外のワードをチェック
        for (uint32_t i = 0; i < WORD_COUNT - 1; ++i) {
            if (data_[i] != ~Word(0)) {
                return false;
            }
        }
        // 最後のワードをチェック
        constexpr uint32_t EXTRA_BITS = N % BITS_PER_WORD;
        if constexpr (EXTRA_BITS == 0) {
            return data_[WORD_COUNT - 1] == ~Word(0);
        } else {
            return data_[WORD_COUNT - 1] == (Word(1) << EXTRA_BITS) - 1;
        }
    }

    /**
     * @brief いずれかのビットが1かどうか
     * @return 1つでも1があればtrue
     */
    [[nodiscard]] constexpr bool any() const noexcept {
        for (uint32_t i = 0; i < WORD_COUNT; ++i) {
            if (data_[i] != 0) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief すべてのビットが0かどうか
     * @return すべて0ならtrue
     */
    [[nodiscard]] constexpr bool none() const noexcept { return !any(); }

    /**
     * @brief 1のビット数を数える
     * @return 1のビット数
     */
    [[nodiscard]] constexpr uint32_t count() const noexcept {
        uint32_t result = 0;
        for (uint32_t i = 0; i < WORD_COUNT; ++i) {
            Word w = data_[i];
            // ポップカウント（Brian Kernighanのアルゴリズム）
            while (w != 0) {
                w &= w - 1;
                ++result;
            }
        }
        return result;
    }

    /**
     * @brief uint32_t に変換（下位32ビット）
     * @return 下位32ビットの値
     */
    [[nodiscard]] constexpr uint32_t to_uint32() const noexcept { return data_[0]; }

    /**
     * @brief uint64_t に変換（下位64ビット）
     * @return 下位64ビットの値
     */
    [[nodiscard]] constexpr uint64_t to_uint64() const noexcept {
        if constexpr (WORD_COUNT >= 2) {
            return static_cast<uint64_t>(data_[0]) | (static_cast<uint64_t>(data_[1]) << 32);
        } else {
            return static_cast<uint64_t>(data_[0]);
        }
    }

    /**
     * @brief ビット単位AND
     */
    constexpr BitSet& operator&=(const BitSet& other) noexcept {
        for (uint32_t i = 0; i < WORD_COUNT; ++i) {
            data_[i] &= other.data_[i];
        }
        return *this;
    }

    /**
     * @brief ビット単位OR
     */
    constexpr BitSet& operator|=(const BitSet& other) noexcept {
        for (uint32_t i = 0; i < WORD_COUNT; ++i) {
            data_[i] |= other.data_[i];
        }
        return *this;
    }

    /**
     * @brief ビット単位XOR
     */
    constexpr BitSet& operator^=(const BitSet& other) noexcept {
        for (uint32_t i = 0; i < WORD_COUNT; ++i) {
            data_[i] ^= other.data_[i];
        }
        return *this;
    }

    /**
     * @brief ビット単位NOT
     */
    [[nodiscard]] constexpr BitSet operator~() const noexcept {
        BitSet result = *this;
        result.flip();
        return result;
    }

    /**
     * @brief 左シフト
     */
    constexpr BitSet& operator<<=(uint32_t shift) noexcept {
        if (shift >= N) {
            reset();
            return *this;
        }
        if (shift == 0) {
            return *this;
        }

        const uint32_t word_shift = shift / BITS_PER_WORD;
        const uint32_t bit_shift = shift % BITS_PER_WORD;

        if (bit_shift == 0) {
            for (uint32_t i = WORD_COUNT; i > word_shift; --i) {
                data_[i - 1] = data_[i - 1 - word_shift];
            }
        } else {
            for (uint32_t i = WORD_COUNT; i > word_shift + 1; --i) {
                data_[i - 1] = (data_[i - 1 - word_shift] << bit_shift) | (data_[i - 2 - word_shift] >> (BITS_PER_WORD - bit_shift));
            }
            data_[word_shift] = data_[0] << bit_shift;
        }

        for (uint32_t i = 0; i < word_shift; ++i) {
            data_[i] = 0;
        }

        // 余分なビットをクリア
        constexpr uint32_t EXTRA_BITS = N % BITS_PER_WORD;
        if constexpr (EXTRA_BITS != 0) {
            data_[WORD_COUNT - 1] &= (Word(1) << EXTRA_BITS) - 1;
        }

        return *this;
    }

    /**
     * @brief 右シフト
     */
    constexpr BitSet& operator>>=(uint32_t shift) noexcept {
        if (shift >= N) {
            reset();
            return *this;
        }
        if (shift == 0) {
            return *this;
        }

        const uint32_t word_shift = shift / BITS_PER_WORD;
        const uint32_t bit_shift = shift % BITS_PER_WORD;

        if (bit_shift == 0) {
            for (uint32_t i = 0; i < WORD_COUNT - word_shift; ++i) {
                data_[i] = data_[i + word_shift];
            }
        } else {
            for (uint32_t i = 0; i < WORD_COUNT - word_shift - 1; ++i) {
                data_[i] = (data_[i + word_shift] >> bit_shift) | (data_[i + word_shift + 1] << (BITS_PER_WORD - bit_shift));
            }
            data_[WORD_COUNT - word_shift - 1] = data_[WORD_COUNT - 1] >> bit_shift;
        }

        for (uint32_t i = WORD_COUNT - word_shift; i < WORD_COUNT; ++i) {
            data_[i] = 0;
        }

        return *this;
    }

    /**
     * @brief 左シフト（新しいBitSetを返す）
     */
    [[nodiscard]] constexpr BitSet operator<<(uint32_t shift) const noexcept {
        BitSet result = *this;
        result <<= shift;
        return result;
    }

    /**
     * @brief 右シフト（新しいBitSetを返す）
     */
    [[nodiscard]] constexpr BitSet operator>>(uint32_t shift) const noexcept {
        BitSet result = *this;
        result >>= shift;
        return result;
    }

    /**
     * @brief 等価比較
     */
    [[nodiscard]] constexpr bool operator==(const BitSet& other) const noexcept {
        for (uint32_t i = 0; i < WORD_COUNT; ++i) {
            if (data_[i] != other.data_[i]) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief 非等価比較
     */
    [[nodiscard]] constexpr bool operator!=(const BitSet& other) const noexcept { return !(*this == other); }
};

// 二項演算子

template <uint32_t N>
[[nodiscard]] constexpr BitSet<N> operator&(const BitSet<N>& lhs, const BitSet<N>& rhs) noexcept {
    BitSet<N> result = lhs;
    result &= rhs;
    return result;
}

template <uint32_t N>
[[nodiscard]] constexpr BitSet<N> operator|(const BitSet<N>& lhs, const BitSet<N>& rhs) noexcept {
    BitSet<N> result = lhs;
    result |= rhs;
    return result;
}

template <uint32_t N>
[[nodiscard]] constexpr BitSet<N> operator^(const BitSet<N>& lhs, const BitSet<N>& rhs) noexcept {
    BitSet<N> result = lhs;
    result ^= rhs;
    return result;
}

} // namespace omusubi
