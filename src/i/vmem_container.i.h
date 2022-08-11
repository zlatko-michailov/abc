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


namespace abc {

	// --------------------------------------------------------------


	/**
	 * @brief					Flags that control whether to balance upon insert() or erase().
	 * @details					Each operation may have its own set of balance flags.
	 * 							Balancing is beneficial when items are inserted/deleted randomly - it guarantees that each page is at least 50% full.
	 * 							Alternatively, if items are inserted/deleted following some discipline, e.g. stack or queue, not balancing will keep pages dense.
	 */
	using vmem_page_balance_t = std::uint8_t;

	namespace vmem_page_balance {
		/**
		 * @brief				Nothing is to be balanced.
		 */
		constexpr vmem_page_balance_t none		= 0x00;

		/**
		 * @brief				Balance after an operation at the beginning of the container.
		 */
		constexpr vmem_page_balance_t begin		= 0x01;

		/**
		 * @brief				Balance after an operation in the inner section of the container.
		 */
		constexpr vmem_page_balance_t inner		= 0x02;

		/**
		 * @brief				Balance after an operation at the end of the container.
		 */
		constexpr vmem_page_balance_t end		= 0x04;

		/**
		 * @brief				Balance after an operation anywhere on the container.
		 */
		constexpr vmem_page_balance_t all		= 0xff;


		/**
		 * @brief				Returns `true` if all of the given `bits` are set on the given `value`.
		 */
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


	/**
	 * @brief					Operation that has been performed on a page lead (the leading item of a page.)
	 */
	using vmem_container_page_lead_operation_t	= std::uint8_t;

	namespace vmem_container_page_lead_operation {
		/**
		 * @brief				No lead change.
		 */
		constexpr vmem_container_page_lead_operation_t	none			= 0x0;

		/**
		 * @brief				A lead has been erased.
		 * @details				The lead has been the only item on the page that has been erased.
		 */
		constexpr vmem_container_page_lead_operation_t	erase			= 0x1;

		/**
		 * @brief				A new lead has been inserted.
		 * @details				A new item has been inserted to a full page, which has caused a new page to be inserted.
		 * 						This is the lead of the new page.
		 */
		constexpr vmem_container_page_lead_operation_t	insert			= 0x2;

		/**
		 * @brief				A lead has been replaced.
		 * @details				The lead has been erased, but there have been other items on the page.
		 *						The former second item has become the new lead.
		 */
		constexpr vmem_container_page_lead_operation_t	replace			= 0x3;

		/**
		 * @brief				A new lead has been inserted.
		 * @details				A new item has been inserted to a full page, which has caused a new page to be inserted.
		 * 						This is the lead of the original page.
		 */
		constexpr vmem_container_page_lead_operation_t	original		= 0x4;
	}


	/**
	 * @brief					Information about the leading item on a page.
	 * @details					This struct is a union of the properties needed for all kinds of containers.
	 * @tparam T				Item type.
	 */
	template <typename T>
	struct vmem_container_page_lead {
		/**
		 * @brief				Default constructor.
		 */
		vmem_container_page_lead() noexcept;

		/**
		 * @brief				"Copy" constructor.
		 * @details				Used for `vmem_map` to copy the leading keys from another type -
		 * 						the items on the leaf-level pages are of a different type than the items on the inner-level pages. 
		 */
		template <typename Other>
		vmem_container_page_lead(const Other& other) noexcept;

		/**
		 * @brief				Constructor.
		 * @details				Used for `vmem_linked` where only page positions are used.
		 * @param operation		Operation performed on the page.
		 * @param page_pos		Position of the page.
		 */
		vmem_container_page_lead(vmem_container_page_lead_operation_t operation, vmem_page_pos_t page_pos) noexcept;

		/**
		 * @brief				Constructor.
		 * @details				Used for `vmem_map` where only the key of each item is used.
		 * @param operation		Operation performed on the page.
		 * @param page_pos		Position of the page.
		 * @param items_0_key	The key of item[0].
		 * @param items_1_key	The key of item[1].
		 */
		template <typename Key>
		vmem_container_page_lead(vmem_container_page_lead_operation_t operation, vmem_page_pos_t page_pos, const Key& items_0_key, const Key& items_1_key) noexcept;

		/**
		 * @brief				Operation performed on the page.
		 */
		vmem_container_page_lead_operation_t operation;

		/**
		 * @brief				Position of the page.
		 */
		vmem_page_pos_t page_pos;

		/**
		 * @brief				Leading 2 items on the page.
		 */
		T items[2];
	};


