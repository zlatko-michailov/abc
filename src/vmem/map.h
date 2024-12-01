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

#include <algorithm>

#include "list.h"
#include "i/map.i.h"


namespace abc { namespace vmem {

    template <typename Key>
    map_key_level<Key>::map_key_level(container_state* state, vmem::pool* pool, diag::log_ostream* log)
        : base(state, balance_insert, balance_erase, pool, log) {
    }


    // --------------------------------------------------------------


    template <typename Key>
    map_key_level_stack<Key>::map_key_level_stack(stack_state* state, vmem::pool* pool, diag::log_ostream* log)
        : base(state, pool, log) {
    }


    // --------------------------------------------------------------


    template <typename Key, typename T>
    map_value_level<Key, T>::map_value_level(container_state* state, vmem::pool* pool, diag::log_ostream* log)
        : base(state, balance_insert, balance_erase, pool, log) {
    }


    // --------------------------------------------------------------


    template <typename Key, typename T>
    map_result2<Key, T>::map_result2(std::nullptr_t) noexcept
        : iterator(nullptr)
        , ok(false) {
    }


    // --------------------------------------------------------------


    template <typename Key, typename T>
    map_find_result2<Key, T>::map_find_result2(vmem::pool* pool, diag::log_ostream* log) noexcept
        : map_result2<Key, T>(nullptr)
        , path(&_path_state, pool, log) {
    }


    // --------------------------------------------------------------


    template <typename Key, typename T>
    inline constexpr const char* map<Key, T>::origin() noexcept {
        return "abc::vmem::map";
    }


    template <typename Key, typename T>
    inline constexpr std::size_t map<Key, T>::key_items_pos() noexcept {
        return sizeof(map_key_page<Key>) - sizeof(map_key<Key>);
    }


    template <typename Key, typename T>
    inline constexpr std::size_t map<Key, T>::max_key_item_size() noexcept {
        return page_size - key_items_pos();
    }


    template <typename Key, typename T>
    inline constexpr std::size_t map<Key, T>::key_page_capacity() noexcept {
        return max_key_item_size() / sizeof(map_key<Key>);
    }


    template <typename Key, typename T>
    inline constexpr std::size_t map<Key, T>::value_items_pos() noexcept {
        return sizeof(map_value_page<Key, T>) - sizeof(map_value<Key, T>);
    }


    template <typename Key, typename T>
    inline constexpr std::size_t map<Key, T>::max_value_item_size() noexcept {
        return page_size - value_items_pos();
    }


    template <typename Key, typename T>
    inline constexpr std::size_t map<Key, T>::value_page_capacity() noexcept {
        return max_value_item_size() / sizeof(map_value<Key, T>);
    }


