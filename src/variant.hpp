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
#include "first-type.hpp"
#include "remove-cvref.hpp"
#include "smallest-uint.hpp"
#include "storage-for.hpp"
#include "throw-or-terminate.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <variant>

namespace utility::variant {
    class BadAccess : public ::std::bad_variant_access {
        public:
        enum class Error {
            unknown,
            bad_alternative,
            valueless,
        };

        BadAccess() noexcept = default;

        BadAccess(Error error) noexcept : m_error(error) {
        }

        const char* what() const noexcept override {
            switch (m_error) {
                case Error::unknown: {
                    break;
                }
                case Error::bad_alternative: {
                    return "variant is not holding the specified alternative";
                }
                case Error::valueless: {
                    return "variant is valueless by exception";
                }
            }
            return bad_variant_access::what();
        }

        private:
        Error m_error = Error::unknown;
    };
}

namespace utility::variant::detail {
    namespace std = ::std;

    using ::utility::first_type_t;
    using ::utility::remove_cvref_t;
    using ::utility::smallest_uint_t;
    using ::utility::StorageFor;
    using ::utility::throw_or_terminate;

    template <std::size_t n>
    using size_constant = std::integral_constant<std::size_t, n>;

    template <std::size_t id, typename T>
    class Alternative {
        protected:
        static T id_to_type(size_constant<id>);

        static constexpr std::size_t type_ptr_to_id(T*) noexcept {
            return id;
        }

        template <typename... Args>
        [[nodiscard]]
        static std::size_t init(
            void* data, std::in_place_type_t<T>, Args&&... args
        ) {
            new (data) T(std::forward<Args&&>(args)...);
            return id;
        }

        [[nodiscard]]
        static std::size_t init(void* data, const T& value) {
            new (data) T(value);
            return id;
        }

        [[nodiscard]]
        static std::size_t init(void* data, T&& value) {
            new (data) T(std::move(value));
            return id;
        }

        [[nodiscard]]
        static bool assign(std::size_t id_, void* data, const T& value) {
            if (id_ != id) {
                return false;
            }
            *std::launder(reinterpret_cast<T*>(data)) = value;
            return true;
        }

        [[nodiscard]]
        static bool assign(std::size_t id_, void* data, T&& value) {
            if (id_ != id) {
                return false;
            }
            *std::launder(reinterpret_cast<T*>(data)) = std::move(value);
            return true;
        }
    };

    template <typename Ids, typename... Types>
    struct VariantBase;

    template <std::size_t... ids, typename... Types>
    struct VariantBase<std::index_sequence<ids...>, Types...> :
    Alternative<ids, Types>... {
        using Alternative<ids, Types>::id_to_type...;
        using Alternative<ids, Types>::type_ptr_to_id...;
        using Alternative<ids, Types>::init...;
        using Alternative<ids, Types>::assign...;
    };

