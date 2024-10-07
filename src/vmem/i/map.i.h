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

#include <utility>
#include "list.i.h"


namespace abc { namespace vmem {

    /**
     * @brief      Key-level container. Always balancing all operations.
     * @details    A map/B-tree consists of a value-level container, and a stack of key-level containers.
     * @tparam Key Key type.
     */
    template <typename Key>
    class map_key_level
        : public container<map_key<Key>, noheader> {

        using base = container<map_key<Key>, noheader>;
        using diag_base = diag::diag_ready<const char*>;

        static constexpr page_balance balance_insert = page_balance::all;
        static constexpr page_balance balance_erase  = page_balance::all;

    public:
        /**
         * @brief       Constructor.
         * @param state Pointer to a `container_state` instance.
         * @param pool  Pointer to a `pool` instance.
         * @param log   Pointer to a `log_ostream` instance.
         */
        map_key_level(container_state* state, vmem::pool* pool, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        map_key_level(map_key_level<Key>&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        map_key_level(const map_key_level<Key>& other) noexcept = default;
    };


    /**
     * @brief      Stack of key-level containers. For balancing, see `stack`.
     * @details    A map/B-tree consists of a value-level container, and a stack of key-level containers.
     * @tparam Key Key type.
     */
    template <typename Key>
    class map_key_level_stack
        : public stack<container_state> {

        using base = stack<container_state>;
        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief       Constructor.
         * @param state Pointer to a `stack_state` instance.
         * @param pool  Pointer to a `pool` instance.
         * @param log   Pointer to a `log_ostream` instance.
         */
        map_key_level_stack(stack_state* state, vmem::pool* pool, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        map_key_level_stack(map_key_level_stack<Key>&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        map_key_level_stack(const map_key_level_stack<Key>& other) noexcept = default;
    };


    /**
     * @brief      Value-level container. Always balancing all operations.
     * @details    A map/B-tree consists of a value-level container, and a stack of key-level containers.
     * @tparam Key Key type.
     * @tparam T   Value type.
     */
    template <typename Key, typename T>
    class map_value_level
        : public container<map_value<Key, T>, noheader> {

        using base = container<map_value<Key, T>, noheader>;
        using diag_base = diag::diag_ready<const char*>;

        static constexpr page_balance balance_insert = page_balance::all;
        static constexpr page_balance balance_erase  = page_balance::all;

    public:
        /**
         * @brief       Constructor.
         * @param state Pointer to a `container_state` instance.
         * @param pool  Pointer to a `pool` instance.
         * @param log   Pointer to a `log_ostream` instance.
         */
        map_value_level(container_state* state, vmem::pool* pool, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        map_value_level(map_value_level<Key, T>&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        map_value_level(const map_value_level<Key, T>& other) noexcept = default;
    };


    // --------------------------------------------------------------


    template <typename Key, typename T>
    class map;


    /**
     * @brief      Map iterator state.
     * @tparam Key Key type.
     * @tparam T   Value type.
     */
    template <typename Key, typename T>
    using map_iterator_state = basic_iterator_state<map<Key, T>>;


    /**
     * @brief      Map iterator.
     * @tparam Key Key type.
     * @tparam T   Value type.
     */
    template <typename Key, typename T>
    using map_iterator = iterator<map<Key, T>, map_value<Key, T>>;


    /**
     * @brief      Map const iterator.
     * @tparam Key Key type.
     * @tparam T   Value type.
     */
    template <typename Key, typename T>
    using map_const_iterator = const_iterator<map<Key, T>, map_value<Key, T>>;


    // --------------------------------------------------------------


    /**
     * @brief      Result of insert and erase operations that allows this struct to be included in bigger ones.
     * @tparam Key Key type.
     * @tparam T   Value type.
     */
    template <typename Key, typename T>
    struct map_result2 {
        /**
         * @brief Default-like constructor.
         */
        map_result2(std::nullptr_t) noexcept;

        /**
         * @brief Move constructor.
         */
        map_result2(map_result2&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        map_result2(const map_result2& other) = default;

        /**
         * @brief Move assignment.
         */
        map_result2& operator =(map_result2&& other) noexcept = default;

        /**
         * @brief Operation-specific iterator.
         */
        map_iterator<Key, T> iterator;

        /**
         * @brief `true` = the operation was performed; `false` = the operation was not performed.
         */
        bool ok;
    };


    /**
     * @brief      Result of find operations that allows this struct to be included in bigger ones.
     * @details    The result is a stack of page positions representing the path to the item from the root.
     * @tparam Key Key type.
     * @tparam T   Value type.
     */
    template <typename Key, typename T>
    struct map_find_result2
        : public map_result2<Key, T> {

        /**
         * @brief      Constructor.
         * @param pool Pointer to a `pool` instance.
         * @param log  Pointer to a `log_ostream` instance.
         */
        map_find_result2(vmem::pool* pool, diag::log_ostream* log) noexcept;

        /**
         * @brief Move constructor.
         */
        map_find_result2(map_find_result2&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        map_find_result2(const map_find_result2& other) = delete;

        /**
         * @brief Move assignment.
         */
        map_find_result2& operator =(map_find_result2&& other) noexcept = default;

    private:
        /**
         * @brief State for the path stack.
         */
        stack_state _path_state;

    public:
        /**
         * @brief Stack of page positions representing the path to the item from the root.
         */
        temp<stack<page_pos_t>> path;
    };


    // --------------------------------------------------------------


    /**
     * @brief      Map implemented as a B-tree.
     * @tparam Key Key type.
     * @tparam T   Value type.
     */
    template <typename Key, typename T>
    class map
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;
        using iterator_state = map_iterator_state<Key, T>;

    private:
        static constexpr const char* origin() noexcept;

    public:
        using key_type                 = Key;
        using mapped_type              = T;
        using value_type               = map_value<Key, T>;
        using pointer                  = ptr<map_value<Key, T>>;
        using const_pointer            = ptr<const map_value<Key, T>>;
        using reference                = map_value<Key, T>&;
        using const_reference          = const map_value<Key, T>&;
        using iterator                 = map_iterator<Key, T>;
        using const_iterator           = map_const_iterator<Key, T>;
        using reverse_iterator         = iterator;
        using const_reverse_iterator   = const_iterator;
        using result2                  = map_result2<Key, T>;
        using find_result2             = map_find_result2<Key, T>;
        using iterator_bool            = std::pair<map_iterator<Key, T>, bool>;

    private:
        using path_reverse_iterator    = typename stack<page_pos_t>::reverse_iterator;
        using key_level_stack          = map_key_level_stack<Key>;
        using key_level_stack_iterator = typename map_key_level_stack<Key>::iterator;
        using key_level_iterator       = typename map_key_level<Key>::iterator;
        using key_level_result2        = typename map_key_level<Key>::result2;
        using value_level_container    = map_value_level<Key, T>;
        using value_level_iterator     = typename map_value_level<Key, T>::iterator;
        using value_level_result2      = typename map_value_level<Key, T>::result2;
        using page_lead                = typename map_key_level<Key>::page_lead;

    public:
        /**
         * @brief Returns the byte position on each key page where keys start.
         */
        static constexpr std::size_t key_items_pos() noexcept;

        /**
         * @brief Returns the maximum possible size of a key.
         */
        static constexpr std::size_t max_key_item_size() noexcept;

        /**
         * @brief Returns the maximum number of keys that could be stored on a page.
         */
        static constexpr std::size_t key_page_capacity() noexcept;

        /**
         * @brief Returns the byte position on each value page where values start.
         */
        static constexpr std::size_t value_items_pos() noexcept;

        /**
         * @brief Returns the maximum possible size of a value.
         */
        static constexpr std::size_t max_value_item_size() noexcept;

        /**
         * @brief Returns the maximum number of values that could be stored on a page.
         */
        static constexpr std::size_t value_page_capacity() noexcept;

    public:
        /**
         * @brief       Constructor.
         * @param state Pointer to a `map_state` instance.
         * @param pool  Pointer to a `pool` instance.
         * @param log   Pointer to a `log_ostream` instance.
         */
        map(map_state* state, vmem::pool* pool, diag::log_ostream* log);

        /**
         * @brief Move constructor.
         */
        map(map<Key, T>&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        map(const map<Key, T>& other) noexcept = default;

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
        std::size_t            size() const noexcept;

        /**
         * @brief      Inserts an item.
         * @details    Tries to find the item first.
         *             If it is found, the insert is not performed.
         *             If it is not found, an unconditional insert is performed to the `found_result2` path.
         * @param item Item.
         * @return     `result2`
         */
        result2 insert2(const_reference item);

        /**
         * @brief      Inserts an item.
         * @details    Tries to find the item first.
         *             If it is found, the insert is not performed.
         *             If it is not found, an unconditional insert is performed to the `find_result2` path.
         * @param item Item.
         * @return     `iterator_bool`
         */
        iterator_bool insert(const_reference item);

        /**
         * @brief           Inserts a sequence of items.
         * @tparam InputItr Source iterator type.
         * @param first     Begin source iterator.
         * @param last      End source iterator.
         */
        template <typename InputItr>
        void insert(InputItr first, InputItr last);

    private:
        /**
         * @brief             Unconditionally inserts an item at the `find_result2` path.
         * @param find_result Find result.
         * @param item        Item.
         * @return            `result2` 
         */
        result2 insert2(find_result2&& find_result, const_reference item) noexcept;

    public:
        /**
         * @brief     Erases an item.
         * @details   Tries to find the item first.
         *            If it is not found, the erase is not performed.
         *            If it is found, an unconditional erase is performed to the `find_result2` path.
         * @param key Key of the item to be erased.
         * @return    `1` = the item was erased; `0` = the item was not erased.
         */
        std::size_t erase(const Key& key);

        /**
         * @brief           Erases a sequence of items.
         * @tparam InputItr Source iterator type.
         * @param first     Begin source iterator.
         * @param last      End source iterator.
         */
        template <typename InputItr>
        void erase(InputItr first, InputItr last);

    private:
        /**
         * @brief             Unconditionally erases an item at the `find_result2` path.
         * @param find_result Find result.
         * @return            `1` = the item was erased; `0` = the item was not erased.
         */
        std::size_t erase2(find_result2&& find_result);

    // update_key_levels() helpers
    private:
        /**
         * @brief               Inserts/erases keys throughout key levels as necessary after an insert/erase to the value level has finished.
         * @param is_insert     `true` = insert; `false` = erase.
         * @param find_result   `find_result2`
         * @param values_result `value_level_result_2`
         * @return              `result2`
         */
        result2 update_key_levels(bool is_insert, find_result2&& find_result, value_level_result2&& values_result);

        /**
         * @brief              Returns the position of a key on a key page.
         * @param key_page_pos Key page position.
         * @param key          Key
         */
        item_pos_t key_item_pos(page_pos_t key_page_pos, const Key& key);

    public:
        /**
         * @brief Erases all items.
         */
        void clear();

    private:
        friend iterator_state;
        friend const_iterator;
        friend iterator;

        /**
         * @brief     Returns the iterator immediately following a given one.  
         * @param itr Iterator.
         */
        iterator next(const iterator_state& itr) const;

        /**
         * @brief     Returns the iterator immediately preceding a given one.  
         * @param itr Iterator.
         */
        iterator prev(const iterator_state& itr) const;

    public:
        /**
         * @brief     Finds an item by key.
         * @details   Suitable for use in more complex operations like insert and delete.
         * @param key Key.
         * @return    `find_result2` 
         */
        find_result2 find2(const Key& key);

        /**
         * @brief     Finds an item by key.
         * @details   Suitable for direct use.
         * @param key Key.
         * @return    `iterator` 
         */
        iterator find(const Key& key);

        /**
         * @brief     Finds an item by key.
         * @details   Suitable for direct use.
         * @param key Key.
         * @return    `const_iterator` 
         */
        const_iterator find(const Key& key) const;

        /**
         * @brief     Checks if an item with a key exists.
         * @param key Key.
         * @return    `true` = exists; `false` = does not exist. 
         */
        bool contains(const Key& key) const;

        /**
         * @brief     Finds an item by key, and dereferences it.
         * @param key Key.
         * @return    `pointer` 
         */
        pointer operator [](const Key& key);

        /**
         * @brief     Finds an item by key, and dereferences it.
         * @param key Key.
         * @return    `const_pointer` 
         */
        const_pointer operator [](const Key& key) const;

        /**
         * @brief     Dereferences an iterator.
         * @param itr Iterator.
         * @return    `pointer`
         */
        pointer at(const iterator_state& itr);

        /**
         * @brief     Dereferences an iterator.
         * @param itr Iterator.
         * @return    `const_pointer`
         */
        const_pointer at(const iterator_state& itr) const;

    private:
        /**
         * @brief Returns an iterator referencing the first item.
         */
        iterator begin_itr() const noexcept;

        /**
         * @brief Returns an iterator referencing past the last item.
         */
        iterator end_itr() const noexcept;

        /**
         * @brief Returns a reverse iterator referencing the last item.
         */
        reverse_iterator rend_itr() const noexcept;

        /**
         * @brief Returns a reverse iterator referencing before the first item.
         */
        reverse_iterator rbegin_itr() const noexcept;

        /**
         * @brief Converts a value-level iterator to a map iterator.
         */
        iterator itr_from_values(const value_level_iterator& values_itr) const noexcept;

    private:
        map_state*            _state;
        vmem::pool*           _pool;

    private:
        key_level_stack       _key_stack;
        value_level_container _values;
    };


    // --------------------------------------------------------------

} }
