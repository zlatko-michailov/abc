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

#include "../diag/diag_ready.h"
#include "iterator.h"
#include "linked.h"
#include "pool.h"
#include "i/container.i.h"


namespace abc { namespace vmem {

    inline constexpr page_balance operator ~(page_balance value) noexcept {
        return static_cast<page_balance>(~ static_cast<std::uint8_t>(value));
    }

    inline constexpr page_balance operator |(page_balance left, page_balance right) noexcept {
        return static_cast<page_balance>(static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right));
    }

    inline constexpr page_balance operator &(page_balance left, page_balance right) noexcept {
        return static_cast<page_balance>(static_cast<std::uint8_t>(left) & static_cast<std::uint8_t>(right));
    }

    inline constexpr bool test(page_balance value, page_balance bits) noexcept {
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
        std::memmove(&items[0].key, &items_0_key, sizeof(Key));
        std::memmove(&items[1].key, &items_1_key, sizeof(Key));
    }


    // --------------------------------------------------------------


    template <typename T, typename Header>
    inline constexpr const char* container<T, Header>::origin() noexcept {
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
        return _state->front_page_pos == page_pos_nil
            || _state->back_page_pos == page_pos_nil;
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

        diag_base::expect(suborigin, itr.page_pos() != page_pos_nil || (itr.item_pos() == item_pos_nil && empty()), 0x1044a, "itr.page_pos() != page_pos_nil || (itr.item_pos() == item_pos_nil && empty()");
        diag_base::expect(suborigin, itr.item_pos() != item_pos_nil || (itr.page_pos() == _state->back_page_pos && itr.edge() == iterator_edge::end), 0x1044b, "itr.item_pos() != item_pos_nil && (itr.page_pos() == _state->back_page_pos && itr.edge() == iterator_edge::end)");
                                                                                                      //// TODO: ^ was ||

        // Copy the item to a local variable to make sure the reference is valid and copyable before we change any page.
        T item_copy(item);

        // Insert without changing the state.
        result2 result = insert_nostate(itr, item_copy);
        diag_base::expect(suborigin, result.iterator.can_deref(), __TAG__, "result.iterator.can_deref()");

        // Update the front page pos.
        if (_state->front_page_pos == page_pos_nil) {
            _state->front_page_pos = result.iterator.page_pos();
        }

        // Update the back page pos.
        if (_state->back_page_pos == page_pos_nil) {
            _state->back_page_pos = result.iterator.page_pos();
        }
        else if (_state->back_page_pos == itr.page_pos() && result.page_leads[0].page_pos != page_pos_nil) {
            _state->back_page_pos = result.page_leads[0].page_pos;
        } 

        // Update the total item count.
        _state->total_item_count++;

        diag_base::ensure(suborigin, result.iterator.can_deref(), __TAG__, "result.iterator.can_deref()");

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

        diag_base::ensure(suborigin, result.iterator.can_deref(), __TAG__, "result.iterator.can_deref()");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10450, "End: result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, result.page_pos=0x%llx",
                (unsigned long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (unsigned long long)result.page_leads[0].page_pos);

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

        diag_base::ensure(suborigin, result.iterator.can_deref(), __TAG__, "result.iterator.can_deref()");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10452, "End: result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.page_pos=0x%llx",
                (unsigned long long)result.iterator.page_pos(), result.iterator.item_pos(), (unsigned long long)result.page_leads[0].page_pos);

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::insert_nonempty(const_iterator itr, const_reference item) {
        constexpr const char* suborigin = "insert_nonempty";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10453, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x", (unsigned long long)itr.page_pos(), itr.item_pos());

        result2 result;

        vmem::page page(_pool, itr.page_pos(), diag_base::log());
        diag_base::expect(suborigin, page.pos() == itr.page_pos(), __TAG__, "page.pos() == itr.page_pos()");
        diag_base::expect(suborigin, page.ptr() != nullptr, 0x10454, "page.ptr() != nullptr");

        vmem::container_page<T, Header>* container_page = reinterpret_cast<vmem::container_page<T, Header>*>(page.ptr());
        diag_base::put_any(suborigin, diag::severity::verbose, 0x10455, "item_count=%u, page_capacity=%zu", container_page->item_count, (std::size_t)page_capacity());

        if (container_page->item_count == page_capacity()) {
            // The page has no capacity.
            result = insert_with_overflow(itr, item, container_page);
        }
        else {
            // The page has capacity.
            result = insert_with_capacity(itr, item, container_page);
        }

        diag_base::ensure(suborigin, result.iterator.can_deref(), __TAG__, "result.iterator.can_deref()");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10456, "End: result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.page_pos=0x%llx",
                (unsigned long long)result.iterator.page_pos(), result.iterator.item_pos(), (unsigned long long)result.page_leads[0].page_pos);

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::insert_with_overflow(const_iterator itr, const_reference item, container_page<T, Header>* container_page) {
        constexpr const char* suborigin = "insert_with_overflow";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10457, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x", (unsigned long long)itr.page_pos(), itr.item_pos());

        vmem::page new_page(nullptr);
        vmem::container_page<T, Header>* new_container_page = nullptr;

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
        std::memmove(&result.page_leads[0].items[0], &new_container_page->items[0], sizeof(T));
        result.page_leads[1] = page_lead(container_page_lead_operation::original, itr.page_pos());
        std::memmove(&result.page_leads[1].items[0], &container_page->items[0], sizeof(T));

        diag_base::ensure(suborigin, result.iterator.can_deref(), __TAG__, "result.iterator.can_deref()");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10458, "End: result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.page_pos=0x%llx",
                (unsigned long long)result.iterator.page_pos(), result.iterator.item_pos(), (unsigned long long)result.page_leads[0].page_pos);

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::insert_with_capacity(const_iterator itr, const_reference item, vmem::container_page<T, Header>* container_page) {
        constexpr const char* suborigin = "insert_with_overflow";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10459, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x", (unsigned long long)itr.page_pos(), itr.item_pos());

        result2 result;
        result.iterator = iterator(this, itr.page_pos(), itr.item_pos() != item_pos_nil ? itr.item_pos() : container_page->item_count, iterator_edge::none, diag_base::log());

        // Shift items from the insertion position to free up a slot.
        std::size_t move_item_count = container_page->item_count - result.iterator.item_pos();
        if (move_item_count > 0) {
            std::memmove(&container_page->items[result.iterator.item_pos() + 1], &container_page->items[result.iterator.item_pos()], move_item_count * sizeof(T));
        }

        // Insert the item.
        ++container_page->item_count;
        std::memmove(&container_page->items[result.iterator.item_pos()], &item, sizeof(T));
        diag_base::put_binary(suborigin, diag::severity::debug, 0x1045a, &container_page->items[result.iterator.item_pos()], std::min(sizeof(T), (std::size_t)16));

        diag_base::ensure(suborigin, result.iterator.can_deref(), __TAG__, "result.iterator.can_deref()");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10459, "End: result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x",
                (unsigned long long)result.iterator.page_pos(), result.iterator.item_pos());

