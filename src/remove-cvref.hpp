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