    template <typename... Types>
    class Variant : StorageFor<Types...>,
    VariantBase<std::make_index_sequence<sizeof...(Types)>, Types...> {
        using Self = Variant;
        using Base = VariantBase<
            std::make_index_sequence<sizeof...(Types)>, Types...
        >;

        using First = first_type_t<Types...>;
        using Storage = StorageFor<Types...>;

        using Base::id_to_type;
        using Base::type_ptr_to_id;

        template <std::size_t id>
        using type_t = decltype(id_to_type(size_constant<id>()));

        template <typename T>
        static constexpr std::size_t id_v = type_ptr_to_id(
            static_cast<T*>(nullptr)
        );

        /* public members */
        /* ============== */

        public:
        Variant() {
            m_id = init(data(), std::in_place_type<First>);
        }

        template <
            typename Arg,
            typename = std::enable_if_t<
                !std::is_same_v<Self, remove_cvref_t<Arg>>
            >
        >
        Variant(Arg&& arg) {
            m_id = init(data(), std::forward<Arg>(arg));
        }

        Variant(const Self& other) noexcept(
            (std::is_nothrow_copy_constructible_v<Types> && ...)
        ) {
            other.visit([this] (const auto& value) {
                using T = remove_cvref_t<decltype(value)>;
                new (data()) T(value);
            });
            m_id = other.m_id;
        }

        Variant(Self&& other) noexcept(
            (std::is_nothrow_move_constructible_v<Types> && ...)
        ) {
            std::move(other).visit([this] (auto&& value) {
                using T = std::remove_reference_t<decltype(value)>;
                new (data()) T(std::move(value));
            });
            m_id = other.m_id;
        }

        template <
            typename Arg,
            typename = std::enable_if_t<
                !std::is_same_v<Self, remove_cvref_t<Arg>>
            >
        >
        Self& operator=(Arg&& arg) {
            if (!assign(m_id, data(), std::forward<Arg>(arg))) {
                destroy();
                m_id = init(data(), std::forward<Arg>(arg));
            }
            return *this;
        }

        Self& operator=(const Self& other) noexcept(
            (std::is_nothrow_copy_assignable_v<Types> && ...)
        ) {
            other.visit([this] (const auto& value) {
                *this = value;
            });
            return *this;
        }

        Self& operator=(Self&& other) noexcept(
            (std::is_nothrow_move_assignable_v<Types> && ...)
        ) {
            std::move(other).visit([this] (auto&& value) {
                *this = std::move(value);
            });
            return *this;
        }

        ~Variant() {
            destroy();
        }

        /* comparison operators */
        /* ==================== */

        bool operator==(const Self& other) const {
            return compare<std::equal_to<>>(other);
        }

        bool operator!=(const Self& other) const {
            return compare<std::not_equal_to<>>(other);
        }

        bool operator<(const Self& other) const {
            return compare<std::less<>>(other);
        }

        bool operator>(const Self& other) const {
            return compare<std::greater<>>(other);
        }

        bool operator<=(const Self& other) const {
            return compare<std::less_equal<>>(other);
        }

        bool operator>=(const Self& other) const {
            return compare<std::greater_equal<>>(other);
        }

        /* observers */
        /* ========= */

        std::size_t index() const noexcept {
            return m_id;
        }

        bool valueless_by_exception() const noexcept {
            #if __cpp_exceptions
                return m_id == valueless_id;
            #else
                return false;
            #endif
        }

        template <typename T>
        bool holds_alternative() const noexcept {
            return m_id == id_v<T>;
        }

        /* modifiers */
        /* ========= */

        template <typename T, typename... Args>
        T& emplace(Args&&... args) {
            destroy();
            T* ptr = new (data()) T(std::forward<Args>(args)...);
            m_id = id_v<T>;
            return *ptr;
        }

        /* get<std::size_t>() */
        /* ================== */

        template <std::size_t i>
        const type_t<i>& get() const & {
            if (auto ptr = get_if<i>()) {
                return *ptr;
            }
            throw_bad_access(BadAccess::Error::bad_alternative);
        }

        template <std::size_t i>
        type_t<i>& get() & {
            return const_cast<type_t<i>&>(
                static_cast<const Self&>(*this).get<i>()
            );
        }

        template <std::size_t i>
        type_t<i>&& get() && {
            return std::move(get<i>());
        }

        /* get_if<std::size_t>() */
        /* ===================== */

        template <std::size_t i>
        const type_t<i>* get_if() const noexcept {
            if (m_id != i) {
                return nullptr;
            }
            return std::addressof(get_unchecked<type_t<i>>());
        }

        template <std::size_t i>
        type_t<i>* get_if() noexcept {
            return const_cast<type_t<i>*>(
                static_cast<const Self&>(*this).get_if<i>()
            );
        }

        /* get<typename>() */
        /* =============== */

        template <typename T>
        const T& get() const & {
            return get<id_v<T>>();
        }

        template <typename T>
        T& get() & {
            return get<id_v<T>>();
        }

        template <typename T>
        T&& get() && {
            return std::move(*this).template get<id_v<T>>();
        }

        /* get_if<typename>() */
        /* ================== */

        template <typename T>
        const T* get_if() const noexcept {
            return get_if<id_v<T>>();
        }

        template <typename T>
        T* get_if() noexcept {
            return get_if<id_v<T>>();
        }

        /* get_unchecked() */
        /* =============== */

        template <typename T>
        const T& get_unchecked() const & noexcept {
            return *std::launder(reinterpret_cast<const T*>(data()));
        }

        template <typename T>
        T& get_unchecked() & noexcept {
            return *std::launder(reinterpret_cast<T*>(data()));
        }

        template <typename T>
        T&& get_unchecked() && noexcept {
            return std::move(get_unchecked<T>());
        }

        /* visit() */
        /* ======= */

        template <typename Func>
        decltype(auto) visit(Func&& func) const & {
            return handlers_const<Func>[m_id](
                data(), std::forward<Func>(func)
            );
        }

        template <typename Func>
        decltype(auto) visit(Func&& func) & {
            return handlers_lvalue<Func>[m_id](
                data(), std::forward<Func>(func)
            );
        }

        template <typename Func>
        decltype(auto) visit(Func&& func) && {
            return handlers_rvalue<Func>[m_id](
                data(), std::forward<Func>(func)
            );
        }

        /* non-member functions */
        /* ==================== */

        template <typename T>
        friend bool holds_alternative(const Self& self) noexcept {
            return self.holds_alternative<T>();
        }

        /* non-member visit() */
        /* ================== */

        template <typename Func>
        friend decltype(auto) visit(Func&& func, const Self& self) {
            return self.visit(std::forward<Func>(func));
        }

        template <typename Func>
        friend decltype(auto) visit(Func&& func, Self& self) {
            return self.visit(std::forward<Func>(func));
        }

        template <typename Func>
        friend decltype(auto) visit(Func&& func, Self&& self) {
            return std::move(self).visit(std::forward<Func>(func));
        }

        /* non-member get<std::size_t>() */
        /* ============================= */

        template <std::size_t i>
        friend const type_t<i>& get(const Self& self) {
            return self.get<i>();
        }

        template <std::size_t i>
        friend type_t<i>& get(Self& self) {
            return self.get<i>();
        }

        template <std::size_t i>
        friend type_t<i>&& get(Self&& self) {
            return std::move(self).template get<i>();
        }

        /* non-member get_if<std::size_t>() */
        /* ================================ */

        template <std::size_t i>
        friend const type_t<i>* get_if(const Self& self) noexcept {
            return self.get_if<i>();
        }

        template <std::size_t i>
        friend type_t<i>* get_if(Self& self) noexcept {
            return self.get_if<i>();
        }

        /* non-member get<typename>() */
        /* ========================== */

        template <typename T>
        friend const T& get(const Self& self) {
            return self.get<T>();
        }

        template <typename T>
        friend T& get(Self& self) {
            return self.get<T>();
        }

        template <typename T>
        friend T&& get(Self&& self) {
            return std::move(self).template get<T>();
        }

        /* non-member get_if<typename>() */
        /* ============================= */

        template <typename T>
        friend const T* get_if(const Self& self) noexcept {
            return self.get_if<T>();
        }

        template <typename T>
        friend T* get_if(Self& self) noexcept {
            return self.get_if<T>();
        }

        /* private members */
        /* =============== */

        private:
        template <std::size_t, typename>
        friend struct std::variant_alternative;

        static constexpr std::size_t valueless_id = sizeof...(Types);
        smallest_uint_t<valueless_id> m_id = valueless_id;

        using Base::init;
        using Base::assign;

        const void* data() const noexcept {
            return static_cast<const Storage*>(this);
        }

        void* data() noexcept {
            return static_cast<Storage*>(this);
        }

        template <typename CmpFunc>
        bool compare(const Self& other) const {
            if (m_id != other.m_id) {
                return CmpFunc()(m_id, other.m_id);
            }
            return visit([&other] (const auto& value) {
                using T = remove_cvref_t<decltype(value)>;
                return CmpFunc()(value, other.template get_unchecked<T>());
            });
        }

        /* result_(const|lvalue|rvalue)_t */
        /* ============================== */

        template <typename Func>
        using result_const_t = decltype(
            std::declval<Func&>()(std::declval<const First&>())
        );

        template <typename Func>
        using result_lvalue_t = decltype(
            std::declval<Func&>()(std::declval<First&>())
        );

        template <typename Func>
        using result_rvalue_t = decltype(
            std::declval<Func&>()(std::declval<First&&>())
        );

        /* handle_visit_(const|lvalue|rvalue) */
        /* ================================== */

        template <typename T, typename Func>
        static result_const_t<Func>
        handle_visit_const(const void* data, Func&& func) {
            return func(*std::launder(reinterpret_cast<const T*>(data)));
        }

        template <typename T, typename Func>
        static result_lvalue_t<Func>
        handle_visit_lvalue(void* data, Func&& func) {
            return func(*std::launder(reinterpret_cast<T*>(data)));
        }

        template <typename T, typename Func>
        static result_rvalue_t<Func>
        handle_visit_rvalue(void* data, Func&& func) {
            return func(std::move(*std::launder(reinterpret_cast<T*>(data))));
        }

        template <typename Result, typename Data, typename Func>
        [[noreturn]] static Result handle_valueless(Data*, Func&&) {
            throw_bad_access(BadAccess::Error::valueless);
        }

        /* handlers_(const|lvalue|rvalue) */
        /* ============================== */

        template <typename Func>
        static constexpr
        result_const_t<Func> (*handlers_const[])(const void*, Func&&) = {
            &handle_visit_const<Types, Func>...,
            #if __cpp_exceptions
                &handle_valueless<result_const_t<Func>, const void, Func>,
            #endif
        };

        template <typename Func>
        static constexpr
        result_lvalue_t<Func> (*handlers_lvalue[])(void*, Func&&) = {
            &handle_visit_lvalue<Types, Func>...,
            #if __cpp_exceptions
                &handle_valueless<result_lvalue_t<Func>, void, Func>,
            #endif
        };

        template <typename Func>
        static constexpr
        result_rvalue_t<Func> (*handlers_rvalue[])(void*, Func&&) = {
            &handle_visit_rvalue<Types, Func>...,
            #if __cpp_exceptions
                &handle_valueless<result_rvalue_t<Func>, void, Func>,
            #endif
        };

        /* ============================== */

        [[noreturn]] static void throw_bad_access(BadAccess::Error error) {
            throw_or_terminate(BadAccess(error));
        }

        void destroy() noexcept {
            if (valueless_by_exception()) {
                return;
            }
            visit([] (auto& value) {
                using T = std::remove_reference_t<decltype(value)>;
                value.~T();
            });
            #if __cpp_exceptions
                m_id = valueless_id;
            #endif
        }
    };
}

namespace utility::variant {
    /**
     * template <typename... Types>
     * class Variant;
     */
    using detail::Variant;
}

namespace utility {
    /**
     * template <typename... Types>
     * class Variant;
     *
     * A type mostly compatible with std::variant but significantly faster
     * during compilation (for large numbers of alternatives) than libstdc++
     * and libc++.
     */
    using variant::Variant;
}

namespace std {
    template <typename... Types>
    struct variant_size<::utility::Variant<Types...>> :
    integral_constant<size_t, sizeof...(Types)> {
    };

    template <size_t i, typename... Types>
    struct variant_alternative<i, ::utility::Variant<Types...>> {
        using type = typename ::utility::Variant<Types...>::template type_t<i>;
    };
}
