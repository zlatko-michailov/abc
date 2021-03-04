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

#include <cstring>

#include "../exception.h"
#include "../i/vmem.i.h"


namespace abc {

	namespace vmem_page_balance {

		inline bool test(vmem_page_balance_t value, vmem_page_balance_t bits) noexcept {
			return (value & bits) == bits;
		}

	}


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_container<T, Pool, Log>::items_pos() noexcept {
		return sizeof(vmem_container_page<std::uint8_t>) - sizeof(std::uint8_t);
	}


	template <typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_container<T, Pool, Log>::max_item_size() noexcept {
		return vmem_page_size - items_pos();
	}


	template <typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_container<T, Pool, Log>::page_capacity() noexcept {
		return max_item_size() / sizeof(T);
	}


	template <typename T, typename Pool, typename Log>
	inline constexpr bool vmem_container<T, Pool, Log>::is_uninit(const vmem_container_state* state) noexcept {
		return
			// nil
			(
				state != nullptr
				&& state->front_page_pos == vmem_page_pos_nil
				&& state->back_page_pos == vmem_page_pos_nil
				&& state->item_size == 0
			)
			||
			// zero
			(
				state != nullptr
				&& state->front_page_pos == 0
				&& state->back_page_pos == 0
				&& state->item_size == 0
			);
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_container<T, Pool, Log>::vmem_container(vmem_container_state* state, vmem_page_balance_t balance_insert, vmem_page_balance_t balance_erase, Pool* pool, Log* log)
		: _state(state)
		, _balance_insert(balance_insert)
		, _balance_erase(balance_erase)
		, _pool(pool)
		, _log(log) {

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::vmem_container() state=%p, pool=%p", state, pool);
		}

		if (state == nullptr) {
			throw exception<std::logic_error, Log>("vmem_container::vmem_container(state)", __TAG__);
		}

		if (pool == nullptr) {
			throw exception<std::logic_error, Log>("vmem_container::vmem_container(pool)", __TAG__);
		}

		if (sizeof(T) > max_item_size()) {
			throw exception<std::logic_error, Log>("vmem_container::vmem_container(size) excess", __TAG__);
		}

		if (Pool::max_mapped_pages() < vmem_min_mapped_pages) {
			throw exception<std::logic_error, Log>("vmem_container::vmem_container(pool<MaxMappedPages>)", __TAG__);
		}

		if (is_uninit(state)) {
			_state->front_page_pos = vmem_page_pos_nil;
			_state->back_page_pos = vmem_page_pos_nil;
			_state->item_size = sizeof(T);
		}

		if (sizeof(T) != _state->item_size) {
			throw exception<std::logic_error, Log>("vmem_container::vmem_container(size) mismatch", __TAG__);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::vmem_container() front_page_pos=0x%llx, back_page_pos=0x%llx", 
				(long long)_state->front_page_pos, (long long)_state->back_page_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::iterator vmem_container<T, Pool, Log>::begin() noexcept {
		return cbegin();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::const_iterator vmem_container<T, Pool, Log>::begin() const noexcept {
		return cbegin();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::const_iterator vmem_container<T, Pool, Log>::cbegin() const noexcept {
		vmem_page_pos_t page_pos;
		vmem_item_pos_t item_pos;
		begin_pos(page_pos, item_pos);

		if (page_pos == vmem_page_pos_nil) {
			return cend();
		}

		return vmem_container_iterator<T, Pool, Log>(this, page_pos, item_pos, vmem_iterator_edge::none, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::iterator vmem_container<T, Pool, Log>::end() noexcept {
		return cend();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::const_iterator vmem_container<T, Pool, Log>::end() const noexcept {
		return cend();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::const_iterator vmem_container<T, Pool, Log>::cend() const noexcept {
		vmem_page_pos_t page_pos;
		vmem_item_pos_t item_pos;
		end_pos(page_pos, item_pos);

		return vmem_container_iterator<T, Pool, Log>(this, page_pos, item_pos, vmem_iterator_edge::end, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::iterator vmem_container<T, Pool, Log>::rend() noexcept {
		return crend();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::const_iterator vmem_container<T, Pool, Log>::rend() const noexcept {
		return crend();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::const_iterator vmem_container<T, Pool, Log>::crend() const noexcept {
		vmem_page_pos_t page_pos;
		vmem_item_pos_t item_pos;
		rend_pos(page_pos, item_pos);

		if (page_pos == vmem_page_pos_nil) {
			return crbegin();
		}

		return vmem_container_iterator<T, Pool, Log>(this, page_pos, item_pos, vmem_iterator_edge::none, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::iterator vmem_container<T, Pool, Log>::rbegin() noexcept {
		return crbegin();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::const_iterator vmem_container<T, Pool, Log>::rbegin() const noexcept {
		return crbegin();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::const_iterator vmem_container<T, Pool, Log>::crbegin() const noexcept {
		vmem_page_pos_t page_pos;
		vmem_item_pos_t item_pos;
		rbegin_pos(page_pos, item_pos);

		return vmem_container_iterator<T, Pool, Log>(this, page_pos, item_pos, vmem_iterator_edge::rbegin, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::empty() const noexcept {
		return _state->front_page_pos == vmem_page_pos_nil
			|| _state->back_page_pos == vmem_page_pos_nil;
	}


	template <typename T, typename Pool, typename Log>
	inline std::size_t vmem_container<T, Pool, Log>::size() const noexcept {
		return _state->total_item_count;
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::pointer vmem_container<T, Pool, Log>::frontptr() noexcept {
		return begin().ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::const_pointer vmem_container<T, Pool, Log>::frontptr() const noexcept {
		return begin().ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::reference vmem_container<T, Pool, Log>::front() {
		return begin().deref();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::const_reference vmem_container<T, Pool, Log>::front() const {
		return begin().deref();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::pointer vmem_container<T, Pool, Log>::backptr() noexcept {
		return rend().ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::const_pointer vmem_container<T, Pool, Log>::backptr() const noexcept {
		return rend().ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::reference vmem_container<T, Pool, Log>::back() {
		return rend().deref();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::const_reference vmem_container<T, Pool, Log>::back() const {
		return rend().deref();
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::push_back(const_reference item) {
		insert(end(), item);
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::pop_back() {
		erase(rend());
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::push_front(const_reference item) {
		insert(begin(), item);
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::pop_front() {
		erase(begin());
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::iterator vmem_container<T, Pool, Log>::insert(const_iterator itr, const_reference item) {
		if (itr._page_pos == vmem_page_pos_nil && (itr._item_pos != vmem_item_pos_nil || !empty())) {
			throw exception<std::logic_error, Log>("vmem_container::insert(itr.page_pos)", __TAG__);
		}

		if (itr._item_pos == vmem_item_pos_nil && (itr._page_pos != _state->back_page_pos && itr._edge != vmem_iterator_edge::end)) {
			throw exception<std::logic_error, Log>("vmem_container::insert(itr.item_pos)", __TAG__);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_container::insert() Start. page_pos=0x%llx, item_pos=0x%x",
				(long long)itr._page_pos, itr._item_pos);
		}

		vmem_page_pos_t page_pos = itr._page_pos;
		vmem_item_pos_t item_pos = itr._item_pos;
		vmem_page_pos_t new_page_pos = vmem_page_pos_nil;

		// Insert without changing the state.
		bool itr_end = itr._item_pos == vmem_item_pos_nil && itr._edge == vmem_iterator_edge::end;
		bool ok = insert_nostate(/*inout*/ page_pos, /*inout*/ item_pos, item, itr_end, /*out*/ new_page_pos);

		vmem_iterator_edge_t edge = vmem_iterator_edge::none;

		if (ok) {
			// We have inserted successfully.

			// Update the front page pos.
			if (_state->front_page_pos == vmem_page_pos_nil) {
				_state->front_page_pos = page_pos;
			}

			// Update the back page pos.
			if (_state->back_page_pos == vmem_page_pos_nil) {
				_state->back_page_pos = page_pos;
			}
			else if (_state->back_page_pos == itr._page_pos && new_page_pos != vmem_page_pos_nil) {
				_state->back_page_pos = new_page_pos;
			} 

			// Update the total item count.
			_state->total_item_count++;
		}
		else {
			// We have failed to insert.

			// Return end().
			end_pos(page_pos, item_pos);
			edge = vmem_iterator_edge::end;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_container::insert() Done. page_pos=0x%llx, item_pos=0x%x, edge=%u, total_item_count=%zu",
				(long long)page_pos, item_pos, edge, (std::size_t)_state->total_item_count);
		}

		return iterator(this, page_pos, item_pos, edge, _log);
	}


	template <typename T, typename Pool, typename Log>
	template <typename InputItr>
	inline typename vmem_container<T, Pool, Log>::iterator vmem_container<T, Pool, Log>::insert(const_iterator itr, InputItr first, InputItr last) {
		iterator ret(itr);

		for (InputItr item = first; item != last; item++) {
			if (!insert(itr++, *item).can_deref()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::important, __TAG__, "vmem_container::insert() Breaking from the loop.");
				}
				break;
			}
		}

		return ret;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::insert_nostate(/*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, const_reference item, bool itr_end, /*out*/ vmem_page_pos_t& new_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::insert_nostate() Start. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}

		bool ok = true;

		// Copy the item to a local variable to make sure the reference is valid and copyable before we change any page.
		T item_copy(item);

		if (page_pos == vmem_page_pos_nil) {
			item_pos = 0;

			ok = insert_empty(item_copy, /*out*/ page_pos);
		}
		else {
			ok = insert_nonempty(/*inout*/ page_pos, /*inout*/ item_pos, item_copy, itr_end, /*out*/ new_page_pos);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::insert_nostate() Done. ok=%d, page_pos=0x%llx, item_pos=0x%x, new_page_pos=0x%llx",
				ok, (long long)page_pos, item_pos, (long long)new_page_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::insert_empty(const_reference item, /*out*/ vmem_page_pos_t& page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::insert_empty() Start");
		}

		bool ok = true;

		// Create a new page.
		vmem_page<Pool, Log> page(_pool, _log);

		if (page.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_container::insert_empty() Could not load page");
			}
		}

		vmem_item_pos_t item_pos = 0;
		vmem_container_page<T>* container_page = nullptr;

		if (ok) {
			container_page = reinterpret_cast<vmem_container_page<T>*>(page.ptr());

			insert_with_capacity_safe(page.pos(), container_page, /*inout*/ item_pos, item);

			// Set out params.
			page_pos = page.pos();
		}

		// Link the page.
		if (ok) {
			container_page->next_page_pos = vmem_page_pos_nil;
			container_page->prev_page_pos = vmem_page_pos_nil;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::insert_empty() Done. ok=%d, page_pos=0x%llx, item_pos=0x%x",
				ok, (long long)page_pos, item_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::insert_nonempty(/*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, const_reference item, bool itr_end, /*out*/ vmem_page_pos_t& new_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::insert_nonempty() Start. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}

		bool ok = true;

		vmem_page<Pool, Log> page(_pool, page_pos, _log);

		if (page.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_container::insert_nonempty() Could not load page pos=0x%llx", (long long)page_pos);
			}
		}

		if (ok) {
			vmem_container_page<T>* container_page = reinterpret_cast<vmem_container_page<T>*>(page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::insert_nonempty() item_count=%u, page_capacity=%zu", container_page->item_count, (std::size_t)page_capacity());
			}

			if (container_page->item_count == page_capacity()) {
				// The page has no capacity.

				// Preset the out params.
				page_pos = page.pos();

				// Insert the item into the page.
				ok = insert_with_overflow(/*inout*/ page_pos, container_page, /*inout*/ item_pos, item, itr_end, /*out*/ new_page_pos);
			}
			else {
				// The page has capacity.

				// Set the out params.
				page_pos = page.pos();

				// Insert the item into the page.
				insert_with_capacity_safe(page_pos, container_page, /*inout*/ item_pos, item);
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::insert_nonempty() Done. ok=%d, page_pos=0x%llx, item_pos=0x%x, new_page_pos=0x%llx",
				ok, (long long)page_pos, item_pos, (long long)new_page_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::insert_with_overflow(/*inout*/ vmem_page_pos_t& page_pos, vmem_container_page<T>* container_page, /*inout*/ vmem_item_pos_t& item_pos, const_reference item, bool itr_end, /*out*/ vmem_page_pos_t& new_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::insert_with_overflow() Start. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}

		bool ok = true;

		// Decide whether we should balance before we alter any inout parameter.
		bool balance = should_balance_insert(container_page, item_pos, itr_end);

		vmem_page<Pool, Log> new_page(nullptr);
		vmem_container_page<T>* new_container_page = nullptr;
		ok = insert_page_after(page_pos, container_page, /*out*/ new_page, /*out*/ new_container_page);

		if (ok) {
			new_page_pos = new_page.pos();

			if (balance) {
				balance_split_safe(page_pos, container_page, new_page_pos, new_container_page);
			}

			if (item_pos != vmem_item_pos_nil && item_pos <= container_page->item_count) {
				// Inserting to the former page.

				// Set the out params.
				// Nothing to do.

				// Insert the item into the page.
				insert_with_capacity_safe(page_pos, container_page, /*inout*/ item_pos, item);
			}
			else {
				// Inserting to the latter page.

				// Set the out params.
				page_pos = new_page_pos;
				if (item_pos == vmem_item_pos_nil) {
					item_pos = new_container_page->item_count;
				}
				else {
					item_pos -= container_page->item_count;
				}

				// Insert the item into the page.
				insert_with_capacity_safe(new_page_pos, new_container_page, /*inout*/ item_pos, item);
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::insert_with_overflow() Done. ok=%d, page_pos=0x%llx, item_pos=0x%x, new_page_pos=0x%llx",
				ok, (long long)page_pos, item_pos, (long long)new_page_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::insert_with_capacity_safe(vmem_page_pos_t page_pos, vmem_container_page<T>* container_page, /*inout*/ vmem_item_pos_t& item_pos, const_reference item) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::insert_with_capacity_safe() Start. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}

		// Set the out param.
		if (item_pos == vmem_item_pos_nil) {
			item_pos = container_page->item_count;
		}

		// Shift items from the insertion position to free up a slot.
		std::size_t move_item_count = container_page->item_count - item_pos;
		if (move_item_count > 0) {
			std::memmove(&container_page->items[item_pos + 1], &container_page->items[item_pos], move_item_count * sizeof(T));
		}

		// Insert the item.
		++container_page->item_count;
		std::memmove(&container_page->items[item_pos], &item, sizeof(T));

		if (_log != nullptr) {
			_log->put_binary(category::abc::vmem, severity::abc::debug, __TAG__, &container_page->items[item_pos], std::min(sizeof(T), (std::size_t)16));
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::insert_page_capacity_safe() Done. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::balance_split_safe(vmem_page_pos_t page_pos, vmem_container_page<T>* container_page, vmem_page_pos_t new_page_pos, vmem_container_page<T>* new_container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::balance() Start. page_pos=0x%llx, new_page_pos=0x%llx",
				(long long)page_pos, (long long)new_page_pos);
		}

		constexpr std::size_t new_page_item_count = page_capacity() / 2;
		constexpr std::size_t page_item_count = page_capacity() - new_page_item_count;
		std::memmove(&new_container_page->items[0], &container_page->items[page_item_count], new_page_item_count * sizeof(T));
		new_container_page->item_count = new_page_item_count;
		container_page->item_count = page_item_count;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::balance() Done. page_pos=0x%llx, item_count=%u, new_page_pos=0x%llx, new_item_count=%u",
				(long long)page_pos, container_page->item_count, (long long)new_page_pos, new_container_page->item_count);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::insert_page_after(vmem_page_pos_t page_pos, vmem_container_page<T>* container_page, /*out*/ vmem_page<Pool, Log>& new_page, /*out*/ vmem_container_page<T>*& new_container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::insert_page_after() Start. page_pos=0x%llx",
				(long long)page_pos);
		}

		bool ok = true;
		
		vmem_page<Pool, Log> new_page_local(_pool, _log);
		vmem_container_page<T>* new_container_page_local = nullptr;

		if (new_page_local.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_container::insert_page_after() Could not create page");
			}
		}

		if (ok) {
			new_container_page_local = reinterpret_cast<vmem_container_page<T>*>(new_page_local.ptr());

			// Insert page - after.
			new_container_page_local->next_page_pos = container_page->next_page_pos;
			new_container_page_local->prev_page_pos = page_pos;
			new_container_page_local->item_count = 0;

			if (container_page->next_page_pos != vmem_page_pos_nil) {
				vmem_page<Pool, Log> next_page(_pool, container_page->next_page_pos, _log);

				if (next_page.ptr() == nullptr) {
					ok = false;

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_container::insert_page_after() Could not load page pos=0x%llx", (long long)container_page->next_page_pos);
					}
				}

				if (ok) {
					vmem_container_page<T>* next_container_page = reinterpret_cast<vmem_container_page<T>*>(next_page.ptr());

					next_container_page->prev_page_pos = new_page_local.pos();
				}
			}

			if (!ok) {
				new_page_local.free();
				new_container_page_local = nullptr;
			}
		}

		if (ok) {
			container_page->next_page_pos = new_page_local.pos();

			new_page = std::move(new_page_local);
			new_container_page = new_container_page_local;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::insert_page_after() Done. ok=%d, page_pos=0x%llx, new_page_pos=0x%llx",
				ok, (long long)page_pos, (long long)new_page.pos());
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::should_balance_insert(const vmem_container_page<T>* container_page, vmem_item_pos_t item_pos, bool itr_end) const noexcept {
		bool balance = false;

		if (container_page->prev_page_pos == vmem_page_pos_nil && item_pos == 0) {
			balance = vmem_page_balance::test(_balance_insert, vmem_page_balance::begin);
		}
		else if (container_page->next_page_pos == vmem_page_pos_nil && itr_end) {
			balance = vmem_page_balance::test(_balance_insert, vmem_page_balance::end);
		}
		else {
			balance = vmem_page_balance::test(_balance_insert, vmem_page_balance::inner);
		}

		return balance;
	}


	// ..............................................................


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::iterator vmem_container<T, Pool, Log>::erase(const_iterator itr) {
		if (!itr.can_deref()) {
			throw exception<std::logic_error, Log>("vmem_container::erase(itr)", __TAG__);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_container::erase() Begin. page_pos=0x%llx, item_pos=0x%x, edge=%u, total_item_count=%zu",
				(long long)itr._page_pos, itr._item_pos, itr._edge, (std::size_t)_state->total_item_count);
		}

		bool ok = true;
		vmem_page_pos_t page_pos = itr._page_pos;
		vmem_item_pos_t item_pos = itr._item_pos;
		vmem_iterator_edge_t edge = vmem_iterator_edge::none;
		vmem_page_pos_t front_page_pos = _state->front_page_pos;
		vmem_page_pos_t back_page_pos = _state->back_page_pos;

		ok = erase_nostate(/*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ edge, /*inout*/ front_page_pos, /*inout*/ back_page_pos);

		if (ok) {
			// Update the total item count.
			_state->total_item_count--;

			// Update the front and back pages.
			_state->front_page_pos = front_page_pos;
			_state->back_page_pos = back_page_pos;
		}
		else {
			end_pos(page_pos, item_pos);
			edge = vmem_iterator_edge::end;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_container::erase() Done. page_pos=0x%llx, item_pos=0x%x, edge=%u, total_item_count=%zu",
				(long long)page_pos, item_pos, edge, (std::size_t)_state->total_item_count);
		}

		return iterator(this, page_pos, item_pos, edge, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_container<T, Pool, Log>::iterator vmem_container<T, Pool, Log>::erase(const_iterator first, const_iterator last) {
		iterator item = first;

		while (item != last) {
			if (!item.can_deref()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::important, __TAG__, "vmem_container::erase() Breaking from the loop.");
				}

				break;
			}

			item = erase(item);
		}

		return item;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::erase_nostate(/*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge, /*inout*/ vmem_page_pos_t& front_page_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::erase_nostate() Start. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}

		bool ok = true;

		vmem_page<Pool, Log> page(_pool, page_pos, _log);

		if (page.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_container::erase() Could not load page pos=0x%llx", (long long)page_pos);
			}
		}
		else {
			vmem_container_page<T>* container_page = reinterpret_cast<vmem_container_page<T>*>(page.ptr());

			if (container_page->item_count > 1) {
				// Determine whether we should balance before any inout parameter gets altered.
				bool balance = should_balance_erase(container_page, item_pos);

				// There are many items on the page.
				erase_from_many_safe(container_page, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ edge);

				// Balance if item count drops below half of capacity.
				if (balance && 2 * container_page->item_count <= page_capacity()) {
					balance_merge_safe(/*inout*/ page, /*inout*/ container_page, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ back_page_pos);
				}
			}
			else {
				// There is only one item on the page.
				ok = erase_from_one(/*inout*/ page, /*inout*/ container_page, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ edge, /*inout*/ front_page_pos, /*inout*/ back_page_pos);
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::erase_nostate() Done. ok=%d, page_pos=0x%llx, item_pos=0x%x, edge=%u, front_page_pos=0x%llx, back_page_pos=0x%llx",
				ok, (long long)page_pos, item_pos, edge, front_page_pos, back_page_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::erase_from_many_safe(vmem_container_page<T>* container_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::erase_from_many_safe() Start. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}

		if (item_pos < container_page->item_count - 1) {
			// To delete an item before the last one, pull up the remaining elements.

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::erase_from_many_safe() Middle.");
			}

			std::size_t move_item_count = container_page->item_count - item_pos - 1;
			std::memmove(&container_page->items[item_pos], &container_page->items[item_pos + 1], move_item_count * sizeof(T));
		}
		else {
			// To delete the last (back) item on a page, there is nothing to do.

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::erase_from_many_safe() Last.");
			}

			// If we are deleting the last item on a page, the next item is item 0 on the next page or end().
			if (container_page->next_page_pos != vmem_page_pos_nil) {
				page_pos = container_page->next_page_pos;
				item_pos = 0;
			}
			else {
				end_pos(page_pos, item_pos);
				edge = vmem_iterator_edge::end;
			}
		}

		// The main part of deleting an item from a page is decrementing the count.
		container_page->item_count--;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::erase_from_many_safe() Done. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::erase_from_one(/*inout*/ vmem_page<Pool, Log>& page, /*inout*/ vmem_container_page<T>* container_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge, /*inout*/ vmem_page_pos_t& front_page_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::erase_from_one() Start. page_pos=0x%llx",
				(long long)page_pos);
		}

		// Save prev_page_pos and next_page_pos.
		vmem_page_pos_t prev_page_pos = container_page->prev_page_pos;
		vmem_page_pos_t next_page_pos = container_page->next_page_pos;

		// We can free the current page now.
		container_page = nullptr;
		page.free();

		// Connect the two adjaceent pages.
		// The next item is item 0 on the next page or end().
		bool ok = link_pages(prev_page_pos, next_page_pos, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ edge, /*inout*/ front_page_pos, /*inout*/ back_page_pos);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::erase_from_one() Done. ok=%d", ok);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::balance_merge_safe(/*inout*/ vmem_page<Pool, Log>& page, /*inout*/ vmem_container_page<T>* container_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::balance_merge_safe() Start. page_pos=0x%llx",
				(long long)page.pos());
		}

		bool ok = false;

		// Try the next page.
		if (container_page->next_page_pos != vmem_page_pos_nil) {
			ok = balance_merge_next(page, container_page, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ back_page_pos);
		}

		// If the next page didn'twork out, try the previous page.
		if (!ok && container_page->prev_page_pos != vmem_page_pos_nil) {
			ok = balance_merge_prev(/*inout*/ page, /*inout*/ container_page, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ back_page_pos);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::balance_merge_safe() Done. ok=%d, page_pos=0x%llx",
				ok, (long long)page.pos());
		}
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::balance_merge_next(vmem_page<Pool, Log>& page, vmem_container_page<T>* container_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::balance_merge_next_safe() Start. page_pos=0x%llx",
				(long long)page.pos());
		}

		bool ok = true;

		vmem_page<Pool, Log> next_page(_pool, container_page->next_page_pos, _log);
		vmem_container_page<T>* next_container_page = nullptr;

		if (next_page.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::balance_merge_next_safe() Could not load page pos=0x%llx",
					(long long)container_page->next_page_pos);
			}
		}

		if (ok) {
			next_container_page = reinterpret_cast<vmem_container_page<T>*>(next_page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::balance_merge_next_safe() page_item_count=%u, next_page_pos=0x%llx, next_page_item_count=%u",
					container_page->item_count, (long long)next_page.pos(), next_container_page->item_count);
			}

			if (container_page->item_count + next_container_page->item_count > page_capacity()) {
				ok = false;
			}
		}

		if (ok) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::balance_merge_next_safe() Do.");
			}

			// Merge the items from the next page into this one.
			std::memmove(&container_page->items[container_page->item_count], &next_container_page->items[0], next_container_page->item_count * sizeof(T));

			// Fix the next item, if it was item[0] on the next page.
			if (page_pos == next_page.pos()) {
				page_pos = page.pos();
				item_pos = container_page->item_count;
			}

			// Update the item count on this page.
			container_page->item_count += next_container_page->item_count;

			// Free the next page.
			if (next_container_page->next_page_pos == vmem_page_pos_nil) {
				container_page->next_page_pos = vmem_page_pos_nil;
				back_page_pos = page.pos();
			}
			else {
				vmem_page<Pool, Log> next_next_page(_pool, next_container_page->next_page_pos, _log);

				if (next_next_page.ptr() == nullptr) {
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_container::balance_merge_next_safe() Next next. Could not load page pos=0x%llx",
							(long long)next_container_page->next_page_pos);
					}
				}
				else {
					vmem_container_page<T>* next_next_container_page = reinterpret_cast<vmem_container_page<T>*>(next_next_page.ptr());

					container_page->next_page_pos = next_container_page->next_page_pos;
					next_next_container_page->prev_page_pos = page.pos();
				}
			}

			next_container_page = nullptr;
			next_page.free();
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::balance_merge_next_safe() Done. ok=%d, page_pos=0x%llx",
				ok, (long long)page.pos());
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::balance_merge_prev(/*inout*/ vmem_page<Pool, Log>& page, /*inout*/ vmem_container_page<T>* container_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::balance_merge_prev() Start. page_pos=0x%llx",
				(long long)page.pos());
		}

		bool ok = true;

		vmem_page<Pool, Log> prev_page(_pool, container_page->prev_page_pos, _log);
		vmem_container_page<T>* prev_container_page = nullptr;

		if (prev_page.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::balance_merge_prev() Could not load page pos=0x%llx",
					(long long)container_page->prev_page_pos);
			}
		}

		if (ok) {
			prev_container_page = reinterpret_cast<vmem_container_page<T>*>(prev_page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::balance_merge_prev() page_pos=0x%llx, page_item_count=%u, prev_page_pos=0x%llx, prev_page_item_count=%u",
					(long long)page.pos(), container_page->item_count, (long long)prev_page.pos(), prev_container_page->item_count);
			}

			if (container_page->item_count + prev_container_page->item_count > page_capacity()) {
				ok = false;
			}
		}

		if (ok) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::balance_merge_prev() Do.");
			}

			// Merge the items from this page into the previous one.
			std::memmove(&prev_container_page->items[prev_container_page->item_count], &container_page->items[0], container_page->item_count * sizeof(T));

			// Fix the next page and item.
			page_pos = prev_page.pos();
			if (item_pos != vmem_item_pos_nil) {
				item_pos += prev_container_page->item_count;
			}

			// Update the item count on the previous page.
			prev_container_page->item_count += container_page->item_count;

			// Free this page.
			if (container_page->next_page_pos == vmem_page_pos_nil) {
				prev_container_page->next_page_pos = vmem_page_pos_nil;
				back_page_pos = prev_page.pos();
			}
			else {
				vmem_page<Pool, Log> next_page(_pool, container_page->next_page_pos, _log);

				if (next_page.ptr() == nullptr) {
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_container::balance_merge_prev() Previous next. Could not load page pos=0x%llx",
							(long long)container_page->next_page_pos);
					}
				}
				else {
					vmem_container_page<T>* next_container_page = reinterpret_cast<vmem_container_page<T>*>(next_page.ptr());

					prev_container_page->next_page_pos = container_page->next_page_pos;
					next_container_page->prev_page_pos = prev_page.pos();
				}
			}

			container_page = nullptr;
			page.free();
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::balance_merge_prev() Done. ok=%d, page_pos=0x%llx",
				ok, (long long)page.pos());
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::link_pages(vmem_page_pos_t prev_page_pos, vmem_page_pos_t next_page_pos, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge, /*inout*/ vmem_page_pos_t& front_page_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::link_pages() Start. prev_page_pos=0x%llx, next_page_pos=0x%llx",
				(long long)prev_page_pos, (long long)next_page_pos);
		}

		bool ok = link_next_page(prev_page_pos, next_page_pos, /*inout*/ front_page_pos);

		if (ok) {
			ok = link_prev_page(prev_page_pos, next_page_pos, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ edge, /*inout*/ back_page_pos);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::link_pages() Done. ok=%d, prev_page_pos=0x%llx, next_page_pos=0x%llx",
				ok, (long long)prev_page_pos, (long long)next_page_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::link_next_page(vmem_page_pos_t prev_page_pos, vmem_page_pos_t next_page_pos, /*inout*/ vmem_page_pos_t& front_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::link_next_page() Start. prev_page_pos=0x%llx, next_page_pos=0x%llx",
				(long long)prev_page_pos, (long long)next_page_pos);
		}

		bool ok = true;

		if (prev_page_pos != vmem_page_pos_nil) {
			vmem_page<Pool, Log> prev_page(_pool, prev_page_pos, _log);

			if (prev_page.ptr() == nullptr) {
				ok = false;

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_container::link_next_page() Could not load prev page pos=0x%llx", (long long)prev_page_pos);
				}
			}
			else {
				vmem_container_page<T>* prev_container_page = reinterpret_cast<vmem_container_page<T>*>(prev_page.ptr());

				prev_container_page->next_page_pos = next_page_pos;
			}
		}
		else {
			front_page_pos = next_page_pos;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::link_next_page() Done. ok=%d, prev_page_pos=0x%llx, next_page_pos=0x%llx",
				ok, (long long)prev_page_pos, (long long)next_page_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::link_prev_page(vmem_page_pos_t prev_page_pos, vmem_page_pos_t next_page_pos, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::link_prev_page() Start. prev_page_pos=0x%llx, next_page_pos=0x%llx",
				(long long)prev_page_pos, (long long)next_page_pos);
		}

		bool ok = true;

		if (next_page_pos != vmem_page_pos_nil) {
			vmem_page<Pool, Log> next_page(_pool, next_page_pos, _log);

			if (next_page.ptr() == nullptr) {
				ok = false;

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_container::link_prev_page() Could not load next page pos=0x%llx", (long long)next_page_pos);
				}
			}
			else {
				vmem_container_page<T>* next_container_page = reinterpret_cast<vmem_container_page<T>*>(next_page.ptr());

				next_container_page->prev_page_pos = prev_page_pos;

				page_pos = next_page_pos;
				item_pos = 0;
			}
		}
		else {
			back_page_pos = prev_page_pos;

			end_pos(page_pos, item_pos);
			edge = vmem_iterator_edge::end;
		}
	
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_container::link_prev_page() Done. ok=%d, prev_page_pos=0x%llx, next_page_pos=0x%llx",
				ok, (long long)prev_page_pos, (long long)next_page_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_container<T, Pool, Log>::should_balance_erase(const vmem_container_page<T>* container_page, vmem_item_pos_t item_pos) const noexcept {
		bool balance = false;

		if (container_page->prev_page_pos == vmem_page_pos_nil && item_pos == 0) {
			balance = vmem_page_balance::test(_balance_erase, vmem_page_balance::begin);
		}
		else if (container_page->next_page_pos == vmem_page_pos_nil && item_pos == container_page->item_count - 1) {
			balance = vmem_page_balance::test(_balance_erase, vmem_page_balance::end);
		}
		else {
			balance = vmem_page_balance::test(_balance_erase, vmem_page_balance::inner);
		}

		return balance;
	}


	// ..............................................................


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::clear() noexcept {
		erase(begin(), end());
	}


	// ..............................................................


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::move_next(iterator& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::move_next() Before _page_pos=0x%llx, _item_pos=0x%x, _edge=%u",
				(long long)itr._page_pos, itr._item_pos, itr._edge);
		}

		if (itr._item_pos == vmem_item_pos_nil && itr._edge == vmem_iterator_edge::rbegin) {
			begin_pos(itr._page_pos, itr._item_pos);
			itr._edge = vmem_iterator_edge::none;
		}
		else if (itr._page_pos != vmem_page_pos_nil) {
			vmem_page<Pool, Log> page(_pool, itr._page_pos, _log);

			if (page.ptr() == nullptr) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_container::move_next() Could not load page pos=0x%llx", (long long)itr._page_pos);
				}

				end_pos(itr._page_pos, itr._item_pos);
				itr._edge = vmem_iterator_edge::end;
			}
			else {
				vmem_container_page<T>* container_page = reinterpret_cast<vmem_container_page<T>*>(page.ptr());

				if (itr._item_pos < container_page->item_count - 1) {
					itr._item_pos++;
				}
				else {
					if (container_page->next_page_pos == vmem_page_pos_nil) {
						end_pos(itr._page_pos, itr._item_pos);
						itr._edge = vmem_iterator_edge::end;
					}
					else {
						itr._page_pos = container_page->next_page_pos;
						itr._item_pos = 0;
					}
				}
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::move_next() After _page_pos=0x%llx, _item_pos=0x%x, _edge=%u",
				(long long)itr._page_pos, itr._item_pos, itr._edge);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::move_prev(iterator& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::move_prev() Before _page_pos=0x%llx, _item_pos=0x%x, _edge=%u",
				(long long)itr._page_pos, itr._item_pos, itr._edge);
		}

		if (itr._item_pos == vmem_item_pos_nil && itr._edge == vmem_iterator_edge::end) {
			rend_pos(itr._page_pos, itr._item_pos);
			itr._edge = vmem_iterator_edge::none;
		}
		else if (itr._page_pos != vmem_page_pos_nil) {
			vmem_page<Pool, Log> page(_pool, itr._page_pos, _log);

			if (page.ptr() == nullptr) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_container::move_prev() Could not load page pos=0x%llx", (long long)itr._page_pos);
				}

				rbegin_pos(itr._page_pos, itr._item_pos);
				itr._edge = vmem_iterator_edge::rbegin;
			}
			else {
				vmem_container_page<T>* container_page = reinterpret_cast<vmem_container_page<T>*>(page.ptr());

				if (itr._item_pos > 0) {
					itr._item_pos--;
				}
				else {
					if (container_page->prev_page_pos == vmem_page_pos_nil) {
						rbegin_pos(itr._page_pos, itr._item_pos);
						itr._edge = vmem_iterator_edge::rbegin;
					}
					else {
						vmem_page<Pool, Log> prev_page(_pool, container_page->prev_page_pos, _log);

						if (prev_page.ptr() == nullptr) {
							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_container::move_prev() Could not load page pos=0x%llx", (long long)container_page->prev_page_pos);
							}

							rbegin_pos(itr._page_pos, itr._item_pos);
							itr._edge = vmem_iterator_edge::rbegin;
						}
						else {
							vmem_container_page<T>* prev_container_page = reinterpret_cast<vmem_container_page<T>*>(prev_page.ptr());

							itr._page_pos = container_page->prev_page_pos;
							itr._item_pos = prev_container_page->item_count - 1;
						}
					}
				}
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::move_prev() After _page_pos=0x%llx, _item_pos=0x%x, _edge=%u",
				(long long)itr._page_pos, itr._item_pos, itr._edge);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_ptr<T, Pool, Log> vmem_container<T, Pool, Log>::at(const_iterator& itr) const noexcept {
		vmem_item_pos_t item_pos =
			itr._item_pos == vmem_item_pos_nil ?
				vmem_item_pos_nil :
				items_pos() + (itr._item_pos * sizeof(T));

		return vmem_ptr<T, Pool, Log>(_pool, itr._page_pos, item_pos, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::begin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->front_page_pos;
		item_pos = _state->front_page_pos == vmem_page_pos_nil ? vmem_item_pos_nil : 0; 

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::begin_pos() page_pos=0x%llx, item_pos=0x%x", (long long)page_pos, item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::rbegin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->front_page_pos;
		item_pos = vmem_item_pos_nil;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::rbegin_pos() page_pos=0x%llx, item_pos=0x%x", (long long)page_pos, item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::end_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->back_page_pos;
		item_pos = vmem_item_pos_nil;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::end_pos() page_pos=0x%llx, item_pos=0x%x", (long long)page_pos, item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_container<T, Pool, Log>::rend_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->back_page_pos;

		if (_state->back_page_pos == vmem_page_pos_nil) {
			item_pos = vmem_item_pos_nil;
		}
		else {
			vmem_page<Pool, Log> page(_pool, _state->back_page_pos, _log);

			if (page.ptr() == nullptr) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_container::rend_pos() Could not load page pos=0x%llx", (long long)_state->back_page_pos);
				}

				item_pos = vmem_item_pos_nil;
			}
			else {
				vmem_container_page<T>* container_page = reinterpret_cast<vmem_container_page<T>*>(page.ptr());
				item_pos = container_page->item_count - 1;
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_container::rend_pos() page_pos=0x%llx, item_pos=0x%x", (long long)page_pos, item_pos);
		}
	}

}
