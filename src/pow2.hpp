/*
 * Copyright (C) 2020 taylor.fish <contact@taylor.fish>
 *
 * This file is part of cxxutil.
 *
 * cxxutil is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cxxutil is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cxxutil. If not, see <https://www.gnu.org/licenses/>.
 */

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
