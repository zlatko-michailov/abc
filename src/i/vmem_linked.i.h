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

	template <typename Pool, typename Log>
	class vmem_linked;

	template <typename Pool, typename Log = null_log>
	using vmem_linked_iterator = vmem_iterator<vmem_linked<Pool, Log>, typename vmem_linked<Pool, Log>::value_type, Pool, Log>;


	// --------------------------------------------------------------


	template <typename Pool, typename Log = null_log>
	class vmem_linked {
	public:
		using value_type				= vmem_page_pos_t;
		using pointer					= vmem_ptr<value_type, Pool, Log>;
		using const_pointer				= const pointer;
		using reference					= value_type&;
		using const_reference			= const value_type&;
		using iterator					= vmem_linked_iterator<Pool, Log>;
		using const_iterator			= const iterator;
		using reverse_iterator			= iterator;
		using const_reverse_iterator	= const_iterator;

	public:
		static constexpr bool	is_uninit(const vmem_linked_state* state) noexcept;

	public:
		vmem_linked<Pool, Log>(vmem_linked_state* state, Pool* pool, Log* log);

		vmem_linked<Pool, Log>(const vmem_linked<Pool, Log>& other) noexcept = default;
		vmem_linked<Pool, Log>(vmem_linked<Pool, Log>&& other) noexcept = default;

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

		reference				front();
		const_reference			front() const;

		reference				back();
		const_reference			back() const;

		void					push_back(const_reference page_pos);
		void					pop_back();

		void					push_front(const_reference page_pos);
		void					pop_front();

		iterator				insert(const_iterator itr, const_reference page_pos);
		iterator				erase(const_iterator itr);
		void					clear();
		void					splice(vmem_linked<Pool, Log>& /*inout*/ other);
		void					splice(vmem_linked<Pool, Log>&& other);

	private:
		bool					insert_nostate(const_iterator itr, const_reference page_pos, vmem_page_pos_t back_page_pos) noexcept;
		bool					erase_nostate(const_iterator itr, /*out*/ vmem_page_pos_t& back_page_pos) noexcept;

	private:
		friend iterator;

		void					move_next(/*inout*/ iterator& itr) const noexcept;
		void					move_prev(/*inout*/ iterator& itr) const noexcept;
		pointer					at(const_iterator& itr) const noexcept;

	private:
		vmem_linked_state*		_state;
		Pool*					_pool;
		Log*					_log;
	};


	// --------------------------------------------------------------

}
