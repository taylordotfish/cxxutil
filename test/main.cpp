/*
 * Copyright (C) 2020, 2022 taylor.fish <contact@taylor.fish>
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

#include <cassert>
#include <type_traits>

#include <array-deque.hpp>
#include <box.hpp>
#include <first-type.hpp>
#include <pow2.hpp>
#include <remove-cvref.hpp>
#include <smallest-uint.hpp>
#include <storage-for.hpp>
#include <throw-or-terminate.hpp>
#include <to-address.hpp>
#include <variant.hpp>

using utility::Variant;

int main() {
    Variant<int, int*> v(5);
    assert(v.get<int>() == 5);
    v = nullptr;
    assert(2 == v.visit([] (auto& x) {
        using T = utility::remove_cvref_t<decltype(x)>;
        if constexpr (std::is_same_v<T, int>) {
            return 1;
        } else if constexpr (std::is_same_v<T, int*>) {
            return 2;
        } else {
            return 3;
        }
    }));
}
