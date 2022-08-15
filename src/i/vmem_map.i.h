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

#include <cstddef>
#include <cstdint>
#include <utility>

#include "log.i.h"
#include "vmem_pool.i.h"
#include "vmem_layout.i.h"
#include "vmem_container.i.h"


namespace abc {

	// --------------------------------------------------------------


	/**
	 * @brief					Key-level container. Always balancing all operations.
	 * @details					A map/B-tree consists of a value-level container, and a stack of key-level containers.
	 * @tparam Key				Key type.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Key, typename Pool, typename Log = null_log>
	class vmem_map_key_level : public vmem_container<vmem_map_key<Key>, vmem_noheader, Pool, Log> {
		using base = vmem_container<vmem_map_key<Key>, vmem_noheader, Pool, Log>;

		static constexpr vmem_page_balance_t balance_insert	= vmem_page_balance::all;
		static constexpr vmem_page_balance_t balance_erase	= vmem_page_balance::all;

	public:
		/**
		 * @brief				Constructor.
		 * @param state			Pointer to a `vmem_container_state` instance.
		 * @param pool			Pointer to a `Pool` instance.
		 * @param log			Pointer to a `log_ostream` instance.
		 */
		vmem_map_key_level<Key, Pool, Log>(vmem_container_state* state, Pool* pool, Log* log);

		/**
		 * @brief				Move constructor.
		 */
		vmem_map_key_level<Key, Pool, Log>(vmem_map_key_level<Key, Pool, Log>&& other) noexcept = default;