    template <typename Key, typename T>
    inline map<Key, T>::map(map_state* state, vmem::pool* pool, diag::log_ostream* log)
        : diag_base(abc::copy(origin()), log)
        , _state(state)
        , _pool(pool)
        , _key_stack(&state->keys, pool, log)
        , _values(&state->values, pool, log) {

        constexpr const char* suborigin = "map()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1050a, "Begin: state=%p', balance_insert=%x, balance_erase=%x, pool=%p", state, pool);

        diag_base::expect(suborigin, state != nullptr, 0x1050b, "state != nullptr");
        diag_base::expect(suborigin, pool != nullptr, 0x1050c, "pool != nullptr");
        diag_base::expect(suborigin, sizeof(map_key<Key>) <= max_key_item_size(), 0x1050d, "sizeof(map_key<Key>) <= max_key_item_size()");
        diag_base::expect(suborigin, sizeof(map_value<Key, T>) <= max_value_item_size(), 0x1050e, "sizeof(map_value<Key, T>) <= max_value_item_size()");
        diag_base::expect(suborigin, key_page_capacity() >= 2, 0x1050f, "key_page_capacity() >= 2");
        diag_base::expect(suborigin, !key_level_stack::is_uninit(&_state->keys), __TAG__, "!key_level_stack::is_uninit(_state->keys)");
        diag_base::expect(suborigin, !value_level_container::is_uninit(&_state->values), __TAG__, "!value_level_container::is_uninit(_state->values)");
        diag_base::expect(suborigin, _state->keys.item_size == sizeof(container_state), 0x10511, "_state->keys.item_size == sizeof(container_state)");
        diag_base::expect(suborigin, _state->values.item_size == sizeof(map_value<Key, T>), 0x10512, "_state->values.item_size == sizeof(map_value<Key, T>)");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10513, "End: keys.front_page_pos=0x%llx, keys.back_page_pos=0x%llx,  values.front_page_pos=0x%llx, values.back_page_pos=0x%llx", 
                (unsigned long long)_state->keys.front_page_pos, (unsigned long long)_state->keys.back_page_pos, (unsigned long long)_state->values.front_page_pos, (unsigned long long)_state->values.back_page_pos);
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::iterator map<Key, T>::begin() noexcept {
        return begin_itr();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::const_iterator map<Key, T>::begin() const noexcept {
        return begin_itr();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::const_iterator map<Key, T>::cbegin() const noexcept {
        return begin_itr();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::iterator map<Key, T>::end() noexcept {
        return end_itr();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::const_iterator map<Key, T>::end() const noexcept {
        return end_itr();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::const_iterator map<Key, T>::cend() const noexcept {
        return end_itr();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::iterator map<Key, T>::rend() noexcept {
        return rend_itr();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::const_iterator map<Key, T>::rend() const noexcept {
        return rend_itr();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::const_iterator map<Key, T>::crend() const noexcept {
        return rend_itr();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::iterator map<Key, T>::rbegin() noexcept {
        return rbegin_itr();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::const_iterator map<Key, T>::rbegin() const noexcept {
        return rbegin_itr();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::const_iterator map<Key, T>::crbegin() const noexcept {
        return rbegin_itr();
    }


    template <typename Key, typename T>
    inline bool map<Key, T>::empty() const noexcept {
        return _state->values.front_page_pos == page_pos_nil
            || _state->values.back_page_pos == page_pos_nil;
    }


    template <typename Key, typename T>
    inline std::size_t map<Key, T>::size() const noexcept {
        return _state->values.total_item_count;
    }


    // ..............................................................


    template <typename Key, typename T>
    inline typename map<Key, T>::result2 map<Key, T>::insert2(const_reference item) {
        constexpr const char* suborigin = "insert2(item)";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10514, "Begin:");

        find_result2 find_result = find2(item.key);
        result2 result(nullptr);

        if (!find_result.ok) {
            diag_base::put_any(suborigin, diag::severity::optional, 0x10515, "Not found. Inserting.");

            result = insert2(std::move(find_result), item);
        }
        else {
            diag_base::put_any(suborigin, diag::severity::optional, 0x10516, "Found. Bailing.");

            result.iterator = find_result.iterator;
            result.ok = false;
        }

        diag_base::ensure(suborigin, result.iterator.can_deref(), __TAG__, "result.iterator.can_deref()");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10517, "End: result.ok=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
                result.ok, (unsigned long long)result.iterator.page_pos(), (unsigned)result.iterator.item_pos(), result.iterator.edge());

        return result;
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::iterator_bool map<Key, T>::insert(const_reference item) {
        result2 result = insert2(item);

        return std::make_pair(result.iterator, result.ok);
    }


    template <typename Key, typename T>
    template <typename InputItr>
    inline void map<Key, T>::insert(InputItr first, InputItr last) {
        constexpr const char* suborigin = "insert(first, last)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        for (InputItr item_itr = first; item_itr != last; item_itr++) {
            bool inserted = insert(*item_itr).second;
            diag_base::ensure(suborigin, !inserted, 0x10518, "!inserted");
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::result2 map<Key, T>::insert2(find_result2&& find_result, const_reference item) noexcept {
        constexpr const char* suborigin = "insert2(find_result, item)";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10519, "Begin: find_result.iterator.page_pos=0x%llx, find_result.iterator.item_pos=0x%x, find_result.iterator.edge=%d",
                (unsigned long long)find_result.iterator.page_pos(), (unsigned)find_result.iterator.item_pos(), find_result.iterator.edge());

        diag_base::expect(suborigin, find_result.iterator.is_valid(this), __TAG__, "find_result.iterator.is_valid(this)");
        diag_base::expect(suborigin, find_result.iterator.can_deref() || find_result.iterator == end_itr(), __TAG__, "find_result.iterator.can_deref() || find_result.iterator == end_itr()");

        value_level_iterator values_itr(&_values, find_result.iterator.page_pos(), find_result.iterator.item_pos(), find_result.iterator.edge(), diag_base::log());

        value_level_result2 values_result = _values.insert2(values_itr, item);
        diag_base::expect(suborigin, values_result.iterator.is_valid(&_values), __TAG__, "values_result.iterator.is_valid(&_values)");
        diag_base::expect(suborigin, values_result.iterator.can_deref(), __TAG__, "values_result.iterator.can_deref()");

        result2 result = update_key_levels(true /*is_insert*/, std::move(find_result), std::move(values_result));
        diag_base::expect(suborigin, result.ok, __TAG__, "result.ok");
        diag_base::expect(suborigin, result.iterator.is_valid(this), __TAG__, "result.iterator.is_valid(this)");
        diag_base::expect(suborigin, result.iterator.can_deref(), __TAG__, "result.iterator.can_deref()");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1051a, "End: result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%d",
                (unsigned long long)result.iterator.page_pos(), (unsigned)result.iterator.item_pos(), result.iterator.edge());

        return result;
    }


    // ..............................................................


    template <typename Key, typename T>
    inline std::size_t map<Key, T>::erase(const Key& key) {
        constexpr const char* suborigin = "erase(key)";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1051b, "Begin:");

        std::size_t result = 0;

        find_result2 find_result = find2(key);

        if (find_result.ok) {
            diag_base::put_any(suborigin, diag::severity::optional, 0x1051c, "Found. iterator.page_pos=0x%llx, iterator.item_pos=0x%x, iterator.edge=%d",
                    (long long)find_result.iterator.page_pos(), find_result.iterator.item_pos(), find_result.iterator.edge());

            result = erase2(std::move(find_result));
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1051d, "End: result=%zu", result);

        return result;
    }


    template <typename Key, typename T>
    template <typename InputItr>
    inline void map<Key, T>::erase(InputItr first, InputItr last) {
        constexpr const char* suborigin = "erase(first, last)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        for (InputItr item_itr = first; item_itr != last; item_itr++) {
            std::size_t result = erase(*item_itr);

            diag_base::put_any(suborigin, diag::severity::optional, __TAG__, "result=%zu", result);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename Key, typename T>
    inline std::size_t map<Key, T>::erase2(find_result2&& find_result) {
        constexpr const char* suborigin = "erase(find_result2)";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1051e, "Begin:");

        diag_base::expect(suborigin, find_result.iterator.is_valid(this), __TAG__, "find_result.iterator.is_valid(this)");
        diag_base::expect(suborigin, find_result.iterator.can_deref(), __TAG__, "find_result.iterator.can_deref()");

        value_level_iterator values_itr(&_values, find_result.iterator.page_pos(), find_result.iterator.item_pos(), find_result.iterator.edge(), diag_base::log());

        value_level_result2 values_result = _values.erase2(values_itr);
        diag_base::expect(suborigin, values_result.iterator.is_valid(&_values), __TAG__, "values_result.iterator.is_valid(&_values)");
        values_itr = values_result.iterator;

        result2 result = update_key_levels(false /*is_insert*/, std::move(find_result), std::move(values_result));
        diag_base::expect(suborigin, result.iterator.is_valid(this), __TAG__, "result.iterator.is_valid(this)");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1051f, "End: values_itr.page_pos=0x%llx, values_itr.item_pos=0x%x, values_itr.edge=%d",
                (unsigned long long)values_itr.page_pos(), (unsigned)values_itr.item_pos(), values_itr.edge());

        return 1;
    }


    // ..............................................................


    template <typename Key, typename T>
    inline typename map<Key, T>::result2 map<Key, T>::update_key_levels(bool is_insert, find_result2&& find_result, value_level_result2&& values_result) {
        constexpr const char* suborigin = "update_key_levels";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10520, "Begin:");

        bool should_update_key_levels = _state->values.front_page_pos != _state->values.back_page_pos || _key_stack.size() > 0;

        if (should_update_key_levels
            && (values_result.page_leads[0].operation != container_page_lead_operation::none || values_result.page_leads[1].operation != container_page_lead_operation::none)) {
            diag_base::expect(suborigin, _key_stack.size() == find_result.path.size(), 0x10521, "_key_stack.size(%zu) == find_result.path.size(%zu)", _key_stack.size(), find_result.path.size());

            key_level_stack_iterator key_stack_itr = _key_stack.begin();
            path_reverse_iterator path_itr = find_result.path.rend();

            page_lead page_leads[] = { values_result.page_leads[0], values_result.page_leads[1] };

            // If a new level is to be added, the leading key on the root page will have to be inserted to the new root page.
            // Initialize with the leading key on the leading value page, in case there are no key levels.
            page_pos_t root_page_pos = _state->values.front_page_pos;
            Key root_key{ };
            {
                vmem::page front_value_page(_pool, root_page_pos, diag_base::log());
                diag_base::expect(suborigin, front_value_page.pos() == root_page_pos, __TAG__, "front_value_page.pos() == root_page_pos");
                diag_base::expect(suborigin, front_value_page.ptr() != nullptr, __TAG__, "front_value_page.ptr() != nullptr");

                map_value_page<Key, T>* front_value_container_page = reinterpret_cast<map_value_page<Key, T>*>(front_value_page.ptr());
                std::memmove(&root_key, &front_value_container_page->items[0].key, sizeof(Key));
            }

            // While there is rebalance, keep going back the path (and up the levels).
            while ((page_leads[0].operation != container_page_lead_operation::none || page_leads[1].operation != container_page_lead_operation::none)
                    && key_stack_itr != _key_stack.end()
                    && path_itr != find_result.path.rbegin()) {
                // IMPORTANT: Save the ptr instance to keep the page locked.
                vmem::ptr<container_state> key_level_state_ptr = key_stack_itr.operator->();

                // Get the parent keys container.
                map_key_level<Key> parent_keys(key_level_state_ptr.operator->(), _pool, diag_base::log());
                page_pos_t parent_page_pos = *path_itr;

                // Update the root key and page page with the leading key and page on the current key level.
                {
                    root_page_pos = key_level_state_ptr->front_page_pos;
                    vmem::page front_key_page(_pool, root_page_pos, diag_base::log());
                    diag_base::expect(suborigin, front_key_page.pos() == root_page_pos, __TAG__, "front_key_page.pos() == root_page_pos");
                    diag_base::expect(suborigin, front_key_page.ptr() != nullptr, __TAG__, "front_key_page.ptr() != nullptr");

                    map_key_page<Key>* front_key_container_page = reinterpret_cast<map_key_page<Key>*>(front_key_page.ptr());
                    std::memmove(&root_key, &front_key_container_page->items[0].key, sizeof(Key));
                }

                page_lead parent_page_leads[2];
                std::size_t parent_page_leads_count = 0;

                for (std::size_t i = 0; i < 2; i++) {
                    switch (page_leads[i].operation) {
                        case container_page_lead_operation::replace: {
                            // Find the old key.
                            key_level_iterator parent_keys_first_itr(&parent_keys, parent_page_pos, 0, iterator_edge::none, diag_base::log());
                            key_level_iterator parent_keys_itr = std::find_if(parent_keys_first_itr, parent_keys.end(), [&page_leads, i] (const vmem::map_key<Key>& map_key) { return page_leads[i].items[1].key == map_key.key; }); // Old key
                            diag_base::expect(suborigin, parent_keys_itr.can_deref(), __TAG__, "parent_keys_itr.can_deref()");

                            // Overwrite with the new key.
                            ptr<map_key<Key>> key_ptr = parent_keys_itr.operator->();
                            std::memmove(&key_ptr->key, &page_leads[i].items[0].key, sizeof(Key)); // New key

                            // If the old key was leading, keep the existing page_lead.
                            if (parent_keys_itr.item_pos() == 0) {
                                parent_page_leads[parent_page_leads_count++] = page_leads[i];
                            }
                        }
                        break;

                        case container_page_lead_operation::insert: {
                            // Find the iterator for the new key.
                            key_level_iterator parent_keys_first_itr(&parent_keys, parent_page_pos, 0, iterator_edge::none, diag_base::log());
                            key_level_iterator parent_keys_itr = std::find_if(parent_keys_first_itr, parent_keys.end(), [&page_leads, i] (const vmem::map_key<Key>& map_key) { return page_leads[i].items[0].key < map_key.key; }); // New key

                            map_key<Key> key_item;
                            std::memmove(&key_item.key, &page_leads[i].items[0].key, sizeof(Key));
                            key_item.page_pos = page_leads[i].page_pos;

                            key_level_result2 keys_result = parent_keys.insert2(parent_keys_itr, key_item);

                            if (keys_result.page_leads[0].operation != container_page_lead_operation::none) {
                                parent_page_leads[parent_page_leads_count++] = keys_result.page_leads[0];
                            }
                            if (keys_result.page_leads[1].operation != container_page_lead_operation::none) {
                                diag_base::expect(suborigin, parent_page_leads[parent_page_leads_count].operation == container_page_lead_operation::none, __TAG__, "suborigin, parent_page_leads[%zu].operation == container_page_lead_operation::none", parent_page_leads_count);
                                parent_page_leads[parent_page_leads_count++] = keys_result.page_leads[1];
                            }
                        }
                        break;

                        case container_page_lead_operation::erase: {
                            // Find the iterator for the old key.
                            key_level_iterator parent_keys_first_itr(&parent_keys, parent_page_pos, 0, iterator_edge::none, diag_base::log());
                            key_level_iterator parent_keys_itr = std::find_if(parent_keys_first_itr, parent_keys.end(), [&page_leads, i] (const vmem::map_key<Key>& map_key) { return page_leads[i].items[0].key == map_key.key; }); // Old key

                            key_level_result2 keys_result = parent_keys.erase2(parent_keys_itr);

                            if (keys_result.page_leads[0].operation != container_page_lead_operation::none) {
                                parent_page_leads[parent_page_leads_count++] = keys_result.page_leads[0];
                            }
                            if (keys_result.page_leads[1].operation != container_page_lead_operation::none) {
                                diag_base::expect(suborigin, parent_page_leads[parent_page_leads_count].operation == container_page_lead_operation::none, __TAG__, "suborigin, parent_page_leads[%zu].operation == container_page_lead_operation::none", parent_page_leads_count);
                                parent_page_leads[parent_page_leads_count++] = keys_result.page_leads[1];
                            }
                        }
                        break;

                        default:
                            diag_base::expect(suborigin, page_leads[i].operation == container_page_lead_operation::none, __TAG__, "page_leads[%zu].operation(%u) == container_page_lead_operation::none", i, page_leads[i].operation);
                        break;
                    }
                }

                // The parent page leads become current when we move up to the parent level.
                page_leads[0] = parent_page_leads[0];
                page_leads[1] = parent_page_leads[1];

#if 0 //// TODO:
                key_level_result2 keys_result;
                if (is_insert) {
                    // page_leads[0] - insert; new page
                    // page_leads[1] - original; used only when a new level is created
                    diag_base::expect(suborigin, page_leads[0].operation == container_page_lead_operation::insert, __TAG__, "page_leads[0].operation == container_page_lead_operation::insert");
                    
                    item_pos_t parent_item_pos = key_item_pos(parent_page_pos, page_leads[0].items[0].key);
                    diag_base::expect(suborigin, parent_item_pos != item_pos_nil, __TAG__, "parent_item_pos != item_pos_nil");

                    // Since the item is being inserted, it must not exist, nevertheless on a key page.
                    // Therefore, parent_item_pos must be incremented.
                    key_level_iterator parent_keys_itr(&parent_keys, parent_page_pos, ++parent_item_pos, iterator_edge::none, diag_base::log());

                    map_key<Key> key_item;
                    std::memmove(&key_item.key, &page_leads[0].items[0].key, sizeof(Key));
                    key_item.page_pos = page_leads[0].page_pos;

                    keys_result = parent_keys.insert2(parent_keys_itr, key_item);
                }
                else {
                    // page_leads[0] - replace or none; doesn't create new leads
                    // page_leads[1] - erase
                    if (page_leads[0].operation == container_page_lead_operation::replace) {
                        item_pos_t parent_item_pos = key_item_pos(parent_page_pos, page_leads[0].items[0].key);
                        diag_base::expect(suborigin, parent_item_pos != item_pos_nil, __TAG__, "parent_item_pos != item_pos_nil");
    
                        key_level_iterator parent_keys_itr(&parent_keys, parent_page_pos, parent_item_pos, iterator_edge::none, diag_base::log());
                        if (parent_keys_itr.can_deref()) {
                            ptr<map_key<Key>> key_ptr = parent_keys_itr.operator->();
                            std::memmove(&key_ptr->key, &page_leads[0].items[1].key, sizeof(Key));
                        }

                        keys_result.page_leads[0] = page_leads[0];
                        keys_result.page_leads[1] = page_leads[1];
                    }

                    //// TODO: Why is lead[1] 'none' when erase 0x04?
                    if (page_leads[1].operation == container_page_lead_operation::erase) {
                        item_pos_t parent_item_pos = key_item_pos(parent_page_pos, page_leads[1].items[0].key);
                        diag_base::expect(suborigin, parent_item_pos != item_pos_nil, __TAG__, "parent_item_pos != item_pos_nil");

                        key_level_iterator parent_keys_itr(&parent_keys, parent_page_pos, parent_item_pos, iterator_edge::none, diag_base::log());

                        keys_result = parent_keys.erase2(parent_keys_itr); //// TODO: Should not overwrite lead[0]
                    }
                }

                page_leads[0] = keys_result.page_leads[0];
                page_leads[1] = keys_result.page_leads[1];
#endif

                key_stack_itr++;
                path_itr--;
            } // while (rebalance)

            // If there is still a rebalance, then a key level at the top has to be added.
            for (std::size_t i = 0; i < 2; i++) {
                switch (page_leads[i].operation) {
                    case container_page_lead_operation::insert: {
                        container_state new_keys_state;
                        map_key_level<Key> new_keys(&new_keys_state, _pool, diag_base::log());

                        // Lead/root key and page.
                        // If the other lead is a replace, that should be the new root key.
                        // Otherwise, the saved root key from above should be used.
                        map_key<Key> lead_key_item;
                        lead_key_item.page_pos = root_page_pos;
                        if (page_leads[1 - i].operation == container_page_lead_operation::replace) {
                            std::memmove(&lead_key_item.key, &page_leads[1 - i].items[0].key, sizeof(Key)); // New key
                        }
                        else {
                            std::memmove(&lead_key_item.key, &root_key, sizeof(Key)); 
                        }
                        new_keys.push_back(std::move(lead_key_item));

                        // New page
                        map_key<Key> new_key_item;
                        std::memmove(&new_key_item.key, &page_leads[i].items[0].key, sizeof(Key)); // New key
                        new_key_item.page_pos = page_leads[i].page_pos;
                        new_keys.push_back(std::move(new_key_item));

                        _key_stack.push_back(std::move(new_keys_state));
                    }
                    break;

                    case container_page_lead_operation::erase: {
                        // Handled below.
                    }
                    break;

                    case container_page_lead_operation::replace: {
                        // This is processed as part of insert or below for erase.
                    }
                    break;

                    default:
                        diag_base::expect(suborigin, page_leads[i].operation == container_page_lead_operation::none, __TAG__, "page_leads[%zu].operation(%u) == container_page_lead_operation::none", i, page_leads[i].operation);
                    break;
                }
            }

            // If there is a single key left on the top-level page, that page has to be removed.
            if (!_key_stack.empty()) {
                container_state top_keys_state = _key_stack.back();
                map_key_level<Key> top_keys(&top_keys_state, _pool, diag_base::log());

                if (top_keys.size() == 1) {
                    _key_stack.pop_back();
                }
            }

#if 0
            if (is_insert) {
                // If there is still a rebalance, then a key level at the top has to be added.
                if (page_leads[0].page_pos != page_pos_nil) {
                    container_state new_keys_state;
                    map_key_level<Key> new_keys(&new_keys_state, _pool, diag_base::log());

                    // original
                    map_key<Key> other_key_item;
                    std::memmove(&other_key_item.key, &page_leads[1].items[0].key, sizeof(Key));
                    other_key_item.page_pos = page_leads[1].page_pos;
                    new_keys.push_back(std::move(other_key_item));

                    // new page
                    map_key<Key> new_key_item;
                    std::memmove(&new_key_item.key, &page_leads[0].items[0].key, sizeof(Key));
                    new_key_item.page_pos = page_leads[0].page_pos;
                    new_keys.push_back(std::move(new_key_item));

                    _key_stack.push_back(std::move(new_keys_state));
                }
            }
            else {
                // If there is a single key left on the top-level page, that page has to be removed.
                if (!_key_stack.empty()) {
                    std::size_t top_keys_size = 2;
                    {
                        container_state top_keys_state = _key_stack.back();
                        map_key_level<Key> top_keys(&top_keys_state, _pool, diag_base::log());
                        top_keys_size = top_keys.size();
                    }

                    if (top_keys_size == 1) {
                        _key_stack.pop_back();
                    }
                }
            }
#endif

            diag_base::put_any(suborigin, diag::severity::optional, 0x10523, "key_stack.size=%zu", _key_stack.size());
        }

        result2 result(nullptr);
        result.iterator = iterator(this, values_result.iterator.page_pos(), values_result.iterator.item_pos(), values_result.iterator.edge(), diag_base::log());
        result.ok = true;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10524, "End: ok=%d, iterator.page_pos=0x%llx, iterator.item_pos=0x%x, iterator.edge=%d",
                result.ok, (unsigned long long)result.iterator.page_pos(), (unsigned)result.iterator.item_pos(), result.iterator.edge());

        return result;
    }


    template <typename Key, typename T>
    inline item_pos_t map<Key, T>::key_item_pos(page_pos_t key_page_pos, const Key& key) {
        constexpr const char* suborigin = "key_item_pos(page_pos)";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10525, "Begin: key_page_pos=0x%llx, key=0x%llx...", (unsigned long long)key_page_pos, *(unsigned long long*)&key);

        vmem::page page(_pool, key_page_pos, diag_base::log());
        diag_base::expect(suborigin, page.pos() == key_page_pos, __TAG__, "page.pos() == key_page_pos");
        diag_base::expect(suborigin, page.ptr() != nullptr, 0x10526, "page.ptr() != nullptr");

        map_key_page<Key>* key_page = reinterpret_cast<map_key_page<Key>*>(page.ptr());
        item_pos_t item_pos = key_item_pos(key_page, key);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10528, "End: item_pos=0x%x", (unsigned)item_pos);

        return item_pos;
    }


    template <typename Key, typename T>
    inline item_pos_t map<Key, T>::key_item_pos(map_key_page<Key>* key_page, const Key& key) {
        constexpr const char* suborigin = "key_item_pos(key_page)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: key_page=%p, key=0x%llx...", key_page, *(unsigned long long*)&key);

        diag_base::expect(suborigin, key_page != nullptr, __TAG__, "key_page != nullptr");

        // Key page: when done, item_pos should reference the biggest key that is smaller or equal to key.
        item_pos_t item_pos = 0;
        for (item_pos_t i = 1; i < key_page->item_count && key_page->items[i].key <= key; i++) {
            diag_base::put_any(suborigin, diag::severity::verbose, __TAG__, "item[%u]=0x%llx..., key=0x%llx...", (unsigned)i, *(unsigned long long*)&key_page->items[i].key, *(unsigned long long*)&key);

            item_pos = i;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: item_pos=0x%x", (unsigned)item_pos);

        return item_pos;
    }


    // ..............................................................


    template <typename Key, typename T>
    inline void map<Key, T>::clear() {
        constexpr const char* suborigin = "clear";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        for (key_level_stack_iterator key_stack_itr = _key_stack.rend(); key_stack_itr != _key_stack.rbegin(); key_stack_itr--) {
            // IMPORTANT: Save the ptr instance to keep the page locked.
            vmem::ptr<container_state> key_level_state_ptr = key_stack_itr.operator->();

            map_key_level<Key> keys(key_level_state_ptr.operator->(), _pool, diag_base::log());
            keys.clear();
        }

        _key_stack.clear();

        _values.clear();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // ..............................................................


    template <typename Key, typename T>
    inline typename map<Key, T>::iterator map<Key, T>::next(const iterator_state& itr) const {
        constexpr const char* suborigin = "next";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10529, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
                (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), itr.edge());

        diag_base::expect(suborigin, itr.is_valid(this), __TAG__, "itr.is_valid(this)");
        diag_base::expect(suborigin, itr.is_rbegin() || itr.can_deref(), __TAG__, "itr.is_rbegin() || itr.can_deref()");

        value_level_iterator values_itr(&_values, itr.page_pos(), itr.item_pos(), itr.edge(), diag_base::log());
        values_itr++;
        iterator result = iterator(this, values_itr.page_pos(), values_itr.item_pos(), values_itr.edge(), diag_base::log());

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1052a, "End: result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
                (unsigned long long)result.page_pos(), (unsigned)result.item_pos(), result.edge());

        return result;
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::iterator map<Key, T>::prev(const iterator_state& itr) const {
        constexpr const char* suborigin = "prev";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1052b, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
                (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), itr.edge());

        diag_base::expect(suborigin, itr.is_valid(this), __TAG__, "itr.is_valid(this)");
        diag_base::expect(suborigin, itr.is_rbegin() || itr.can_deref(), __TAG__, "itr.is_rbegin() || itr.can_deref()");

        value_level_iterator values_itr(&_values, itr.page_pos(), itr.item_pos(), itr.edge(), diag_base::log());
        values_itr--;
        iterator result = iterator(this, values_itr.page_pos(), values_itr.item_pos(), values_itr.edge(), diag_base::log());

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1052c, "End: result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
                (unsigned long long)result.page_pos(), (unsigned)result.item_pos(), result.edge());

        return result;
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::find_result2 map<Key, T>::find2(const Key& key) {
        constexpr const char* suborigin = "find2";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1052d, "Begin:");

        page_pos_t page_pos = page_pos_nil;
        item_pos_t item_pos = item_pos_nil;
        bool is_found = false;
        bool is_found_final = false;

        find_result2 result(_pool, diag_base::log());

        if (!_key_stack.empty()) {
            // There are key levels.

            // There must be a single root page.
            diag_base::expect(suborigin, _key_stack.back().front_page_pos == _key_stack.back().back_page_pos, __TAG__, "_key_stack.back().front_page_pos == _key_stack.back().back_page_pos");

            // Start with the root page.
            page_pos = _key_stack.back().front_page_pos;
            diag_base::expect(suborigin, page_pos != page_pos_nil, __TAG__, "page_pos != page_pos_nil");

            // Push the root page into the path.
            result.path.push_back(page_pos);
            diag_base::put_any(suborigin, diag::severity::optional, 0x1052e, "Loop key levels=%zu, Add root page_pos=0x%llx", _key_stack.size(), (unsigned long long)page_pos);

            // From the current/parent page, find the child page (on the next level).
            for (std::size_t level = 0; level < _key_stack.size(); level++) {
                diag_base::expect(suborigin, page_pos != page_pos_nil, __TAG__, "page_pos != page_pos_nil");

                vmem::page page(_pool, page_pos, diag_base::log());
                diag_base::expect(suborigin, page.pos() == page_pos, __TAG__, "page.pos() == page_pos");
                diag_base::expect(suborigin, page.ptr() != nullptr, 0x1052f, "page.ptr() != nullptr");

                map_key_page<Key>* key_page = reinterpret_cast<map_key_page<Key>*>(page.ptr());
                diag_base::put_any(suborigin, diag::severity::optional, 0x10530, "Examine key lev=%zu, page_pos=0x%llx", level, (unsigned long long)page.pos());

                // Find the key on the key page.
                item_pos = key_item_pos(key_page, key);

                //// TODO: This logic is not needed.
                // If key is smaller than the smallest key in the tree, 0 will be returned.
                // In that case, if key is strictly smaller than item[0], this search is finalized as 'not found'.
                /*if (item_pos == 0 && key < key_page->items[0].key) {
                    is_found = false;
                    is_found_final = true;
                }*/
                diag_base::expect(suborigin, item_pos < key_page->item_count, __TAG__, "item_pos < key_page->item_count");

                // Child page pos.
                page_pos = key_page->items[item_pos].page_pos;
                diag_base::put_any(suborigin, diag::severity::optional, 0x10533, "Child page_pos=0x%llx", (unsigned long long)page_pos);

                // The page on the leaf level is a value page. It should not be on the path. The pages from all other levels should be.
                if (level != _key_stack.size() - 1) {
                    result.path.push_back(page_pos);
                    diag_base::put_any(suborigin, diag::severity::optional, __TAG__, "Push page_pos=0x%llx", (unsigned long long)page_pos);
                }
            }
        }
        else {
            // There are no key levels. There must be at most 1 value page.
            diag_base::expect(suborigin, _state->values.front_page_pos == _state->values.back_page_pos, __TAG__, "_state->values.front_page_pos == _state->values.back_page_pos");
            diag_base::put_any(suborigin, diag::severity::optional, 0x10534, "No key levels. value page_pos=0x%llx", (unsigned long long)_state->values.front_page_pos);
            page_pos = _state->values.front_page_pos;
        }

        // page_pos is nil when the structure is empty. Otherwise, it is not.
        diag_base::expect(suborigin, page_pos != page_pos_nil || empty(), __TAG__, "page_pos != page_pos_nil || empty()");

        if (page_pos != page_pos_nil) {
            // The leaf page is a value page.
            vmem::page page(_pool, page_pos, diag_base::log());
            diag_base::expect(suborigin, page.pos() == page_pos, __TAG__, "page.pos() == page_pos");
            diag_base::expect(suborigin, page.ptr() != nullptr, 0x10535, "page.ptr() != nullptr");

            map_value_page<Key, T>* value_page = reinterpret_cast<map_value_page<Key, T>*>(page.ptr());

            // Value page: when done, item_pos should reference the smallest key that is bigger or equal to key.
            item_pos = value_page->item_count;
            for (item_pos_t i = value_page->item_count - 1; 0 <= i && i < value_page->item_count && key <= value_page->items[i].key; i--) {
                diag_base::put_any(suborigin, diag::severity::verbose, __TAG__, "item[%u]=0x%llx..., key=0x%llx...", (unsigned)i, *(unsigned long long*)&value_page->items[i].key, *(unsigned long long*)&key);

                item_pos = i;
            }

            if (!is_found_final) {
                is_found = item_pos < value_page->item_count && value_page->items[item_pos].key == key;
            }

            result.iterator = iterator(this, page_pos, item_pos, iterator_edge::none, diag_base::log());

            diag_base::put_any(suborigin, diag::severity::optional, 0x10536, "Value item_pos=%u, is_found=%d", (unsigned)item_pos, is_found);
        }
        else {
            result.iterator = end_itr();
        }

        result.ok = is_found;

        diag_base::put_any(suborigin, diag::severity::optional, 0x10536, "End:  result.ok=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
                result.ok, (unsigned long long)result.iterator.page_pos(), (unsigned)result.iterator.item_pos(), result.iterator.edge());

        return result;
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::iterator map<Key, T>::find(const Key& key) {
        result2 result = find2(key);
        return result.ok ? result.iterator : end_itr();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::const_iterator map<Key, T>::find(const Key& key) const {
        return const_cast<map<Key, T>>(this)->find(key);
    }


    template <typename Key, typename T>
    inline bool map<Key, T>::contains(const Key& key) const {
        return find(key).can_deref();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::pointer map<Key, T>::operator [](const Key& key) {
        return find(key).operator->();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::const_pointer map<Key, T>::operator [](const Key& key) const {
        return const_cast<map<Key, T>*>(this)->operator[](key);
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::pointer map<Key, T>::at(const iterator_state& itr) {
        value_level_iterator values_itr(&_values, itr.page_pos(), itr.item_pos(), itr.edge(), diag_base::log());
        return values_itr.operator->();
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::const_pointer map<Key, T>::at(const iterator_state& itr) const {
        return const_cast<map<Key, T>*>(this)->at(itr);
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::iterator map<Key, T>::begin_itr() const noexcept {
        return itr_from_values(_values.begin());
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::iterator map<Key, T>::end_itr() const noexcept {
        return itr_from_values(_values.end());
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::reverse_iterator map<Key, T>::rend_itr() const noexcept {
        return itr_from_values(_values.rend());
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::reverse_iterator map<Key, T>::rbegin_itr() const noexcept {
        return itr_from_values(_values.rbegin());
    }


    template <typename Key, typename T>
    inline typename map<Key, T>::iterator map<Key, T>::itr_from_values(const value_level_iterator& values_itr) const noexcept {
        constexpr const char* suborigin = "itr_from_values";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        iterator itr(this, values_itr.page_pos(), values_itr.item_pos(), values_itr.edge(), diag_base::log());

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10537, "End: page_pos=0x%llx, item_pos=0x%x, edge=%u",
                (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), itr.edge());

        return itr;
    }


    template <typename Key, typename T>
    inline void map<Key, T>::log_internals(std::function<std::string(const Key&)>&& format_key, diag::severity_t severity) {
        constexpr const char* suborigin = "log_internals";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        // Log key levels.
        if (!_key_stack.empty()) {
            std::size_t level = 0;
            for (typename key_level_stack::iterator key_stack_itr = _key_stack.rend(); key_stack_itr != _key_stack.rbegin(); key_stack_itr--, level++) {
                diag_base::put_any(suborigin, severity, __TAG__, "key level=%zu:", level);

                // IMPORTANT: Save the ptr instance to keep the page locked.
                vmem::ptr<container_state> key_level_state_ptr = key_stack_itr.operator->();

                map_key_level<Key> keys(key_level_state_ptr.operator->(), _pool, diag_base::log());
                for (typename map_key_level<Key>::const_iterator keys_itr = keys.cbegin(); keys_itr != keys.cend(); keys_itr++) {
                    std::string key_str(format_key(keys_itr->key));
                    diag_base::put_any(suborigin, severity, __TAG__, "  [ page_pos=0x%llx : item_pos=0x%x ] key>='%s' -> page_pos=0x%llx", 
                        (unsigned long long)keys_itr.page_pos(), (unsigned)keys_itr.item_pos(), key_str.c_str(), (unsigned long long)keys_itr->page_pos);
                }
            }
        }

        // Log value level.
        {
            diag_base::put_any(suborigin, severity, __TAG__, "value level:");

            for (typename value_level_container::const_iterator values_itr = _values.cbegin(); values_itr != _values.cend(); values_itr++) {
                std::string key_str(format_key(values_itr->key));
                diag_base::put_any(suborigin, severity, __TAG__, "  [ page_pos=0x%llx : item_pos=0x%x ] key='%s'", 
                    (unsigned long long)values_itr.page_pos(), (unsigned)values_itr.item_pos(), key_str.c_str());
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }

} }
