#pragma once
#include "remove-cvref.hpp"
#include <memory>
#include <type_traits>
#include <utility>

namespace utility::detail::box {
    namespace std = ::std;

    using ::utility::remove_cvref_t;

    template <typename T>
    class Box {
        public:
        template <
            typename... Args,
            typename = std::enable_if_t<
                sizeof...(Args) != 1 || !(
                    std::is_same_v<remove_cvref_t<Args>, Box> || ...
                )
            >
        >
        Box(Args&&... args) :
        m_value(std::make_unique<T>(std::forward<Args>(args)...)) {
        }

        Box(const Box& other) noexcept(
            std::is_nothrow_copy_constructible_v<T>
        ) : Box(*other) {
        }

        Box(Box&& other) noexcept(
            std::is_nothrow_move_constructible_v<T>
        ) : Box(std::move(*other)) {
        }

        Box& operator=(const Box& other) noexcept(
            std::is_nothrow_copy_assignable_v<T>
        ) {
            **this = *other;
            return *this;
        }

        Box& operator=(Box&& other) noexcept(
            std::is_nothrow_move_assignable_v<T>
        ) {
            **this = std::move(*other);
            return *this;
        }

        const T& operator*() const & noexcept {
            return *m_value;
        }

        T& operator*() & noexcept {
            return *m_value;
        }

        T&& operator*() && noexcept {
            return std::move(**this);
        }

        const T* operator->() const noexcept {
            return get();
        }

        T* operator->() noexcept {
            return get();
        }

        const T* get() const noexcept {
            return m_value.get();
        }

        T* get() noexcept {
            return m_value.get();
        }

        private:
        std::unique_ptr<T> m_value = std::make_unique<T>();
    };
}

namespace utility {
    /**
     * template <typename T>
     * class Box;
     *
     * An owned smart pointer with value-like semantics: copying the smart
     * pointer copies the heap-allocated data.
     */
    using detail::box::Box;
}
