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


	using vmem_page_balance_t = std::uint16_t;
	namespace vmem_page_balance {
		constexpr vmem_page_balance_t op_insert		= 0x0001;
		constexpr vmem_page_balance_t op_erase		= 0x0002;
		constexpr vmem_page_balance_t op_all		= 0x00ff;

		constexpr vmem_page_balance_t pos_begin		= 0x0100;
		constexpr vmem_page_balance_t pos_inner		= 0x0200;
		constexpr vmem_page_balance_t pos_end		= 0x0400;
		constexpr vmem_page_balance_t pos_all		= 0xff00;

		constexpr vmem_page_balance_t never			= 0x0000;
		constexpr vmem_page_balance_t always		= op_all | pos_all;
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
		vmem_container<T, Pool, Log>(vmem_container_state* state, vmem_page_balance_t balance, Pool* pool, Log* log);
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
		bool					insert_nostate(/*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, const_reference item, bool always_balance, bool itr_end, /*out*/ vmem_page_pos_t& new_page_pos) noexcept;
		bool					insert_empty(const_reference item, /*out*/ vmem_page_pos_t& page_pos) noexcept;
		bool					insert_nonempty(/*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, const_reference item, bool always_balance, bool itr_end, /*out*/ vmem_page_pos_t& new_page_pos) noexcept;
		bool					insert_with_overflow(/*inout*/ vmem_page_pos_t& page_pos, vmem_container_page<T>* list_page, /*inout*/ vmem_item_pos_t& item_pos, const_reference item, bool balance, /*out*/ vmem_page_pos_t& new_page_pos) noexcept;
		void					insert_with_capacity_safe(vmem_page_pos_t page_pos, vmem_container_page<T>* list_page, /*inout*/ vmem_item_pos_t& item_pos, const_reference item) noexcept;
		void					balance_split_safe(vmem_page_pos_t page_pos, vmem_container_page<T>* list_page, vmem_page_pos_t new_page_pos, vmem_container_page<T>* new_list_page) noexcept;
		bool					insert_page_after(vmem_page_pos_t page_pos, vmem_container_page<T>* list_page, /*out*/ vmem_page<Pool, Log>& new_page, /*out*/ vmem_container_page<T>*& new_list_page) noexcept;

	// erase() helpers
	private:
		bool					erase_nostate(/*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge, /*inout*/ vmem_page_pos_t& front_page_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept;
		void					erase_from_many_safe(vmem_container_page<T>* list_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge) noexcept;
		bool					erase_from_one(/*inout*/ vmem_page<Pool, Log>& page, /*inout*/ vmem_container_page<T>* list_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge, /*inout*/ vmem_page_pos_t& front_page_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept;
		void					balance_merge_safe(/*inout*/ vmem_page<Pool, Log>& page, /*inout*/ vmem_container_page<T>* list_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept;
		bool					balance_merge_next(vmem_page<Pool, Log>& page, vmem_container_page<T>* list_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept;
		bool					balance_merge_prev(/*inout*/ vmem_page<Pool, Log>& page, /*inout*/ vmem_container_page<T>* list_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept;
		bool					link_pages(vmem_page_pos_t prev_page_pos, vmem_page_pos_t next_page_pos, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge, /*inout*/ vmem_page_pos_t& front_page_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept;
		bool					link_next_page(vmem_page_pos_t prev_page_pos, vmem_page_pos_t next_page_pos, /*inout*/ vmem_page_pos_t& front_page_pos) noexcept;
		bool					link_prev_page(vmem_page_pos_t prev_page_pos, vmem_page_pos_t next_page_pos, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept;

	private:
		friend iterator;

		void					move_next(iterator& itr) const noexcept;
		void					move_prev(iterator& itr) const noexcept;
		pointer					at(const_iterator& itr) const noexcept;

	private:
		void					begin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept;
		void					rbegin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept;
		void					end_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept;
		void					rend_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept;

	private:
		vmem_container_state*	_state;
		vmem_page_balance_t		_balance;
		Pool*					_pool;
		Log*					_log;
	};


	// --------------------------------------------------------------

}
