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
#include <cstdint>
#include <type_traits>

namespace utility::detail::smallest_uint {
    namespace std = ::std;

    template <std::uintmax_t max, typename... Ints>
    struct first_suitable_uint;

    template <std::uintmax_t max, typename Int, typename... Ints>
    struct first_suitable_uint<max, Int, Ints...> {
        using type = typename std::conditional_t<
            max <= static_cast<Int>(-1),
            std::enable_if<true, Int>,
            first_suitable_uint<max, Ints...>
        >::type;
    };
}

namespace utility {
    /**
     * Gets the smallest unsigned integer type large enough to hold `value`.
     */
    template <::std::uintmax_t value>
    struct smallest_uint : detail::smallest_uint::first_suitable_uint<
        value,
        ::std::uint_least8_t,
        ::std::uint_least16_t,
        ::std::uint_least32_t,
        ::std::uint_least64_t,
        ::std::uintmax_t
    > {
    };

    template <::std::uintmax_t value>
    using smallest_uint_t = typename smallest_uint<value>::type;
}
