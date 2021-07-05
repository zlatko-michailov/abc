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
#include "vmem_pool.i.h"
#include "vmem_layout.i.h"


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


	template <typename T, typename Header, typename Pool, typename Log>
	class vmem_container;

	template <typename T, typename Header, typename Pool, typename Log = null_log>
	using vmem_container_iterator_state = _vmem_iterator_state<vmem_container<T, Header, Pool, Log>, Pool, Log>;

	template <typename T, typename Header, typename Pool, typename Log = null_log>
	using vmem_container_iterator = vmem_iterator<vmem_container<T, Header, Pool, Log>, T, Pool, Log>;

	template <typename T, typename Header, typename Pool, typename Log = null_log>
	using vmem_container_const_iterator = vmem_iterator<vmem_container<T, Header, Pool, Log>, T, Pool, Log>;


	// --------------------------------------------------------------


	using vmem_container_page_lead_operation_t	= std::uint8_t;

	namespace vmem_container_page_lead_operation {
		constexpr vmem_container_page_lead_operation_t	none			= 0x0;
		constexpr vmem_container_page_lead_operation_t	erase			= 0x1;
		constexpr vmem_container_page_lead_operation_t	insert			= 0x2;
		constexpr vmem_container_page_lead_operation_t	replace			= 0x3;
		constexpr vmem_container_page_lead_operation_t	supplemental	= 0x4;
	}

	template <typename T>
	struct vmem_container_page_lead {
		vmem_container_page_lead() noexcept;
		template <typename Other>
		vmem_container_page_lead(const Other& other) noexcept;
		vmem_container_page_lead(vmem_container_page_lead_operation_t operation, vmem_page_pos_t page_pos) noexcept;
		template <typename Key>
		vmem_container_page_lead(vmem_container_page_lead_operation_t operation, const Key& items_0_key, const Key& items_1_key, vmem_page_pos_t page_pos) noexcept;

		vmem_container_page_lead_operation_t			operation;
		T												items[2];
		vmem_page_pos_t									page_pos;
	};


	template <typename T, typename Header, typename Pool, typename Log>
	struct vmem_container_result2 {
		vmem_container_iterator<T, Header, Pool, Log>	iterator	= nullptr;
		vmem_container_page_lead<T>						page_leads[2];
	};


	// --------------------------------------------------------------


	template <typename T, typename Header, typename Pool, typename Log = null_log>
	class vmem_container {
		using iterator_state			= vmem_container_iterator_state<T, Header, Pool, Log>;

	public:
		using value_type				= T;
		using pointer					= vmem_ptr<T, Pool, Log>;
		using const_pointer				= vmem_ptr<const T, Pool, Log>;
		using reference					= T&;
		using const_reference			= const T&;
		using iterator					= vmem_container_iterator<T, Header, Pool, Log>;
		using const_iterator			= vmem_container_const_iterator<T, Header, Pool, Log>;
		using reverse_iterator			= iterator;
		using const_reverse_iterator	= const_iterator;
		using result2					= vmem_container_result2<T, Header, Pool, Log>;
		using page_lead					= vmem_container_page_lead<T>;

	public:
		static constexpr std::size_t	items_pos() noexcept;
		static constexpr std::size_t	max_item_size() noexcept;
		static constexpr std::size_t	page_capacity() noexcept;
		static constexpr bool			is_uninit(const vmem_container_state* state) noexcept;

	public:
		vmem_container<T, Header, Pool, Log>(vmem_container_state* state, vmem_page_balance_t balance_insert, vmem_page_balance_t balance_erase, Pool* pool, Log* log);
		vmem_container<T, Header, Pool, Log>(const vmem_container<T, Header, Pool, Log>& other) noexcept = default;
		vmem_container<T, Header, Pool, Log>(vmem_container<T, Header, Pool, Log>&& other) noexcept = default;

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

		result2					insert2(const_iterator itr, const_reference item);
		iterator				insert(const_iterator itr, const_reference item);
		template <typename InputItr>
		iterator				insert(const_iterator itr, InputItr first, InputItr last);

		result2					erase2(const_iterator itr);
		iterator				erase(const_iterator itr);
		iterator				erase(const_iterator first, const_iterator last);

		void					clear() noexcept;

	// insert() helpers
	private:
		result2					insert_nostate(const_iterator itr, const_reference item) noexcept;
		result2					insert_empty(const_reference item) noexcept;
		result2					insert_nonempty(const_iterator itr, const_reference item) noexcept;
		result2					insert_with_overflow(const_iterator itr, const_reference item, vmem_container_page<T, Header>* container_page) noexcept;
		result2					insert_with_capacity(const_iterator itr, const_reference item, vmem_container_page<T, Header>* container_page) noexcept;
		void					balance_split(vmem_page_pos_t page_pos, vmem_container_page<T, Header>* container_page, vmem_page_pos_t new_page_pos, vmem_container_page<T, Header>* new_container_page) noexcept;
		bool					insert_page_after(vmem_page_pos_t after_page_pos, vmem_page<Pool, Log>& new_page, vmem_container_page<T, Header>*& new_container_page) noexcept;
		bool					should_balance_insert(const_iterator itr, const vmem_container_page<T, Header>* container_page) const noexcept;

	// erase() helpers
	private:
		result2					erase_nostate(const_iterator itr) noexcept;
		result2					erase_from_many(const_iterator itr, vmem_container_page<T, Header>* container_page) noexcept;
		result2					balance_merge(const_iterator itr, vmem_page<Pool, Log>& page, vmem_container_page<T, Header>* container_page) noexcept;
		result2					balance_merge_next(const_iterator itr, vmem_page<Pool, Log>& page, vmem_container_page<T, Header>* container_page) noexcept;
		result2					balance_merge_prev(const_iterator itr, vmem_page<Pool, Log>& page, vmem_container_page<T, Header>* container_page) noexcept;
		bool					erase_page(vmem_page<Pool, Log>& page) noexcept;
		bool					erase_page_pos(vmem_page_pos_t page_pos) noexcept;
		bool					should_balance_erase(const vmem_container_page<T, Header>* container_page, vmem_item_pos_t item_pos) const noexcept;

	private:
		friend iterator_state;
		friend const_iterator;
		friend iterator;

		iterator				next(const iterator_state& itr) const noexcept;
		iterator				prev(const iterator_state& itr) const noexcept;
		pointer					at(const iterator_state& itr) const noexcept;

	private:
		iterator				begin_itr() const noexcept;
		iterator				end_itr() const noexcept;
		reverse_iterator		rend_itr() const noexcept;
		reverse_iterator		rbegin_itr() const noexcept;

	private:
		vmem_container_state*	_state;
		vmem_page_balance_t		_balance_insert;
		vmem_page_balance_t		_balance_erase;
		Pool*					_pool;
		Log*					_log;
	};


	// --------------------------------------------------------------

}
