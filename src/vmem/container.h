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

#include "../../diag/diag_ready.h"
#include "iterator.h"
#include "linked.h"
#include "pool.h"
#include "i/container.i.h"


namespace abc { namespace vmem {

    inline page_balance operator ~(page_balance value) noexcept {
        return static_cast<page_balance>(~ static_cast<std::uint8_t>(value));
    }

    inline page_balance operator |(page_balance left, page_balance right) noexcept {
        return static_cast<page_balance>(static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right));
    }

    inline page_balance operator &(page_balance left, page_balance right) noexcept {
        return static_cast<page_balance>(static_cast<std::uint8_t>(left) & static_cast<std::uint8_t>(right));
    }

    inline bool test(page_balance value, page_balance bits) noexcept {
        return (value & bits) == bits;
    }


    // --------------------------------------------------------------


    template <typename T>
    container_page_lead<T>::container_page_lead() noexcept
        : container_page_lead(container_page_lead_operation::none, page_pos_nil) {
    }


    template <typename T>
    template <typename Other>
    container_page_lead<T>::container_page_lead(const Other& other) noexcept
        : container_page_lead(other.operation, other.page_pos, other.items[0].key, other.items[1].key) {
    }


    template <typename T>
    container_page_lead<T>::container_page_lead(container_page_lead_operation operation, page_pos_t page_pos) noexcept
        : operation(operation)
        , page_pos(page_pos)
        , items { } {
    }


    template <typename T>
    template <typename Key>
    container_page_lead<T>::container_page_lead(container_page_lead_operation operation, page_pos_t page_pos, const Key& items_0_key, const Key& items_1_key) noexcept
        : container_page_lead(operation, page_pos) {
        items[0].key = items_0_key;
        items[1].key = items_1_key;
    }


    // --------------------------------------------------------------


    inline constexpr const char* pool::origin() noexcept {
        return "abc::vmem::container";
    }


    template <typename T, typename Header>
    inline constexpr std::size_t container<T, Header>::items_pos() noexcept {
        return sizeof(container_page<T, Header>) - sizeof(T);
    }


    template <typename T, typename Header>
    inline constexpr std::size_t container<T, Header>::max_item_size() noexcept {
        return page_size - items_pos();
    }


    template <typename T, typename Header>
    inline constexpr std::size_t container<T, Header>::page_capacity() noexcept {
        return max_item_size() / sizeof(T);
    }


    template <typename T, typename Header>
    inline constexpr bool container<T, Header>::is_uninit(const container_state* state) noexcept {
        return
            // nil
            (
                state != nullptr
                && state->front_page_pos == page_pos_nil
                && state->back_page_pos == page_pos_nil
                && state->item_size == 0
            )
            ||
            // zero
            (
                state != nullptr
                && state->front_page_pos == 0
                && state->back_page_pos == 0
                && state->item_size == 0
            );
    }


