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

#include <cstdint>

#include "log.i.h"


namespace abc {

	template <typename Pool, typename Log>
	class vmem_linked;

	template <typename Pool, typename Log = null_log>
	using vmem_linked_iterator_state = vmem_basic_iterator_state<vmem_linked<Pool, Log>, Pool, Log>;

	template <typename Pool, typename Log = null_log>
	using vmem_linked_iterator = vmem_iterator<vmem_linked<Pool, Log>, vmem_page_pos_t, Pool, Log>;

	template <typename Pool, typename Log = null_log>
	using vmem_linked_const_iterator = vmem_const_iterator<vmem_linked<Pool, Log>, vmem_page_pos_t, Pool, Log>;


	// --------------------------------------------------------------


	/**
	 * @brief					Doubly linked list of pool pages.
	 * @details					The struct is stateless. It uses an external state.
	 * @tparam Pool				Pool. Must be a specialization of `vmem_pool`.
	 * @tparam Log				Logging facility.
	 */
	template <typename Pool, typename Log = null_log>
	class vmem_linked {
		using iterator_state			= vmem_linked_iterator_state<Pool, Log>;

	public:
		using pool_type					= Pool;
		using value_type				= vmem_page_pos_t;
		using pointer					= vmem_ptr<value_type, Pool, Log>;
		using const_pointer				= vmem_ptr<const value_type, Pool, Log>;
		using reference					= value_type&;
		using const_reference			= const value_type&;
		using iterator					= vmem_linked_iterator<Pool, Log>;
		using const_iterator			= vmem_linked_const_iterator<Pool, Log>;
		using reverse_iterator			= iterator;
		using const_reverse_iterator	= const_iterator;

	public:
		/**
		 * @brief				Returns `true` if the given state is uninitialized; `false` if it is initialized.
		 */
		static constexpr bool	is_uninit(const vmem_linked_state* state) noexcept;

	public:
		/**
		 * @brief				Constructor.
		 * @param state			Pointer to a `vmem_linked_state` instance.
		 * @param pool			Pointer to a `vmem_pool` instance.
		 * @param log			Pointer to a `log_ostream` instance.
		 */
		vmem_linked<Pool, Log>(vmem_linked_state* state, Pool* pool, Log* log);

		/**
		 * @brief				Move constructor.
		 */
		vmem_linked<Pool, Log>(vmem_linked<Pool, Log>&& other) noexcept = default;

		/**
		 * @brief				Copy constructor.
		 */
		vmem_linked<Pool, Log>(const vmem_linked<Pool, Log>& other) noexcept = default;

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

		/**
		 * @brief				Links a page after the back.
		 * @param page_pos		Position of the page to be linked.
		 */
		void push_back(const_reference page_pos);

		/**
		 * @brief				Unlinks the back page.
		 */
		void pop_back();

		/**
		 * @brief				Links a page before the front.
		 * @param page_pos		Position of the page to be linked.
		 */
		void push_front(const_reference page_pos);

		/**
		 * @brief				Unlinks the front page.
		 */
		void pop_front();

		/**
		 * @brief				Links a page at the given iterator.
		 * @param itr			Iterator.
		 * @param page_pos		Position of the page to be linked.
		 * @return				Iterator referencing the newly linked page.
		 */
		iterator insert(const_iterator itr, const_reference page_pos);

		/**
		 * @brief				Links a page at the given iterator.
		 * @param itr			Iterator.
		 * @return				Iterator referencing the page next to the newly linked one.
		 */
		iterator erase(const_iterator itr);

		/**
		 * @brief				Unlinks all the pages.
		 */
		void clear();

		/**
		 * @brief				Links the other linked list at the end of this one.
		 * @param other			Other linked list. Its state is reset after this.
		 */
		void splice(vmem_linked<Pool, Log>& other);

		/**
		 * @brief				Links the other linked list at the end of this one.
		 * @param other			Other linked list. Its state is reset after this.
		 */
		void splice(vmem_linked<Pool, Log>&& other);

	private:
		/**
		 * @brief				Links a page at the given iterator without modifying the state.
		 * @param itr			Iterator.
		 * @param page_pos		Position of the page to be linked.
		 * @param back_page_pos	Position of the back page.
		 * @return				`true` == success; `false` = error.
		 */
		bool insert_nostate(const_iterator itr, const_reference page_pos, vmem_page_pos_t back_page_pos) noexcept;

		/**
		 * @brief				Links a page at the given iterator.
		 * @param itr			Iterator.
		 * @param back_page_pos	Position of the back page.
		 * @return				`true` == success; `false` = error.
		 */
		bool erase_nostate(const_iterator itr, vmem_page_pos_t& back_page_pos) noexcept;

	private:
		friend iterator_state;
		friend const_iterator;
		friend iterator;

		/**
		 * @brief				Returns the iterator immediately following a given one.  
		 * @param itr			Iterator.
		 */
		iterator next(const iterator_state& itr) const noexcept;

		/**
		 * @brief				Returns the iterator immediately preceding a given one.  
		 * @param itr			Iterator.
		 */
		iterator prev(const iterator_state& itr) const noexcept;

		/**
		 * @brief				Dereferences an iterator.
		 * @param itr			Iterator.
		 * @return				`vmem_ptr`. 
		 */
		pointer at(const iterator_state& itr) const noexcept;

	private:
		/**
		 * @brief				Returns an iterator referencing the first page.
		 */
		iterator begin_itr() const noexcept;

		/**
		 * @brief				Returns an iterator referencing past the last page.
		 */
		iterator end_itr() const noexcept;

		/**
		 * @brief				Returns a reverse iterator referencing the last page.
		 */
		reverse_iterator rend_itr() const noexcept;

		/**
		 * @brief				Returns a reverse iterator referencing before the first page.
		 */
		reverse_iterator rbegin_itr() const noexcept;

	private:
		vmem_linked_state*		_state;
		Pool*					_pool;
		Log*					_log;
	};


	// --------------------------------------------------------------

}
