/*
MIT License

Copyright (c) 2018-2023 Zlatko Michailov 

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


	template <typename T>
	vmem_container_page_lead<T>::vmem_container_page_lead() noexcept
		: vmem_container_page_lead(vmem_container_page_lead_operation::none, vmem_page_pos_nil) {
	}


	template <typename T>
	template <typename Other>
	vmem_container_page_lead<T>::vmem_container_page_lead(const Other& other) noexcept
		: vmem_container_page_lead(other.operation, other.page_pos, other.items[0].key, other.items[1].key) {
	}


	template <typename T>
	vmem_container_page_lead<T>::vmem_container_page_lead(vmem_container_page_lead_operation_t operation, vmem_page_pos_t page_pos) noexcept
		: operation(operation)
		, page_pos(page_pos)
		, items { } {
	}


	template <typename T>
	template <typename Key>
	vmem_container_page_lead<T>::vmem_container_page_lead(vmem_container_page_lead_operation_t operation, vmem_page_pos_t page_pos, const Key& items_0_key, const Key& items_1_key) noexcept
		: vmem_container_page_lead(operation, page_pos) {
		vmem_copy(items[0].key, items_0_key);
		vmem_copy(items[1].key, items_1_key);
	}


	// --------------------------------------------------------------


	template <typename T, typename Header, typename Pool, typename Log>
	inline constexpr std::size_t vmem_container<T, Header, Pool, Log>::items_pos() noexcept {
		return sizeof(vmem_container_page<T, Header>) - sizeof(T);
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline constexpr std::size_t vmem_container<T, Header, Pool, Log>::max_item_size() noexcept {
		return vmem_page_size - items_pos();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline constexpr std::size_t vmem_container<T, Header, Pool, Log>::page_capacity() noexcept {
		return max_item_size() / sizeof(T);
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline constexpr bool vmem_container<T, Header, Pool, Log>::is_uninit(const vmem_container_state* state) noexcept {
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


	template <typename T, typename Header, typename Pool, typename Log>
	inline vmem_container<T, Header, Pool, Log>::vmem_container(vmem_container_state* state, vmem_page_balance_t balance_insert, vmem_page_balance_t balance_erase, Pool* pool, Log* log)
		: _state(state)
		, _balance_insert(balance_insert)
		, _balance_erase(balance_erase)
		, _pool(pool)
		, _log(log) {

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10443, "vmem_container::vmem_container() state=%p, pool=%p", state, pool);
		}

		if (state == nullptr) {
			throw exception<std::logic_error, Log>("vmem_container::vmem_container(state)", 0x10444);
		}

		if (pool == nullptr) {
			throw exception<std::logic_error, Log>("vmem_container::vmem_container(pool)", 0x10445);
		}

		if (sizeof(T) > max_item_size()) {
			throw exception<std::logic_error, Log>("vmem_container::vmem_container(size) excess", 0x10446);
		}

		if (Pool::max_mapped_pages() < vmem_min_mapped_pages) {
			throw exception<std::logic_error, Log>("vmem_container::vmem_container(pool<MaxMappedPages>)", 0x10447);
		}

		if (is_uninit(state)) {
			_state->front_page_pos = vmem_page_pos_nil;
			_state->back_page_pos = vmem_page_pos_nil;
			_state->item_size = sizeof(T);
		}

		if (sizeof(T) != _state->item_size) {
			throw exception<std::logic_error, Log>("vmem_container::vmem_container(size) mismatch", 0x10448);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10449, "vmem_container::vmem_container() front_page_pos=0x%llx, back_page_pos=0x%llx", 
				(long long)_state->front_page_pos, (long long)_state->back_page_pos);
		}
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::iterator vmem_container<T, Header, Pool, Log>::begin() noexcept {
		return begin_itr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::const_iterator vmem_container<T, Header, Pool, Log>::begin() const noexcept {
		return begin_itr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::const_iterator vmem_container<T, Header, Pool, Log>::cbegin() const noexcept {
		return begin_itr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::iterator vmem_container<T, Header, Pool, Log>::end() noexcept {
		return end_itr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::const_iterator vmem_container<T, Header, Pool, Log>::end() const noexcept {
		return end_itr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::const_iterator vmem_container<T, Header, Pool, Log>::cend() const noexcept {
		return end_itr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::iterator vmem_container<T, Header, Pool, Log>::rend() noexcept {
		return rend_itr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::const_iterator vmem_container<T, Header, Pool, Log>::rend() const noexcept {
		return rend_itr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::const_iterator vmem_container<T, Header, Pool, Log>::crend() const noexcept {
		return rend_itr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::iterator vmem_container<T, Header, Pool, Log>::rbegin() noexcept {
		return rbegin_itr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::const_iterator vmem_container<T, Header, Pool, Log>::rbegin() const noexcept {
		return rbegin_itr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::const_iterator vmem_container<T, Header, Pool, Log>::crbegin() const noexcept {
		return rbegin_itr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline bool vmem_container<T, Header, Pool, Log>::empty() const noexcept {
		return _state->front_page_pos == vmem_page_pos_nil
			|| _state->back_page_pos == vmem_page_pos_nil;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline std::size_t vmem_container<T, Header, Pool, Log>::size() const noexcept {
		return _state->total_item_count;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::pointer vmem_container<T, Header, Pool, Log>::frontptr() noexcept {
		return begin().ptr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::const_pointer vmem_container<T, Header, Pool, Log>::frontptr() const noexcept {
		return begin().ptr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::reference vmem_container<T, Header, Pool, Log>::front() {
		return begin().deref();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::const_reference vmem_container<T, Header, Pool, Log>::front() const {
		return begin().deref();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::pointer vmem_container<T, Header, Pool, Log>::backptr() noexcept {
		return rend().ptr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::const_pointer vmem_container<T, Header, Pool, Log>::backptr() const noexcept {
		return rend().ptr();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::reference vmem_container<T, Header, Pool, Log>::back() {
		return rend().deref();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::const_reference vmem_container<T, Header, Pool, Log>::back() const {
		return rend().deref();
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline void vmem_container<T, Header, Pool, Log>::push_back(const_reference item) {
		insert(end(), item);
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline void vmem_container<T, Header, Pool, Log>::pop_back() {
		erase(rend());
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline void vmem_container<T, Header, Pool, Log>::push_front(const_reference item) {
		insert(begin(), item);
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline void vmem_container<T, Header, Pool, Log>::pop_front() {
		erase(begin());
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::result2 vmem_container<T, Header, Pool, Log>::insert2(const_iterator itr, const_reference item) {
		if (itr.page_pos() == vmem_page_pos_nil && (itr.item_pos() != vmem_item_pos_nil || !empty())) {
			throw exception<std::logic_error, Log>("vmem_container::insert2(itr.page_pos)", 0x1044a);
		}

		if (itr.item_pos() == vmem_item_pos_nil && (itr.page_pos() != _state->back_page_pos && itr.edge() != vmem_iterator_edge::end)) {
			throw exception<std::logic_error, Log>("vmem_container::insert2(itr.item_pos)", 0x1044b);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1044c, "vmem_container::insert2() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge());
		}

		// Copy the item to a local variable to make sure the reference is valid and copyable before we change any page.
		T item_copy(item);

		// Insert without changing the state.
		result2 result = insert_nostate(itr, item_copy);

		if (result.iterator.is_valid()) {
			// We have inserted successfully.

			// Update the front page pos.
			if (_state->front_page_pos == vmem_page_pos_nil) {
				_state->front_page_pos = result.iterator.page_pos();
			}

			// Update the back page pos.
			if (_state->back_page_pos == vmem_page_pos_nil) {
				_state->back_page_pos = result.iterator.page_pos();
			}
			else if (_state->back_page_pos == itr.page_pos() && result.page_leads[0].page_pos != vmem_page_pos_nil) {
				_state->back_page_pos = result.page_leads[0].page_pos;
			} 

			// Update the total item count.
			_state->total_item_count++;
		}
		else {
			// We have failed to insert.

			// Return end().
			result.iterator = end_itr();
			result.page_leads[0] = result.page_leads[1] = page_lead();
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1044d, "vmem_container::insert2() Done. result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, result.page_pos=0x%llx, total_item_count=%zu",
				(long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (long long)result.page_leads[0].page_pos, (std::size_t)_state->total_item_count);
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::iterator vmem_container<T, Header, Pool, Log>::insert(const_iterator itr, const_reference item) {
		return insert2(itr, item).iterator;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	template <typename InputItr>
	inline typename vmem_container<T, Header, Pool, Log>::iterator vmem_container<T, Header, Pool, Log>::insert(const_iterator itr, InputItr first, InputItr last) {
		iterator ret(itr);

		for (InputItr item = first; item != last; item++) {
			if (!insert(itr++, *item).can_deref()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::important, 0x1044e, "vmem_container::insert() Breaking from the loop.");
				}
				break;
			}
		}

		return ret;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::result2 vmem_container<T, Header, Pool, Log>::insert_nostate(const_iterator itr, const_reference item) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1044f, "vmem_container::insert_nostate() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge());
		}

		result2 result;

		if (itr.page_pos() == vmem_page_pos_nil) {
			result = insert_empty(item);
		}
		else {
			result = insert_nonempty(itr, item);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10450, "vmem_container::insert_nostate() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, result.page_pos=0x%llx",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (long long)result.page_leads[0].page_pos);
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::result2 vmem_container<T, Header, Pool, Log>::insert_empty(const_reference item) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10451, "vmem_container::insert_empty() Start");
		}

		result2 result;

		vmem_page<Pool, Log> page(nullptr);
		vmem_container_page<T, Header>* container_page = nullptr;
		bool ok = insert_page_after(vmem_page_pos_nil, page, container_page);

		if (ok) {
			iterator itr = iterator(this, page.pos(), 0, vmem_iterator_edge::none, _log);
			result = insert_with_capacity(itr, item, container_page);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10452, "vmem_container::insert_empty() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.page_pos=0x%llx",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), (long long)result.page_leads[0].page_pos);
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::result2 vmem_container<T, Header, Pool, Log>::insert_nonempty(const_iterator itr, const_reference item) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10453, "vmem_container::insert_nonempty() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
				(long long)itr.page_pos(), itr.item_pos());
		}

		result2 result;

		vmem_page<Pool, Log> page(_pool, itr.page_pos(), _log);

		if (page.ptr() == nullptr) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x10454, "vmem_container::insert_nonempty() Could not load page pos=0x%llx", (long long)page.pos());
			}
		}
		else {
			vmem_container_page<T, Header>* container_page = reinterpret_cast<vmem_container_page<T, Header>*>(page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10455, "vmem_container::insert_nonempty() item_count=%u, page_capacity=%zu", container_page->item_count, (std::size_t)page_capacity());
			}

			if (container_page->item_count == page_capacity()) {
				// The page has no capacity.
				result = insert_with_overflow(itr, item, container_page);
			}
			else {
				// The page has capacity.
				result = insert_with_capacity(itr, item, container_page);
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10456, "vmem_container::insert_nonempty() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.page_pos=0x%llx",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), (long long)result.page_leads[0].page_pos);
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::result2 vmem_container<T, Header, Pool, Log>::insert_with_overflow(const_iterator itr, const_reference item, vmem_container_page<T, Header>* container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10457, "vmem_container::insert_with_overflow() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
				(long long)itr.page_pos(), itr.item_pos());
		}

		result2 result;

		// Decide whether we should balance before we alter container_page.
		bool balance = should_balance_insert(itr, container_page);

		vmem_page<Pool, Log> new_page(nullptr);
		vmem_container_page<T, Header>* new_container_page = nullptr;
		bool ok = insert_page_after(itr.page_pos(), new_page, new_container_page);

		if (ok) {
			if (balance) {
				balance_split(itr.page_pos(), container_page, new_page.pos(), new_container_page);
			}

			if (itr.item_pos() != vmem_item_pos_nil && itr.item_pos() <= container_page->item_count) {
				// Inserting to the former page.
				result = insert_with_capacity(itr, item, container_page);
			}
			else {
				// Inserting to the latter page.
				vmem_container_iterator<T, Header, Pool, Log> new_itr(this, new_page.pos(), itr.item_pos() != vmem_item_pos_nil ? itr.item_pos() - container_page->item_count : new_container_page->item_count, vmem_iterator_edge::none, _log);
				result = insert_with_capacity(new_itr, item, new_container_page);
			}

			// page_leads[0] - insert; new page
			// page_leads[1] - original; used only when a new level is created
			result.page_leads[0] = page_lead(vmem_container_page_lead_operation::insert, new_page.pos());
			vmem_copy(result.page_leads[0].items[0], new_container_page->items[0]);
			result.page_leads[1] = page_lead(vmem_container_page_lead_operation::original, itr.page_pos());
			vmem_copy(result.page_leads[1].items[0], container_page->items[0]);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10458, "vmem_container::insert_with_overflow() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.page_pos=0x%llx",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), (long long)result.page_leads[0].page_pos);
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::result2 vmem_container<T, Header, Pool, Log>::insert_with_capacity(const_iterator itr, const_reference item, vmem_container_page<T, Header>* container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10459, "vmem_container::insert_with_capacity() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
				(long long)itr.page_pos(), itr.item_pos());
		}

		result2 result;
		result.iterator = iterator(this, itr.page_pos(), itr.item_pos() != vmem_item_pos_nil ? itr.item_pos() : container_page->item_count, vmem_iterator_edge::none, _log);

		// Shift items from the insertion position to free up a slot.
		std::size_t move_item_count = container_page->item_count - result.iterator.item_pos();
		if (move_item_count > 0) {
			std::memmove(&container_page->items[result.iterator.item_pos() + 1], &container_page->items[result.iterator.item_pos()], move_item_count * sizeof(T));
		}

		// Insert the item.
		++container_page->item_count;
		vmem_copy(container_page->items[result.iterator.item_pos()], item);

		if (_log != nullptr) {
			_log->put_binary(category::abc::vmem, severity::abc::debug, 0x1045a, &container_page->items[result.iterator.item_pos()], std::min(sizeof(T), (std::size_t)16));
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1045b, "vmem_container::insert_with_capacity() Done. result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x",
				(long long)result.iterator.page_pos(), result.iterator.item_pos());
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline void vmem_container<T, Header, Pool, Log>::balance_split(vmem_page_pos_t page_pos, vmem_container_page<T, Header>* container_page, vmem_page_pos_t new_page_pos, vmem_container_page<T, Header>* new_container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1045c, "vmem_container::balance() Start. page_pos=0x%llx, new_page_pos=0x%llx",
				(long long)page_pos, (long long)new_page_pos);
		}

		constexpr std::size_t new_page_item_count = page_capacity() / 2;
		constexpr std::size_t page_item_count = page_capacity() - new_page_item_count;
		std::memmove(&new_container_page->items[0], &container_page->items[page_item_count], new_page_item_count * sizeof(T));
		new_container_page->item_count = new_page_item_count;
		container_page->item_count = page_item_count;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1045d, "vmem_container::balance() Done. page_pos=0x%llx, item_count=%u, new_page_pos=0x%llx, new_item_count=%u",
				(long long)page_pos, container_page->item_count, (long long)new_page_pos, new_container_page->item_count);
		}
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline bool vmem_container<T, Header, Pool, Log>::insert_page_after(vmem_page_pos_t after_page_pos, vmem_page<Pool, Log>& new_page, vmem_container_page<T, Header>*& new_container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1045e, "vmem_container::insert_page_after() Start. after_page_pos=0x%llx",
				(long long)after_page_pos);
		}

		bool ok = true;
		
		vmem_page<Pool, Log> new_page_local(_pool, _log);
		vmem_container_page<T, Header>* new_container_page_local = nullptr;

		if (new_page_local.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x1045f, "vmem_container::insert_page_after() Could not create page");
			}
		}

		if (ok) {
			new_container_page_local = reinterpret_cast<vmem_container_page<T, Header>*>(new_page_local.ptr());

			vmem_linked<Pool, Log> linked(_state, _pool, _log);

			vmem_linked_iterator<Pool, Log> itr = linked.end();
			if (after_page_pos != vmem_page_pos_nil) {
				itr = vmem_linked_iterator<Pool, Log>(&linked, after_page_pos, vmem_item_pos_nil, vmem_iterator_edge::none, _log);
				itr++;
			}

			vmem_linked_iterator<Pool, Log> new_itr = linked.insert(itr, new_page_local.pos());
			ok = new_itr != linked.end();

			if (!ok) {
				new_page_local.free();
				new_container_page_local = nullptr;
			}
		}

		if (ok) {
			new_container_page_local->item_count = 0;

			new_page = std::move(new_page_local);
			new_container_page = new_container_page_local;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10460, "vmem_container::insert_page_after() Done. ok=%d, after_page_pos=0x%llx, new_page_pos=0x%llx",
				ok, (long long)after_page_pos, (long long)new_page.pos());
		}

		return ok;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline bool vmem_container<T, Header, Pool, Log>::should_balance_insert(const_iterator itr, const vmem_container_page<T, Header>* container_page) const noexcept {
		bool balance = false;

		if (container_page->prev_page_pos == vmem_page_pos_nil && itr.item_pos() == 0) {
			balance = vmem_page_balance::test(_balance_insert, vmem_page_balance::begin);
		}
		else if (container_page->next_page_pos == vmem_page_pos_nil && itr.item_pos() == vmem_item_pos_nil && itr.edge() == vmem_iterator_edge::end) {
			balance = vmem_page_balance::test(_balance_insert, vmem_page_balance::end);
		}
		else {
			balance = vmem_page_balance::test(_balance_insert, vmem_page_balance::inner);
		}

		return balance;
	}


	// ..............................................................


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::result2 vmem_container<T, Header, Pool, Log>::erase2(const_iterator itr) {
		if (!itr.can_deref()) {
			throw exception<std::logic_error, Log>("vmem_container::erase(itr)", 0x10461);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x10462, "vmem_container::erase() Begin. page_pos=0x%llx, item_pos=0x%x, edge=%u, total_item_count=%zu",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge(), (std::size_t)_state->total_item_count);
		}

		result2 result = erase_nostate(itr);

		if (result.iterator.is_valid()) {
			// Update the total item count.
			_state->total_item_count--;
		}
		else {
			result = result2();
			result.iterator = end_itr();
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x10463, "vmem_container::erase() Done. result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, total_item_count=%zu",
				(long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (std::size_t)_state->total_item_count);
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::iterator vmem_container<T, Header, Pool, Log>::erase(const_iterator itr) {
		return erase2(itr).iterator;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::iterator vmem_container<T, Header, Pool, Log>::erase(const_iterator first, const_iterator last) {
		iterator itr = first;

		while (itr != last) {
			if (!itr.can_deref()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::important, 0x10464, "vmem_container::erase(first, last) Breaking from the loop.");
				}

				break;
			}

			itr = erase(itr);
		}

		return itr;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::result2 vmem_container<T, Header, Pool, Log>::erase_nostate(const_iterator itr) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10465, "vmem_container::erase_nostate() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
				(long long)itr.page_pos(), itr.item_pos());
		}

		result2 result;

		vmem_page<Pool, Log> page(_pool, itr.page_pos(), _log);

		if (page.ptr() == nullptr) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x10466, "vmem_container::erase() Could not load page pos=0x%llx", (long long)itr.page_pos());
			}
		}
		else {
			vmem_container_page<T, Header>* container_page = reinterpret_cast<vmem_container_page<T, Header>*>(page.ptr());

			if (container_page->item_count > 1) {
				// Determine whether we should balance before any inout parameter gets altered.
				bool balance = should_balance_erase(container_page, itr.item_pos());

				// There are many items on the page.
				result = erase_from_many(itr, container_page);

				// Balance if item count drops below half of capacity.
				if (balance && 2 * container_page->item_count <= page_capacity()) {
					result2 res = balance_merge(result.iterator, page, container_page);

					res.page_leads[0] = result.page_leads[0];
					result = res;
				}
			}
			else {
				// Erasing the only item on a page means erasing the page.
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, 0x10467, "vmem_container::erase_nostate() Only.");
				}

				if (container_page->next_page_pos != vmem_page_pos_nil) {
					result.iterator = iterator(this, container_page->next_page_pos, 0, vmem_iterator_edge::none, _log);
				}
				else {
					result.iterator = end_itr();
				}

				// page_leads[0] - none
				// page_leads[1] - erase
				result.page_leads[0] = page_lead();
				result.page_leads[1] = page_lead(vmem_container_page_lead_operation::erase, page.pos());
				vmem_copy(result.page_leads[1].items[0], container_page->items[0]);

				erase_page(page);
				container_page = nullptr;
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10468, "vmem_container::erase_nostate() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge());
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::result2 vmem_container<T, Header, Pool, Log>::erase_from_many(const_iterator itr, vmem_container_page<T, Header>* container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10469, "vmem_container::erase_from_many() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
				(long long)itr.page_pos(), itr.item_pos());
		}

		result2 result;

		if (itr.item_pos() < container_page->item_count - 1) {
			if (itr.item_pos() == 0) {
				// page_leads[0] - replace
				// page_leads[1] - none
				result.page_leads[0] = page_lead(vmem_container_page_lead_operation::replace, itr.page_pos());
				vmem_copy(result.page_leads[0].items[0], container_page->items[0]);
				vmem_copy(result.page_leads[0].items[1], container_page->items[1]);
				result.page_leads[1] = page_lead();
			}

			// To delete an item before the last one, pull up the remaining elements.

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x1046a, "vmem_container::erase_from_many() Middle.");
			}

			std::size_t move_item_count = container_page->item_count - itr.item_pos() - 1;
			std::memmove(&container_page->items[itr.item_pos()], &container_page->items[itr.item_pos() + 1], move_item_count * sizeof(T));

			result.iterator = itr;
		}
		else {
			// To delete the last (back) item on a page, there is nothing to do.

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x1046b, "vmem_container::erase_from_many() Last.");
			}

			// If we are deleting the last item on a page, the next item is item 0 on the next page or end().
			if (container_page->next_page_pos != vmem_page_pos_nil) {
				result.iterator = iterator(this, container_page->next_page_pos, 0, vmem_iterator_edge::none, _log);
			}
			else {
				result.iterator = end_itr();
			}
		}

		// The main part of deleting an item from a page is decrementing the count.
		container_page->item_count--;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1046c, "vmem_container::erase_from_many() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge());
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::result2 vmem_container<T, Header, Pool, Log>::balance_merge(const_iterator itr, vmem_page<Pool, Log>& page, vmem_container_page<T, Header>* container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1046d, "vmem_container::balance_merge_safe() Start. page_pos=0x%llx",
				(long long)page.pos());
		}

		result2 result;
		result.iterator = itr;

		// Try the next page.
		if (container_page->next_page_pos != vmem_page_pos_nil) {
			result = balance_merge_next(itr, page, container_page);
		}

		// Try the previous page.
		if (container_page->prev_page_pos != vmem_page_pos_nil) {
			result = balance_merge_prev(itr, page, container_page);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1046e, "vmem_container::balance_merge_safe() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos());
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::result2 vmem_container<T, Header, Pool, Log>::balance_merge_next(const_iterator itr, vmem_page<Pool, Log>& page, vmem_container_page<T, Header>* container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1046f, "vmem_container::balance_merge_next_safe() Start. page_pos=0x%llx",
				(long long)page.pos());
		}

		result2 result;
		result.iterator = itr;

		vmem_page<Pool, Log> next_page(_pool, container_page->next_page_pos, _log);
		vmem_container_page<T, Header>* next_container_page = nullptr;

		if (next_page.ptr() == nullptr) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10470, "vmem_container::balance_merge_next_safe() Could not load page pos=0x%llx",
					(long long)container_page->next_page_pos);
			}
		}
		else {
			next_container_page = reinterpret_cast<vmem_container_page<T, Header>*>(next_page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10471, "vmem_container::balance_merge_next_safe() page_item_count=%u, next_page_pos=0x%llx, next_page_item_count=%u",
					container_page->item_count, (long long)next_page.pos(), next_container_page->item_count);
			}

			if (container_page->item_count + next_container_page->item_count <= page_capacity()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, 0x10472, "vmem_container::balance_merge_next_safe() Do.");
				}

				// page_leads[0] - none
				// page_leads[1] - erase
				result.page_leads[0] = page_lead();
				result.page_leads[1] = page_lead(vmem_container_page_lead_operation::erase, next_page.pos());
				vmem_copy(result.page_leads[1].items[0], next_container_page->items[0]);

				// Merge the items from the next page into this one.
				std::memmove(&container_page->items[container_page->item_count], &next_container_page->items[0], next_container_page->item_count * sizeof(T));

				// Fix the next item, if it was item[0] on the next page.
				if (itr.page_pos() == next_page.pos()) {
					result.iterator = iterator(this, page.pos(), container_page->item_count, vmem_iterator_edge::none, _log);
				}

				// Update the item count on this page.
				container_page->item_count += next_container_page->item_count;

				// Free the next page.
				erase_page(next_page);
				next_container_page = nullptr;
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10473, "vmem_container::balance_merge_next_safe() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge());
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::result2 vmem_container<T, Header, Pool, Log>::balance_merge_prev(const_iterator itr, vmem_page<Pool, Log>& page, vmem_container_page<T, Header>* container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10474, "vmem_container::balance_merge_prev() Start. page_pos=0x%llx",
				(long long)page.pos());
		}

		result2 result;
		result.iterator = itr;


		vmem_page<Pool, Log> prev_page(_pool, container_page->prev_page_pos, _log);
		vmem_container_page<T, Header>* prev_container_page = nullptr;

		if (prev_page.ptr() == nullptr) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10475, "vmem_container::balance_merge_prev() Could not load page pos=0x%llx",
					(long long)container_page->prev_page_pos);
			}
		}
		else {
			prev_container_page = reinterpret_cast<vmem_container_page<T, Header>*>(prev_page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10476, "vmem_container::balance_merge_prev() page_pos=0x%llx, page_item_count=%u, prev_page_pos=0x%llx, prev_page_item_count=%u",
					(long long)page.pos(), container_page->item_count, (long long)prev_page.pos(), prev_container_page->item_count);
			}

			if (container_page->item_count + prev_container_page->item_count <= page_capacity()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, 0x10477, "vmem_container::balance_merge_prev() Do.");
				}

				// page_leads[0] - none
				// page_leads[1] - erase
				result.page_leads[0] = page_lead();
				result.page_leads[1] = page_lead(vmem_container_page_lead_operation::erase, page.pos());
				vmem_copy(result.page_leads[1].items[0], container_page->items[0]);

				// Merge the items from this page into the previous one.
				std::memmove(&prev_container_page->items[prev_container_page->item_count], &container_page->items[0], container_page->item_count * sizeof(T));

				// Update the result only if itr references this page.
				// If we deleted the last item on this page, itr references item[0] on the next page, and will not be affected by this balancing.
				if (itr.page_pos() == page.pos()) {
					if (itr.item_pos() != vmem_item_pos_nil) {
						result.iterator = iterator(this, prev_page.pos(), itr.item_pos() + prev_container_page->item_count, vmem_iterator_edge::none, _log);
					}
					else {
						result.iterator = iterator(this, prev_page.pos(), itr.item_pos(), itr.edge(), _log);
					}
				}

				// Update the item count on the previous page.
				prev_container_page->item_count += container_page->item_count;

				// Free this page.
				erase_page(page);
				container_page = nullptr;
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10478, "vmem_container::balance_merge_prev() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge());
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline bool vmem_container<T, Header, Pool, Log>::erase_page(vmem_page<Pool, Log>& page) noexcept {
		vmem_page_pos_t page_pos = page.pos();

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10479, "vmem_container::erase_page() Start. page_pos=0x%llx",
				(long long)page_pos);
		}

		bool ok = erase_page_pos(page_pos);

		if (ok) {
			page.free();
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1047a, "vmem_container::erase_page() Done. ok=%d, page_pos=0x%llx",
				ok, (long long)page_pos);
		}

		return ok;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline bool vmem_container<T, Header, Pool, Log>::erase_page_pos(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1047b, "vmem_container::erase_page_pos() Start. page_pos=0x%llx",
				(long long)page_pos);
		}

		vmem_linked<Pool, Log> linked(_state, _pool, _log);

		vmem_linked_iterator<Pool, Log> itr(&linked, page_pos, vmem_item_pos_nil, vmem_iterator_edge::none, _log);
		linked.erase(itr);
		bool ok = true;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1047c, "vmem_container::erase_page_pos() Done. ok=%d, page_pos=0x%llx",
				(int)ok, (long long)page_pos);
		}

		return ok;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline bool vmem_container<T, Header, Pool, Log>::should_balance_erase(const vmem_container_page<T, Header>* container_page, vmem_item_pos_t item_pos) const noexcept {
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


	template <typename T, typename Header, typename Pool, typename Log>
	inline void vmem_container<T, Header, Pool, Log>::clear() noexcept {
		vmem_linked<Pool, Log> linked(_state, _pool, _log);
		linked.clear();
	}


	// ..............................................................


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::iterator vmem_container<T, Header, Pool, Log>::next(const iterator_state& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1047d, "vmem_container::next() Before itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge());
		}

		iterator result = end_itr();

		if (itr.item_pos() == vmem_item_pos_nil && itr.edge() == vmem_iterator_edge::rbegin) {
			result = begin_itr();
		}
		else if (itr.page_pos() != vmem_page_pos_nil) {
			vmem_page<Pool, Log> page(_pool, itr.page_pos(), _log);

			if (page.ptr() == nullptr) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, 0x1047e, "vmem_container::next() Could not load page pos=0x%llx", (long long)itr.page_pos());
				}
			}
			else {
				vmem_container_page<T, Header>* container_page = reinterpret_cast<vmem_container_page<T, Header>*>(page.ptr());

				if (itr.item_pos() < container_page->item_count - 1) {
					result = iterator(this, itr.page_pos(), itr.item_pos() + 1, vmem_iterator_edge::none, _log);
				}
				else {
					if (container_page->next_page_pos != vmem_page_pos_nil) {
						result = iterator(this, container_page->next_page_pos, 0, vmem_iterator_edge::none, _log);
					}
				}
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1047f, "vmem_container::next() After result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
				(long long)result.page_pos(), result.item_pos(), result.edge());
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::iterator vmem_container<T, Header, Pool, Log>::prev(const iterator_state& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10480, "vmem_container::prev() Before itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge());
		}

		iterator result = rbegin_itr();

		if (itr.item_pos() == vmem_item_pos_nil && itr.edge() == vmem_iterator_edge::end) {
			result = rend_itr();
		}
		else if (itr.page_pos() != vmem_page_pos_nil) {
			vmem_page<Pool, Log> page(_pool, itr.page_pos(), _log);

			if (page.ptr() == nullptr) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, 0x10481, "vmem_container::prev() Could not load page pos=0x%llx", (long long)itr.page_pos());
				}
			}
			else {
				vmem_container_page<T, Header>* container_page = reinterpret_cast<vmem_container_page<T, Header>*>(page.ptr());

				if (itr.item_pos() > 0) {
					result = iterator(this, itr.page_pos(), itr.item_pos() - 1, vmem_iterator_edge::none, _log);
				}
				else {
					if (container_page->prev_page_pos != vmem_page_pos_nil) {
						vmem_page<Pool, Log> prev_page(_pool, container_page->prev_page_pos, _log);

						if (prev_page.ptr() == nullptr) {
							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::warning, 0x10482, "vmem_container::prev() Could not load page pos=0x%llx", (long long)container_page->prev_page_pos);
							}
						}
						else {
							vmem_container_page<T, Header>* prev_container_page = reinterpret_cast<vmem_container_page<T, Header>*>(prev_page.ptr());
		
							result = iterator(this, container_page->prev_page_pos, prev_container_page->item_count - 1, vmem_iterator_edge::none, _log);
						}
					}
				}
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10483, "vmem_container::prev() After result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
				(long long)result.page_pos(), result.item_pos(), result.edge());
		}

		return result;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::pointer vmem_container<T, Header, Pool, Log>::at(const iterator_state& itr) const noexcept {
		vmem_item_pos_t byte_pos =
			itr.item_pos() == vmem_item_pos_nil ?
				vmem_item_pos_nil :
				items_pos() + (itr.item_pos() * sizeof(T));

		return pointer(_pool, itr.page_pos(), byte_pos, _log);
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::iterator vmem_container<T, Header, Pool, Log>::begin_itr() const noexcept {
		iterator itr(this, _state->back_page_pos, vmem_item_pos_nil, vmem_iterator_edge::end, _log);

		if (_state->front_page_pos != vmem_page_pos_nil) {
			itr = iterator(this, _state->front_page_pos, 0, vmem_iterator_edge::none, _log);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10484, "vmem_container::begin_itr() page_pos=0x%llx, item_pos=0x%x, edge=%u",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge());
		}

		return itr;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::iterator vmem_container<T, Header, Pool, Log>::end_itr() const noexcept {
		iterator itr(this, _state->back_page_pos, vmem_item_pos_nil, vmem_iterator_edge::end, _log);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10486, "vmem_container::end_itr() page_pos=0x%llx, item_pos=0x%x, edge=%u",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge());
		}

		return itr;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::reverse_iterator vmem_container<T, Header, Pool, Log>::rend_itr() const noexcept {
		iterator itr(this, _state->front_page_pos, vmem_item_pos_nil, vmem_iterator_edge::rbegin, _log);

		if (_state->back_page_pos != vmem_page_pos_nil) {
			vmem_page<Pool, Log> page(_pool, _state->back_page_pos, _log);

			if (page.ptr() == nullptr) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, 0x10487, "vmem_container::rend_itr() Could not load page pos=0x%llx", (long long)_state->back_page_pos);
				}
			}
			else {
				vmem_container_page<T, Header>* container_page = reinterpret_cast<vmem_container_page<T, Header>*>(page.ptr());
				itr = iterator(this, _state->back_page_pos, container_page->item_count - 1, vmem_iterator_edge::none, _log);
			}
		}


		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10488, "vmem_container::rbegin_itr() page_pos=0x%llx, item_pos=0x%x, edge=%u",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge());
		}

		return itr;
	}


	template <typename T, typename Header, typename Pool, typename Log>
	inline typename vmem_container<T, Header, Pool, Log>::reverse_iterator vmem_container<T, Header, Pool, Log>::rbegin_itr() const noexcept {
		iterator itr(this, _state->front_page_pos, vmem_item_pos_nil, vmem_iterator_edge::rbegin, _log);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10485, "vmem_container::rbegin_itr() page_pos=0x%llx, item_pos=0x%x, edge=%u",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge());
		}

		return itr;
	}


	// --------------------------------------------------------------

}