    template <typename T, typename Header>
    inline container<T, Header>::container(container_state* state, page_balance balance_insert, page_balance balance_erase, vmem::pool* pool, diag::log_ostream* log)
        : diag_base(abc::copy(origin()), log)
        , _state(state)
        , _balance_insert(balance_insert)
        , _balance_erase(balance_erase)
        , _pool(pool) {

        constexpr const char* suborigin = "container()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10443, "Begin: state=%p', balance_insert=%x, balance_erase=%x, pool=%p", state, balance_insert, balance_erase, pool);

        diag_base::expect(suborigin, state != nullptr, 0x10444, "state != nullptr");
        diag_base::expect(suborigin, pool != nullptr, 0x10445, "pool != nullptr");
        diag_base::expect(suborigin, sizeof(T) <= max_item_size(), 0x10446, "sizeof(T) <= max_item_size()");

        if (is_uninit(state)) {
            _state->front_page_pos = page_pos_nil;
            _state->back_page_pos = page_pos_nil;
            _state->item_size = sizeof(T);
        }

        diag_base::ensure(suborigin, _state->item_size == sizeof(T), 0x10448, "state != nullptr");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10449, "Begin: front_page_pos=0x%llx, back_page_pos=0x%llx", (unsigned long long)_state->front_page_pos, (unsigned long long)_state->back_page_pos);
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::begin() noexcept {
        return begin_itr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::const_iterator container<T, Header>::begin() const noexcept {
        return begin_itr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::const_iterator container<T, Header>::cbegin() const noexcept {
        return begin_itr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::end() noexcept {
        return end_itr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::const_iterator container<T, Header>::end() const noexcept {
        return end_itr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::const_iterator container<T, Header>::cend() const noexcept {
        return end_itr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::rend() noexcept {
        return rend_itr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::const_iterator container<T, Header>::rend() const noexcept {
        return rend_itr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::const_iterator container<T, Header>::crend() const noexcept {
        return rend_itr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::rbegin() noexcept {
        return rbegin_itr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::const_iterator container<T, Header>::rbegin() const noexcept {
        return rbegin_itr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::const_iterator container<T, Header>::crbegin() const noexcept {
        return rbegin_itr();
    }


    template <typename T, typename Header>
    inline bool container<T, Header>::empty() const noexcept {
        return _state->front_page_pos == vmem_page_pos_nil
            || _state->back_page_pos == vmem_page_pos_nil;
    }


    template <typename T, typename Header>
    inline std::size_t container<T, Header>::size() const noexcept {
        return _state->total_item_count;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::pointer container<T, Header>::frontptr() noexcept {
        return begin().ptr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::const_pointer container<T, Header>::frontptr() const noexcept {
        return begin().ptr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::reference container<T, Header>::front() {
        return begin().deref();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::const_reference container<T, Header>::front() const {
        return begin().deref();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::pointer container<T, Header>::backptr() noexcept {
        return rend().ptr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::const_pointer container<T, Header>::backptr() const noexcept {
        return rend().ptr();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::reference container<T, Header>::back() {
        return rend().deref();
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::const_reference container<T, Header>::back() const {
        return rend().deref();
    }


    template <typename T, typename Header>
    inline void container<T, Header>::push_back(const_reference item) {
        insert(end(), item);
    }


    template <typename T, typename Header>
    inline void container<T, Header>::pop_back() {
        erase(rend());
    }


    template <typename T, typename Header>
    inline void container<T, Header>::push_front(const_reference item) {
        insert(begin(), item);
    }


    template <typename T, typename Header>
    inline void container<T, Header>::pop_front() {
        erase(begin());
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::insert2(const_iterator itr, const_reference item) {
        constexpr const char* suborigin = "insert2()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1044c, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u", (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), itr.edge());

        diag_base::expect(suborigin, itr.page_pos() != vmem_page_pos_nil || (itr.item_pos() == vmem_item_pos_nil && empty()), 0x1044a, "itr.page_pos() != vmem_page_pos_nil || (itr.item_pos() == vmem_item_pos_nil && empty()");
        diag_base::expect(suborigin, itr.item_pos() != vmem_item_pos_nil || (itr.page_pos() == _state->back_page_pos && itr.edge() == iterator_edge::end), 0x1044b, "itr.item_pos() != vmem_item_pos_nil && (itr.page_pos() == _state->back_page_pos || itr.edge() == iterator_edge::end)");
                                                                                                          //// TODO: ^ was ||

        // Copy the item to a local variable to make sure the reference is valid and copyable before we change any page.
        T item_copy(item);

        // Insert without changing the state.
        result2 result = insert_nostate(itr, item_copy);

        if (result.iterator.is_valid()) {
            // We have inserted successfully.

            // Update the front page pos.
            if (_state->front_page_pos == vmem_page_pos_nil) {
                _state->front_page_pos = result.iterator.page_pos();
            }

            // Update the back page pos.
            if (_state->back_page_pos == vmem_page_pos_nil) {
                _state->back_page_pos = result.iterator.page_pos();
            }
            else if (_state->back_page_pos == itr.page_pos() && result.page_leads[0].page_pos != vmem_page_pos_nil) {
                _state->back_page_pos = result.page_leads[0].page_pos;
            } 

            // Update the total item count.
            _state->total_item_count++;
        }
        else {
            // We have failed to insert.

            // Return end().
            result.iterator = end_itr();
            result.page_leads[0] = result.page_leads[1] = page_lead();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1044d, "End: result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, result.page_pos=0x%llx, total_item_count=%zu",
                (unsigned long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (unsigned long long)result.page_leads[0].page_pos, (std::size_t)_state->total_item_count);

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::insert(const_iterator itr, const_reference item) {
        return insert2(itr, item).iterator;
    }


    template <typename T, typename Header>
    template <typename InputItr>
    inline typename container<T, Header>::iterator container<T, Header>::insert(const_iterator itr, InputItr first, InputItr last) {
        constexpr const char* suborigin = "insert(first, last)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u", (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), itr.edge());

        iterator ret(itr);

        for (InputItr item = first; item != last; item++) {
            iterator tmp_itr = insert(itr++, *item);
            diag_base::ensure(suborigin, tmp_itr.can_deref(), 0x1044e, "tmp_itr.can_deref()");
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        return ret;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::insert_nostate(const_iterator itr, const_reference item) {
        constexpr const char* suborigin = "insert_nostate";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1044f, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u", (unsigned long long)itr.page_pos(), itr.item_pos(), itr.edge());

        result2 result;

        if (itr.page_pos() == page_pos_nil) {
            result = insert_empty(item);
        }
        else {
            result = insert_nonempty(itr, item);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10450, "End: result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, result.page_pos=0x%llx",
                result.iterator.is_valid(), (unsigned long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (unsigned long long)result.page_leads[0].page_pos);

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::insert_empty(const_reference item) {
        constexpr const char* suborigin = "insert_empty";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10451, "Begin:");

        vmem::page new_page(nullptr);
        vmem::container_page<T, Header>* new_container_page = nullptr;

        insert_page_after(page_pos_nil, new_page, new_container_page);
        diag_base::expect(suborigin, new_page.pos() != page_pos_nil, __TAG__, "new_page.pos() != page_pos_nil");
        diag_base::expect(suborigin, new_page.ptr() != nullptr, __TAG__, "new_page.ptr() != nullptr");
        diag_base::expect(suborigin, new_container_page == new_page.ptr(), __TAG__, "new_container_page == new_page.ptr()");

        iterator itr = iterator(this, new_page.pos(), 0, iterator_edge::none, diag_base::log());
        result2 result = insert_with_capacity(itr, item, new_container_page);

        diag_base::ensure(suborigin, result.iterator.is_valid(), __TAG__, "result.iterator.is_valid()");
        diag_base::ensure(suborigin, result.iterator.page_pos() != page_pos_nil, __TAG__, "result.iterator.page_pos() != page_pos_nil");
        diag_base::ensure(suborigin, result.page_leads[0].page_pos != page_pos_nil, __TAG__, "result.page_leads[0].page_pos != page_pos_nil");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10452, "End: result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.page_pos=0x%llx",
                result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), (long long)result.page_leads[0].page_pos);

        return result;
    }


    //// TODO: Continue here.
    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::insert_nonempty(const_iterator itr, const_reference item) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x10453, "container::insert_nonempty() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
                (long long)itr.page_pos(), itr.item_pos());
        }

        result2 result;

        vmem_page<Pool, Log> page(_pool, itr.page_pos(), _log);

        if (page.ptr() == nullptr) {
            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::warning, 0x10454, "container::insert_nonempty() Could not load page pos=0x%llx", (long long)page.pos());
            }
        }
        else {
            container_page<T, Header>* container_page = reinterpret_cast<container_page<T, Header>*>(page.ptr());

            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::abc::debug, 0x10455, "container::insert_nonempty() item_count=%u, page_capacity=%zu", container_page->item_count, (std::size_t)page_capacity());
            }

            if (container_page->item_count == page_capacity()) {
                // The page has no capacity.
                result = insert_with_overflow(itr, item, container_page);
            }
            else {
                // The page has capacity.
                result = insert_with_capacity(itr, item, container_page);
            }
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x10456, "container::insert_nonempty() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.page_pos=0x%llx",
                result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), (long long)result.page_leads[0].page_pos);
        }

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::insert_with_overflow(const_iterator itr, const_reference item, container_page<T, Header>* container_page) {
        constexpr const char* suborigin = "insert_with_overflow";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10457, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x", (unsigned long long)itr.page_pos(), itr.item_pos());

        vmem::page new_page(nullptr);
        container_page<T, Header>* new_container_page = nullptr;

        insert_page_after(itr.page_pos(), new_page, new_container_page);
        diag_base::expect(suborigin, new_page.pos() != page_pos_nil, __TAG__, "new_page.pos() != page_pos_nil");
        diag_base::expect(suborigin, new_page.ptr() != nullptr, __TAG__, "new_page.ptr() != nullptr");
        diag_base::expect(suborigin, new_container_page == new_page.ptr(), __TAG__, "new_container_page == new_page.ptr()");

        // Balance if needed. Do that before inserting.
        if (should_balance_insert(itr, container_page)) {
            balance_split(itr.page_pos(), container_page, new_page.pos(), new_container_page);
        }

        result2 result;

        if (itr.item_pos() != item_pos_nil && itr.item_pos() <= container_page->item_count) {
            // Inserting to the former page.
            result = insert_with_capacity(itr, item, container_page);
        }
        else {
            // Inserting to the latter page.
            container_iterator<T, Header> new_itr(this, new_page.pos(), itr.item_pos() != item_pos_nil ? itr.item_pos() - container_page->item_count : new_container_page->item_count, iterator_edge::none, diag_base::log());
            result = insert_with_capacity(new_itr, item, new_container_page);
        }

        // page_leads[0] - insert; new page
        // page_leads[1] - original; used only when a new level is created
        result.page_leads[0] = page_lead(container_page_lead_operation::insert, new_page.pos());
        result.page_leads[0].items[0] = new_container_page->items[0];
        result.page_leads[1] = page_lead(container_page_lead_operation::original, itr.page_pos());
        result.page_leads[1].items[0], container_page->items[0];

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10458, "End: result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.page_pos=0x%llx",
                result.iterator.is_valid(), (unsigned long long)result.iterator.page_pos(), result.iterator.item_pos(), (unsigned long long)result.page_leads[0].page_pos);

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::insert_with_capacity(const_iterator itr, const_reference item, container_page<T, Header>* container_page) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x10459, "container::insert_with_capacity() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
                (long long)itr.page_pos(), itr.item_pos());
        }

        result2 result;
        result.iterator = iterator(this, itr.page_pos(), itr.item_pos() != vmem_item_pos_nil ? itr.item_pos() : container_page->item_count, vmem_iterator_edge::none, _log);

        // Shift items from the insertion position to free up a slot.
        std::size_t move_item_count = container_page->item_count - result.iterator.item_pos();
        if (move_item_count > 0) {
            std::memmove(&container_page->items[result.iterator.item_pos() + 1], &container_page->items[result.iterator.item_pos()], move_item_count * sizeof(T));
        }

        // Insert the item.
        ++container_page->item_count;
        vmem_copy(container_page->items[result.iterator.item_pos()], item);

        if (_log != nullptr) {
            _log->put_binary(category::abc::vmem, severity::abc::debug, 0x1045a, &container_page->items[result.iterator.item_pos()], std::min(sizeof(T), (std::size_t)16));
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x1045b, "container::insert_with_capacity() Done. result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x",
                (long long)result.iterator.page_pos(), result.iterator.item_pos());
        }

        return result;
    }


    template <typename T, typename Header>
    inline void container<T, Header>::balance_split(vmem_page_pos_t page_pos, container_page<T, Header>* container_page, vmem_page_pos_t new_page_pos, container_page<T, Header>* new_container_page) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x1045c, "container::balance() Start. page_pos=0x%llx, new_page_pos=0x%llx",
                (long long)page_pos, (long long)new_page_pos);
        }

        constexpr std::size_t new_page_item_count = page_capacity() / 2;
        constexpr std::size_t page_item_count = page_capacity() - new_page_item_count;
        std::memmove(&new_container_page->items[0], &container_page->items[page_item_count], new_page_item_count * sizeof(T));
        new_container_page->item_count = new_page_item_count;
        container_page->item_count = page_item_count;

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x1045d, "container::balance() Done. page_pos=0x%llx, item_count=%u, new_page_pos=0x%llx, new_item_count=%u",
                (long long)page_pos, container_page->item_count, (long long)new_page_pos, new_container_page->item_count);
        }
    }


    template <typename T, typename Header>
    inline void container<T, Header>::insert_page_after(page_pos_t after_page_pos, vmem::page& new_page, container_page<T, Header>*& new_container_page) {
        constexpr const char* suborigin = "insert_page_after";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1045e, "Begin: after_page_pos=0x%llx", (unsigned long long)after_page_pos);

        vmem::page new_page_local(_pool, diag_base::log());
        diag_base::expect(suborigin, new_page_local.ptr() != nullptr, 0x1045f, "new_page_local.ptr() != nullptr");

        container_page<T, Header>* new_container_page_local = reinterpret_cast<container_page<T, Header>*>(new_page_local.ptr());
        new_container_page_local->item_count = 0;

        vmem::linked linked(_state, _pool, diag_base::log());
        linked_iterator<Pool, Log> itr = linked.end();

        if (after_page_pos != page_pos_nil) {
            itr = linked_iterator(&linked, after_page_pos, item_pos_nil, iterator_edge::none, diag_base::log());
            itr++;
        }

        linked_iterator new_itr = linked.insert(itr, new_page_local.pos());
        diag_base::expect(suborigin, new_itr != linked.end(), __TAG__, "new_itr != linked.end()");

        new_page = std::move(new_page_local);
        new_container_page = new_container_page_local;

        diag_base::ensure(suborigin, new_page.pos() != page_pos_nil, __TAG__, "new_page.pos() != page_pos_nil");
        diag_base::ensure(suborigin, new_page.ptr() != nullptr, __TAG__, "new_page.ptr() != nullptr");
        diag_base::ensure(suborigin, new_container_page_local == new_page.ptr(), __TAG__, "new_container_page_local == new_page.ptr()");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10460, "Enc: after_page_pos=0x%llx, new_page_pos=0x%llx", (unsigned long long)after_page_pos, (unsigned long long)new_page.pos());
    }


    template <typename T, typename Header>
    inline bool container<T, Header>::should_balance_insert(const_iterator itr, const container_page<T, Header>* container_page) const noexcept {
        bool balance = false;

        if (container_page->prev_page_pos == vmem_page_pos_nil && itr.item_pos() == 0) {
            balance = vmem_page_balance::test(_balance_insert, vmem_page_balance::begin);
        }
        else if (container_page->next_page_pos == vmem_page_pos_nil && itr.item_pos() == vmem_item_pos_nil && itr.edge() == vmem_iterator_edge::end) {
            balance = vmem_page_balance::test(_balance_insert, vmem_page_balance::end);
        }
        else {
            balance = vmem_page_balance::test(_balance_insert, vmem_page_balance::inner);
        }

        return balance;
    }


    // ..............................................................


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::erase2(const_iterator itr) {
        if (!itr.can_deref()) {
            throw exception<std::logic_error, Log>("container::erase(itr)", 0x10461);
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::important, 0x10462, "container::erase() Begin. page_pos=0x%llx, item_pos=0x%x, edge=%u, total_item_count=%zu",
                (long long)itr.page_pos(), itr.item_pos(), itr.edge(), (std::size_t)_state->total_item_count);
        }

        result2 result = erase_nostate(itr);

        if (result.iterator.is_valid()) {
            // Update the total item count.
            _state->total_item_count--;
        }
        else {
            result = result2();
            result.iterator = end_itr();
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::important, 0x10463, "container::erase() Done. result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, total_item_count=%zu",
                (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (std::size_t)_state->total_item_count);
        }

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::erase(const_iterator itr) {
        return erase2(itr).iterator;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::erase(const_iterator first, const_iterator last) {
        iterator itr = first;

        while (itr != last) {
            if (!itr.can_deref()) {
                if (_log != nullptr) {
                    _log->put_any(category::abc::vmem, severity::important, 0x10464, "container::erase(first, last) Breaking from the loop.");
                }

                break;
            }

            itr = erase(itr);
        }

        return itr;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::erase_nostate(const_iterator itr) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x10465, "container::erase_nostate() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
                (long long)itr.page_pos(), itr.item_pos());
        }

        result2 result;

        vmem_page<Pool, Log> page(_pool, itr.page_pos(), _log);

        if (page.ptr() == nullptr) {
            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::warning, 0x10466, "container::erase() Could not load page pos=0x%llx", (long long)itr.page_pos());
            }
        }
        else {
            container_page<T, Header>* container_page = reinterpret_cast<container_page<T, Header>*>(page.ptr());

            if (container_page->item_count > 1) {
                // Determine whether we should balance before any inout parameter gets altered.
                bool balance = should_balance_erase(container_page, itr.item_pos());

                // There are many items on the page.
                result = erase_from_many(itr, container_page);

                // Balance if item count drops below half of capacity.
                if (balance && 2 * container_page->item_count <= page_capacity()) {
                    result2 res = balance_merge(result.iterator, page, container_page);

                    res.page_leads[0] = result.page_leads[0];
                    result = res;
                }
            }
            else {
                // Erasing the only item on a page means erasing the page.
                if (_log != nullptr) {
                    _log->put_any(category::abc::vmem, severity::abc::debug, 0x10467, "container::erase_nostate() Only.");
                }

                if (container_page->next_page_pos != vmem_page_pos_nil) {
                    result.iterator = iterator(this, container_page->next_page_pos, 0, vmem_iterator_edge::none, _log);
                }
                else {
                    result.iterator = end_itr();
                }

                // page_leads[0] - none
                // page_leads[1] - erase
                result.page_leads[0] = page_lead();
                result.page_leads[1] = page_lead(container_page_lead_operation::erase, page.pos());
                vmem_copy(result.page_leads[1].items[0], container_page->items[0]);

                erase_page(page);
                container_page = nullptr;
            }
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x10468, "container::erase_nostate() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
                result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge());
        }

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::erase_from_many(const_iterator itr, container_page<T, Header>* container_page) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x10469, "container::erase_from_many() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
                (long long)itr.page_pos(), itr.item_pos());
        }

        result2 result;

        if (itr.item_pos() < container_page->item_count - 1) {
            if (itr.item_pos() == 0) {
                // page_leads[0] - replace
                // page_leads[1] - none
                result.page_leads[0] = page_lead(container_page_lead_operation::replace, itr.page_pos());
                vmem_copy(result.page_leads[0].items[0], container_page->items[0]);
                vmem_copy(result.page_leads[0].items[1], container_page->items[1]);
                result.page_leads[1] = page_lead();
            }

            // To delete an item before the last one, pull up the remaining elements.

            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::abc::debug, 0x1046a, "container::erase_from_many() Middle.");
            }

            std::size_t move_item_count = container_page->item_count - itr.item_pos() - 1;
            std::memmove(&container_page->items[itr.item_pos()], &container_page->items[itr.item_pos() + 1], move_item_count * sizeof(T));

            result.iterator = itr;
        }
        else {
            // To delete the last (back) item on a page, there is nothing to do.

            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::abc::debug, 0x1046b, "container::erase_from_many() Last.");
            }

            // If we are deleting the last item on a page, the next item is item 0 on the next page or end().
            if (container_page->next_page_pos != vmem_page_pos_nil) {
                result.iterator = iterator(this, container_page->next_page_pos, 0, vmem_iterator_edge::none, _log);
            }
            else {
                result.iterator = end_itr();
            }
        }

        // The main part of deleting an item from a page is decrementing the count.
        container_page->item_count--;

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x1046c, "container::erase_from_many() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
                result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge());
        }

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::balance_merge(const_iterator itr, vmem_page<Pool, Log>& page, container_page<T, Header>* container_page) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x1046d, "container::balance_merge_safe() Start. page_pos=0x%llx",
                (long long)page.pos());
        }

        result2 result;
        result.iterator = itr;

        // Try the next page.
        if (container_page->next_page_pos != vmem_page_pos_nil) {
            result = balance_merge_next(itr, page, container_page);
        }

        // Try the previous page.
        if (container_page->prev_page_pos != vmem_page_pos_nil) {
            result = balance_merge_prev(itr, page, container_page);
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x1046e, "container::balance_merge_safe() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x",
                result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos());
        }

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::balance_merge_next(const_iterator itr, vmem_page<Pool, Log>& page, container_page<T, Header>* container_page) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x1046f, "container::balance_merge_next_safe() Start. page_pos=0x%llx",
                (long long)page.pos());
        }

        result2 result;
        result.iterator = itr;

        vmem_page<Pool, Log> next_page(_pool, container_page->next_page_pos, _log);
        container_page<T, Header>* next_container_page = nullptr;

        if (next_page.ptr() == nullptr) {
            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::abc::debug, 0x10470, "container::balance_merge_next_safe() Could not load page pos=0x%llx",
                    (long long)container_page->next_page_pos);
            }
        }
        else {
            next_container_page = reinterpret_cast<container_page<T, Header>*>(next_page.ptr());

            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::abc::debug, 0x10471, "container::balance_merge_next_safe() page_item_count=%u, next_page_pos=0x%llx, next_page_item_count=%u",
                    container_page->item_count, (long long)next_page.pos(), next_container_page->item_count);
            }

            if (container_page->item_count + next_container_page->item_count <= page_capacity()) {
                if (_log != nullptr) {
                    _log->put_any(category::abc::vmem, severity::abc::optional, 0x10472, "container::balance_merge_next_safe() Do.");
                }

                // page_leads[0] - none
                // page_leads[1] - erase
                result.page_leads[0] = page_lead();
                result.page_leads[1] = page_lead(container_page_lead_operation::erase, next_page.pos());
                vmem_copy(result.page_leads[1].items[0], next_container_page->items[0]);

                // Merge the items from the next page into this one.
                std::memmove(&container_page->items[container_page->item_count], &next_container_page->items[0], next_container_page->item_count * sizeof(T));

                // Fix the next item, if it was item[0] on the next page.
                if (itr.page_pos() == next_page.pos()) {
                    result.iterator = iterator(this, page.pos(), container_page->item_count, vmem_iterator_edge::none, _log);
                }

                // Update the item count on this page.
                container_page->item_count += next_container_page->item_count;

                // Free the next page.
                erase_page(next_page);
                next_container_page = nullptr;
            }
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x10473, "container::balance_merge_next_safe() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
                result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge());
        }

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::balance_merge_prev(const_iterator itr, vmem_page<Pool, Log>& page, container_page<T, Header>* container_page) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x10474, "container::balance_merge_prev() Start. page_pos=0x%llx",
                (long long)page.pos());
        }

        result2 result;
        result.iterator = itr;


        vmem_page<Pool, Log> prev_page(_pool, container_page->prev_page_pos, _log);
        container_page<T, Header>* prev_container_page = nullptr;

        if (prev_page.ptr() == nullptr) {
            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::abc::debug, 0x10475, "container::balance_merge_prev() Could not load page pos=0x%llx",
                    (long long)container_page->prev_page_pos);
            }
        }
        else {
            prev_container_page = reinterpret_cast<container_page<T, Header>*>(prev_page.ptr());

            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::abc::debug, 0x10476, "container::balance_merge_prev() page_pos=0x%llx, page_item_count=%u, prev_page_pos=0x%llx, prev_page_item_count=%u",
                    (long long)page.pos(), container_page->item_count, (long long)prev_page.pos(), prev_container_page->item_count);
            }

            if (container_page->item_count + prev_container_page->item_count <= page_capacity()) {
                if (_log != nullptr) {
                    _log->put_any(category::abc::vmem, severity::abc::optional, 0x10477, "container::balance_merge_prev() Do.");
                }

                // page_leads[0] - none
                // page_leads[1] - erase
                result.page_leads[0] = page_lead();
                result.page_leads[1] = page_lead(container_page_lead_operation::erase, page.pos());
                vmem_copy(result.page_leads[1].items[0], container_page->items[0]);

                // Merge the items from this page into the previous one.
                std::memmove(&prev_container_page->items[prev_container_page->item_count], &container_page->items[0], container_page->item_count * sizeof(T));

                // Update the result only if itr references this page.
                // If we deleted the last item on this page, itr references item[0] on the next page, and will not be affected by this balancing.
                if (itr.page_pos() == page.pos()) {
                    if (itr.item_pos() != vmem_item_pos_nil) {
                        result.iterator = iterator(this, prev_page.pos(), itr.item_pos() + prev_container_page->item_count, vmem_iterator_edge::none, _log);
                    }
                    else {
                        result.iterator = iterator(this, prev_page.pos(), itr.item_pos(), itr.edge(), _log);
                    }
                }

                // Update the item count on the previous page.
                prev_container_page->item_count += container_page->item_count;

                // Free this page.
                erase_page(page);
                container_page = nullptr;
            }
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x10478, "container::balance_merge_prev() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
                result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge());
        }

        return result;
    }


    template <typename T, typename Header>
    inline bool container<T, Header>::erase_page(vmem_page<Pool, Log>& page) noexcept {
        vmem_page_pos_t page_pos = page.pos();

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x10479, "container::erase_page() Start. page_pos=0x%llx",
                (long long)page_pos);
        }

        bool ok = erase_page_pos(page_pos);

        if (ok) {
            page.free();
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x1047a, "container::erase_page() Done. ok=%d, page_pos=0x%llx",
                ok, (long long)page_pos);
        }

        return ok;
    }


    template <typename T, typename Header>
    inline bool container<T, Header>::erase_page_pos(vmem_page_pos_t page_pos) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x1047b, "container::erase_page_pos() Start. page_pos=0x%llx",
                (long long)page_pos);
        }

        vmem_linked<Pool, Log> linked(_state, _pool, _log);

        vmem_linked_iterator<Pool, Log> itr(&linked, page_pos, vmem_item_pos_nil, vmem_iterator_edge::none, _log);
        linked.erase(itr);
        bool ok = true;

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x1047c, "container::erase_page_pos() Done. ok=%d, page_pos=0x%llx",
                (int)ok, (long long)page_pos);
        }

        return ok;
    }


    template <typename T, typename Header>
    inline bool container<T, Header>::should_balance_erase(const container_page<T, Header>* container_page, vmem_item_pos_t item_pos) const noexcept {
        bool balance = false;

        if (container_page->prev_page_pos == vmem_page_pos_nil && item_pos == 0) {
            balance = vmem_page_balance::test(_balance_erase, vmem_page_balance::begin);
        }
        else if (container_page->next_page_pos == vmem_page_pos_nil && item_pos == container_page->item_count - 1) {
            balance = vmem_page_balance::test(_balance_erase, vmem_page_balance::end);
        }
        else {
            balance = vmem_page_balance::test(_balance_erase, vmem_page_balance::inner);
        }

        return balance;
    }


    // ..............................................................


    template <typename T, typename Header>
    inline void container<T, Header>::clear() noexcept {
        vmem_linked<Pool, Log> linked(_state, _pool, _log);
        linked.clear();
    }


    // ..............................................................


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::next(const iterator_state& itr) const noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x1047d, "container::next() Before itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
                (long long)itr.page_pos(), itr.item_pos(), itr.edge());
        }

        iterator result = end_itr();

        if (itr.item_pos() == vmem_item_pos_nil && itr.edge() == vmem_iterator_edge::rbegin) {
            result = begin_itr();
        }
        else if (itr.page_pos() != vmem_page_pos_nil) {
            vmem_page<Pool, Log> page(_pool, itr.page_pos(), _log);

            if (page.ptr() == nullptr) {
                if (_log != nullptr) {
                    _log->put_any(category::abc::vmem, severity::warning, 0x1047e, "container::next() Could not load page pos=0x%llx", (long long)itr.page_pos());
                }
            }
            else {
                container_page<T, Header>* container_page = reinterpret_cast<container_page<T, Header>*>(page.ptr());

                if (itr.item_pos() < container_page->item_count - 1) {
                    result = iterator(this, itr.page_pos(), itr.item_pos() + 1, vmem_iterator_edge::none, _log);
                }
                else {
                    if (container_page->next_page_pos != vmem_page_pos_nil) {
                        result = iterator(this, container_page->next_page_pos, 0, vmem_iterator_edge::none, _log);
                    }
                }
            }
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x1047f, "container::next() After result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
                (long long)result.page_pos(), result.item_pos(), result.edge());
        }

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::prev(const iterator_state& itr) const noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x10480, "container::prev() Before itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
                (long long)itr.page_pos(), itr.item_pos(), itr.edge());
        }

        iterator result = rbegin_itr();

        if (itr.item_pos() == vmem_item_pos_nil && itr.edge() == vmem_iterator_edge::end) {
            result = rend_itr();
        }
        else if (itr.page_pos() != vmem_page_pos_nil) {
            vmem_page<Pool, Log> page(_pool, itr.page_pos(), _log);

            if (page.ptr() == nullptr) {
                if (_log != nullptr) {
                    _log->put_any(category::abc::vmem, severity::warning, 0x10481, "container::prev() Could not load page pos=0x%llx", (long long)itr.page_pos());
                }
            }
            else {
                container_page<T, Header>* container_page = reinterpret_cast<container_page<T, Header>*>(page.ptr());

                if (itr.item_pos() > 0) {
                    result = iterator(this, itr.page_pos(), itr.item_pos() - 1, vmem_iterator_edge::none, _log);
                }
                else {
                    if (container_page->prev_page_pos != vmem_page_pos_nil) {
                        vmem_page<Pool, Log> prev_page(_pool, container_page->prev_page_pos, _log);

                        if (prev_page.ptr() == nullptr) {
                            if (_log != nullptr) {
                                _log->put_any(category::abc::vmem, severity::warning, 0x10482, "container::prev() Could not load page pos=0x%llx", (long long)container_page->prev_page_pos);
                            }
                        }
                        else {
                            container_page<T, Header>* prev_container_page = reinterpret_cast<container_page<T, Header>*>(prev_page.ptr());
        
                            result = iterator(this, container_page->prev_page_pos, prev_container_page->item_count - 1, vmem_iterator_edge::none, _log);
                        }
                    }
                }
            }
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x10483, "container::prev() After result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
                (long long)result.page_pos(), result.item_pos(), result.edge());
        }

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::pointer container<T, Header>::at(const iterator_state& itr) const noexcept {
        vmem_item_pos_t byte_pos =
            itr.item_pos() == vmem_item_pos_nil ?
                vmem_item_pos_nil :
                items_pos() + (itr.item_pos() * sizeof(T));

        return pointer(_pool, itr.page_pos(), byte_pos, _log);
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::begin_itr() const noexcept {
        iterator itr(this, _state->back_page_pos, vmem_item_pos_nil, vmem_iterator_edge::end, _log);

        if (_state->front_page_pos != vmem_page_pos_nil) {
            itr = iterator(this, _state->front_page_pos, 0, vmem_iterator_edge::none, _log);
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x10484, "container::begin_itr() page_pos=0x%llx, item_pos=0x%x, edge=%u",
                (long long)itr.page_pos(), itr.item_pos(), itr.edge());
        }

        return itr;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::end_itr() const noexcept {
        iterator itr(this, _state->back_page_pos, vmem_item_pos_nil, vmem_iterator_edge::end, _log);

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x10486, "container::end_itr() page_pos=0x%llx, item_pos=0x%x, edge=%u",
                (long long)itr.page_pos(), itr.item_pos(), itr.edge());
        }

        return itr;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::reverse_iterator container<T, Header>::rend_itr() const noexcept {
        iterator itr(this, _state->front_page_pos, vmem_item_pos_nil, vmem_iterator_edge::rbegin, _log);

        if (_state->back_page_pos != vmem_page_pos_nil) {
            vmem_page<Pool, Log> page(_pool, _state->back_page_pos, _log);

            if (page.ptr() == nullptr) {
                if (_log != nullptr) {
                    _log->put_any(category::abc::vmem, severity::warning, 0x10487, "container::rend_itr() Could not load page pos=0x%llx", (long long)_state->back_page_pos);
                }
            }
            else {
                container_page<T, Header>* container_page = reinterpret_cast<container_page<T, Header>*>(page.ptr());
                itr = iterator(this, _state->back_page_pos, container_page->item_count - 1, vmem_iterator_edge::none, _log);
            }
        }


        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x10488, "container::rbegin_itr() page_pos=0x%llx, item_pos=0x%x, edge=%u",
                (long long)itr.page_pos(), itr.item_pos(), itr.edge());
        }

        return itr;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::reverse_iterator container<T, Header>::rbegin_itr() const noexcept {
        iterator itr(this, _state->front_page_pos, vmem_item_pos_nil, vmem_iterator_edge::rbegin, _log);

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x10485, "container::rbegin_itr() page_pos=0x%llx, item_pos=0x%x, edge=%u",
                (long long)itr.page_pos(), itr.item_pos(), itr.edge());
        }

        return itr;
    }


    // --------------------------------------------------------------

} }
