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
#include <utility>

#include "log.i.h"
#include "vmem_pool.i.h"
#include "vmem_layout.i.h"
#include "vmem_container.i.h"


namespace abc {

	// --------------------------------------------------------------


	template <typename Key, typename Pool, typename Log = null_log>
	using vmem_map_key_level = vmem_container<vmem_map_key<Key>, vmem_map_key_header, Pool, Log>;


	template <typename Key, typename Pool, typename Log = null_log>
	using vmem_map_key_level_stack = vmem_stack<vmem_container_state, Pool, Log>;


	template <typename Key, typename T, typename Pool, typename Log = null_log>
	using vmem_map_value_level = vmem_container<vmem_map_value<Key, T>, vmem_map_value_header, Pool, Log>;


	// --------------------------------------------------------------


	template <typename Key, typename T, typename Pool, typename Log>
	class vmem_map;

	template <typename Key, typename T, typename Pool, typename Log = null_log>
	using vmem_map_iterator = vmem_iterator<vmem_map<Key, T, Pool, Log>, vmem_map_value<Key, T>, Pool, Log>;


	// --------------------------------------------------------------


	template <typename Key, typename T, typename Pool, typename Log>
	struct vmem_map_result2 {
		vmem_map_result2(nullptr_t) noexcept;

		vmem_map_iterator<Key, T, Pool, Log>	iterator;
		bool									ok;
	};


	template <typename Key, typename T, typename Pool, typename Log>
	struct vmem_map_find_result2 : public vmem_map_result2<Key, T, Pool, Log> {
		vmem_map_find_result2(nullptr_t) noexcept;
		vmem_map_find_result2(const vmem_map_find_result2& other) = delete;
		vmem_map_find_result2(vmem_map_find_result2&& other) noexcept = default;
		~vmem_map_find_result2() noexcept;

		vmem_stack_state						path_state;
	};


	// --------------------------------------------------------------


	template <typename Key, typename T, typename Pool, typename Log = null_log>
	class vmem_map {
	public:
		using key_type					= Key;
		using mapped_type				= T;
		using value_type				= vmem_map_value<Key, T>;
		using pointer					= vmem_ptr<vmem_map_value<Key, T>, Pool, Log>;
		using const_pointer				= const pointer;
		using reference					= vmem_map_value<Key, T>&;
		using const_reference			= const vmem_map_value<Key, T>&;
		using iterator					= vmem_map_iterator<Key, T, Pool, Log>;
		using const_iterator			= const iterator;
		using reverse_iterator			= iterator;
		using const_reverse_iterator	= const_iterator;
		using result2					= vmem_map_result2<Key, T, Pool, Log>;
		using find_result2				= vmem_map_find_result2<Key, T, Pool, Log>;
		using iterator_bool				= std::pair<vmem_map_iterator<Key, T, Pool, Log>, bool>;

	public:
		static constexpr std::size_t	key_items_pos() noexcept;
		static constexpr std::size_t	max_key_item_size() noexcept;
		static constexpr std::size_t	key_page_capacity() noexcept;

		static constexpr std::size_t	value_items_pos() noexcept;
		static constexpr std::size_t	max_value_item_size() noexcept;
		static constexpr std::size_t	value_page_capacity() noexcept;

		static constexpr bool			is_uninit(const vmem_map_state* state) noexcept;

	public:
		vmem_map<Key, T, Pool, Log>(vmem_map_state* state, Pool* pool, Log* log);
		vmem_map<Key, T, Pool, Log>(const vmem_map<Key, T, Pool, Log>& other) noexcept = default;
		vmem_map<Key, T, Pool, Log>(vmem_map<Key, T, Pool, Log>&& other) noexcept = default;

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

		result2					insert2(const_reference item);
		iterator_bool			insert(const_reference item);
		template <typename InputItr>
		void					insert(InputItr first, InputItr last);

#ifdef REMOVE ////
		iterator				erase(const_iterator itr);
		iterator				erase(const_iterator first, const_iterator last);

		void					clear() noexcept;
#endif

	// insert() helpers
	private:
		result2					insert2(find_result2&& find_result, const_reference item);

	// erase() helpers
	private:

#ifdef REMOVE ////
	private:
		friend iterator;

		iterator				next(const_iterator& itr) const noexcept;
		iterator				prev(const_iterator& itr) const noexcept;
		pointer					at(const_iterator& itr) const noexcept;
#endif

	public:
		find_result2			find2(const Key& key) noexcept;
		iterator				find(const Key& key) noexcept;
		const_iterator			find(const Key& key) const noexcept;

		bool					contains(const Key& key) const noexcept;

		pointer					operator [] (const Key& key) noexcept;
		const_pointer			operator [] (const Key& key) const noexcept;

		pointer					at(const_iterator& itr) noexcept;
		const pointer			at(const_iterator& itr) const noexcept;

	private:
		iterator				begin_itr() const noexcept;
		iterator				rbegin_itr() const noexcept;
		iterator				end_itr() const noexcept;
		iterator				rend_itr() const noexcept;
		iterator				itr_from_values(typename vmem_map_value_level<Key, T, Pool, Log>::iterator values_itr) const noexcept;

	private:
		vmem_map_state*			_state;
		Pool*					_pool;
		Log*					_log;
	};

	// --------------------------------------------------------------

}
