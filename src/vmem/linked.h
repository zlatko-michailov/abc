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

#include <cstring>

#include "../root/util.h"
#include "../diag/diag_ready.h"
#include "ptr.h"
#include "iterator.h"
#include "pool.h"
#include "i/linked.i.h"


namespace abc { namespace vmem {

    inline constexpr const char* linked::origin() noexcept {
        return "abc::vmem::linked";
    }


    inline constexpr bool linked::is_uninit(const linked_state* state) noexcept {
        return
            // nil
            (
                state != nullptr
                && state->front_page_pos == page_pos_nil
                && state->back_page_pos == page_pos_nil
            )
            ||
            // zero
            (
                state != nullptr
                && state->front_page_pos == 0
                && state->back_page_pos == 0
            );
    }


    inline linked::linked(linked_state* state, vmem::pool* pool, diag::log_ostream* log, bool is_free_pages)
        : diag_base(abc::copy(origin()), log)
        , _state(state)
        , _pool(pool)
        , _is_free_pages(is_free_pages) {

        constexpr const char* suborigin = "linked()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1048a, "Begin: _state=%p, _pool=%p", _state, _pool);

        diag_base::expect(suborigin, _state != nullptr, 0x1048b, "_state != nullptr");
        diag_base::expect(suborigin, _pool != nullptr, 0x1048c, "_pool != nullptr");

        if (is_uninit(_state)) {
            _state->front_page_pos = page_pos_nil;
            _state->back_page_pos = page_pos_nil;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: front_page_pos=0x%llx, back_page_pos=0x%llx", 
                (unsigned long long)_state->front_page_pos, (unsigned long long)_state->back_page_pos);
    }


    inline linked::linked(linked_state* state, vmem::pool* pool, diag::log_ostream* log)
        : linked(state, pool, log, false /*is_free_pages*/) {
    }


    inline typename linked::iterator linked::begin() noexcept {
        return begin_itr();
    }


    inline typename linked::const_iterator linked::begin() const noexcept {
        return begin_itr();
    }


    inline typename linked::const_iterator linked::cbegin() const noexcept {
        return begin_itr();
    }


    inline typename linked::iterator linked::end() noexcept {
        return end_itr();
    }


    inline typename linked::const_iterator linked::end() const noexcept {
        return end_itr();
    }


    inline typename linked::const_iterator linked::cend() const noexcept {
        return end_itr();
    }


    inline typename linked::iterator linked::rend() noexcept {
        return rend_itr();
    }


    inline typename linked::const_iterator linked::rend() const noexcept {
        return rend_itr();
    }


    inline typename linked::const_iterator linked::crend() const noexcept {
        return rend_itr();
    }


    inline typename linked::iterator linked::rbegin() noexcept {
        return rbegin_itr();
    }


    inline typename linked::const_iterator linked::rbegin() const noexcept {
        return rbegin_itr();
    }


    inline typename linked::const_iterator linked::crbegin() const noexcept {
        return rbegin_itr();
    }


    inline bool linked::empty() const noexcept {
        return _state->front_page_pos == page_pos_nil
            || _state->back_page_pos == page_pos_nil;
    }


    inline typename linked::reference linked::front() {
        return *at(begin());
    }


    inline typename linked::const_reference linked::front() const {
        return *at(begin());
    }


    inline typename linked::reference linked::back() {
        return *at(rend());
    }


    inline typename linked::const_reference linked::back() const {
        return *at(rend());
    }


    inline void linked::push_back(const_reference page_pos) {
        insert(end(), page_pos);
    }


    inline void linked::pop_back() {
        erase(rend());
    }


    inline void linked::push_front(const_reference page_pos) {
        insert(begin(), page_pos);
    }


    inline void linked::pop_front() {
        erase(begin());
    }


    // ..............................................................


    inline typename linked::iterator linked::insert(const_iterator itr, const_reference page_pos) {
        constexpr const char* suborigin = "insert()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10490, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, page_pos=0x%llx",
            (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), (unsigned long long)page_pos);

        diag_base::expect(suborigin, itr.item_pos() == item_pos_nil, 0x1048e, "itr.item_pos() == item_pos_nil");
        diag_base::expect(suborigin, itr.page_pos() != page_pos_nil || itr.edge() == iterator_edge::end, 0x1048f, "itr.page_pos() != page_pos_nil || itr.edge() == iterator_edge::end");

        // Regardless of where we insert, the result will be this iterator upon success.
        iterator result(this, page_pos, item_pos_nil, iterator_edge::none, diag_base::log());

