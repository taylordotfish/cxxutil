#pragma once

namespace utility {
    /**
     * Gets the first type in a parameter pack.
     */
    template <typename T, typename...>
    struct first_type {
        using type = T;
    };

    template <typename... Types>
    using first_type_t = typename first_type<Types...>::type;
}
