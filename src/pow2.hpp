#pragma once
#include <climits>
#include <cstddef>

namespace utility {
    /**
     * Gets the largest power of 2 less than or equal to `value`.
     */
    template <typename T>
    constexpr T pow2_floor(T value) noexcept {
        value >>= 1;
        for (::std::size_t i = 1; i < sizeof(T) * CHAR_BIT; i *= 2) {
            value |= value >> i;
        }
        return value + 1;
    }

    /**
     * Gets the smallest power of 2 greater than or equal to `value`.
     */
    template <typename T>
    constexpr T pow2_ceil(T value) noexcept {
        --value;
        for (::std::size_t i = 1; i < sizeof(T) * CHAR_BIT; i *= 2) {
            value |= value >> i;
        }
        return value + 1;
    }
}
