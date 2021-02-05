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
#include <algorithm>
#include <cstddef>
#include <type_traits>

namespace utility::detail::storage_for {
    namespace std = ::std;

    template <typename... Types>
    constexpr std::size_t max_size() {
        return std::max({std::size_t(0), sizeof(Types)...});
    }

    template <typename... Types>
    constexpr std::size_t max_align() {
        return std::max({std::size_t(0), alignof(Types)...});
    }

    template <typename... Types>
    struct StorageImpl {
        private:
        static constexpr std::size_t size = max_size<Types...>();
        static constexpr std::size_t align = max_align<Types...>();
        alignas(align) unsigned char m_data[size];
    };

    template <>
    struct StorageImpl <> {
    };
}

namespace utility {
    /**
     * A type suitable for use as storage for any type in `Types`. This is
     * like std::aligned_union_t<0, Types...>, but faster than the
     * implementation in libstdc++ and libc++.
     */
    template <typename... Types>
    struct StorageFor : ::std::conditional_t<
        (::std::is_empty_v<Types> && ...),
        detail::storage_for::StorageImpl<>,
        detail::storage_for::StorageImpl<Types...>
    > {
    };
}