        return result;
    }


    template <typename T, typename Header>
    inline void container<T, Header>::balance_split(page_pos_t page_pos, vmem::container_page<T, Header>* container_page, page_pos_t new_page_pos, vmem::container_page<T, Header>* new_container_page) {
        constexpr const char* suborigin = "balance_split";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1045c, "Begin: page_pos=0x%llx, new_page_pos=0x%llx", (unsigned long long)page_pos, (unsigned long long)new_page_pos);

        constexpr std::size_t new_page_item_count = page_capacity() / 2;
        constexpr std::size_t page_item_count = page_capacity() - new_page_item_count;
        for (std::size_t i = 0; i < new_page_item_count; i++) {
            new_container_page->items[i] = container_page->items[page_item_count + i];
        }
        new_container_page->item_count = new_page_item_count;
        container_page->item_count = page_item_count;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1045d, "End: page_pos=0x%llx, item_count=%u, new_page_pos=0x%llx, new_item_count=%u",
                (unsigned long long)page_pos, (unsigned)container_page->item_count, (unsigned long long)new_page_pos, (unsigned)new_container_page->item_count);
    }


    template <typename T, typename Header>
    inline void container<T, Header>::insert_page_after(page_pos_t after_page_pos, vmem::page& new_page, vmem::container_page<T, Header>*& new_container_page) {
        constexpr const char* suborigin = "insert_page_after";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1045e, "Begin: after_page_pos=0x%llx", (unsigned long long)after_page_pos);

        vmem::page new_page_local(_pool, diag_base::log());
        diag_base::expect(suborigin, new_page_local.ptr() != nullptr, 0x1045f, "new_page_local.ptr() != nullptr");

        vmem::container_page<T, Header>* new_container_page_local = reinterpret_cast<vmem::container_page<T, Header>*>(new_page_local.ptr());
        new_container_page_local->item_count = 0;

        vmem::linked linked(_state, _pool, diag_base::log());
        linked_iterator itr = linked.end();

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

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10460, "End: after_page_pos=0x%llx, new_page_pos=0x%llx", (unsigned long long)after_page_pos, (unsigned long long)new_page.pos());
    }


    template <typename T, typename Header>
    inline bool container<T, Header>::should_balance_insert(const_iterator itr, const vmem::container_page<T, Header>* container_page) const noexcept {
        bool should_balance = false;

        if (container_page->prev_page_pos == page_pos_nil && itr.item_pos() == 0) {
            should_balance = vmem::test(_balance_insert, page_balance::begin);
        }
        else if (container_page->next_page_pos == page_pos_nil && itr.item_pos() == item_pos_nil && itr.edge() == iterator_edge::end) {
            should_balance = vmem::test(_balance_insert, page_balance::end);
        }
        else {
            should_balance = vmem::test(_balance_insert, page_balance::inner);
        }

        return should_balance;
    }


    // ..............................................................


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::erase2(const_iterator itr) {
        constexpr const char* suborigin = "erase2";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10462, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u, total_item_count=%zu",
                (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), itr.edge(), (std::size_t)_state->total_item_count);

        diag_base::expect(suborigin, itr.can_deref(), 0x10461, "itr.can_deref()");

        result2 result = erase_nostate(itr);
        diag_base::expect(suborigin, result.iterator.is_valid(this), __TAG__, "result.iterator.is_valid(this)");

        // Update the total item count.
        _state->total_item_count--;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10463, "End: result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, total_item_count=%zu",
                (unsigned long long)result.iterator.page_pos(), (unsigned)result.iterator.item_pos(), result.iterator.edge(), (std::size_t)_state->total_item_count);

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::erase(const_iterator itr) {
        return erase2(itr).iterator;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::erase(const_iterator first, const_iterator last) {
        constexpr const char* suborigin = "erase(first, last)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        iterator itr = first;

        while (itr != last) {
            itr = erase(itr);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        return itr;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::erase_nostate(const_iterator itr) {
        constexpr const char* suborigin = "erase_nostate";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10465, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
                (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), itr.edge());

        diag_base::expect(suborigin, itr.can_deref(), __TAG__, "itr.can_deref()");

        result2 result;

        vmem::page page(_pool, itr.page_pos(), diag_base::log());
        diag_base::expect(suborigin, page.pos() == itr.page_pos(), __TAG__, "page.pos() == itr.page_pos()");
        diag_base::expect(suborigin, page.ptr() != nullptr, 0x10466, "page.ptr() != nullptr");

        vmem::container_page<T, Header>* container_page = reinterpret_cast<vmem::container_page<T, Header>*>(page.ptr());
        diag_base::put_any(suborigin, diag::severity::verbose, __TAG__, "item_count=%u, page_capacity=%zu", container_page->item_count, (std::size_t)page_capacity());

        if (container_page->item_count > 1) {
            bool should_balance = should_balance_erase(container_page, itr.item_pos());

            result = erase_from_many(itr, container_page);

            // Balance if the item count drops below half of capacity.
            if (should_balance && 2 * container_page->item_count <= page_capacity()) {
                container_page_lead<T> page_lead_0 = result.page_leads[0];

                result = balance_merge(result.iterator, page, container_page);

                result.page_leads[0] = page_lead_0;
            }
        }
        else {
            // Erasing the only item on a page means erasing the page.
            diag_base::put_any(suborigin, diag::severity::optional, 0x10467, "Erase from one");

            if (container_page->next_page_pos != page_pos_nil) {
                result.iterator = iterator(this, container_page->next_page_pos, 0, iterator_edge::none, diag_base::log());
            }
            else {
                result.iterator = end_itr();
            }

            // page_leads[0] - none
            // page_leads[1] - erase
            result.page_leads[0] = page_lead();
            result.page_leads[1] = page_lead(container_page_lead_operation::erase, page.pos());
            std::memmove(&result.page_leads[1].items[0], &container_page->items[0], sizeof(T));

            erase_page(page);
            container_page = nullptr;
        }

        diag_base::ensure(suborigin, result.iterator.is_valid(this), __TAG__, "result.iterator.is_valid(this)");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10468, "End: result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
                (unsigned long long)result.iterator.page_pos(), (unsigned)result.iterator.item_pos(), result.iterator.edge());

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::erase_from_many(const_iterator itr, vmem::container_page<T, Header>* container_page) {
        constexpr const char* suborigin = "erase_from_many";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10469, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
                (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), itr.edge());

        diag_base::expect(suborigin, itr.can_deref(), __TAG__, "itr.can_deref()");

        result2 result;

        if (itr.item_pos() < container_page->item_count - 1) {
            if (itr.item_pos() == 0) {
                // page_leads[0] - replace
                // page_leads[1] - none
                result.page_leads[0] = page_lead(container_page_lead_operation::replace, itr.page_pos());
                std::memmove(&result.page_leads[0].items[0], &container_page->items[0], sizeof(T));
                std::memmove(&result.page_leads[0].items[1], &container_page->items[1], sizeof(T));
                result.page_leads[1] = page_lead();
            }

            // To delete an item before the last one, pull up the remaining elements.
            diag_base::put_any(suborigin, diag::severity::optional, 0x1046a, "Middle: itr.item_pos=0x%x, item_count=%u", (unsigned)itr.item_pos(), (unsigned)container_page->item_count);

            std::size_t move_item_count = container_page->item_count - itr.item_pos() - 1;
            std::memmove(&container_page->items[itr.item_pos()], &container_page->items[itr.item_pos() + 1], move_item_count * sizeof(T));

            result.iterator = itr;
        }
        else {
            // To delete the last (back) item on a page, there is nothing to do.
            diag_base::put_any(suborigin, diag::severity::optional, 0x1046b, "Last: itr.item_pos=0x%x, item_count=%u", (unsigned)itr.item_pos(), (unsigned)container_page->item_count);

            // If we are deleting the last item on a page, the next item is item 0 on the next page or end().
            if (container_page->next_page_pos != page_pos_nil) {
                result.iterator = iterator(this, container_page->next_page_pos, 0, iterator_edge::none, diag_base::log());
            }
            else {
                result.iterator = end_itr();
            }
        }

        // The main part of deleting an item from a page is decrementing the count.
        container_page->item_count--;

        diag_base::ensure(suborigin, result.iterator.is_valid(this), __TAG__, "result.iterator.is_valid(this)");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1046c, "End: result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
                (unsigned long long)result.iterator.page_pos(), (unsigned)result.iterator.item_pos(), result.iterator.edge());

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::balance_merge(const_iterator itr, vmem::page& page, vmem::container_page<T, Header>* container_page) {
        constexpr const char* suborigin = "balance_merge";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1046d, "Begin: page_pos=0x%llx", (unsigned long long)page.pos());

        result2 result;
        result.iterator = itr;

        // Try the next page.
        if (container_page->next_page_pos != page_pos_nil) {
            result = balance_merge_next(itr, page, container_page);
        }

        // Try the previous page.
        if (container_page->prev_page_pos != page_pos_nil) {
            result = balance_merge_prev(itr, page, container_page);
        }

        diag_base::ensure(suborigin, result.iterator.is_valid(this), __TAG__, "result.iterator.is_valid(this)");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1046e, "End: result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
                (unsigned long long)result.iterator.page_pos(), (unsigned)result.iterator.item_pos(), result.iterator.edge());

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::balance_merge_next(const_iterator itr, vmem::page& page, vmem::container_page<T, Header>* container_page) {
        constexpr const char* suborigin = "balance_merge_next";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1046f, "Begin: page_pos=0x%llx", (unsigned long long)page.pos());

        result2 result;
        result.iterator = itr;

        vmem::page next_page(_pool, container_page->next_page_pos, diag_base::log());
        diag_base::expect(suborigin, next_page.pos() == container_page->next_page_pos, __TAG__, "next_page.pos() == container_page->next_page_pos");
        diag_base::expect(suborigin, next_page.ptr() != nullptr, 0x10470, "next_page.ptr() != nullptr");

        vmem::container_page<T, Header>* next_container_page = reinterpret_cast<vmem::container_page<T, Header>*>(page.ptr());
        diag_base::put_any(suborigin, diag::severity::optional, 0x10471, "page_item_count=%u, next_page_pos=0x%llx, next_page_item_count=%u",
                (unsigned)container_page->item_count, (unsigned long long)next_page.pos(), (unsigned)next_container_page->item_count);

        if (container_page->item_count + next_container_page->item_count <= page_capacity()) {
            // page_leads[0] - none
            // page_leads[1] - erase
            result.page_leads[0] = page_lead();
            result.page_leads[1] = page_lead(container_page_lead_operation::erase, next_page.pos());
            std::memmove(&result.page_leads[1].items[0], &next_container_page->items[0], sizeof(T));

            // Merge the items from the next page into this one.
            std::memmove(&container_page->items[container_page->item_count], &next_container_page->items[0], next_container_page->item_count * sizeof(T));

            // Fix the next item, if it was item[0] on the next page.
            if (itr.page_pos() == next_page.pos()) {
                result.iterator = iterator(this, page.pos(), container_page->item_count, iterator_edge::none, diag_base::log());
            }

            // Update the item count on this page.
            container_page->item_count += next_container_page->item_count;

            // Free the next page.
            erase_page(next_page);
            next_container_page = nullptr;
        }

        diag_base::ensure(suborigin, result.iterator.is_valid(this), __TAG__, "result.iterator.is_valid(this)");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10473, "End: result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
                (unsigned long long)result.iterator.page_pos(), (unsigned)result.iterator.item_pos(), result.iterator.edge());

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::result2 container<T, Header>::balance_merge_prev(const_iterator itr, vmem::page& page, vmem::container_page<T, Header>* container_page) {
        constexpr const char* suborigin = "balance_merge_prev";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10474, "Begin: page_pos=0x%llx", (unsigned long long)page.pos());

        result2 result;
        result.iterator = itr;

        vmem::page prev_page(_pool, container_page->prev_page_pos, diag_base::log());
        diag_base::expect(suborigin, prev_page.pos() == container_page->prev_page_pos, __TAG__, "prev_page.pos() == container_page->prev_page_pos");
        diag_base::expect(suborigin, prev_page.ptr() != nullptr, 0x10475, "prev_page.ptr() != nullptr");

        vmem::container_page<T, Header>* prev_container_page = reinterpret_cast<vmem::container_page<T, Header>*>(page.ptr());
        diag_base::put_any(suborigin, diag::severity::optional, 0x10476, "page_item_count=%u, prev_page_pos=0x%llx, prev_page_item_count=%u",
                (unsigned)container_page->item_count, (unsigned long long)prev_page.pos(), (unsigned)prev_container_page->item_count);

        if (container_page->item_count + prev_container_page->item_count <= page_capacity()) {
            // page_leads[0] - none
            // page_leads[1] - erase
            result.page_leads[0] = page_lead();
            result.page_leads[1] = page_lead(container_page_lead_operation::erase, page.pos());
            std::memmove(&result.page_leads[1].items[0], &container_page->items[0], sizeof(T));

            // Merge the items from this page into the previous one.
            std::memmove(&prev_container_page->items[prev_container_page->item_count], &container_page->items[0], container_page->item_count * sizeof(T));

            // Update the result only if itr references this page.
            // If we deleted the last item on this page, itr references item[0] on the next page, and will not be affected by this balancing.
            if (itr.page_pos() == page.pos()) {
                if (itr.item_pos() != item_pos_nil) {
                    result.iterator = iterator(this, prev_page.pos(), itr.item_pos() + prev_container_page->item_count, iterator_edge::none, diag_base::log());
                }
                else {
                    result.iterator = iterator(this, prev_page.pos(), itr.item_pos(), itr.edge(), diag_base::log());
                }
            }

            // Update the item count on the previous page.
            prev_container_page->item_count += container_page->item_count;

            // Free this page.
            erase_page(page);
            container_page = nullptr;
        }

        diag_base::ensure(suborigin, result.iterator.is_valid(this), __TAG__, "result.iterator.is_valid(this)");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10478, "End: result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
                (unsigned long long)result.iterator.page_pos(), (unsigned)result.iterator.item_pos(), result.iterator.edge());

        return result;
    }


    template <typename T, typename Header>
    inline void container<T, Header>::erase_page(vmem::page& page) {
        constexpr const char* suborigin = "erase_page";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10479, "Begin: page_pos=0x%llx", (unsigned long long)page.pos());

        page_pos_t page_pos = page.pos();
        erase_page_pos(page_pos);
        page.free();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1047a, "End: page_pos=0x%llx", (unsigned long long)page_pos);
    }


    template <typename T, typename Header>
    inline void container<T, Header>::erase_page_pos(page_pos_t page_pos) {
        constexpr const char* suborigin = "erase_page_pos";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1047b, "Begin: page_pos=0x%llx", (unsigned long long)page_pos);

        vmem::linked linked(_state, _pool, diag_base::log());
        linked_iterator itr(&linked, page_pos, item_pos_nil, iterator_edge::none, diag_base::log());
        linked.erase(itr);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1047c, "End: page_pos=0x%llx", (unsigned long long)page_pos);
    }


    template <typename T, typename Header>
    inline bool container<T, Header>::should_balance_erase(const vmem::container_page<T, Header>* container_page, item_pos_t item_pos) const noexcept {
        bool should_balance = false;

        if (container_page->prev_page_pos == page_pos_nil && item_pos == 0) {
            should_balance = vmem::test(_balance_erase, page_balance::begin);
        }
        else if (container_page->next_page_pos == page_pos_nil && item_pos == container_page->item_count - 1) {
            should_balance = vmem::test(_balance_erase, page_balance::end);
        }
        else {
            should_balance = vmem::test(_balance_erase, page_balance::inner);
        }

        return should_balance;
    }


    // ..............................................................


    template <typename T, typename Header>
    inline void container<T, Header>::clear() {
        vmem::linked linked(_state, _pool, diag_base::log());
        linked.clear();
    }


    // ..............................................................


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::next(const iterator_state& itr) const {
        constexpr const char* suborigin = "next";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1047d, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
                (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), itr.edge());

        diag_base::expect(suborigin, itr.is_valid(this), __TAG__, "itr.is_valid(this)");
        diag_base::expect(suborigin, itr.is_rbegin() || itr.can_deref(), __TAG__, "itr.is_rbegin() || itr.can_deref()");

        iterator result = end_itr();

        if (itr.is_rbegin()) {
            result = begin_itr();
        }
        else {
            vmem::page page(_pool, itr.page_pos(), diag_base::log());
            diag_base::expect(suborigin, page.pos() == itr.page_pos(), __TAG__, "page.pos() == itr.page_pos()");
            diag_base::expect(suborigin, page.ptr() != nullptr, 0x1047e, "page.ptr() != nullptr");

            vmem::container_page<T, Header>* container_page = reinterpret_cast<vmem::container_page<T, Header>*>(page.ptr());
            diag_base::put_any(suborigin, diag::severity::verbose, __TAG__, "item_count=%u, page_capacity=%zu", container_page->item_count, (std::size_t)page_capacity());

            if (itr.item_pos() < container_page->item_count - 1) {
                result = iterator(this, itr.page_pos(), itr.item_pos() + 1, iterator_edge::none, diag_base::log());
            }
            else {
                if (container_page->next_page_pos != page_pos_nil) {
                    // The first item on the next page is well known - 0.
                    result = iterator(this, container_page->next_page_pos, 0, iterator_edge::none, diag_base::log());
                }
            }
        }

        diag_base::ensure(suborigin, result.is_valid(this), __TAG__, "result.is_valid(this)");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1047f, "End: result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
                (unsigned long long)result.page_pos(), (unsigned)result.item_pos(), result.edge());

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::prev(const iterator_state& itr) const {
        constexpr const char* suborigin = "prev";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10480, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
                (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), itr.edge());

        diag_base::expect(suborigin, itr.is_valid(this), __TAG__, "itr.is_valid(this)");
        diag_base::expect(suborigin, itr.is_end() || itr.can_deref(), __TAG__, "itr.is_end() || itr.can_deref()");

        iterator result = rbegin_itr();

        if (itr.is_end()) {
            result = rend_itr();
        }
        else {
            vmem::page page(_pool, itr.page_pos(), diag_base::log());
            diag_base::expect(suborigin, page.pos() == itr.page_pos(), __TAG__, "page.pos() == itr.page_pos()");
            diag_base::expect(suborigin, page.ptr() != nullptr, 0x10481, "page.ptr() != nullptr");

            vmem::container_page<T, Header>* container_page = reinterpret_cast<vmem::container_page<T, Header>*>(page.ptr());
            diag_base::put_any(suborigin, diag::severity::verbose, __TAG__, "item_count=%u, page_capacity=%zu", container_page->item_count, (std::size_t)page_capacity());

            if (itr.item_pos() > 0) {
                result = iterator(this, itr.page_pos(), itr.item_pos() - 1, iterator_edge::none, diag_base::log());
            }
            else {
                if (container_page->prev_page_pos != page_pos_nil) {
                    // The last item on the previous page has to be determined.
                    vmem::page prev_page(_pool, container_page->prev_page_pos, diag_base::log());
                    diag_base::expect(suborigin, page.pos() == itr.page_pos(), __TAG__, "page.pos() == itr.page_pos()");
                    diag_base::expect(suborigin, page.ptr() != nullptr, 0x10482, "page.ptr() != nullptr");

                    vmem::container_page<T, Header>* prev_container_page = reinterpret_cast<vmem::container_page<T, Header>*>(prev_page.ptr());
                    diag_base::put_any(suborigin, diag::severity::verbose, __TAG__, "item_count=%u, page_capacity=%zu", prev_container_page->item_count, (std::size_t)page_capacity());

                    result = iterator(this, container_page->prev_page_pos, prev_container_page->item_count - 1, iterator_edge::none, diag_base::log());
                }
            }
        }

        diag_base::ensure(suborigin, result.is_valid(this), __TAG__, "result.is_valid(this)");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10483, "End: result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
                (unsigned long long)result.page_pos(), (unsigned)result.item_pos(), result.edge());

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::pointer container<T, Header>::at(const iterator_state& itr) const {
        constexpr const char* suborigin = "at";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
                (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), itr.edge());

        diag_base::expect(suborigin, itr.is_valid(this), __TAG__, "itr.is_valid(this)");

        item_pos_t byte_pos =
            itr.item_pos() == item_pos_nil ?
                item_pos_nil :
                items_pos() + (itr.item_pos() * sizeof(T));

        pointer result(_pool, itr.page_pos(), byte_pos, diag_base::log());

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: result.page_pos=0x%llx, result.byte_pos=0x%x",
                (unsigned long long)result.page_pos(), (unsigned)result.byte_pos());

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::begin_itr() const {
        constexpr const char* suborigin = "begin_itr";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        // If the container is empty, set to "end".
        iterator result(this, _state->back_page_pos, item_pos_nil, iterator_edge::end, diag_base::log());

        // If the container is not empty, set to the first item.
        if (_state->front_page_pos != page_pos_nil) {
            result = iterator(this, _state->front_page_pos, 0, iterator_edge::none, diag_base::log());
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10484, "End: result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
                (unsigned long long)result.page_pos(), (unsigned)result.item_pos(), result.edge());

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::iterator container<T, Header>::end_itr() const {
        constexpr const char* suborigin = "end_itr";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        // Empty or not, set to "end".
        iterator result(this, _state->back_page_pos, item_pos_nil, iterator_edge::end, diag_base::log());

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10486, "End: result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
                (unsigned long long)result.page_pos(), (unsigned)result.item_pos(), result.edge());

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::reverse_iterator container<T, Header>::rend_itr() const {
        constexpr const char* suborigin = "rend_itr";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        // If the container is empty, set to "rbegin".
        iterator result(this, _state->front_page_pos, item_pos_nil, iterator_edge::rbegin, diag_base::log());

        // If the container is not empty, set to the last item.
        if (_state->back_page_pos != page_pos_nil) {
            vmem::page back_page(_pool, _state->back_page_pos, diag_base::log());
            diag_base::expect(suborigin, back_page.pos() == _state->back_page_pos, __TAG__, "back_page.pos() == _state->back_page_pos");
            diag_base::expect(suborigin, back_page.ptr() != nullptr, 0x10487, "back_page.ptr() != nullptr");

            vmem::container_page<T, Header>* back_container_page = reinterpret_cast<vmem::container_page<T, Header>*>(back_page.ptr());
            diag_base::put_any(suborigin, diag::severity::verbose, __TAG__, "item_count=%u, page_capacity=%zu", back_container_page->item_count, (std::size_t)page_capacity());

            result = iterator(this, _state->back_page_pos, back_container_page->item_count - 1, iterator_edge::none, diag_base::log());
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10488, "End: result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
                (unsigned long long)result.page_pos(), (unsigned)result.item_pos(), result.edge());

        return result;
    }


    template <typename T, typename Header>
    inline typename container<T, Header>::reverse_iterator container<T, Header>::rbegin_itr() const {
        constexpr const char* suborigin = "rbegin_itr";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        // Empty or not, set to "rbegin".
        iterator result(this, _state->front_page_pos, item_pos_nil, iterator_edge::rbegin, diag_base::log());

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10488, "End: result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
                (unsigned long long)result.page_pos(), (unsigned)result.item_pos(), result.edge());

        return result;
    }


    // --------------------------------------------------------------

} }
