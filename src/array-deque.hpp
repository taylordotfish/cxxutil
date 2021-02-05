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
#include "pow2.hpp"
#include "to-address.hpp"
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace utility::detail::array_deque {
    namespace std = ::std;

    using ::utility::to_address;
    using ::utility::pow2_floor;
    using ::utility::pow2_ceil;

    template <typename T, typename Allocator>
    class ArrayDeque;

    template <typename T>
    struct IteratorData {
        T* m_buffer = nullptr;
        std::size_t m_capacity = 0;
        std::size_t m_head = 0;
        std::size_t m_index = 0;

        IteratorData() noexcept = default;

        template <typename Allocator>
        IteratorData(
            const ArrayDeque<T, Allocator>& deque, std::size_t i
        ) noexcept :
        m_buffer(deque.m_buffer),
        m_capacity(deque.m_capacity),
        m_head(deque.m_head),
        m_index(i) {
        }

        std::size_t mod_capacity(std::size_t i) const noexcept {
            return i & (m_capacity - 1);
        }
    };

    template <typename T>
    class Iterator : IteratorData<std::remove_const_t<T>> {
        using Self = Iterator;
        using Base = IteratorData<std::remove_const_t<T>>;

        public:
        using difference_type = std::ptrdiff_t;
        using value_type = std::remove_const_t<T>;
        using reference = T&;
        using pointer = T*;
        using iterator_category = std::random_access_iterator_tag;

        Iterator() noexcept = default;

        template <typename = void>
        Iterator(const Iterator<value_type>& other) noexcept :
        Base(other) {
        }

        pointer operator->() const noexcept {
            return const_cast<pointer>(m_buffer + m_index);
        }

        reference operator*() const noexcept {
            return *this->operator->();
        }

        reference operator[](std::size_t i) const noexcept {
            return const_cast<reference>(m_buffer[mod_capacity(m_index + i)]);
        }

        bool operator==(const Self& other) const noexcept {
            return m_index == other.m_index;
        }

        bool operator!=(const Self& other) const noexcept {
            return !(*this == other);
        }

        bool operator<(const Self& other) const noexcept {
            if ((m_index < m_head) == (other.m_index < m_head)) {
                return m_index < other.m_index;
            }
            return m_index >= m_head;
        }

        bool operator>(const Self& other) const noexcept {
            return other < *this;
        }

        bool operator<=(const Self& other) const noexcept {
            return !(*this > other);
        }

        bool operator>=(const Self& other) const noexcept {
            return other <= *this;
        }

        Self& operator+=(difference_type n) noexcept {
            m_index = mod_capacity(m_index + n);
            return *this;
        }

        Self& operator-=(difference_type n) noexcept {
            m_index = mod_capacity(m_index - n);
            return *this;
        }

        Self& operator++() noexcept {
            return *this += 1;
        }

        Self& operator--() noexcept {
            return *this -= 1;
        }

        Self operator++(int) noexcept {
            Self result(*this);
            ++*this;
            return result;
        }

        Self operator--(int) noexcept {
            Self result(*this);
            --*this;
            return result;
        }

        Self operator+(difference_type n) const noexcept {
            Self result(*this);
            result += n;
            return result;
        }

        friend Self operator+(difference_type n, const Self& self) noexcept {
            return self + n;
        }

        Self operator-(difference_type n) const noexcept {
            Self result(*this);
            result -= n;
            return result;
        }

        difference_type operator-(const Self& other) {
            return (
                static_cast<difference_type>(m_index) -
                static_cast<difference_type>(other.m_index)
            );
        }

        private:
        friend Iterator<const T>;

        template <typename, typename>
        friend class ArrayDeque;

        using Base::Base;
        using Base::m_buffer;
        using Base::m_head;
        using Base::m_index;
        using Base::mod_capacity;
    };

    template <typename Allocator>
    struct ArrayDequeBase : Allocator {
        using AllocTraits = std::allocator_traits<Allocator>;
        using AllocPtr = typename AllocTraits::pointer;

        AllocPtr m_buffer = nullptr;

        const Allocator& allocator() const noexcept {
            return *this;
        }

        Allocator& allocator() noexcept {
            return *this;
        }
    };

    struct Data {
        std::size_t m_capacity = 0;
        std::size_t m_head = 0;
        std::size_t m_size = 0;
    };

    // Initial capacity after the first element is inserted.
    inline constexpr std::size_t initial_capacity = 1;

    template <typename T, typename Allocator = std::allocator<T>>
    class ArrayDeque : ArrayDequeBase<Allocator>, Data {
        using Self = ArrayDeque;
        using Base = ArrayDequeBase<Allocator>;
        using typename Base::AllocTraits;
        using typename Base::AllocPtr;

        public:
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using iterator = Iterator<T>;
        using const_iterator = Iterator<const T>;
        using difference_type = std::ptrdiff_t;
        using size_type = std::size_t;

        ArrayDeque() noexcept = default;

        ArrayDeque(const Allocator& alloc) noexcept : Base {alloc} {
        }

        ArrayDeque(const Self& other) :
        Self(
            copy_t(), other,
            AllocTraits::select_on_container_copy_construction(
                other.allocator()
            )
        ) {
        }

        ArrayDeque(const Self& other, const Allocator& alloc) :
        Self(copy_t(), other, alloc) {
        }

        ArrayDeque(Self&& other, const Allocator& alloc) :
        Self(
            other.allocator() == alloc ?
            Self(move_t(), std::move(other), alloc) :
            Self(move_items_t(), std::move(other), alloc)
        ) {
        }

        ArrayDeque(Self&& other) noexcept :
        Self(move_t(), std::move(other), std::move(other.allocator())) {
        }

        Self& operator=(const Self& other) {
            constexpr bool propagate = (
                AllocTraits::propagate_on_container_copy_assignment::value
            );
            if (!propagate || allocator() == other.allocator()) {
                copy_items(other);
            } else {
                Self copy(other, other.allocator());
                swap_members(copy);
            }
            return *this;
        }

        Self& operator=(Self&& other) noexcept(
            AllocTraits::propagate_on_container_move_assignment::value ||
            AllocTraits::is_always_equal::value
        ) {
            constexpr bool propagate = (
                AllocTraits::propagate_on_container_move_assignment::value
            );
            if (propagate || allocator() == other.allocator()) {
                swap_members(other);
            } else {
                move_items(other);
            }
            return *this;
        }

        ~ArrayDeque() {
            for (std::size_t i = 0; i < size(); ++i) {
                destroy(buffer_ptr(i));
            }
            deallocate(m_buffer, capacity());
        }

        void swap(Self& other) noexcept(
            AllocTraits::propagate_on_container_move_assignment::value ||
            AllocTraits::is_always_equal::value
        ) {
            constexpr bool propagate = (
                AllocTraits::propagate_on_container_swap::value
            );
            if (propagate || allocator() == other.allocator()) {
                swap_members(other);
            } else {
                std::swap(*this, other);
            }
        }

        friend void swap(Self& first, Self& second) noexcept(noexcept(
            first.swap(second)
        )) {
            first.swap(second);
        }

        /* comparison operators */
        /* ==================== */

        bool operator==(const Self& other) const {
            if (size() != other.size()) {
                return false;
            }
            return std::equal(begin(), end(), other.begin());
        }

        bool operator!=(const Self& other) const {
            return !(*this == other);
        }

        bool operator<(const Self& other) const {
            return std::lexicographical_compare(
                begin(), end(), other.begin(), other.end()
            );
        }

        bool operator>(const Self& other) const {
            return other < *this;
        }

        bool operator<=(const Self& other) const {
            return !(*this > other);
        }

        bool operator>=(const Self& other) const {
            return other <= *this;
        }

        /* size/capacity observers */
        /* ======================= */

        std::size_t size() const noexcept {
            return m_size;
        }

        std::size_t capacity() const noexcept {
            return m_capacity;
        }

        std::size_t max_size() const noexcept {
            return pow2_floor(AllocTraits::max_size(allocator()));
        }

        [[nodiscard]] bool empty() const noexcept {
            return size() == 0;
        }

        /* element accessors */
        /* ================= */

        const T& operator[](std::size_t i) const noexcept {
            assert(i < size());
            return *item_ptr(i);
        }

        T& operator[](std::size_t i) noexcept {
            return const_cast<T&>(static_cast<const Self&>(*this)[i]);
        }

        const T& at(std::size_t i) const {
            if (i >= size()) {
                throw std::out_of_range("ArrayDeque::at(): bad index");
            }
            return (*this)[i];
        }

        T& at(std::size_t i) {
            return const_cast<T&>(static_cast<const Self&>(*this).at(i));
        }

        const T& front() const noexcept {
            assert(!empty());
            return m_buffer[m_head];
        }

        T& front() noexcept {
            return const_cast<T&>(
                static_cast<const Self&>(*this).front()
            );
        }

        const T& back() const noexcept {
            assert(!empty());
            return (*this)[m_size - 1];
        }

        T& back() noexcept {
            return const_cast<T&>(
                static_cast<const Self&>(*this).back()
            );
        }

        /* modifiers */
        /* ========= */

        void push_front(const T& obj) {
            emplace_front(obj);
        }

        void push_front(T&& obj) {
            emplace_front(std::move(obj));
        }

        void push_back(const T& obj) {
            emplace_back(obj);
        }

        void push_back(T&& obj) {
            emplace_back(std::move(obj));
        }

        template <typename... Args>
        void emplace_front(Args&&... args) {
            ensure_capacity();
            construct(buffer_ptr(m_head), std::forward<Args>(args)...);
            m_head = mod_capacity(m_head - 1);
            ++m_size;
        }

        template <typename... Args>
        void emplace_back(Args&&... args) {
            ensure_capacity();
            construct(item_ptr(size()), std::forward<Args>(args)...);
            ++m_size;
        }

        void pop_front() noexcept {
            assert(!empty());
            destroy(buffer_ptr(m_head));
            m_head = mod_capacity(m_head + 1);
            --m_size;
        }

        void pop_back() noexcept {
            assert(!empty());
            destroy(item_ptr(m_size - 1));
            --m_size;
        }

        void clear() noexcept {
            destroy();
            reset();
        }

        template <std::size_t capacity>
        void reserve() {
            constexpr auto real_capacity = pow2_ceil(capacity);
            reserve_unchecked(real_capacity);
        }

        void reserve(std::size_t capacity) {
            reserve_unchecked(pow2_ceil(capacity));
        }

        void reserve_log(std::size_t log_capacity) {
            reserve_unchecked(1 << log_capacity);
        }

        void shrink_to_fit() {
            std::size_t new_capacity = pow2_ceil(size());
            if (new_capacity < capacity()) {
                resize(new_capacity);
            }
        }

        /* iteration */
        /* ========= */

        const_iterator begin() const noexcept {
            return {*this, 0};
        }

        iterator begin() noexcept {
            return {*this, 0};
        }

        const_iterator end() const noexcept {
            return {*this, size()};
        }

        iterator end() noexcept {
            return {*this, size()};
        }

        /* private members */
        /* =============== */

        private:
        friend IteratorData<T>;
        using Base::m_buffer;
        using Base::allocator;

        struct move_t {};
        struct copy_t {};
        struct move_items_t {};

        /**
         * Generic copy constructor.
         */
        template <typename Alloc>
        ArrayDeque(copy_t, const Self& other, Alloc&& alloc) :
        Base {std::forward<Alloc>(alloc), allocate(other.m_capacity)},
        Data {other.m_capacity} {
            try {
                for (; m_size < other.size(); ++m_size) {
                    construct(buffer_ptr(m_size), other[m_size]);
                }
            } catch (...) {
                destroy();
                throw;
            }
        }

        /**
         * Generic move constructor.
         */
        template <typename Alloc>
        ArrayDeque(move_t, Self&& other, Alloc&& alloc) :
        Base {std::forward<Alloc>(alloc), other.m_buffer}, Data(other) {
            other.reset();
        }

        /**
         * Move-constructs each element, but doesn't reuse memory.
         */
        ArrayDeque(move_items_t, Self&& other, const Allocator& alloc) :
        Self(alloc) {
            try {
                for (; m_size < other.size(); ++m_size) {
                    construct(buffer_ptr(m_size), std::move(other[m_size]));
                }
            } catch (...) {
                destroy();
                throw;
            }
        }

        /**
         * Move-assigns or move-constructs each element, but doesn't reuse
         * memory.
         */
        void move_items(Self& other) {
            assign_items<std::remove_reference_t<T>&&>(other);
        }

        /**
         * Copy-assigns or copy-constructs each element.
         */
        void copy_items(const Self& other) {
            assign_items<const T&>(other);
        }

        /**
         * Assigns or constructs each element after casting the element
         * to ItemType.
         */
        template <typename ItemType, typename Other>
        void assign_items(Other&& other) {
            std::size_t i = 0;
            std::size_t min_size = std::min(size(), other.size());

            for (; i < min_size; ++i) {
                (*this)[i] = static_cast<ItemType>(other[i]);
            }
            for (; i < other.size(); ++i) {
                push_back(static_cast<ItemType>(other[i]));
            }
            while (i < size()) {
                pop_back();
            }
        }

        /**
         * Swaps members with another object, including the allocator.
         */
        void swap_members(Self& other) noexcept {
            using std::swap;
            swap(static_cast<Data&>(*this), static_cast<Data&>(other));
            swap(allocator(), other.allocator());
            swap(m_buffer, other.m_buffer);
        }

        /* resizing */
        /* ======== */

        void ensure_capacity() {
            if (size() < capacity()) {
                return;
            }
            if (m_buffer) {
                grow_buffer();
            } else {
                init_buffer();
            }
        }

        void init_buffer(std::size_t capacity = initial_capacity) {
            m_buffer = allocate(capacity);
            m_capacity = capacity;
        }

        void grow_buffer() {
            std::size_t new_capacity = capacity() * 2;
            if (new_capacity < capacity()) {
                throw std::runtime_error("ArrayDeque: capacity overflow");
            }
            resize(new_capacity);
        }

        void resize(std::size_t new_capacity) {
            Data old_data = *this;
            auto old_buffer = m_buffer;
            auto it = begin();
            auto end = this->end();

            m_buffer = allocate(new_capacity);
            m_capacity = new_capacity;
            m_head = 0;
            m_size = 0;

            try {
                for (; it != end; ++it, ++m_size) {
                    construct(buffer_ptr(m_size), std::move(*it));
                }
            } catch (...) {
                destroy();
                m_buffer = old_buffer;
                static_cast<Data&>(*this) = old_data;
                throw;
            }
        }

        void reserve_unchecked(std::size_t new_capacity) {
            if (new_capacity <= capacity()) {
                return;
            }
            if (m_buffer) {
                resize(new_capacity);
            } else {
                init_buffer(new_capacity);
            }
        }

        /* allocation */
        /* ========== */

        AllocPtr allocate(std::size_t size) {
            if (size == 0) {
                return nullptr;
            }
            return AllocTraits::allocate(allocator(), size);
        }

        void deallocate(AllocPtr memory, std::size_t size) noexcept {
            if (memory) {
                AllocTraits::deallocate(allocator(), memory, size);
            }
        }

        template <typename... Args>
        void construct(AllocPtr obj, Args&&... args) {
            AllocTraits::construct(
                allocator(), obj, std::forward<Args>(args)...
            );
        }

        void destroy(AllocPtr obj) noexcept {
            AllocTraits::destroy(allocator(), obj);
        }

        /* ========== */

        std::size_t mod_capacity(std::size_t value) const noexcept {
            return value & (capacity() - 1);
        }

        T* buffer_ptr() const noexcept {
            return to_address(m_buffer);
        }

        T* buffer_ptr(std::size_t i) const noexcept {
            return buffer_ptr() + i;
        }

        T* item_ptr(std::size_t i) const noexcept {
            return buffer_ptr(mod_capacity(m_head + i));
        }

        void reset() noexcept {
            m_capacity = 0;
            m_head = 0;
            m_size = 0;
            m_buffer = nullptr;
        }

        void destroy() noexcept {
            for (std::size_t i = 0; i < size(); ++i) {
                destroy(buffer_ptr(i));
            }
            deallocate(m_buffer, capacity());
        }
    };
}

namespace utility {
    /**
     * template <typename T, typename Allocator = std::allocator<T>>
     * class ArrayDeque;
     *
     * A deque (double-ended queue) implemented as a dynamically resizing
     * array.
     */
    using detail::array_deque::ArrayDeque;
}
