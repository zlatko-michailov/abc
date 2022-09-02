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


namespace abc {

	/**
	 * @brief					Iterator edge - special positions.
	 */
	using vmem_iterator_edge_t = std::uint8_t;

	namespace vmem_iterator_edge {
		/**
		 * @brief				Not an edge.
		 */
		constexpr vmem_iterator_edge_t	none	= 0;

		/**
		 * @brief				Before front.
		 */
		constexpr vmem_iterator_edge_t	rbegin	= 1;

		/**
		 * @brief				After back.
		 */
		constexpr vmem_iterator_edge_t	end		= 2;
	}


	// --------------------------------------------------------------


	/**
	 * @brief					Generic iterator state. For internal use.
	 * @details					This class does the heavy lifting for iterators.
	 * @tparam Container		Container.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Container, typename Pool, typename Log = null_log>
	class _vmem_iterator_state {
	public:
		using container					= Container;

	public:
		/**
		 * @brief				Constructor.
		 * @param container		Container.
		 * @param page_pos		Page position.
		 * @param item_pos		Item position.
		 * @param edge			Edge.
		 * @param log			Pointer to a `log_ostream` instance.
		 */
		_vmem_iterator_state<Container, Pool, Log>(const Container* container, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos, vmem_iterator_edge_t edge, Log* log) noexcept;

		/**
		 * @brief				Move constructor.
		 */
		_vmem_iterator_state<Container, Pool, Log>(_vmem_iterator_state<Container, Pool, Log>&& other) noexcept = default;

		/**
		 * @brief				Copy constructor.
		 */
		_vmem_iterator_state<Container, Pool, Log>(const _vmem_iterator_state<Container, Pool, Log>& other) = default;

		/**
		 * @brief				Default-like constructor.
		 */
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
		/**
		 * @brief				Checks whether this iterator state is associated with a container.
		 */
		bool is_valid() const noexcept;

		/**
		 * @brief				Checks whether this iterator state can be dereferenced.
		 */
		bool can_deref() const noexcept;

	private:
		/**
		 * @brief				Increments this iterator state, and returns a self reference.
		 */
		_vmem_iterator_state<Container, Pool, Log>& inc() noexcept;

		/**
		 * @brief				Decrements this iterator state, and returns a self reference.
		 */
		_vmem_iterator_state<Container, Pool, Log>& dec() noexcept;

	public:
		/**
		 * @brief				Returns the page position.
		 */
		vmem_page_pos_t page_pos() const noexcept;

		/**
		 * @brief				Returns the item position.
		 */
		vmem_item_pos_t item_pos() const noexcept;

		/**
		 * @brief				Returns the edge.
		 */
		vmem_iterator_edge_t edge() const noexcept;

	protected:
		const Container*								_container;
		vmem_page_pos_t									_page_pos;
		vmem_item_pos_t									_item_pos;
		vmem_iterator_edge_t							_edge;
		Log*											_log;
	};


	/**
	 * @brief					Generic iterator. For internal use.
	 * @details					This class is a stateless wrapper around `_vmem_iterator_state`.
	 * @tparam Base				Base class - a `_vmem_iterator_state` specialization.
	 * @tparam Container		Container.
	 * @tparam T				Item type.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Base, typename Container, typename T, typename Pool, typename Log = null_log>
	class vmem_basic_iterator : public Base {
	public:
		using value_type				= T;
		using pointer					= vmem_ptr<T, Pool, Log>;
		using const_pointer				= vmem_ptr<const T, Pool, Log>;
		using reference					= T&;
		using const_reference			= const T&;

	public:
		/**
		 * @brief				Constructor.
		 * @param container		Container
		 * @param page_pos		Page position.
		 * @param item_pos		Item position.
		 * @param edge			Edge.
		 * @param log			Pointer to a `log_ostream` instance.
		 */
		vmem_basic_iterator<Base, Container, T, Pool, Log>(const Container* container, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos, vmem_iterator_edge_t edge, Log* log) noexcept;

		/**
		 * @brief				Move constructor.
		 */
		vmem_basic_iterator<Base, Container, T, Pool, Log>(vmem_basic_iterator<Base, Container, T, Pool, Log>&& other) noexcept = default;

		/**
		 * @brief				Copy constructor.
		 */
		vmem_basic_iterator<Base, Container, T, Pool, Log>(const vmem_basic_iterator<Base, Container, T, Pool, Log>& other) = default;

		/**
		 * @brief				Default-like constructor.
		 */
		vmem_basic_iterator<Base, Container, T, Pool, Log>(std::nullptr_t) noexcept;

	public:
		vmem_basic_iterator<Base, Container, T, Pool, Log>&	operator =(const vmem_basic_iterator<Base, Container, T, Pool, Log>& other) noexcept = default;
		vmem_basic_iterator<Base, Container, T, Pool, Log>&	operator =(vmem_basic_iterator<Base, Container, T, Pool, Log>&& other) noexcept = default;

	public:
		pointer											operator ->() noexcept;
		const_pointer									operator ->() const noexcept;
		reference										operator *();
		const_reference									operator *() const;

	private:
		friend Container;

		/**
		 * @brief				Returns a `vmem_ptr` pointing at the item in memory, if the iterator is valid.
		 */
		pointer											ptr() const noexcept;

		/**
		 * @brief				Returns a reference to the item in memory, if the iterator is valid. Otherwise, it throws.
		 */
		reference										deref() const;
	};


	/**
	 * @brief					Generic const iterator.
	 * @tparam Container		Container.
	 * @tparam T				Item type.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Container, typename T, typename Pool, typename Log = null_log>
	using vmem_const_iterator = vmem_basic_iterator<_vmem_iterator_state<Container, Pool, Log>, Container, const T, Pool, Log>;


	/**
	 * @brief					Generic iterator.
	 * @tparam Container		Container.
	 * @tparam T				Item type.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Container, typename T, typename Pool, typename Log = null_log>
	using vmem_iterator = vmem_basic_iterator<vmem_const_iterator<Container, T, Pool, Log>, Container, T, Pool, Log>;


	// --------------------------------------------------------------

}
