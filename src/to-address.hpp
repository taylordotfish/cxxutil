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
