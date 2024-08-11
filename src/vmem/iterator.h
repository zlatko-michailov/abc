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

#include "../diag/diag_ready.h"
#include "i/iterator.i.h"


namespace abc { namespace vmem {

    template <typename Container>
    inline constexpr const char* basic_iterator_state<Container>::origin() noexcept {
        return "abc::vmem::basic_iterator_state";
    }


    template <typename Container>
    inline basic_iterator_state<Container>::basic_iterator_state(const Container* container, page_pos_t page_pos, item_pos_t item_pos, iterator_edge edge, diag::log_ostream* log) noexcept
        : diag_base(abc::copy(origin()), log),
         _container(container)
        , _page_pos(page_pos)
        , _item_pos(item_pos)
        , _edge(edge) {

        constexpr const char* suborigin = "basic_iterator_state()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10604, "Begin: _page_pos=0x%llx, _item_pos=0x%x", (unsigned long long)_page_pos, (unsigned)_item_pos);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename Container>
    inline basic_iterator_state<Container>::basic_iterator_state(std::nullptr_t, diag::log_ostream* log) noexcept
        : basic_iterator_state<Container>(nullptr, page_pos_nil, item_pos_nil, iterator_edge::end, log) {
    }


    template <typename Container>
    inline bool basic_iterator_state<Container>::operator ==(const basic_iterator_state<Container>& other) const noexcept {
        constexpr const char* suborigin = "operator ==()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10605, "Begin: _page_pos=0x%llx, _item_pos=0x%x, _edge=%u, other._page_pos=0x%llx, other._item_pos=0x%x, other._edge=%u",
            (unsigned long long)_page_pos, (unsigned)_item_pos, (unsigned)_edge, (unsigned long long)other._page_pos, (unsigned)other._item_pos, (unsigned)other._edge);

        bool are_equal = _container == other._container
            && _page_pos == other._page_pos
            && _item_pos == other._item_pos
            && _edge == other._edge;

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: are_equal=%d", are_equal);

        return are_equal;
    }


    template <typename Container>
    inline bool basic_iterator_state<Container>::operator !=(const basic_iterator_state<Container>& other) const noexcept {
        return !operator ==(other);
    }


    template <typename Container>
    inline bool basic_iterator_state<Container>::is_valid() const noexcept {
        return _container != nullptr;
    }


    template <typename Container>
    inline bool basic_iterator_state<Container>::can_deref() const noexcept {
        return is_valid()
            && _page_pos != page_pos_nil
            && _item_pos != item_pos_nil
            && _edge == iterator_edge::none;
    }


    template <typename Container>
    inline const Container* basic_iterator_state<Container>::container() const noexcept {
        return _container;
    }


    template <typename Container>
    inline page_pos_t basic_iterator_state<Container>::page_pos() const noexcept {
        return _page_pos;
    }


    template <typename Container>
    inline item_pos_t basic_iterator_state<Container>::item_pos() const noexcept {
        return _item_pos;
    }


    template <typename Container>
    inline iterator_edge basic_iterator_state<Container>::edge() const noexcept {
        return _edge;
    }


    // --------------------------------------------------------------


    template <typename Base, typename Container, typename T>
    inline basic_iterator<Base, Container, T>::basic_iterator(const Container* container, page_pos_t page_pos, item_pos_t item_pos, iterator_edge edge, diag::log_ostream* log) noexcept
        : Base(container, page_pos, item_pos, edge, log) {
    }


    template <typename Base, typename Container, typename T>
    template <typename OtherIterator>
    inline basic_iterator<Base, Container, T>::basic_iterator(const OtherIterator& other) noexcept
        : Base(other.container(), other.page_pos(), other.item_pos(), other.edge(), other.log()) {
    }


    template <typename Base, typename Container, typename T>
    inline basic_iterator<Base, Container, T>::basic_iterator(std::nullptr_t, diag::log_ostream* log) noexcept
        : Base(nullptr, log) {
    }


    template <typename Base, typename Container, typename T>
    inline bool basic_iterator<Base, Container, T>::operator ==(const basic_iterator<Base, Container, T>& other) const noexcept {
        return Base::operator ==(other);
    }


    template <typename Base, typename Container, typename T>
    inline bool basic_iterator<Base, Container, T>::operator !=(const basic_iterator<Base, Container, T>& other) const noexcept {
        return Base::operator !=(other);
    }


