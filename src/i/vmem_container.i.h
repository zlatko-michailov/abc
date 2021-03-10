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

	// --------------------------------------------------------------


	using vmem_page_balance_t = std::uint8_t;
	namespace vmem_page_balance {
		constexpr vmem_page_balance_t none		= 0x00;
		constexpr vmem_page_balance_t begin		= 0x01;
		constexpr vmem_page_balance_t inner		= 0x02;
		constexpr vmem_page_balance_t end		= 0x04;
		constexpr vmem_page_balance_t all		= 0xff;


		bool test(vmem_page_balance_t value, vmem_page_balance_t bits) noexcept;
	}


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log>
	class vmem_container;

	template <typename T, typename Pool, typename Log = null_log>
	using vmem_container_iterator = vmem_iterator<vmem_container<T, Pool, Log>, T, Pool, Log>;


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log = null_log>
	class vmem_container {
	public:
		using value_type				= T;
		using pointer					= vmem_ptr<T, Pool, Log>;
		using const_pointer				= const pointer;
		using reference					= T&;
		using const_reference			= const T&;
		using iterator					= vmem_container_iterator<T, Pool, Log>;
		using const_iterator			= const iterator;
		using reverse_iterator			= iterator;
		using const_reverse_iterator	= const_iterator;

	public:
		static constexpr std::size_t	items_pos() noexcept;
		static constexpr std::size_t	max_item_size() noexcept;
		static constexpr std::size_t	page_capacity() noexcept;
		static constexpr bool			is_uninit(const vmem_container_state* state) noexcept;

	public:
		vmem_container<T, Pool, Log>(vmem_container_state* state, vmem_page_balance_t balance_insert, vmem_page_balance_t balance_erase, Pool* pool, Log* log);
		vmem_container<T, Pool, Log>(const vmem_container<T, Pool, Log>& other) noexcept = default;
		vmem_container<T, Pool, Log>(vmem_container<T, Pool, Log>&& other) noexcept = default;

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

	// insert() helpers
	private:
		bool					insert_nostate(const_iterator itr, const_reference item, /*out*/ vmem_page_pos_t& page_pos, /*out*/ vmem_item_pos_t& item_pos, /*out*/ vmem_page_pos_t& new_page_pos) noexcept;
		bool					insert_empty(const_reference item, /*out*/ vmem_page_pos_t& page_pos) noexcept;
		bool					insert_nonempty(const_iterator itr, const_reference item, /*out*/ vmem_page_pos_t& page_pos, /*out*/ vmem_item_pos_t& item_pos, /*out*/ vmem_page_pos_t& new_page_pos) noexcept;
		bool					insert_with_overflow(const_iterator itr, const_reference item, vmem_container_page<T>* container_page, /*out*/ vmem_page_pos_t& page_pos, /*out*/ vmem_item_pos_t& item_pos, /*out*/ vmem_page_pos_t& new_page_pos) noexcept;
		void					insert_with_capacity_safe(const_iterator itr, const_reference item, vmem_container_page<T>* container_page, /*out*/ vmem_item_pos_t& item_pos) noexcept;
		void					balance_split_safe(vmem_page_pos_t page_pos, vmem_container_page<T>* container_page, vmem_page_pos_t new_page_pos, vmem_container_page<T>* new_container_page) noexcept;
		bool					insert_page_after(vmem_page_pos_t after_page_pos, /*out*/ vmem_page<Pool, Log>& new_page, /*out*/ vmem_container_page<T>*& new_container_page) noexcept;
		bool					should_balance_insert(const_iterator itr, const vmem_container_page<T>* container_page) const noexcept;

	// erase() helpers
	private:
		bool					erase_nostate(const_iterator itr, /*out*/ vmem_page_pos_t& page_pos, /*out*/ vmem_item_pos_t& item_pos, /*out*/ vmem_iterator_edge_t& edge) noexcept;
		void					erase_from_many_safe(const_iterator itr, vmem_container_page<T>* container_page, /*out*/ vmem_page_pos_t& page_pos, /*out*/ vmem_item_pos_t& item_pos, /*out*/ vmem_iterator_edge_t& edge) noexcept;
		void					balance_merge_safe(/*inout*/ vmem_page<Pool, Log>& page, /*inout*/ vmem_container_page<T>* container_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos) noexcept;
		bool					balance_merge_next(vmem_page<Pool, Log>& page, vmem_container_page<T>* container_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos) noexcept;
		bool					balance_merge_prev(/*inout*/ vmem_page<Pool, Log>& page, /*inout*/ vmem_container_page<T>* container_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos) noexcept;
		bool					erase_page(/*inout*/ vmem_page<Pool, Log>& page, /*inout*/ vmem_container_page<T>* container_page) noexcept;
		bool					erase_page_pos(vmem_page_pos_t page_pos) noexcept;
		bool					should_balance_erase(const vmem_container_page<T>* container_page, vmem_item_pos_t item_pos) const noexcept;

	private:
		friend iterator;

		iterator				next(const_iterator& itr) const noexcept;
		iterator				prev(const_iterator& itr) const noexcept;
		pointer					at(const_iterator& itr) const noexcept;

	private:
		iterator				begin_itr() const noexcept;
		iterator				rbegin_itr() const noexcept;
		iterator				end_itr() const noexcept;
		iterator				rend_itr() const noexcept;

	private:
		vmem_container_state*	_state;
		vmem_page_balance_t		_balance_insert;
		vmem_page_balance_t		_balance_erase;
		Pool*					_pool;
		Log*					_log;
	};


	// --------------------------------------------------------------

}
