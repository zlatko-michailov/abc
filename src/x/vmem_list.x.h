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

	template <typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_list<T, Pool, Log>::items_pos() noexcept {
		return sizeof(_vmem_list_page<std::uint8_t>) - sizeof(std::uint8_t);
	}


	template <typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_list<T, Pool, Log>::max_item_size() noexcept {
		return vmem_page_size - items_pos();
	}


	template <typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_list<T, Pool, Log>::page_capacity() noexcept {
		return max_item_size() / sizeof(T);
	}


	template <typename T, typename Pool, typename Log>
	inline constexpr bool vmem_list<T, Pool, Log>::is_uninit(const vmem_list_state* state) noexcept {
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
	inline vmem_list<T, Pool, Log>::vmem_list(vmem_list_state* state, Pool* pool, Log* log)
		: _state(state)
		, _pool(pool)
		, _log(log) {

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1034b, "vmem_list::vmem_list() state=%p, pool=%p", state, pool);
		}

		if (state == nullptr) {
			throw exception<std::logic_error, Log>("vmem_list::vmem_list(state)", 0x1034c);
		}

		if (pool == nullptr) {
			throw exception<std::logic_error, Log>("vmem_list::vmem_list(pool)", 0x1034d);
		}

		if (sizeof(T) > max_item_size()) {
			throw exception<std::logic_error, Log>("vmem_list::vmem_list(size) excess", 0x1034e);
		}

		if (Pool::max_mapped_pages() < vmem_min_mapped_pages) {
			throw exception<std::logic_error, Log>("vmem_list::vmem_list(pool<MaxMappedPages>)", 0x10401);
		}

		if (is_uninit(state)) {
			_state->front_page_pos = vmem_page_pos_nil;
			_state->back_page_pos = vmem_page_pos_nil;
			_state->item_size = sizeof(T);
		}

		if (sizeof(T) != _state->item_size) {
			throw exception<std::logic_error, Log>("vmem_list::vmem_list(size) mismatch", 0x1034f);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10350, "vmem_list::vmem_list() front_page_pos=0x%llx, back_page_pos=0x%llx", 
				(long long)_state->front_page_pos, (long long)_state->back_page_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::begin() noexcept {
		return cbegin();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::begin() const noexcept {
		return cbegin();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::cbegin() const noexcept {
		vmem_page_pos_t page_pos;
		vmem_item_pos_t item_pos;
		begin_pos(page_pos, item_pos);

		return vmem_list_iterator<T, Pool, Log>(this, page_pos, item_pos, vmem_iterator_edge::none, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::end() noexcept {
		return cend();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::end() const noexcept {
		return cend();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::cend() const noexcept {
		vmem_page_pos_t page_pos;
		vmem_item_pos_t item_pos;
		end_pos(page_pos, item_pos);

		return vmem_list_iterator<T, Pool, Log>(this, page_pos, item_pos, vmem_iterator_edge::end, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::rend() noexcept {
		return crend();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::rend() const noexcept {
		return crend();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::crend() const noexcept {
		vmem_page_pos_t page_pos;
		vmem_item_pos_t item_pos;
		rend_pos(page_pos, item_pos);

		return vmem_list_iterator<T, Pool, Log>(this, page_pos, item_pos, vmem_iterator_edge::none, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::rbegin() noexcept {
		return crbegin();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::rbegin() const noexcept {
		return crbegin();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::crbegin() const noexcept {
		vmem_page_pos_t page_pos;
		vmem_item_pos_t item_pos;
		rbegin_pos(page_pos, item_pos);

		return vmem_list_iterator<T, Pool, Log>(this, page_pos, item_pos, vmem_iterator_edge::rbegin, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::empty() const noexcept {
		return _state->front_page_pos == vmem_page_pos_nil
			|| _state->back_page_pos == vmem_page_pos_nil;
	}


	template <typename T, typename Pool, typename Log>
	inline std::size_t vmem_list<T, Pool, Log>::size() const noexcept {
		return _state->total_item_count;
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::pointer vmem_list<T, Pool, Log>::frontptr() noexcept {
		return begin().ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_pointer vmem_list<T, Pool, Log>::frontptr() const noexcept {
		return begin().ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::reference vmem_list<T, Pool, Log>::front() {
		return begin().deref();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_reference vmem_list<T, Pool, Log>::front() const {
		return begin().deref();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::pointer vmem_list<T, Pool, Log>::backptr() noexcept {
		return rend().ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_pointer vmem_list<T, Pool, Log>::backptr() const noexcept {
		return rend().ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::reference vmem_list<T, Pool, Log>::back() {
		return rend().deref();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_reference vmem_list<T, Pool, Log>::back() const {
		return rend().deref();
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::push_back(const_reference item) {
		insert(end(), item);
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::pop_back() {
		erase(rend());
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::push_front(const_reference item) {
		insert(begin(), item);
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::pop_front() {
		erase(begin());
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::insert(const_iterator itr, const_reference item) {
		if (itr._page_pos == vmem_page_pos_nil && (itr._item_pos != vmem_item_pos_nil || !empty())) {
			throw exception<std::logic_error, Log>("vmem_list::insert(itr.page_pos)", 0x10351);
		}

		if (itr._item_pos == vmem_item_pos_nil && (itr._page_pos != _state->back_page_pos && itr._edge != vmem_iterator_edge::end)) {
			throw exception<std::logic_error, Log>("vmem_list::insert(itr.item_pos)", 0x10352);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_list::insert() Start. page_pos=0x%llx, item_pos=0x%x",
				(long long)itr._page_pos, itr._item_pos);
		}

		vmem_page_pos_t page_pos = itr._page_pos;
		vmem_item_pos_t item_pos = itr._item_pos;
		vmem_page_pos_t new_page_pos = vmem_page_pos_nil;

		// Insert without changing the list state.
		bool itr_end = itr._item_pos == vmem_item_pos_nil && itr._edge == vmem_iterator_edge::end;
		bool ok = insert_nostate(/*inout*/ page_pos, /*inout*/ item_pos, item, false, itr_end, /*out*/ new_page_pos);

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
			_log->put_any(category::abc::vmem, severity::abc::important, 0x10364, "vmem_list::insert() Done. page_pos=0x%llx, item_pos=0x%x, edge=%u, total_item_count=%zu",
				(long long)page_pos, item_pos, edge, (std::size_t)_state->total_item_count);
		}

		return iterator(this, page_pos, item_pos, edge, _log);
	}


	template <typename T, typename Pool, typename Log>
	template <typename InputItr>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::insert(const_iterator itr, InputItr first, InputItr last) {
		iterator ret(itr);

		for (InputItr item = first; item != last; item++) {
			if (!insert(itr++, *item).can_deref()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::important, 0x10365, "vmem_list::insert() Breaking from the loop.");
				}
				break;
			}
		}

		return ret;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::insert_nostate(/*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, const_reference item, bool always_balance, bool itr_end, /*out*/ vmem_page_pos_t& new_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::insert_nostate() Start. page_pos=0x%llx, item_pos=0x%x",
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
			ok = insert_nonempty(/*inout*/ page_pos, /*inout*/ item_pos, item_copy, false, itr_end, /*out*/ new_page_pos);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::insert_nostate() Done. ok=%d, page_pos=0x%llx, item_pos=0x%x, new_page_pos=0x%llx",
				ok, (long long)page_pos, item_pos, (long long)new_page_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::insert_empty(const_reference item, /*out*/ vmem_page_pos_t& page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::insert_empty() Start");
		}

		bool ok = true;

		// Create a new page.
		vmem_page<Pool, Log> page(_pool, _log);

		if (page.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_list::insert_empty() Could not load page");
			}
		}

		vmem_item_pos_t item_pos = 0;
		_vmem_list_page<T>* list_page = nullptr;

		if (ok) {
			list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());

			insert_with_capacity_safe(page.pos(), list_page, /*inout*/ item_pos, item);

			// Set out params.
			page_pos = page.pos();
		}

		// Link the page.
		if (ok) {
			list_page->next_page_pos = vmem_page_pos_nil;
			list_page->prev_page_pos = vmem_page_pos_nil;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::insert_empty() Done. ok=%d, page_pos=0x%llx, item_pos=0x%x",
				ok, (long long)page_pos, item_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::insert_nonempty(/*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, const_reference item, bool always_balance, bool itr_end, /*out*/ vmem_page_pos_t& new_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::insert_nonempty() Start. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}

		bool ok = true;

		vmem_page<Pool, Log> page(_pool, page_pos, _log);

		if (page.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x10356, "vmem_list::insert_nonempty() Could not load page pos=0x%llx", (long long)page_pos);
			}
		}

		if (ok) {
			_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10357, "vmem_list::insert_nonempty() item_count=%u, page_capacity=%zu", list_page->item_count, (std::size_t)page_capacity());
			}

			if (list_page->item_count == page_capacity()) {
				// The page has no capacity.

				// Preset the out params.
				page_pos = page.pos();
				item_pos = item_pos;

				// Balance unless inserting at the end of the last page.
				bool balance = always_balance || !(list_page->next_page_pos == vmem_page_pos_nil && itr_end);

				// Insert the item into the page.
				ok = insert_with_overflow(/*inout*/ page_pos, list_page, /*inout*/ item_pos, item, balance, /*out*/ new_page_pos);
			}
			else {
				// The page has capacity.

				// Set the out params.
				page_pos = page.pos();
				item_pos = item_pos;

				// Insert the item into the page.
				insert_with_capacity_safe(page_pos, list_page, /*inout*/ item_pos, item);
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::insert_nonempty() Done. ok=%d, page_pos=0x%llx, item_pos=0x%x, new_page_pos=0x%llx",
				ok, (long long)page_pos, item_pos, (long long)new_page_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::insert_with_overflow(/*inout*/ vmem_page_pos_t& page_pos, _vmem_list_page<T>* list_page, /*inout*/ vmem_item_pos_t& item_pos, const_reference item, bool balance, /*out*/ vmem_page_pos_t& new_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::insert_with_overflow() Start. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}

		bool ok = true;

		vmem_page<Pool, Log> new_page(nullptr);
		_vmem_list_page<T>* new_list_page = nullptr;
		ok = insert_page_after(page_pos, list_page, /*out*/ new_page, /*out*/ new_list_page);

		if (ok) {
			new_page_pos = new_page.pos();

			// Split the items evenly among the 2 pages unless we are inserting at the end.
			// This exception fills up pages fully when items keep being added at the end.
			if (balance) {
				balance_split_safe(page_pos, list_page, new_page_pos, new_list_page);
			}

			if (item_pos != vmem_item_pos_nil && item_pos <= list_page->item_count) {
				// Inserting to the former page.

				// Set the out params.
				// Nothing to do.

				// Insert the item into the page.
				insert_with_capacity_safe(page_pos, list_page, /*inout*/ item_pos, item);
			}
			else {
				// Inserting to the latter page.

				// Set the out params.
				page_pos = new_page_pos;
				if (item_pos == vmem_item_pos_nil) {
					item_pos = new_list_page->item_count;
				}
				else {
					item_pos -= list_page->item_count;
				}

				// Insert the item into the page.
				insert_with_capacity_safe(new_page_pos, new_list_page, /*inout*/ item_pos, item);
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::insert_with_overflow() Done. ok=%d, page_pos=0x%llx, item_pos=0x%x, new_page_pos=0x%llx",
				ok, (long long)page_pos, item_pos, (long long)new_page_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::insert_with_capacity_safe(vmem_page_pos_t page_pos, _vmem_list_page<T>* list_page, /*inout*/ vmem_item_pos_t& item_pos, const_reference item) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::insert_with_capacity_safe() Start. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}

		// Set the out param.
		if (item_pos == vmem_item_pos_nil) {
			item_pos = list_page->item_count;
		}

		// Shift items from the insertion position to free up a slot.
		std::size_t move_item_count = list_page->item_count - item_pos;
		if (move_item_count > 0) {
			std::memmove(&list_page->items[item_pos + 1], &list_page->items[item_pos], move_item_count * sizeof(T));
		}

		// Insert the item.
		++list_page->item_count;
		std::memmove(&list_page->items[item_pos], &item, sizeof(T));

		if (_log != nullptr) {
			_log->put_binary(category::abc::vmem, severity::abc::debug, __TAG__, &list_page->items[item_pos], std::min(sizeof(T), (std::size_t)16));
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::insert_page_capacity_safe() Done. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::balance_split_safe(vmem_page_pos_t page_pos, _vmem_list_page<T>* list_page, vmem_page_pos_t new_page_pos, _vmem_list_page<T>* new_list_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::balance() Start. page_pos=0x%llx, new_page_pos=0x%llx",
				(long long)page_pos, (long long)new_page_pos);
		}

		constexpr std::size_t new_page_item_count = page_capacity() / 2;
		constexpr std::size_t page_item_count = page_capacity() - new_page_item_count;
		std::memmove(&new_list_page->items[0], &list_page->items[page_item_count], new_page_item_count * sizeof(T));
		new_list_page->item_count = new_page_item_count;
		list_page->item_count = page_item_count;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::balance() Done. page_pos=0x%llx, item_count=%u, new_page_pos=0x%llx, new_item_count=%u",
				(long long)page_pos, list_page->item_count, (long long)new_page_pos, new_list_page->item_count);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::insert_page_after(vmem_page_pos_t page_pos, _vmem_list_page<T>* list_page, /*out*/ vmem_page<Pool, Log>& new_page, /*out*/ _vmem_list_page<T>*& new_list_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::insert_page_after() Start. page_pos=0x%llx",
				(long long)page_pos);
		}

		bool ok = true;
		
		vmem_page<Pool, Log> new_page_local(_pool, _log);
		_vmem_list_page<T>* new_list_page_local = nullptr;

		if (new_page_local.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_list::insert_page_after() Could not create page");
			}
		}

		if (ok) {
			new_list_page_local = reinterpret_cast<_vmem_list_page<T>*>(new_page_local.ptr());

			// Insert page - after.
			new_list_page_local->next_page_pos = list_page->next_page_pos;
			new_list_page_local->prev_page_pos = page_pos;
			new_list_page_local->item_count = 0;

			if (list_page->next_page_pos != vmem_page_pos_nil) {
				vmem_page<Pool, Log> next_page(_pool, list_page->next_page_pos, _log);

				if (next_page.ptr() == nullptr) {
					ok = false;

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_list::insert_page_after() Could not load page pos=0x%llx", (long long)list_page->next_page_pos);
					}
				}

				if (ok) {
					_vmem_list_page<T>* next_list_page = reinterpret_cast<_vmem_list_page<T>*>(next_page.ptr());

					next_list_page->prev_page_pos = new_page_local.pos();
				}
			}

			if (!ok) {
				new_page_local.free();
				new_list_page_local = nullptr;
			}
		}

		if (ok) {
			list_page->next_page_pos = new_page_local.pos();

			new_page = std::move(new_page_local);
			new_list_page = new_list_page_local;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::insert_page_after() Done. ok=%d, page_pos=0x%llx, new_page_pos=0x%llx",
				ok, (long long)page_pos, (long long)new_page.pos());
		}

		return ok;
	}


	// ..............................................................


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::erase(const_iterator itr) {
		if (!itr.can_deref()) {
			throw exception<std::logic_error, Log>("vmem_list::erase(itr)", 0x10366);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_list::erase() Begin. page_pos=0x%llx, item_pos=0x%x, edge=%u, total_item_count=%zu",
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
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_list::erase() Done. page_pos=0x%llx, item_pos=0x%x, edge=%u, total_item_count=%zu",
				(long long)page_pos, item_pos, edge, (std::size_t)_state->total_item_count);
		}

		return iterator(this, page_pos, item_pos, edge, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::erase(const_iterator first, const_iterator last) {
		iterator item = first;

		while (item != last) {
			if (!item.can_deref()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::important, 0x1036f, "vmem_list::erase() Breaking from the loop.");
				}

				break;
			}

			item = erase(item);
		}

		return item;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::erase_nostate(/*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge, /*inout*/ vmem_page_pos_t& front_page_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::erase_nostate() Start. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}

		bool ok = true;

		vmem_page<Pool, Log> page(_pool, page_pos, _log);

		if (page.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x10367, "vmem_list::erase() Could not load page pos=0x%llx", (long long)page_pos);
			}
		}
		else {
			_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());

			if (list_page->item_count > 1) {
				// There are many items on the page.
				erase_from_many_safe(list_page, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ edge);

				// Balance if item count drops below half of capacity.
				if (2 * list_page->item_count <= page_capacity()) {
					balance_merge_safe(/*inout*/ page, /*inout*/ list_page, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ back_page_pos);
				}
			}
			else {
				// There is only one item on the page.
				ok = erase_from_one(/*inout*/ page, /*inout*/ list_page, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ edge, /*inout*/ front_page_pos, /*inout*/ back_page_pos);
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::erase_nostate() Done. ok=%d, page_pos=0x%llx, item_pos=0x%x, edge=%u, front_page_pos=0x%llx, back_page_pos=0x%llx",
				ok, (long long)page_pos, item_pos, edge, front_page_pos, back_page_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::erase_from_many_safe(_vmem_list_page<T>* list_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::erase_from_many_safe() Start. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}

		if (item_pos < list_page->item_count - 1) {
			// To delete an item before the last one, pull up the remaining elements.

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10369, "vmem_list::erase_from_many_safe() Middle.");
			}

			std::size_t move_item_count = list_page->item_count - item_pos - 1;
			std::memmove(&list_page->items[item_pos], &list_page->items[item_pos + 1], move_item_count * sizeof(T));
		}
		else {
			// To delete the last (back) item on a page, there is nothing to do.

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x1036a, "vmem_list::erase_from_many_safe() Last.");
			}

			// If we are deleting the last item on a page, the next item is item 0 on the next page or end().
			if (list_page->next_page_pos != vmem_page_pos_nil) {
				page_pos = list_page->next_page_pos;
				item_pos = 0;
			}
			else {
				end_pos(page_pos, item_pos);
				edge = vmem_iterator_edge::end;
			}
		}

		// The main part of deleting an item from a page is decrementing the count.
		list_page->item_count--;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::erase_from_many_safe() Done. page_pos=0x%llx, item_pos=0x%x",
				(long long)page_pos, item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::erase_from_one(/*inout*/ vmem_page<Pool, Log>& page, /*inout*/ _vmem_list_page<T>* list_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge, /*inout*/ vmem_page_pos_t& front_page_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::erase_from_one() Start. page_pos=0x%llx",
				(long long)page_pos);
		}

		// Save prev_page_pos and next_page_pos.
		vmem_page_pos_t prev_page_pos = list_page->prev_page_pos;
		vmem_page_pos_t next_page_pos = list_page->next_page_pos;

		// We can free the current page now.
		list_page = nullptr;
		page.free();

		// Connect the two adjaceent pages.
		// The next item is item 0 on the next page or end().
		bool ok = link_pages(prev_page_pos, next_page_pos, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ edge, /*inout*/ front_page_pos, /*inout*/ back_page_pos);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::erase_from_one() Done. ok=%d", ok);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::balance_merge_safe(/*inout*/ vmem_page<Pool, Log>& page, /*inout*/ _vmem_list_page<T>* list_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::balance_merge_safe() Start. page_pos=0x%llx",
				(long long)page.pos());
		}

		bool ok = false;

		// Try the next page.
		if (list_page->next_page_pos != vmem_page_pos_nil) {
			ok = balance_merge_next(page, list_page, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ back_page_pos);
		}

		// If the next page didn'twork out, try the previous page.
		if (!ok && list_page->prev_page_pos != vmem_page_pos_nil) {
			ok = balance_merge_prev(/*inout*/ page, /*inout*/ list_page, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ back_page_pos);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::balance_merge_safe() Done. ok=%d, page_pos=0x%llx",
				ok, (long long)page.pos());
		}
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::balance_merge_next(vmem_page<Pool, Log>& page, _vmem_list_page<T>* list_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::balance_merge_next_safe() Start. page_pos=0x%llx",
				(long long)page.pos());
		}

		bool ok = true;

		vmem_page<Pool, Log> next_page(_pool, list_page->next_page_pos, _log);
		_vmem_list_page<T>* next_list_page = nullptr;

		if (next_page.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10405, "vmem_list::balance_merge_next_safe() Could not load page pos=0x%llx",
					(long long)list_page->next_page_pos);
			}
		}

		if (ok) {
			next_list_page = reinterpret_cast<_vmem_list_page<T>*>(next_page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10406, "vmem_list::balance_merge_next_safe() page_item_count=%u, next_page_pos=0x%llx, next_page_item_count=%u",
					list_page->item_count, (long long)next_page.pos(), next_list_page->item_count);
			}

			if (list_page->item_count + next_list_page->item_count > page_capacity()) {
				ok = false;
			}
		}

		if (ok) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, 0x10407, "vmem_list::balance_merge_next_safe() Do.");
			}

			// Merge the items from the next page into this one.
			std::memmove(&list_page->items[list_page->item_count], &next_list_page->items[0], next_list_page->item_count * sizeof(T));

			// Fix the next item, if it was item[0] on the next page.
			if (page_pos == next_page.pos()) {
				page_pos = page.pos();
				item_pos = list_page->item_count;
			}

			// Update the item count on this page.
			list_page->item_count += next_list_page->item_count;

			// Free the next page.
			if (next_list_page->next_page_pos == vmem_page_pos_nil) {
				list_page->next_page_pos = vmem_page_pos_nil;
				back_page_pos = page.pos();
			}
			else {
				vmem_page<Pool, Log> next_next_page(_pool, next_list_page->next_page_pos, _log);

				if (next_next_page.ptr() == nullptr) {
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::warning, 0x10408, "vmem_list::balance_merge_next_safe() Next next. Could not load page pos=0x%llx",
							(long long)next_list_page->next_page_pos);
					}
				}
				else {
					_vmem_list_page<T>* next_next_list_page = reinterpret_cast<_vmem_list_page<T>*>(next_next_page.ptr());

					list_page->next_page_pos = next_list_page->next_page_pos;
					next_next_list_page->prev_page_pos = page.pos();
				}
			}

			next_list_page = nullptr;
			next_page.free();
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::balance_merge_next_safe() Done. ok=%d, page_pos=0x%llx",
				ok, (long long)page.pos());
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::balance_merge_prev(/*inout*/ vmem_page<Pool, Log>& page, /*inout*/ _vmem_list_page<T>* list_page, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::balance_merge_prev() Start. page_pos=0x%llx",
				(long long)page.pos());
		}

		bool ok = true;

		vmem_page<Pool, Log> prev_page(_pool, list_page->prev_page_pos, _log);
		_vmem_list_page<T>* prev_list_page = nullptr;

		if (prev_page.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10409, "vmem_list::balance_merge_prev() Could not load page pos=0x%llx",
					(long long)list_page->prev_page_pos);
			}
		}

		if (ok) {
			prev_list_page = reinterpret_cast<_vmem_list_page<T>*>(prev_page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x1040a, "vmem_list::balance_merge_prev() page_pos=0x%llx, page_item_count=%u, prev_page_pos=0x%llx, prev_page_item_count=%u",
					(long long)page.pos(), list_page->item_count, (long long)prev_page.pos(), prev_list_page->item_count);
			}

			if (list_page->item_count + prev_list_page->item_count > page_capacity()) {
				ok = false;
			}
		}

		if (ok) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, 0x1040b, "vmem_list::balance_merge_prev() Do.");
			}

			// Merge the items from this page into the previous one.
			std::memmove(&prev_list_page->items[prev_list_page->item_count], &list_page->items[0], list_page->item_count * sizeof(T));

			// Fix the next page and item.
			page_pos = prev_page.pos();
			if (item_pos != vmem_item_pos_nil) {
				item_pos += prev_list_page->item_count;
			}

			// Update the item count on the previous page.
			prev_list_page->item_count += list_page->item_count;

			// Free this page.
			if (list_page->next_page_pos == vmem_page_pos_nil) {
				prev_list_page->next_page_pos = vmem_page_pos_nil;
				back_page_pos = prev_page.pos();
			}
			else {
				vmem_page<Pool, Log> next_page(_pool, list_page->next_page_pos, _log);

				if (next_page.ptr() == nullptr) {
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::warning, 0x1040c, "vmem_list::balance_merge_prev() Previous next. Could not load page pos=0x%llx",
							(long long)list_page->next_page_pos);
					}
				}
				else {
					_vmem_list_page<T>* next_list_page = reinterpret_cast<_vmem_list_page<T>*>(next_page.ptr());

					prev_list_page->next_page_pos = list_page->next_page_pos;
					next_list_page->prev_page_pos = prev_page.pos();
				}
			}

			list_page = nullptr;
			page.free();
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::balance_merge_prev() Done. ok=%d, page_pos=0x%llx",
				ok, (long long)page.pos());
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::link_pages(vmem_page_pos_t prev_page_pos, vmem_page_pos_t next_page_pos, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge, /*inout*/ vmem_page_pos_t& front_page_pos, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::link_pages() Start. prev_page_pos=0x%llx, next_page_pos=0x%llx",
				(long long)prev_page_pos, (long long)next_page_pos);
		}

		bool ok = link_next_page(prev_page_pos, next_page_pos, /*inout*/ front_page_pos);

		if (ok) {
			ok = link_prev_page(prev_page_pos, next_page_pos, /*inout*/ page_pos, /*inout*/ item_pos, /*inout*/ edge, /*inout*/ back_page_pos);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::link_pages() Done. ok=%d, prev_page_pos=0x%llx, next_page_pos=0x%llx",
				ok, (long long)prev_page_pos, (long long)next_page_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::link_next_page(vmem_page_pos_t prev_page_pos, vmem_page_pos_t next_page_pos, /*inout*/ vmem_page_pos_t& front_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::link_next_page() Start. prev_page_pos=0x%llx, next_page_pos=0x%llx",
				(long long)prev_page_pos, (long long)next_page_pos);
		}

		bool ok = true;

		if (prev_page_pos != vmem_page_pos_nil) {
			vmem_page<Pool, Log> prev_page(_pool, prev_page_pos, _log);

			if (prev_page.ptr() == nullptr) {
				ok = false;

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, 0x1036c, "vmem_list::link_next_page() Could not load prev page pos=0x%llx", (long long)prev_page_pos);
				}
			}
			else {
				_vmem_list_page<T>* prev_list_page = reinterpret_cast<_vmem_list_page<T>*>(prev_page.ptr());

				prev_list_page->next_page_pos = next_page_pos;
			}
		}
		else {
			front_page_pos = next_page_pos;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::link_next_page() Done. ok=%d, prev_page_pos=0x%llx, next_page_pos=0x%llx",
				ok, (long long)prev_page_pos, (long long)next_page_pos);
		}

		return ok;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::link_prev_page(vmem_page_pos_t prev_page_pos, vmem_page_pos_t next_page_pos, /*inout*/ vmem_page_pos_t& page_pos, /*inout*/ vmem_item_pos_t& item_pos, /*inout*/ vmem_iterator_edge_t& edge, /*inout*/ vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::link_prev_page() Start. prev_page_pos=0x%llx, next_page_pos=0x%llx",
				(long long)prev_page_pos, (long long)next_page_pos);
		}

		bool ok = true;

		if (next_page_pos != vmem_page_pos_nil) {
			vmem_page<Pool, Log> next_page(_pool, next_page_pos, _log);

			if (next_page.ptr() == nullptr) {
				ok = false;

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, 0x1036d, "vmem_list::link_prev_page() Could not load next page pos=0x%llx", (long long)next_page_pos);
				}
			}
			else {
				_vmem_list_page<T>* next_list_page = reinterpret_cast<_vmem_list_page<T>*>(next_page.ptr());

				next_list_page->prev_page_pos = prev_page_pos;

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
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::link_prev_page() Done. ok=%d, prev_page_pos=0x%llx, next_page_pos=0x%llx",
				ok, (long long)prev_page_pos, (long long)next_page_pos);
		}

		return ok;
	}


	// ..............................................................


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::clear() noexcept {
		erase(begin(), end());
	}


	// ..............................................................


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::move_next(iterator& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10370, "vmem_list::move_next() Before _page_pos=0x%llx, _item_pos=0x%x, _edge=%u",
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
					_log->put_any(category::abc::vmem, severity::warning, 0x10371, "vmem_list::move_next() Could not load page pos=0x%llx", (long long)itr._page_pos);
				}

				end_pos(itr._page_pos, itr._item_pos);
				itr._edge = vmem_iterator_edge::end;
			}
			else {
				_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());

				if (itr._item_pos < list_page->item_count - 1) {
					itr._item_pos++;
				}
				else {
					if (list_page->next_page_pos == vmem_page_pos_nil) {
						end_pos(itr._page_pos, itr._item_pos);
						itr._edge = vmem_iterator_edge::end;
					}
					else {
						itr._page_pos = list_page->next_page_pos;
						itr._item_pos = 0;
					}
				}
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10372, "vmem_list::move_next() After _page_pos=0x%llx, _item_pos=0x%x, _edge=%u",
				(long long)itr._page_pos, itr._item_pos, itr._edge);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::move_prev(iterator& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10373, "vmem_list::move_prev() Before _page_pos=0x%llx, _item_pos=0x%x, _edge=%u",
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
					_log->put_any(category::abc::vmem, severity::warning, 0x10374, "vmem_list::move_prev() Could not load page pos=0x%llx", (long long)itr._page_pos);
				}

				rbegin_pos(itr._page_pos, itr._item_pos);
				itr._edge = vmem_iterator_edge::rbegin;
			}
			else {
				_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());

				if (itr._item_pos > 0) {
					itr._item_pos--;
				}
				else {
					if (list_page->prev_page_pos == vmem_page_pos_nil) {
						rbegin_pos(itr._page_pos, itr._item_pos);
						itr._edge = vmem_iterator_edge::rbegin;
					}
					else {
						vmem_page<Pool, Log> prev_page(_pool, list_page->prev_page_pos, _log);

						if (prev_page.ptr() == nullptr) {
							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::warning, 0x10375, "vmem_list::move_prev() Could not load page pos=0x%llx", (long long)list_page->prev_page_pos);
							}

							rbegin_pos(itr._page_pos, itr._item_pos);
							itr._edge = vmem_iterator_edge::rbegin;
						}
						else {
							_vmem_list_page<T>* prev_list_page = reinterpret_cast<_vmem_list_page<T>*>(prev_page.ptr());

							itr._page_pos = list_page->prev_page_pos;
							itr._item_pos = prev_list_page->item_count - 1;
						}
					}
				}
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10376, "vmem_list::move_prev() After _page_pos=0x%llx, _item_pos=0x%x, _edge=%u",
				(long long)itr._page_pos, itr._item_pos, itr._edge);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_ptr<T, Pool, Log> vmem_list<T, Pool, Log>::at(const_iterator& itr) const noexcept {
		vmem_item_pos_t item_pos =
			itr._item_pos == vmem_item_pos_nil ?
				vmem_item_pos_nil :
				items_pos() + (itr._item_pos * sizeof(T));

		return vmem_ptr<T, Pool, Log>(_pool, itr._page_pos, item_pos, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::begin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->front_page_pos;
		item_pos = _state->front_page_pos == vmem_page_pos_nil ? vmem_item_pos_nil : 0; 

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10377, "vmem_list::begin_pos() page_pos=0x%llx, item_pos=0x%x", (long long)page_pos, item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::rbegin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->front_page_pos;
		item_pos = vmem_item_pos_nil;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10378, "vmem_list::rbegin_pos() page_pos=0x%llx, item_pos=0x%x", (long long)page_pos, item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::end_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->back_page_pos;
		item_pos = vmem_item_pos_nil;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10379, "vmem_list::end_pos() page_pos=0x%llx, item_pos=0x%x", (long long)page_pos, item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::rend_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->back_page_pos;

		if (_state->back_page_pos == vmem_page_pos_nil) {
			item_pos = vmem_item_pos_nil;
		}
		else {
			vmem_page<Pool, Log> page(_pool, _state->back_page_pos, _log);

			if (page.ptr() == nullptr) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, 0x1037a, "vmem_list::rend_pos() Could not load page pos=0x%llx", (long long)_state->back_page_pos);
				}

				item_pos = vmem_item_pos_nil;
			}
			else {
				_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());
				item_pos = list_page->item_count - 1;
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1037b, "vmem_list::rend_pos() page_pos=0x%llx, item_pos=0x%x", (long long)page_pos, item_pos);
		}
	}

}
