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