        // Insert without changing the state.
        insert_nostate(itr, page_pos, _state->back_page_pos);

        // Update the front page pos, if needed.
        if (_state->front_page_pos == page_pos_nil || _state->front_page_pos == itr.page_pos()) {
            _state->front_page_pos = page_pos;
        }

        // Update the back page pos, if needed.
        if (_state->back_page_pos == page_pos_nil || itr.edge() == iterator_edge::end) {
            _state->back_page_pos = page_pos;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10491, "End:");

        return result;
    }


    inline void linked::insert_nostate(const_iterator itr, const_reference page_pos, page_pos_t back_page_pos) {
        constexpr const char* suborigin = "insert_nostate()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10492, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, page_pos=0x%llx",
            (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), (unsigned long long)page_pos);

        diag_base::expect(suborigin, page_pos != page_pos_nil, __TAG__, "page_pos != page_pos_nil");

        vmem::page page(_pool, page_pos, diag_base::log());
        diag_base::expect(suborigin, page.ptr() != nullptr, 0x10493, "page.ptr() != nullptr");

        // Init the page layout.
        vmem::linked_page* linked_page = reinterpret_cast<vmem::linked_page*>(page.ptr());
        linked_page->page_pos = page_pos;
        linked_page->prev_page_pos = page_pos_nil;
        linked_page->next_page_pos = page_pos_nil;

        if (empty()) {
            // Nothing to do.
        }
        else if (itr.page_pos() == page_pos_nil) {
            // Inserting at the end.

            diag_base::expect(suborigin, back_page_pos != page_pos_nil, __TAG__, "back_page_pos != page_pos_nil");

            vmem::page back_page(_pool, back_page_pos, diag_base::log());
            diag_base::expect(suborigin, back_page.ptr() != nullptr, 0x10494, "page.ptr() != nullptr");

            vmem::linked_page* back_linked_page = reinterpret_cast<vmem::linked_page*>(back_page.ptr());
            back_linked_page->next_page_pos = page.pos();
            linked_page->prev_page_pos = back_page_pos;
        }
        else {
            // Inserting at the middle or at the front.
            // A previous page may or may not exist, but the next page does, and itr is pointing at it.

            diag_base::expect(suborigin, itr.page_pos() != page_pos_nil, __TAG__, "itr.page_pos() != page_pos_nil");

            vmem::page next_page(_pool, itr.page_pos(), diag_base::log());
            diag_base::expect(suborigin, next_page.ptr() != nullptr, 0x10495, "page.ptr() != nullptr");

            vmem::linked_page* next_linked_page = reinterpret_cast<vmem::linked_page*>(next_page.ptr());

            if (next_linked_page->prev_page_pos == page_pos_nil) {
                // Inserting at the front.
                linked_page->next_page_pos = next_page.pos();
                next_linked_page->prev_page_pos = page.pos();
            }
            else {
                // Inserting at the middle.
                diag_base::expect(suborigin, next_linked_page->prev_page_pos != page_pos_nil, __TAG__, "next_linked_page->prev_page_pos != page_pos_nil");

                vmem::page prev_page(_pool, next_linked_page->prev_page_pos, diag_base::log());
                diag_base::expect(suborigin, next_page.ptr() != nullptr, 0x10496, "page.ptr() != nullptr");
    
                vmem::linked_page* prev_linked_page = reinterpret_cast<vmem::linked_page*>(prev_page.ptr());
                prev_linked_page->next_page_pos = page.pos();
                linked_page->prev_page_pos = prev_page.pos();

                linked_page->next_page_pos = next_page.pos();
                next_linked_page->prev_page_pos = page.pos();
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10497, "End:");
    }


    // ..............................................................


    inline typename linked::iterator linked::erase(const_iterator itr) {
        constexpr const char* suborigin = "erase()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10499, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x", (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos());

        diag_base::expect(suborigin, itr.item_pos() == item_pos_nil, __TAG__, "itr.item_pos() == item_pos_nil");
        diag_base::expect(suborigin, itr.page_pos() != page_pos_nil && itr.edge() == iterator_edge::none, 0x10498, "itr.page_pos() != page_pos_nil && itr.edge() == iterator_edge::none");

        // The result, upon success, is the next of itr.
        iterator result = next(itr);

        page_pos_t back_page_pos = page_pos_nil;
        erase_nostate(itr, back_page_pos);

        // Update the front page pos, if needed.
        if (_state->front_page_pos == itr.page_pos()) {
            _state->front_page_pos = result.page_pos();
        }

        // Update the back page pos, if needed.
        if (_state->back_page_pos == itr.page_pos()) {
            _state->back_page_pos = back_page_pos;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1049a, "End:");

        return result;
    }


