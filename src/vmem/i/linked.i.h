/*
MIT License

Copyright (c) 2018-2024 Zlatko Michailov 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#pragma once

#include "../../diag/i/diag_ready.i.h"
#include "layout.i.h"
#include "ptr.i.h"
#include "iterator.i.h"


namespace abc { namespace vmem {

    class linked;

    using linked_iterator_state = basic_iterator_state<linked>;

    using linked_iterator = iterator<linked, page_pos_t>;

    using linked_const_iterator = const_iterator<linked, page_pos_t>;


    // --------------------------------------------------------------


    /**
     * @brief    Doubly linked list of pool pages.
     * @details The struct is stateless. It uses an external state.
     */
    class linked
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;
        using iterator_state = linked_iterator_state;

    public:
        using value_type             = page_pos_t;
        using pointer                = ptr<value_type>;
        using const_pointer          = ptr<const value_type>;
        using reference              = value_type&;
        using const_reference        = const value_type&;
        using iterator               = linked_iterator;
        using const_iterator         = linked_const_iterator;
        using reverse_iterator       = iterator;
        using const_reverse_iterator = const_iterator;

    public:
        /**
         * @brief Returns `true` if the given state is uninitialized; `false` if it is initialized.
         */
        static constexpr bool is_uninit(const linked_state* state) noexcept;

    public:
        /**
         * @brief       Constructor.
         * @param state Pointer to a `linked_state` instance.
         * @param pool  Pointer to a `pool` instance.
         * @param log   Pointer to a `log_ostream` instance.
         */
        linked(linked_state* state, pool* pool, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        linked(linked&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        linked(const linked& other) noexcept = default;

    public:
        iterator               begin() noexcept;
        const_iterator         begin() const noexcept;
        const_iterator         cbegin() const noexcept;

        iterator               end() noexcept;
        const_iterator         end() const noexcept;
        const_iterator         cend() const noexcept;

        reverse_iterator       rend() noexcept;
        const_reverse_iterator rend() const noexcept;
        const_reverse_iterator crend() const noexcept;

        reverse_iterator       rbegin() noexcept;
        const_reverse_iterator rbegin() const noexcept;
        const_reverse_iterator crbegin() const noexcept;

    public:
        bool                   empty() const noexcept;

        reference              front();
        const_reference        front() const;

        reference              back();
        const_reference        back() const;

        /**
         * @brief          Links a page after the back.
         * @param page_pos Position of the page to be linked.
         */
        void push_back(const_reference page_pos);

        /**
         * @brief Unlinks the back page.
         */
        void pop_back();

        /**
         * @brief          Links a page before the front.
         * @param page_pos Position of the page to be linked.
         */
        void push_front(const_reference page_pos);

        /**
         * @brief Unlinks the front page.
         */
        void pop_front();

        /**
         * @brief          Links a page at the given iterator.
         * @param itr      Iterator.
         * @param page_pos Position of the page to be linked.
         * @return         Iterator referencing the newly linked page.
         */
        iterator insert(const_iterator itr, const_reference page_pos);

        /**
         * @brief     Links a page at the given iterator.
         * @param itr Iterator.
         * @return    Iterator referencing the page next to the newly linked one.
         */
        iterator erase(const_iterator itr);

        /**
         * @brief Unlinks all the pages.
         */
        void clear();

        /**
         * @brief       Links the other linked list at the end of this one.
         * @param other Other linked list. Its state is reset after this.
         */
        void splice(linked& other);

        /**
         * @brief       Links the other linked list at the end of this one.
         * @param other Other linked list. Its state is reset after this.
         */
        void splice(linked&& other);

    private:
        /**
         * @brief               Links a page at the given iterator without modifying the state.
         * @param itr           Iterator.
         * @param page_pos      Position of the page to be linked.
         * @param back_page_pos Position of the back page.
         * @return              `true` == success; `false` = error.
         */
        bool insert_nostate(const_iterator itr, const_reference page_pos, page_pos_t back_page_pos) noexcept;

        /**
         * @brief               Unlinks a page at the given iterator.
         * @param itr           Iterator.
         * @param back_page_pos Position of the back page.
         * @return              `true` == success; `false` = error.
         */
        bool erase_nostate(const_iterator itr, page_pos_t& back_page_pos) noexcept;

    private:
        friend iterator_state;
        friend const_iterator;
        friend iterator;

        /**
         * @brief     Returns the iterator immediately following a given one.  
         * @param itr Iterator.
         */
        iterator next(const iterator_state& itr) const noexcept;

        /**
         * @brief     Returns the iterator immediately preceding a given one.  
         * @param itr Iterator.
         */
        iterator prev(const iterator_state& itr) const noexcept;

        /**
         * @brief     Dereferences an iterator.
         * @param itr Iterator.
         * @return    A `ptr` instance. 
         */
        pointer at(const iterator_state& itr) const noexcept;

    private:
        /**
         * @brief Returns an iterator referencing the first page.
         */
        iterator begin_itr() const noexcept;

        /**
         * @brief Returns an iterator referencing past the last page.
         */
        iterator end_itr() const noexcept;

        /**
         * @brief Returns a reverse iterator referencing the last page.
         */
        reverse_iterator rend_itr() const noexcept;

        /**
         * @brief Returns a reverse iterator referencing before the first page.
         */
        reverse_iterator rbegin_itr() const noexcept;

    private:
        linked_state* _state;
        pool*         _pool;
    };


    // --------------------------------------------------------------

} }
