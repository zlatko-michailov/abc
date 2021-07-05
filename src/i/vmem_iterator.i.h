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

#include <cstdint>

#include "log.i.h"


namespace abc {

	using vmem_iterator_edge_t = std::uint8_t;

	namespace vmem_iterator_edge {
		constexpr vmem_iterator_edge_t	none	= 0;
		constexpr vmem_iterator_edge_t	rbegin	= 1; // before front
		constexpr vmem_iterator_edge_t	end		= 2; // after back
	}


	// --------------------------------------------------------------


	template <typename Container, typename Pool, typename Log = null_log>
	class _vmem_iterator_state {
	public:
		using container					= Container;

	public:
		_vmem_iterator_state<Container, Pool, Log>(const Container* container, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos, vmem_iterator_edge_t edge, Log* log) noexcept;
		_vmem_iterator_state<Container, Pool, Log>(const _vmem_iterator_state<Container, Pool, Log>& other) = default;
		_vmem_iterator_state<Container, Pool, Log>(_vmem_iterator_state<Container, Pool, Log>&& other) noexcept = default;
		_vmem_iterator_state<Container, Pool, Log>(std::nullptr_t) noexcept;

	public:
		_vmem_iterator_state<Container, Pool, Log>&		operator =(const _vmem_iterator_state<Container, Pool, Log>& other) noexcept = default;
		_vmem_iterator_state<Container, Pool, Log>&		operator =(_vmem_iterator_state<Container, Pool, Log>&& other) noexcept = default;

	public:
		bool											operator ==(const _vmem_iterator_state<Container, Pool, Log>& other) const noexcept;
		bool											operator !=(const _vmem_iterator_state<Container, Pool, Log>& other) const noexcept;
		_vmem_iterator_state<Container, Pool, Log>&		operator ++() noexcept;
		_vmem_iterator_state<Container, Pool, Log>&		operator ++(int) noexcept;
		_vmem_iterator_state<Container, Pool, Log>&		operator --() noexcept;
		_vmem_iterator_state<Container, Pool, Log>&		operator --(int) noexcept;

	public:
		bool											is_valid() const noexcept;
		bool											can_deref() const noexcept;

	private:
		_vmem_iterator_state<Container, Pool, Log>&		inc() noexcept;
		_vmem_iterator_state<Container, Pool, Log>&		dec() noexcept;

	public:
		vmem_page_pos_t									page_pos() const noexcept;
		vmem_item_pos_t									item_pos() const noexcept;
		vmem_iterator_edge_t							edge() const noexcept;

	protected:
		const Container*								_container;
		vmem_page_pos_t									_page_pos;
		vmem_item_pos_t									_item_pos;
		vmem_iterator_edge_t							_edge;
		Log*											_log;
	};


	template <typename Base, typename Container, typename T, typename Pool, typename Log = null_log>
	class _vmem_iterator : public Base {
	public:
		using value_type				= T;
		using pointer					= vmem_ptr<T, Pool, Log>;
		using const_pointer				= vmem_ptr<const T, Pool, Log>;
		using reference					= T&;
		using const_reference			= const T&;

	public:
		_vmem_iterator<Base, Container, T, Pool, Log>(const Container* container, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos, vmem_iterator_edge_t edge, Log* log) noexcept;
		_vmem_iterator<Base, Container, T, Pool, Log>(const _vmem_iterator<Base, Container, T, Pool, Log>& other) = default;
		_vmem_iterator<Base, Container, T, Pool, Log>(_vmem_iterator<Base, Container, T, Pool, Log>&& other) noexcept = default;
		_vmem_iterator<Base, Container, T, Pool, Log>(std::nullptr_t) noexcept;

	public:
		_vmem_iterator<Base, Container, T, Pool, Log>&	operator =(const _vmem_iterator<Base, Container, T, Pool, Log>& other) noexcept = default;
		_vmem_iterator<Base, Container, T, Pool, Log>&	operator =(_vmem_iterator<Base, Container, T, Pool, Log>&& other) noexcept = default;

	public:
		pointer											operator ->() noexcept;
		const_pointer									operator ->() const noexcept;
		reference										operator *();
		const_reference									operator *() const;

	private:
		friend Container;

		pointer											ptr() const noexcept;
		reference										deref() const;
	};


	template <typename Container, typename T, typename Pool, typename Log = null_log>
	using vmem_const_iterator = _vmem_iterator<_vmem_iterator_state<Container, Pool, Log>, Container, const T, Pool, Log>;


	template <typename Container, typename T, typename Pool, typename Log = null_log>
	using vmem_iterator = _vmem_iterator<vmem_const_iterator<Container, T, Pool, Log>, Container, T, Pool, Log>;


	// --------------------------------------------------------------

}