    inline void linked::erase_nostate(const_iterator itr, page_pos_t& back_page_pos) {
        constexpr const char* suborigin = "erase()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1049b, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x", (unsigned long long)itr.page_pos(), (unsigned)itr.item_pos());

        diag_base::expect(suborigin, itr.page_pos() != page_pos_nil, __TAG__, "itr.page_pos() != page_pos_nil");

        vmem::page page(_pool, itr.page_pos(), diag_base::log());
        diag_base::expect(suborigin, page.ptr() != nullptr, 0x1049c, "page.ptr() != nullptr");

        vmem::linked_page* linked_page = reinterpret_cast<vmem::linked_page*>(page.ptr());

        if (linked_page->prev_page_pos != page_pos_nil) {
            // There is a prev page.
            vmem::page prev_page(_pool, linked_page->prev_page_pos, diag_base::log());
            diag_base::expect(suborigin, prev_page.ptr() != nullptr, 0x1049d, "prev_page.ptr() != nullptr");

            vmem::linked_page* prev_linked_page = reinterpret_cast<vmem::linked_page*>(prev_page.ptr());
            prev_linked_page->next_page_pos = linked_page->next_page_pos;
        }

        if (linked_page->next_page_pos != page_pos_nil) {
            // There is a next page.
            vmem::page next_page(_pool, linked_page->next_page_pos, diag_base::log());
            diag_base::expect(suborigin, next_page.ptr() != nullptr, 0x1049e, "next_page.ptr() != nullptr");

            vmem::linked_page* next_linked_page = reinterpret_cast<vmem::linked_page*>(next_page.ptr());
            next_linked_page->prev_page_pos = linked_page->prev_page_pos;
        }
        else {
            // There is no next page, which means we are deleting the back page.
            // Export back the new back page pos.
            back_page_pos = linked_page->prev_page_pos;
        }

        linked_page = nullptr;
        
