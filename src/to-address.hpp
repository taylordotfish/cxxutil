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
#include <memory>

namespace utility::detail::to_address {
    template <class T>
    T* to_address(T* pointer) noexcept {
        return pointer;
    }

    /**
     * Polyfill for C++20's std::to_address().
     */
    template <class T>
    auto to_address(const T& pointer) noexcept {
        return to_address(pointer.operator->());
    }
}

namespace utility {
    #if __cpp_lib_to_address
        using ::std::to_address;
    #else
        using detail::to_address::to_address;
    #endif
}
