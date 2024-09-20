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

#include "container.i.h"


namespace abc { namespace vmem {

    /**
     * @brief    List iterator.
     * @tparam T Item type.
     */
    template <typename T>
    using list_iterator = container_iterator<T, noheader>;


    /**
     * @brief    List const iterator.
     * @tparam T Item type.
     */
    template <typename T>
    using list_const_iterator = container_const_iterator<T, noheader>;


    // --------------------------------------------------------------


    /**
     * @brief    List - supports insert and erase everywhere.
     * @details  Balancing policies: insert - always except on end; erase - always.
     * @tparam T Item type.
     */
    template <typename T>
    class list 
        : public container<T, noheader> {

        using base = container<T, noheader>;
        using diag_base = diag::diag_ready<const char*>;

        static constexpr page_balance balance_insert = ~page_balance::end;
        static constexpr page_balance balance_erase  = page_balance::all;  // A stack would still be kept dense.

    public:
        /**
         * @brief       Constructor.
         * @param state Pointer to a `list_state` instance.
         * @param pool  Pointer to a `pool` instance.
         * @param log   Pointer to a `log_ostream` instance.
         */
        list(list_state* state, vmem::pool* pool, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        list(list<T>&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        list(const list<T>& other) noexcept = default;
    };


    // --------------------------------------------------------------


    /**
     * @brief    Stack iterator.
     * @tparam T Item type.
     */
    template <typename T>
    using stack_iterator = container_iterator<T, noheader>;


    /**
     * @brief    Stack const iterator.
     * @tparam T Item type.
     */
    template <typename T>
    using stack_const_iterator = container_const_iterator<T, noheader>;


    // --------------------------------------------------------------


    /**
     * @brief    Stack - supports insert and erase only at end.
     * @details  Balancing policies: insert - never; erase - never.
     * @tparam T Item type.
     */
    template <typename T>
    class stack
        : public container<T, noheader> {

        using base = container<T, noheader>;
        using diag_base = diag::diag_ready<const char*>;

        static constexpr page_balance balance_insert = page_balance::none;
        static constexpr page_balance balance_erase  = page_balance::none;

    public:
        /**
         * @brief       Constructor.
         * @param state Pointer to a `list_state` instance.
         * @param pool  Pointer to a `pool` instance.
         * @param log   Pointer to a `log_ostream` instance.
         */
        stack(stack_state* state, vmem::pool* pool, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        stack(stack<T>&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        stack(const stack<T>& other) noexcept = default;

    public:
        typename base::result2  insert2(typename base::const_iterator itr, typename base::const_reference item) = delete;
        typename base::iterator insert(typename base::const_iterator itr, typename base::const_reference item) = delete;
        template <typename InputItr>
        typename base::iterator insert(typename base::const_iterator itr, InputItr first, InputItr last) = delete;

        typename base::result2  erase2(typename base::const_iterator itr) = delete;
        typename base::iterator erase(typename base::const_iterator itr) = delete;
        typename base::iterator erase(typename base::const_iterator first, typename base::const_iterator last) = delete;
    };


    // --------------------------------------------------------------


    /**
     * @brief  Facility that clears a container upon its own destruction. 
     * @tparam Container.
     */
    template <typename Container>
    class temp
        : public Container {

    public:
        /**
         * @brief       Constructor.
         * @tparam Args Argument types.
         * @param args  Arguments.
         */
        template <typename... Args>
        temp(Args&&... args);

        /**
         * @brief Move constructor.
         */
        temp(temp<Container>&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        temp(const temp<Container>& other) = delete;

        /**
         * @brief Destructor constructor.
         */
        ~temp() noexcept;
    };


    // --------------------------------------------------------------

} }
