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
