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

	template <typename T, typename Pool, typename Log>
	class vmem_list;

	template <typename T, typename Pool, typename Log = null_log>
	using vmem_list_iterator = vmem_iterator<vmem_list<T, Pool, Log>, T, Pool, Log>;


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log = null_log>
	class vmem_list {
	public:
		using value_type				= T;
		using pointer					= vmem_ptr<T, Pool, Log>;
		using const_pointer				= const pointer;
		using reference					= T&;
		using const_reference			= const T&;
		using iterator					= vmem_list_iterator<T, Pool, Log>;
		using const_iterator			= const iterator;
		using reverse_iterator			= iterator;
		using const_reverse_iterator	= const_iterator;

	public:
		static constexpr std::size_t	items_pos() noexcept;
		static constexpr std::size_t	max_item_size() noexcept;
		static constexpr std::size_t	page_capacity() noexcept;
		static constexpr bool			is_uninit(const vmem_list_state* state) noexcept;

	public:
		vmem_list<T, Pool, Log>(vmem_list_state* state, Pool* pool, Log* log);

		vmem_list<T, Pool, Log>(const vmem_list<T, Pool, Log>& other) noexcept = default;
		vmem_list<T, Pool, Log>(vmem_list<T, Pool, Log>&& other) noexcept = default;

	public:
		iterator				begin() noexcept;
		const_iterator			begin() const noexcept;
		const_iterator			cbegin() const noexcept;

		iterator				end() noexcept;
		const_iterator			end() const noexcept;
		const_iterator			cend() const noexcept;

		reverse_iterator		rend() noexcept;
		const_reverse_iterator	rend() const noexcept;
		const_reverse_iterator	crend() const noexcept;

		reverse_iterator		rbegin() noexcept;
		const_reverse_iterator	rbegin() const noexcept;
		const_reverse_iterator	crbegin() const noexcept;

	public:
		bool					empty() const noexcept;
		std::size_t				size() const noexcept;

		pointer					frontptr() noexcept;
		const_pointer			frontptr() const noexcept;

		reference				front();
		const_reference			front() const;

		pointer					backptr() noexcept;
		const_pointer			backptr() const noexcept;

		reference				back();
		const_reference			back() const;

		void					push_back(const_reference item);
		void					pop_back();

		void					push_front(const_reference item);
		void					pop_front();

		iterator				insert(const_iterator itr, const_reference item);
		template <typename InputItr>
		iterator				insert(const_iterator itr, InputItr first, InputItr last);
		iterator				erase(const_iterator itr);
		iterator				erase(const_iterator first, const_iterator last);
		void					clear() noexcept;

	private:
		friend vmem_list_iterator<T, Pool, Log>;

		void					move_next(iterator& itr) const noexcept;
		void					move_prev(iterator& itr) const noexcept;
		pointer					at(const_iterator& itr) const noexcept;

	private:
		void					begin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept;
		void					rbegin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept;
		void					end_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept;
		void					rend_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept;

	private:
		vmem_list_state*		_state;
		Pool*					_pool;
		Log*					_log;
	};


	// --------------------------------------------------------------

}