	/**
	 * @brief					Result of insert/erase operations.
	 * @details					`std` containers return just an iterator. `abc` containers comply with that.
	 * 							`abc` containers have `insert2()` and `erase2()` that return additional information about any page split/merge.
	 * @tparam T				Item type.
	 * @tparam Header			Page header.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename T, typename Header, typename Pool, typename Log>
	struct vmem_container_result2 {
		vmem_container_iterator<T, Header, Pool, Log>	iterator		= nullptr;
		vmem_container_page_lead<T>						page_leads[2]	= { };
	};


	// --------------------------------------------------------------


	/**
	 * @brief					Sequence of items laid out over a `vmem_linked` (doubly linked list of pages).
	 * @details					Items are densely stored at the beginning of each page. Any page may not be full.
	 * 							Supports balancing - maintains at least 50% occupancy on all pages.
	 * @tparam T				Item type.
	 * @tparam Header			Page header.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
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
		/**
		 * @brief				Returns the byte position on each page where items start.
		 */
		static constexpr std::size_t items_pos() noexcept;

		/**
		 * @brief				Returns the maximum possible size of an item.
		 */
		static constexpr std::size_t max_item_size() noexcept;

		/**
		 * @brief				Returns the maximum number of items that could be stored on a page.
		 */
		static constexpr std::size_t page_capacity() noexcept;

		/**
		 * @brief				Returns `true` if the given state is uninitialized; `false` if it is initialized.
		 */
		static constexpr bool is_uninit(const vmem_container_state* state) noexcept;

	public:
		/**
		 * @brief				Constructor.
		 * @param state			Pointer to a `vmem_container_state` instance.
		 * @param balance_insert Balancing policy on insert.
		 * @param balance_erase	Balancing policy on erase.
		 * @param pool			Pointer to a `vmem_pool` instance.
		 * @param log			Pointer to a `log_ostream` instance.
		 */
		vmem_container<T, Header, Pool, Log>(vmem_container_state* state, vmem_page_balance_t balance_insert, vmem_page_balance_t balance_erase, Pool* pool, Log* log);

		/**
		 * @brief				Move constructor.
		 */
		vmem_container<T, Header, Pool, Log>(vmem_container<T, Header, Pool, Log>&& other) noexcept = default;

		/**
		 * @brief				Copy constructor.
		 */
		vmem_container<T, Header, Pool, Log>(const vmem_container<T, Header, Pool, Log>& other) noexcept = default;

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

		/**
		 * @brief				Copies an item after the back. A new page may be linked.
		 * @param item			Item.
		 */
		void push_back(const_reference item);

		/**
		 * @brief				Removed the back item. A page may be unlinked.
		 */
		void pop_back();

		/**
		 * @brief				Copies an item before the front. A new page may be linked.
		 * @param item			Item.
		 */
		void push_front(const_reference item);

		/**
		 * @brief				Removes the front item. A page may be unlinked.
		 */
		void pop_front();

		/**
		 * @brief				Copies an item at an iterator.
		 * @param itr			Iterator.
		 * @param item			Item.
		 * @return				`result2` - suitable for incorporating this struct into a bigger one.
		 */
		result2	 insert2(const_iterator itr, const_reference item);

		/**
		 * @brief				Copies an item at an iterator.
		 * @param itr			Iterator.
		 * @param item			Item.
		 * @return				`iterator` to the inserted item - suitable for client use.
		 */
		iterator insert(const_iterator itr, const_reference item);

		/**
		 * @brief				Copies a sequence of items at an iterator.
		 * @tparam InputItr		Source iterator type.
		 * @param itr			Target iterator.
		 * @param first			Begin source iterator.
		 * @param last			End source iterator.
		 * @return				`iterator` to the first inserted item.
		 */
		template <typename InputItr>
		iterator insert(const_iterator itr, InputItr first, InputItr last);

		/**
		 * @brief				Removes the item at an iterator.
		 * @param itr			Iterator.
		 * @return				`result2` - suitable for incorporating this struct into a bigger one.
		 */
		result2 erase2(const_iterator itr);

		/**
		 * @brief				Removes the item at an iterator.
		 * @param itr			Iterator.
		 * @return				`iterator` to the item following the erased one - suitable for client use.
		 */
		iterator erase(const_iterator itr);

		/**
		 * @brief				Removes a sequence of items.
		 * @param first			Begin iterator.
		 * @param last			End iterator.
		 * @return				`iterator` to the item following the last erased one.
		 */
		iterator erase(const_iterator first, const_iterator last);

		/**
		 * @brief				Erases all items.
		 */
		void clear() noexcept;

	// insert() helpers
	private:
		/**
		 * @brief				Insert an item at an iterator without modifying the container's state.
		 * @param itr			Iterator.
		 * @param item			Item.
		 */
		result2 insert_nostate(const_iterator itr, const_reference item) noexcept;

		/**
		 * @brief				Insert an item to an empty container.
		 * @param item			Item.
		 */
		result2 insert_empty(const_reference item) noexcept;

		/**
		 * @brief				Insert an item to a non-empty container.
		 * @param itr			Iterator.
		 * @param item			Item.
		 */
		result2 insert_nonempty(const_iterator itr, const_reference item) noexcept;

		/**
		 * @brief				Insert an item to a page that is full.
		 * @param itr			Iterator.
		 * @param item			Item.
		 * @param container_page Pointer to a `vmem_container_page`.
		 */
		result2 insert_with_overflow(const_iterator itr, const_reference item, vmem_container_page<T, Header>* container_page) noexcept;

