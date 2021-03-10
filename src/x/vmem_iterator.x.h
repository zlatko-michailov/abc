/*
MIT License

Copyright (c) 2018-2021 Zlatko Michailov 

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

#include "../exception.h"
#include "../i/vmem.i.h"


namespace abc {

	template <typename Container, typename T, typename Pool, typename Log>
	inline vmem_iterator<Container, T, Pool, Log>::vmem_iterator(const Container* container, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos, vmem_iterator_edge_t edge, Log* log) noexcept
		: _container(container)
		, _page_pos(page_pos)
		, _item_pos(item_pos)
		, _edge(edge)
		, _log(log) {

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10348, "vmem_iterator::vmem_iterator() _page_pos=0x%llx, _item_pos=0x%x", (long long)_page_pos, _item_pos);
		}
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline vmem_iterator<Container, T, Pool, Log>::vmem_iterator(std::nullptr_t) noexcept
		: vmem_iterator<Container, T, Pool, Log>(nullptr, vmem_page_pos_nil, vmem_item_pos_nil, vmem_iterator_edge::end, nullptr) {
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline bool vmem_iterator<Container, T, Pool, Log>::operator ==(const vmem_iterator<Container, T, Pool, Log>& other) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_iterator::operator ==() _page_pos=0x%llx, _item_pos=0x%x, _edge=%u, other._page_pos=0x%llx, other._item_pos=0x%x, other._edge=%u",
				(long long)_page_pos, _item_pos, _edge, (long long)other._page_pos, other._item_pos, other._edge);
		}

		return _container == other._container
			&& _page_pos == other._page_pos
			&& _item_pos == other._item_pos
			&& _edge == other._edge;
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline bool vmem_iterator<Container, T, Pool, Log>::operator !=(const vmem_iterator<Container, T, Pool, Log>& other) const noexcept {
		return !operator ==(other);
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline vmem_iterator<Container, T, Pool, Log>& vmem_iterator<Container, T, Pool, Log>::operator ++() noexcept {
		if (is_valid()) {
			*this = _container->next(*this);
		}

		return *this;
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline vmem_iterator<Container, T, Pool, Log>& vmem_iterator<Container, T, Pool, Log>::operator ++(int) noexcept {
		if (is_valid()) {
			*this = _container->next(*this);
		}

		return *this;
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline vmem_iterator<Container, T, Pool, Log>& vmem_iterator<Container, T, Pool, Log>::operator --() noexcept {
		if (is_valid()) {
			*this = _container->prev(*this);
		}

		return *this;
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline vmem_iterator<Container, T, Pool, Log>& vmem_iterator<Container, T, Pool, Log>::operator --(int) noexcept {
		if (is_valid()) {
			*this = _container->prev(*this);
		}

		return *this;
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline vmem_ptr<T, Pool, Log> vmem_iterator<Container, T, Pool, Log>::operator ->() noexcept {
		return ptr();
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline const vmem_ptr<T, Pool, Log> vmem_iterator<Container, T, Pool, Log>::operator ->() const noexcept {
		return ptr();
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline T& vmem_iterator<Container, T, Pool, Log>::operator *() {
		return deref();
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline const T& vmem_iterator<Container, T, Pool, Log>::operator *() const {
		return deref();
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline bool vmem_iterator<Container, T, Pool, Log>::is_valid() const noexcept {
		return _container != nullptr;
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline bool vmem_iterator<Container, T, Pool, Log>::can_deref() const noexcept {
		return is_valid()
			&& _page_pos != vmem_page_pos_nil
			&& _item_pos != vmem_item_pos_nil
			&& _edge == vmem_iterator_edge::none;
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline vmem_ptr<T, Pool, Log> vmem_iterator<Container, T, Pool, Log>::ptr() const noexcept {
		return is_valid() ? _container->at(*this) : vmem_ptr<T, Pool, Log>(nullptr);
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline T& vmem_iterator<Container, T, Pool, Log>::deref() const {
		vmem_ptr<T, Pool, Log> p = ptr();

		if (p == nullptr) {
			throw exception<std::runtime_error, Log>("vmem_iterator::deref() Dereferencing invalid iterator", 0x1034a);
		}

		return *p;
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline vmem_page_pos_t vmem_iterator<Container, T, Pool, Log>::page_pos() const noexcept {
		return _page_pos;
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline vmem_item_pos_t vmem_iterator<Container, T, Pool, Log>::item_pos() const noexcept {
		return _item_pos;
	}


	template <typename Container, typename T, typename Pool, typename Log>
	inline vmem_iterator_edge_t vmem_iterator<Container, T, Pool, Log>::edge() const noexcept {
		return _edge;
	}

}
