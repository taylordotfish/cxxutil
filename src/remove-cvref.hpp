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
#include <type_traits>

namespace utility::detail::remove_cvref {
    /**
     * Polyfill for C++20's std::remove_cvref.
     */
    template <typename T>
    struct remove_cvref : ::std::remove_cv<::std::remove_reference_t<T>> {
    };

    template <typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;
}

namespace utility {
    #if __cpp_lib_remove_cvref
        using ::std::remove_cvref;
        using ::std::remove_cvref_t;
    #else
        using detail::remove_cvref::remove_cvref;
        using detail::remove_cvref::remove_cvref_t;
    #endif
}