        if (!_is_free_pages) {
            page.free();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1049f, "End:");
    }


    // ..............................................................


    inline void linked::clear() {
        _pool->clear_linked(*this);

        _state->front_page_pos = page_pos_nil;
        _state->back_page_pos = page_pos_nil;
    }


    // ..............................................................


    inline void linked::splice(linked& other) {
        splice(std::move(other));
    }


    inline void linked::splice(linked&& other) {
        constexpr const char* suborigin = "splice()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x104a1, "Begin: front_page_pos=0x%llx, back_page_pos=0x%llx, other.front_page_pos=0x%llx, other.back_page_pos=0x%llx",
                (unsigned long long)_state->front_page_pos, (unsigned long long)_state->back_page_pos, (unsigned long long)other._state->front_page_pos, (unsigned long long)other._state->back_page_pos);

        diag_base::expect(suborigin, _state != other._state, 0x104a0, "_state != other._state");

        if (other.empty()) {
            // Nothing to do.
        }
        else if (empty()) {
            // Take over the other state.
            *_state = *other._state;
        }
        else {
            // Connect the back page of this and the front page of the other.
            {
                diag_base::expect(suborigin, _state->back_page_pos != page_pos_nil, __TAG__, "_state->back_page_pos != page_pos_nil");

                // ... this back to the other front.
                vmem::page back_page(_pool, _state->back_page_pos, diag_base::log());
                diag_base::expect(suborigin, back_page.ptr() != nullptr, 0x104a2, "back_page.ptr() != nullptr");

                vmem::linked_page* back_linked_page = reinterpret_cast<vmem::linked_page*>(back_page.ptr());
                back_linked_page->next_page_pos = other._state->front_page_pos;
            }

            {
                diag_base::expect(suborigin, other._state->front_page_pos != page_pos_nil, __TAG__, "other._state->front_page_pos != page_pos_nil");

                // ... the other front to this back.
                vmem::page other_front_page(_pool, other._state->front_page_pos, diag_base::log());
                diag_base::expect(suborigin, other_front_page.ptr() != nullptr, 0x104a3, "other_front_page.ptr() != nullptr");

                vmem::linked_page* other_front_linked_page = reinterpret_cast<vmem::linked_page*>(other_front_page.ptr());
                other_front_linked_page->prev_page_pos = _state->back_page_pos;
            }

            // Update this state.
            _state->back_page_pos = other._state->back_page_pos;

            // Empty the other state.
            other._state->front_page_pos = page_pos_nil;
            other._state->back_page_pos = page_pos_nil;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104a4, "End:");
    }


    // ..............................................................


    inline typename linked::iterator linked::next(const iterator_state& itr) const {
        constexpr const char* suborigin = "next()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x104a5, "Begin: itr.page_pos=0x%llx, itr.edge=%u, itr.item_pos=0x%x",
            (unsigned long long)itr.page_pos(), (unsigned)itr.edge(), (unsigned)itr.item_pos());

        diag_base::expect(suborigin, itr.item_pos() == item_pos_nil, __TAG__, "itr.item_pos() == item_pos_nil");

        iterator result = end_itr();

        if (itr == static_cast<iterator_state>(end())) {
            // Nothing to do.
        }
        else if (itr == static_cast<iterator_state>(rbegin())) {
            result = begin_itr();
        }
        else if (itr == static_cast<iterator_state>(rend())) {
            // Nothing to do.
        }
        else if (itr.page_pos() != page_pos_nil) {
            vmem::page page(_pool, itr.page_pos(), diag_base::log());
            diag_base::expect(suborigin, page.ptr() != nullptr, 0x104a6, "page.ptr() != nullptr");

            vmem::linked_page* linked_page = reinterpret_cast<vmem::linked_page*>(page.ptr());

            iterator_edge edge = linked_page->next_page_pos == page_pos_nil ? iterator_edge::end : iterator_edge::none;
            result = iterator(this, linked_page->next_page_pos, item_pos_nil, edge, diag_base::log());
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104a7, "End: result.page_pos=0x%llx, result.edge=%u", (unsigned long long)result.page_pos(), (unsigned)result.edge());

        return result;
    }


    inline typename linked::iterator linked::prev(const iterator_state& itr) const {
        constexpr const char* suborigin = "next()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x104a8, "Begin: itr.page_pos=0x%llx, itr.edge=%u, itr.item_pos=0x%x",
            (unsigned long long)itr.page_pos(), (unsigned)itr.edge(), (unsigned)itr.item_pos());

        diag_base::expect(suborigin, itr.item_pos() == item_pos_nil, __TAG__, "itr.item_pos() == item_pos_nil");

        iterator result = rbegin_itr();

        if (itr == static_cast<iterator_state>(rbegin())) {
            // Nothing to do.
        }
        else if (itr == static_cast<iterator_state>(begin())) {
            // Nothing to do.
        }
        else if (itr == static_cast<iterator_state>(end())) {
            result = rend_itr();
        }
        else if (itr.page_pos() != page_pos_nil) {
            vmem::page page(_pool, itr.page_pos(), diag_base::log());
            diag_base::expect(suborigin, page.ptr() != nullptr, 0x104a9, "page.ptr() != nullptr");

            vmem::linked_page* linked_page = reinterpret_cast<vmem::linked_page*>(page.ptr());

            iterator_edge edge = linked_page->prev_page_pos == page_pos_nil ? iterator_edge::rbegin : iterator_edge::none;
            result = iterator(this, linked_page->prev_page_pos, item_pos_nil, edge, diag_base::log());
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104aa, "End: result.page_pos=0x%llx, result.edge=%u", (unsigned long long)result.page_pos(), (unsigned)result.edge());

        return result;
    }


    inline typename linked::pointer linked::at(const iterator_state& itr) const {
        return pointer(_pool, itr.page_pos(), 0, diag_base::log());
    }


    inline typename linked::iterator linked::begin_itr() const noexcept {
        if (_state->front_page_pos == page_pos_nil) {
            return end_itr();
        }

        return iterator(this, _state->front_page_pos, item_pos_nil, iterator_edge::none, diag_base::log());
    }


    inline typename linked::iterator linked::end_itr() const noexcept {
        return iterator(this, page_pos_nil, item_pos_nil, iterator_edge::end, diag_base::log());
    }


    inline typename linked::reverse_iterator linked::rend_itr() const noexcept {
        if (_state->back_page_pos == page_pos_nil) {
            return rbegin_itr();
        }

        return iterator(this, _state->back_page_pos, item_pos_nil, iterator_edge::none, diag_base::log());
    }


    inline typename linked::reverse_iterator linked::rbegin_itr() const noexcept {
        return iterator(this, page_pos_nil, item_pos_nil, iterator_edge::rbegin, diag_base::log());
    }

} }