    template <typename Base, typename Container, typename T>
    inline basic_iterator<Base, Container, T>& basic_iterator<Base, Container, T>::operator ++() noexcept {
        constexpr const char* suborigin = "operator ++()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x107ae, "Begin: _page_pos=0x%llx, _item_pos=0x%x, _edge=%u", (unsigned long long)base::_page_pos, (unsigned)base::_item_pos, (unsigned)base::_edge);

        if (Base::is_valid()) {
            *this = Base::_container->next(*this);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: _page_pos=0x%llx, _item_pos=0x%x, _edge=%u", (unsigned long long)base::_page_pos, (unsigned)base::_item_pos, (unsigned)base::_edge);

        return *this;
    }


    template <typename Base, typename Container, typename T>
    inline basic_iterator<Base, Container, T> basic_iterator<Base, Container, T>::operator ++(int) noexcept {
        constexpr const char* suborigin = "operator ++(int)";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x107af, "Begin: _page_pos=0x%llx, _item_pos=0x%x, _edge=%u", (unsigned long long)base::_page_pos, (unsigned)base::_item_pos, (unsigned)base::_edge);

        basic_iterator<Base, Container, T> thisCopy = *this;

        if (Base::is_valid()) {
            *this = Base::_container->next(*this);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: _page_pos=0x%llx, _item_pos=0x%x, _edge=%u", (unsigned long long)base::_page_pos, (unsigned)base::_item_pos, (unsigned)base::_edge);

        return thisCopy;
    }


    template <typename Base, typename Container, typename T>
    inline basic_iterator<Base, Container, T>& basic_iterator<Base, Container, T>::operator --() noexcept {
        constexpr const char* suborigin = "operator --()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x107b0, "Begin: _page_pos=0x%llx, _item_pos=0x%x, _edge=%u", (unsigned long long)base::_page_pos, (unsigned)base::_item_pos, (unsigned)base::_edge);

        if (Base::is_valid()) {
            *this = Base::_container->prev(*this);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: _page_pos=0x%llx, _item_pos=0x%x, _edge=%u", (unsigned long long)base::_page_pos, (unsigned)base::_item_pos, (unsigned)base::_edge);

        return *this;
    }


    template <typename Base, typename Container, typename T>
    inline basic_iterator<Base, Container, T> basic_iterator<Base, Container, T>::operator --(int) noexcept {
        constexpr const char* suborigin = "operator --(int)";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x107b1, "Begin: _page_pos=0x%llx, _item_pos=0x%x, _edge=%u", (unsigned long long)base::_page_pos, (unsigned)base::_item_pos, (unsigned)base::_edge);

        basic_iterator<Base, Container, T> thisCopy = *this;

        if (Base::is_valid()) {
            *this = Base::_container->prev(*this);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: _page_pos=0x%llx, _item_pos=0x%x, _edge=%u", (unsigned long long)base::_page_pos, (unsigned)base::_item_pos, (unsigned)base::_edge);

        return thisCopy;
    }


    template <typename Base, typename Container, typename T>
    inline typename basic_iterator<Base, Container, T>::pointer basic_iterator<Base, Container, T>::operator ->() noexcept {
        return ptr();
    }


    template <typename Base, typename Container, typename T>
    inline typename basic_iterator<Base, Container, T>::const_pointer basic_iterator<Base, Container, T>::operator ->() const noexcept {
        return ptr();
    }


    template <typename Base, typename Container, typename T>
    inline T& basic_iterator<Base, Container, T>::operator *() {
        return deref();
    }


    template <typename Base, typename Container, typename T>
    inline const T& basic_iterator<Base, Container, T>::operator *() const {
        return deref();
    }


    template <typename Base, typename Container, typename T>
    inline typename basic_iterator<Base, Container, T>::pointer basic_iterator<Base, Container, T>::ptr() const noexcept {
        if (base::is_valid()) {
            typename Container::pointer ptr = const_cast<Container*>(base::_container)->at(*this);
            return pointer(ptr.pool(), ptr.page_pos(), ptr.byte_pos(), diag_base::log());
        }
        
        return pointer(nullptr);
    }


    template <typename Base, typename Container, typename T>
    inline T& basic_iterator<Base, Container, T>::deref() const {
        constexpr const char* suborigin = "deref()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: _page_pos=0x%llx, _item_pos=0x%x, _edge=%u", (unsigned long long)base::_page_pos, (unsigned)base::_item_pos, (unsigned)base::_edge);

        pointer p = ptr();
        diag_base::expect(suborigin, p != nullptr, 0x10606, "p != nullptr");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x107b2, "End: page_pos=0x%llx, byte_pos=0x%x", (unsigned long long)p.page_pos(), (unsigned)p.byte_pos());

        return *p;
    }


    // --------------------------------------------------------------



} }