		/**
		 * @brief				Insert an item to a page that has capacity.
		 * @param itr			Iterator.
		 * @param item			Item.
		 * @param container_page Pointer to a `vmem_container_page`.
		 */
		result2 insert_with_capacity(const_iterator itr, const_reference item, vmem_container_page<T, Header>* container_page) noexcept;

		/**
		 * @brief				Splits items among two pages.
		 * @param page_pos		Source page position.
		 * @param container_page Source page - pointer to `vmem_container_page`.
		 * @param new_page_pos	New page position.
		 * @param new_container_page New page - pointer to `vmem_container_page`.
		 */
		void balance_split(vmem_page_pos_t page_pos, vmem_container_page<T, Header>* container_page, vmem_page_pos_t new_page_pos, vmem_container_page<T, Header>* new_container_page) noexcept;

		/**
		 * @brief				Inserts a new page after another page.
		 * @param after_page_pos Position of the page to insert the new one after.
		 * @param new_page		The newly inserted page as a reference to `vmem_page`. The caller must keep this page locked until the insert is complete.
		 * @param new_container_page The newly inserted page as a pointer to `vmem_container_page`.
		 * @return				`true` = success; `false` = error.
		 */
		bool insert_page_after(vmem_page_pos_t after_page_pos, vmem_page<Pool, Log>& new_page, vmem_container_page<T, Header>*& new_container_page) noexcept;

		/**
		 * @brief				Evaluates the balancing policy on insert.
		 * @param itr			Iterator to insert at.
		 * @param container_page Pointer to a `vmem_container_page` to insert to.
		 * @return				`true` if an insert is subject to balancing items; `false` otherwise.
		 */
		bool should_balance_insert(const_iterator itr, const vmem_container_page<T, Header>* container_page) const noexcept;

	// erase() helpers
	private:
		/**
		 * @brief				Erases an item at an iterator without modifying the container's state.
		 * @param itr			Iterator.
		 * @return				`result2`. 
		 */
		result2 erase_nostate(const_iterator itr) noexcept;

		/**
		 * @brief				Erases an item from a page that has more than one item.
		 * @param itr			Iterator.
		 * @param container_page Pointer to a `vmem_container_page`.
		 * @return				`result2`. 
		 */
		result2 erase_from_many(const_iterator itr, vmem_container_page<T, Header>* container_page) noexcept;

		/**
		 * @brief				Merges the items of the given page with one of its adjacent pages.
		 * @param itr			Iterator.
		 * @param page			Page to be merged as a reference to `vmem_page`.
		 * @param container_page Page to be merged as a pointer to a `vmem_container_page`
		 * @return				`result2`. 
		 */
		result2 balance_merge(const_iterator itr, vmem_page<Pool, Log>& page, vmem_container_page<T, Header>* container_page) noexcept;

		/**
		 * @brief				Merges the items of the given page with the one following it.
		 * @param itr			Iterator.
		 * @param page			Page to be merged as a reference to `vmem_page`.
		 * @param container_page Page to be merged as a pointer to a `vmem_container_page`
		 * @return				`result2`. 
		 */
		result2 balance_merge_next(const_iterator itr, vmem_page<Pool, Log>& page, vmem_container_page<T, Header>* container_page) noexcept;

		/**
		 * @brief				Merges the items of the given page with the one preceding it.
		 * @param itr			Iterator.
		 * @param page			Page to be merged as a reference to `vmem_page`.
		 * @param container_page Page to be merged as a pointer to a `vmem_container_page`
		 * @return				`result2`. 
		 */
		result2 balance_merge_prev(const_iterator itr, vmem_page<Pool, Log>& page, vmem_container_page<T, Header>* container_page) noexcept;

		/**
		 * @brief				Unlinks a page from the container, and puts it on the pool's free page list.
		 * @param page			Page to be erased as a reference to `vmem_page`.
		 * @return				`true` = success; `false` = error.
		 */
		bool erase_page(vmem_page<Pool, Log>& page) noexcept;

		/**
		 * @brief				Unlinks a page from the container.
		 * @param page_pos		Page position.
		 * @return				`true` = success; `false` = error.
		 */
		bool erase_page_pos(vmem_page_pos_t page_pos) noexcept;

		/**
		 * @brief				Evaluates the balancing policy on erase.
		 * @param container_page Pointer to a `vmem_container_page` to erase from.
		 * @param item_pos 
		 * @return				`true` if an erase is subject to balancing items; `false` otherwise.
		 */
		bool should_balance_erase(const vmem_container_page<T, Header>* container_page, vmem_item_pos_t item_pos) const noexcept;

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

	private:
		vmem_container_state*	_state;
		vmem_page_balance_t		_balance_insert;
		vmem_page_balance_t		_balance_erase;
		Pool*					_pool;
		Log*					_log;
	};


	// --------------------------------------------------------------

}
