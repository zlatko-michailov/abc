/*
MIT License

Copyright (c) 2018-2022 Zlatko Michailov 

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
#include "vmem_container.i.h"


namespace abc {

	template <typename T, typename Pool, typename Log = null_log>
	using vmem_list_iterator = vmem_container_iterator<T, vmem_noheader, Pool, Log>;

	template <typename T, typename Pool, typename Log = null_log>
	using vmem_list_const_iterator = vmem_container_const_iterator<T, vmem_noheader, Pool, Log>;


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log = null_log>
	class vmem_list : public vmem_container<T, vmem_noheader, Pool, Log> {
		using base = vmem_container<T, vmem_noheader, Pool, Log>;

		static constexpr vmem_page_balance_t balance_insert	= ~vmem_page_balance::end;
		static constexpr vmem_page_balance_t balance_erase	= vmem_page_balance::all;	// A stack would still be kept dense.

	public:
		vmem_list<T, Pool, Log>(vmem_list_state* state, Pool* pool, Log* log);
		vmem_list<T, Pool, Log>(const vmem_list<T, Pool, Log>& other) noexcept = default;
		vmem_list<T, Pool, Log>(vmem_list<T, Pool, Log>&& other) noexcept = default;
	};


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log = null_log>
	using vmem_stack_iterator = vmem_container_iterator<T, vmem_noheader, Pool, Log>;

	template <typename T, typename Pool, typename Log = null_log>
	using vmem_stack_const_iterator = vmem_container_const_iterator<T, vmem_noheader, Pool, Log>;


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log = null_log>
	class vmem_stack : public vmem_container<T, vmem_noheader, Pool, Log> {
		using base = vmem_container<T, vmem_noheader, Pool, Log>;

		static constexpr vmem_page_balance_t balance_insert	= vmem_page_balance::none;
		static constexpr vmem_page_balance_t balance_erase	= vmem_page_balance::none;

	public:
		vmem_stack<T, Pool, Log>(vmem_stack_state* state, Pool* pool, Log* log);
		vmem_stack<T, Pool, Log>(const vmem_stack<T, Pool, Log>& other) noexcept = default;
		vmem_stack<T, Pool, Log>(vmem_stack<T, Pool, Log>&& other) noexcept = default;

	public:
		typename base::result2	insert2(typename base::const_iterator itr, typename base::const_reference item) = delete;
		typename base::iterator	insert(typename base::const_iterator itr, typename base::const_reference item) = delete;
		template <typename InputItr>
		typename base::iterator	insert(typename base::const_iterator itr, InputItr first, InputItr last) = delete;

		typename base::result2	erase2(typename base::const_iterator itr) = delete;
		typename base::iterator	erase(typename base::const_iterator itr) = delete;
		typename base::iterator	erase(typename base::const_iterator first, typename base::const_iterator last) = delete;
	};


	// --------------------------------------------------------------


	template <typename Container>
	class vmem_temp : public Container {
	public:
		template <typename... Args>
		vmem_temp<Container>(Args&&... args);
		vmem_temp<Container>(vmem_temp<Container>&& other) noexcept = default;

		~vmem_temp<Container>() noexcept;
	};


	// --------------------------------------------------------------

}
