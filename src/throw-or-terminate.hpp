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
#include <exception>
#include <iostream>
#include <utility>

namespace utility {
    /**
     * Throws the given exception if exceptions are enabled; otherwise,
     * prints an error message and calls std::terminate().
     */
    template <typename T>
    [[noreturn]] void throw_or_terminate(T&& exception) {
        namespace std = ::std;
        #if __cpp_exceptions
            throw std::forward<T>(exception);
        #else
            #if __cpp_rtti
                std::cerr << "terminating with uncaught exception of type ";
                std::cerr << typeid(T).name() << ": ";
            #else
                std::cerr << "terminating with uncaught exception: ";
            #endif
            std::cerr << exception.what() << std::endl;
            std::terminate();
        #endif
    }
}