		/**
		 * @brief				Copy constructor.
		 */
		vmem_map_key_level<Key, Pool, Log>(const vmem_map_key_level<Key, Pool, Log>& other) noexcept = default;
	};


	/**
	 * @brief					Stack of key-level containers. For balancing, see `vmem_stack`.
	 * @details					A map/B-tree consists of a value-level container, and a stack of key-level containers.
	 * @tparam Key				Key type.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Key, typename Pool, typename Log = null_log>
	class vmem_map_key_level_stack : public vmem_stack<vmem_container_state, Pool, Log> {
		using base = vmem_stack<vmem_container_state, Pool, Log>;

	public:
		/**
		 * @brief				Constructor.
		 * @param state			Pointer to a `vmem_stack_state` instance.
		 * @param pool			Pointer to a `Pool` instance.
		 * @param log			Pointer to a `log_ostream` instance.
		 */
		vmem_map_key_level_stack<Key, Pool, Log>(vmem_stack_state* state, Pool* pool, Log* log);

		/**
		 * @brief				Move constructor.
		 */
		vmem_map_key_level_stack<Key, Pool, Log>(vmem_map_key_level_stack<Key, Pool, Log>&& other) noexcept = default;

		/**
		 * @brief				Copy constructor.
		 */
		vmem_map_key_level_stack<Key, Pool, Log>(const vmem_map_key_level_stack<Key, Pool, Log>& other) noexcept = default;
	};


	/**
	 * @brief					Value-level container. Always balancing all operations.
	 * @details					A map/B-tree consists of a value-level container, and a stack of key-level containers.
	 * @tparam Key				Key type.
	 * @tparam T				Value type.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Key, typename T, typename Pool, typename Log = null_log>
	class vmem_map_value_level : public vmem_container<vmem_map_value<Key, T>, vmem_noheader, Pool, Log> {
		using base = vmem_container<vmem_map_value<Key, T>, vmem_noheader, Pool, Log>;

		static constexpr vmem_page_balance_t balance_insert	= vmem_page_balance::all;
		static constexpr vmem_page_balance_t balance_erase	= vmem_page_balance::all;

	public:
		/**
		 * @brief				Constructor.
		 * @param state			Pointer to a `vmem_container_state` instance.
		 * @param pool			Pointer to a `Pool` instance.
		 * @param log			Pointer to a `log_ostream` instance.
		 */
		vmem_map_value_level<Key, T, Pool, Log>(vmem_container_state* state, Pool* pool, Log* log);

		/**
		 * @brief				Move constructor.
		 */
		vmem_map_value_level<Key, T, Pool, Log>(vmem_map_value_level<Key, T, Pool, Log>&& other) noexcept = default;

		/**
		 * @brief				Copy constructor.
		 */
		vmem_map_value_level<Key, T, Pool, Log>(const vmem_map_value_level<Key, T, Pool, Log>& other) noexcept = default;
	};


	// --------------------------------------------------------------


	template <typename Key, typename T, typename Pool, typename Log>
	class vmem_map;


	/**
	 * @brief					Map iterator state.
	 * @tparam Key				Key type.
	 * @tparam T				Value type.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Key, typename T, typename Pool, typename Log = null_log>
	using vmem_map_iterator_state = _vmem_iterator_state<vmem_map<Key, T, Pool, Log>, Pool, Log>;


	/**
	 * @brief					Map iterator.
	 * @tparam Key				Key type.
	 * @tparam T				Value type.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Key, typename T, typename Pool, typename Log = null_log>
	using vmem_map_iterator = vmem_iterator<vmem_map<Key, T, Pool, Log>, vmem_map_value<Key, T>, Pool, Log>;


	/**
	 * @brief					Map const iterator.
	 * @tparam Key				Key type.
	 * @tparam T				Value type.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Key, typename T, typename Pool, typename Log = null_log>
	using vmem_map_const_iterator = vmem_const_iterator<vmem_map<Key, T, Pool, Log>, vmem_map_value<Key, T>, Pool, Log>;


	// --------------------------------------------------------------


	/**
	 * @brief					Result of insert and erase operations that allows this struct to be included in bigger ones.
	 * @tparam Key				Key type.
	 * @tparam T				Value type.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Key, typename T, typename Pool, typename Log>
	struct vmem_map_result2 {
		/**
		 * @brief				Default-like constructor.
		 */
		vmem_map_result2(std::nullptr_t) noexcept;

		/**
		 * @brief				Move constructor.
		 */
		vmem_map_result2(vmem_map_result2&& other) noexcept = default;

		/**
		 * @brief				Copy constructor.
		 */
		vmem_map_result2(const vmem_map_result2& other) = default;

		vmem_map_result2& operator =(vmem_map_result2&& other) noexcept = default;

		/**
		 * @brief				Operation-specific iterator.
		 */
		vmem_map_iterator<Key, T, Pool, Log> iterator;

		/**
		 * @brief				`true` = the operation was performed; `false` = the operation was not performed.
		 */
		bool ok;
	};


	/**
	 * @brief					Result of find operations that allows this struct to be included in bigger ones.
	 * @details					The result is a stack of page positions representing the path to the item from the root.
	 * @tparam Key				Key type.
	 * @tparam T				Value type.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Key, typename T, typename Pool, typename Log>
	struct vmem_map_find_result2 : public vmem_map_result2<Key, T, Pool, Log> {
		/**
		 * @brief				Constructor.
		 * @param pool			Pointer to a `vmem_pool` instance.
		 * @param log			Pointer to a `log_ostream` instance.
		 */
		vmem_map_find_result2(Pool* pool, Log* log) noexcept;

		/**
		 * @brief				Move constructor.
		 */
		vmem_map_find_result2(vmem_map_find_result2&& other) noexcept = default;

		/**
		 * @brief				Copy constructor.
		 */
		vmem_map_find_result2(const vmem_map_find_result2& other) = delete;

		vmem_map_find_result2& operator =(vmem_map_find_result2&& other) noexcept = default;


	private:
		/**
		 * @brief				State for the path stack.
		 */
		vmem_stack_state _path_state;

	public:
		/**
		 * @brief				Stack of page positions representing the path to the item from the root.
		 */
		vmem_temp<vmem_stack<vmem_page_pos_t, Pool, Log>> path;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					Map implemented as a B-tree.
	 * @tparam Key				Key type.
	 * @tparam T				Value type.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Key, typename T, typename Pool, typename Log = null_log>
	class vmem_map {
		using iterator_state			= vmem_map_iterator_state<Key, T, Pool, Log>;

	public:
		using key_type					= Key;
		using mapped_type				= T;
		using value_type				= vmem_map_value<Key, T>;
		using pointer					= vmem_ptr<vmem_map_value<Key, T>, Pool, Log>;
		using const_pointer				= vmem_ptr<const vmem_map_value<Key, T>, Pool, Log>;
		using reference					= vmem_map_value<Key, T>&;
		using const_reference			= const vmem_map_value<Key, T>&;
		using iterator					= vmem_map_iterator<Key, T, Pool, Log>;
		using const_iterator			= vmem_map_const_iterator<Key, T, Pool, Log>;
		using reverse_iterator			= iterator;
		using const_reverse_iterator	= const_iterator;
		using result2					= vmem_map_result2<Key, T, Pool, Log>;
		using find_result2				= vmem_map_find_result2<Key, T, Pool, Log>;
		using iterator_bool				= std::pair<vmem_map_iterator<Key, T, Pool, Log>, bool>;

	private:
		using path_reverse_iterator		= typename vmem_stack<vmem_page_pos_t, Pool, Log>::reverse_iterator;
		using key_level_stack			= vmem_map_key_level_stack<Key, Pool, Log>;
		using key_level_stack_iterator	= typename vmem_map_key_level_stack<Key, Pool, Log>::iterator;
		using key_level_iterator		= typename vmem_map_key_level<Key, Pool, Log>::iterator;
		using key_level_result2			= typename vmem_map_key_level<Key, Pool, Log>::result2;
		using value_level_container		= vmem_map_value_level<Key, T, Pool, Log>;
		using value_level_iterator		= typename vmem_map_value_level<Key, T, Pool, Log>::iterator;
		using value_level_result2		= typename vmem_map_value_level<Key, T, Pool, Log>::result2;
		using page_lead					= typename vmem_map_key_level<Key, Pool, Log>::page_lead;

	public:
		/**
		 * @brief				Returns the byte position on each key page where keys start.
		 */
		static constexpr std::size_t key_items_pos() noexcept;

		/**
		 * @brief				Returns the maximum possible size of a key.
		 */
		static constexpr std::size_t max_key_item_size() noexcept;

		/**
		 * @brief				Returns the maximum number of keys that could be stored on a page.
		 */
		static constexpr std::size_t key_page_capacity() noexcept;

		/**
		 * @brief				Returns the byte position on each value page where values start.
		 */
		static constexpr std::size_t value_items_pos() noexcept;

		/**
		 * @brief				Returns the maximum possible size of a value.
		 */
		static constexpr std::size_t max_value_item_size() noexcept;

		/**
		 * @brief				Returns the maximum number of values that could be stored on a page.
		 */
		static constexpr std::size_t value_page_capacity() noexcept;

	public:
		/**
		 * @brief				Constructor.
		 * @param state			Pointer to a `vmem_map_state` instance.
		 * @param pool			Pointer to a `Pool` instance.
		 * @param log			Pointer to a `log_ostream` instance.
		 */
		vmem_map<Key, T, Pool, Log>(vmem_map_state* state, Pool* pool, Log* log);

		/**
		 * @brief				Move constructor.
		 */
		vmem_map<Key, T, Pool, Log>(vmem_map<Key, T, Pool, Log>&& other) noexcept = default;

		/**
		 * @brief				Copy constructor.
		 */
		vmem_map<Key, T, Pool, Log>(const vmem_map<Key, T, Pool, Log>& other) noexcept = default;

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

		/**
		 * @brief				Inserts an item.
		 * @details				Tries to find the item first.
		 *						If it is found, the insert is not performed.
		 *						If it is not found, an unconditional insert is performed to the `found_result2` path.
		 * @param item			Item.
		 * @return				`result2`
		 */
		result2 insert2(const_reference item);

		/**
		 * @brief				Inserts an item.
		 * @details				Tries to find the item first.
		 *						If it is found, the insert is not performed.
		 *						If it is not found, an unconditional insert is performed to the `find_result2` path.
		 * @param item			Item.
		 * @return				`iterator_bool`
		 */
		iterator_bool insert(const_reference item);

		/**
		 * @brief				Inserts a sequence of items.
		 * @tparam InputItr		Source iterator type.
		 * @param first			Begin source iterator.
		 * @param last			End source iterator.
		 */
		template <typename InputItr>
		void insert(InputItr first, InputItr last);

	private:
		/**
		 * @brief				Unconditionally inserts an item at the `find_result2` path.
		 * @param find_result	Find result.
		 * @param item			Item.
		 * @return				`result2` 
		 */
		result2 insert2(find_result2&& find_result, const_reference item) noexcept;

	public:
		/**
		 * @brief				Erases an item.
		 * @details				Tries to find the item first.
		 *						If it is not found, the erase is not performed.
		 *						If it is found, an unconditional erase is performed to the `find_result2` path.
		 * @param key			Key of the item to be erased.
		 * @return				`1` = the item was erased; `0` = the item was not erased.
		 */
		std::size_t erase(const Key& key);

		/**
		 * @brief				Erases a sequence of items.
		 * @tparam InputItr		Source iterator type.
		 * @param first			Begin source iterator.
		 * @param last			End source iterator.
		 */
		template <typename InputItr>
		void erase(InputItr first, InputItr last);

	private:
		/**
		 * @brief				Unconditionally erases an item at the `find_result2` path.
		 * @param find_result	Find result.
		 * @return				`1` = the item was erased; `0` = the item was not erased.
		 */
		std::size_t erase2(find_result2&& find_result) noexcept;

	// update_key_levels() helpers
	private:
		/**
		 * @brief				Inserts/erases keys throughout key levels as necessary after an insert/erase to the value level has finished.
		 * @param is_insert		`true` = insert; `false` = erase.
		 * @param find_result	`find_result2`
		 * @param values_result	`value_level_result_2`
		 * @return 				`result2`
		 */
		result2 update_key_levels(bool is_insert, find_result2&& find_result, value_level_result2&& values_result) noexcept;

		/**
		 * @brief				Returns the position of a key on a key page.
		 * @param key_page_pos	Key page position.
		 * @param key			Key
		 */
		vmem_item_pos_t key_item_pos(vmem_page_pos_t key_page_pos, const Key& key) noexcept;

	public:
		/**
		 * @brief				Erases all items.
		 */
		void clear() noexcept;

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

	public:
		/**
		 * @brief				Finds an item by key.
		 * @details				Suitable for use in more complex operations like insert and delete.
		 * @param key			Key.
		 * @return				`find_result2` 
		 */
		find_result2 find2(const Key& key) noexcept;

		/**
		 * @brief				Finds an item by key.
		 * @details				Suitable for direct use.
		 * @param key			Key.
		 * @return				`iterator` 
		 */
		iterator find(const Key& key) noexcept;

		/**
		 * @brief				Finds an item by key.
		 * @details				Suitable for direct use.
		 * @param key			Key.
		 * @return				`const_iterator` 
		 */
		const_iterator find(const Key& key) const noexcept;

		/**
		 * @brief				Checks if an item with a key exists.
		 * @param key			Key.
		 * @return				`true` = exists; `false` = does not exist. 
		 */
		bool					contains(const Key& key) const noexcept;

		/**
		 * @brief				Finds an item by key, and dereferences it.
		 * @param key			Key.
		 * @return				`pointer` 
		 */
		pointer operator [] (const Key& key) noexcept;

		/**
		 * @brief				Finds an item by key, and dereferences it.
		 * @param key			Key.
		 * @return				`const_pointer` 
		 */
		const_pointer operator [] (const Key& key) const noexcept;

		/**
		 * @brief				Dereferences an iterator.
		 * @param itr			Iterator.
		 * @return				`pointer`
		 */
		pointer at(const iterator_state& itr) noexcept;

		/**
		 * @brief				Dereferences an iterator.
		 * @param itr			Iterator.
		 * @return				`const_pointer`
		 */
		const_pointer at(const iterator_state& itr) const noexcept;

	private:
		/**
		 * @brief				Returns an iterator referencing the first item.
		 */
		iterator begin_itr() const noexcept;

		/**
		 * @brief				Returns an iterator referencing past the last item.
		 */
		iterator end_itr() const noexcept;

		/**
		 * @brief				Returns a reverse iterator referencing the last item.
		 */
		reverse_iterator rend_itr() const noexcept;

		/**
		 * @brief				Returns a reverse iterator referencing before the first item.
		 */
		reverse_iterator rbegin_itr() const noexcept;

		/**
		 * @brief				Converts a value-level iterator to a map iterator.
		 */
		iterator itr_from_values(value_level_iterator values_itr) const noexcept;

	private:
		vmem_map_state*			_state;
		Pool*					_pool;
		Log*					_log;

	private:
		key_level_stack			_key_stack;
		value_level_container	_values;
	};

	// --------------------------------------------------------------

}
