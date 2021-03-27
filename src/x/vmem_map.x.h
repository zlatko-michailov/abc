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

	// --------------------------------------------------------------


	template <typename Key, typename T, typename Pool, typename Log>
	vmem_map_find_result2<Key, T, Pool, Log>::vmem_map_find_result2(nullptr_t) noexcept
		: actual_iterator(nullptr)
		, expected_iterator(nullptr) {
	}


	// --------------------------------------------------------------


	template <typename Key, typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_map<Key, T, Pool, Log>::key_items_pos() noexcept {
		return sizeof(vmem_map_key_page<Key>) - sizeof(vmem_map_key<Key>);
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_map<Key, T, Pool, Log>::max_key_item_size() noexcept {
		return vmem_page_size - key_items_pos();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_map<Key, T, Pool, Log>::key_page_capacity() noexcept {
		return max_key_item_size() / sizeof(vmem_map_key<Key>);
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_map<Key, T, Pool, Log>::value_items_pos() noexcept {
		return sizeof(vmem_map_value_page<Key, T>) - sizeof(vmem_map_value<Key, T>);
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_map<Key, T, Pool, Log>::max_value_item_size() noexcept {
		return vmem_page_size - value_items_pos();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_map<Key, T, Pool, Log>::value_page_capacity() noexcept {
		return max_value_item_size() / sizeof(vmem_map_value<Key, T>);
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline constexpr bool vmem_map<Key, T, Pool, Log>::is_uninit(const vmem_map_state* state) noexcept {
		return
			// nil
			(
				state != nullptr
				&& state->values.front_page_pos == vmem_page_pos_nil
				&& state->values.back_page_pos == vmem_page_pos_nil
				&& state->values.item_size == 0
			)
			||
			// zero
			(
				state != nullptr
				&& state->values.front_page_pos == 0
				&& state->values.back_page_pos == 0
				&& state->values.item_size == 0
			);
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline vmem_map<Key, T, Pool, Log>::vmem_map(vmem_map_state* state, Pool* pool, Log* log)
		: _state(state)
		, _pool(pool)
		, _log(log) {

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_map::vmem_map() state=%p, pool=%p", state, pool);
		}

		if (state == nullptr) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(state)", __TAG__);
		}

		if (pool == nullptr) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(pool)", __TAG__);
		}

		if (sizeof(vmem_map_key<Key>) > max_key_item_size()) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(key size) excess", __TAG__);
		}

		if (sizeof(vmem_map_value<Key, T>) > max_value_item_size()) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(value size) excess", __TAG__);
		}

		if (Pool::max_mapped_pages() < vmem_min_mapped_pages) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(pool<MaxMappedPages>)", __TAG__);
		}

		if (is_uninit(state)) {
			_state->keys.front_page_pos = vmem_page_pos_nil;
			_state->keys.back_page_pos = vmem_page_pos_nil;
			_state->keys.item_size = sizeof(vmem_map_key<Key>);

			_state->values.front_page_pos = vmem_page_pos_nil;
			_state->values.back_page_pos = vmem_page_pos_nil;
			_state->values.item_size = sizeof(vmem_map_value<Key, T>);
		}

		if (sizeof(vmem_map_key<Key>) != _state->keys.item_size) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(key size) mismatch", __TAG__);
		}

		if (sizeof(vmem_map_value<Key, T>) != _state->values.item_size) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(value size) mismatch", __TAG__);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_map::vmem_map() keys.front_page_pos=0x%llx, keys.back_page_pos=0x%llx,  values.front_page_pos=0x%llx, values.back_page_pos=0x%llx", 
				(long long)_state->keys.front_page_pos, (long long)_state->keys.back_page_pos, (long long)_state->values.front_page_pos, (long long)_state->values.back_page_pos);
		}
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::begin() noexcept {
		return cbegin();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::begin() const noexcept {
		return cbegin();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::cbegin() const noexcept {
		return begin_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::end() noexcept {
		return cend();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::end() const noexcept {
		return cend();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::cend() const noexcept {
		return end_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::rend() noexcept {
		return crend();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::rend() const noexcept {
		return crend();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::crend() const noexcept {
		return rend_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::rbegin() noexcept {
		return crbegin();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::rbegin() const noexcept {
		return crbegin();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::crbegin() const noexcept {
		return rbegin_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline bool vmem_map<Key, T, Pool, Log>::empty() const noexcept {
		return _state->values.front_page_pos == vmem_page_pos_nil
			|| _state->values.back_page_pos == vmem_page_pos_nil;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline std::size_t vmem_map<Key, T, Pool, Log>::size() const noexcept {
		return _state->values.total_item_count;
	}


#ifdef REMOVE ////
	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::insert2(const_iterator itr, const_reference item) {
		if (itr.page_pos() == vmem_page_pos_nil && (itr.item_pos() != vmem_item_pos_nil || !empty())) {
			throw exception<std::logic_error, Log>("vmem_map::insert2(itr.page_pos)", 0x1044a);
		}

		if (itr.item_pos() == vmem_item_pos_nil && (itr.page_pos() != _state->back_page_pos && itr.edge() != vmem_iterator_edge::end)) {
			throw exception<std::logic_error, Log>("vmem_map::insert2(itr.item_pos)", 0x1044b);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1044c, "vmem_map::insert2() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
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
			else if (_state->back_page_pos == itr.page_pos() && result.page_pos != vmem_page_pos_nil) {
				_state->back_page_pos = result.page_pos;
			} 

			// Update the total item count.
			_state->total_item_count++;
		}
		else {
			// We have failed to insert.

			// Return end().
			result.iterator = end_itr();
			result.page_pos = vmem_page_pos_nil;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1044d, "vmem_map::insert2() Done. result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, result.page_pos=0x%llx, total_item_count=%zu",
				(long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (long long)result.page_pos, (std::size_t)_state->total_item_count);
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::insert(const_iterator itr, const_reference item) {
		return insert2(itr, item).iterator;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	template <typename InputItr>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::insert(const_iterator itr, InputItr first, InputItr last) {
		iterator ret(itr);

		for (InputItr item = first; item != last; item++) {
			if (!insert(itr++, *item).can_deref()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::important, 0x1044e, "vmem_map::insert() Breaking from the loop.");
				}
				break;
			}
		}

		return ret;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::insert_nostate(const_iterator itr, const_reference item) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1044f, "vmem_map::insert_nostate() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge());
		}

		result2 result(nullptr);

		if (itr.page_pos() == vmem_page_pos_nil) {
			result = insert_empty(item);
		}
		else {
			result = insert_nonempty(itr, item);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::insert_nostate() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, , result.page_pos=0x%llx",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (long long)result.page_pos);
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::insert_empty(const_reference item) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::insert_empty() Start");
		}

		result2 result(nullptr);

		vmem_page<Pool, Log> page(nullptr);
		vmem_map_page<T, Header>* container_page = nullptr;
		bool ok = insert_page_after(vmem_page_pos_nil, page, container_page);

		if (ok) {
			iterator itr = iterator(this, page.pos(), 0, vmem_iterator_edge::none, _log);
			result = insert_with_capacity(itr, item, container_page);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::insert_empty() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.page_pos=0x%llx",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), (long long)result.page_pos);
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::insert_nonempty(const_iterator itr, const_reference item) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::insert_nonempty() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
				(long long)itr.page_pos(), itr.item_pos());
		}

		result2 result(nullptr);

		vmem_page<Pool, Log> page(_pool, itr.page_pos(), _log);

		if (page.ptr() == nullptr) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_map::insert_nonempty() Could not load page pos=0x%llx", (long long)page.pos());
			}
		}
		else {
			vmem_map_page<T, Header>* container_page = reinterpret_cast<vmem_map_page<T, Header>*>(page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_map::insert_nonempty() item_count=%u, page_capacity=%zu", container_page->item_count, (std::size_t)page_capacity());
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
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::insert_nonempty() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.page_pos=0x%llx",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), (long long)result.page_pos);
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::insert_with_overflow(const_iterator itr, const_reference item, vmem_map_page<T, Header>* container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::insert_with_overflow() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
				(long long)itr.page_pos(), itr.item_pos());
		}

		result2 result(nullptr);

		// Decide whether we should balance before we alter container_page.
		bool balance = should_balance_insert(itr, container_page);

		vmem_page<Pool, Log> new_page(nullptr);
		vmem_map_page<T, Header>* new_container_page = nullptr;
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
				vmem_map_iterator<T, Header, Pool, Log> new_itr(this, new_page.pos(), itr.item_pos() != vmem_item_pos_nil ? itr.item_pos() - container_page->item_count : new_container_page->item_count, vmem_iterator_edge::none, _log);
				result = insert_with_capacity(new_itr, item, new_container_page);
			}

			result.page_pos = new_page.pos();
			std::memmove(&result.item_0, &new_container_page->items[0], sizeof(T));
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::insert_with_overflow() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.page_pos=0x%llx",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), (long long)result.page_pos);
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::insert_with_capacity(const_iterator itr, const_reference item, vmem_map_page<T, Header>* container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::insert_with_capacity() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
				(long long)itr.page_pos(), itr.item_pos());
		}

		result2 result(nullptr);
		result.iterator = iterator(this, itr.page_pos(), itr.item_pos() != vmem_item_pos_nil ? itr.item_pos() : container_page->item_count, vmem_iterator_edge::none, _log);

		// Shift items from the insertion position to free up a slot.
		std::size_t move_item_count = container_page->item_count - result.iterator.item_pos();
		if (move_item_count > 0) {
			std::memmove(&container_page->items[result.iterator.item_pos() + 1], &container_page->items[result.iterator.item_pos()], move_item_count * sizeof(T));
		}

		// Insert the item.
		++container_page->item_count;
		std::memmove(&container_page->items[result.iterator.item_pos()], &item, sizeof(T));

		if (_log != nullptr) {
			_log->put_binary(category::abc::vmem, severity::abc::debug, 0x1045a, &container_page->items[result.iterator.item_pos()], std::min(sizeof(T), (std::size_t)16));
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1045b, "vmem_map::insert_with_capacity() Done. result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x",
				(long long)result.iterator.page_pos(), result.iterator.item_pos());
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline void vmem_map<Key, T, Pool, Log>::balance_split(vmem_page_pos_t page_pos, vmem_map_page<T, Header>* container_page, vmem_page_pos_t new_page_pos, vmem_map_page<T, Header>* new_container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1045c, "vmem_map::balance() Start. page_pos=0x%llx, new_page_pos=0x%llx",
				(long long)page_pos, (long long)new_page_pos);
		}

		constexpr std::size_t new_page_item_count = page_capacity() / 2;
		constexpr std::size_t page_item_count = page_capacity() - new_page_item_count;
		std::memmove(&new_container_page->items[0], &container_page->items[page_item_count], new_page_item_count * sizeof(T));
		new_container_page->item_count = new_page_item_count;
		container_page->item_count = page_item_count;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1045d, "vmem_map::balance() Done. page_pos=0x%llx, item_count=%u, new_page_pos=0x%llx, new_item_count=%u",
				(long long)page_pos, container_page->item_count, (long long)new_page_pos, new_container_page->item_count);
		}
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline bool vmem_map<Key, T, Pool, Log>::insert_page_after(vmem_page_pos_t after_page_pos, vmem_page<Pool, Log>& new_page, vmem_map_page<T, Header>*& new_container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1045e, "vmem_map::insert_page_after() Start. after_page_pos=0x%llx",
				(long long)after_page_pos);
		}

		bool ok = true;
		
		vmem_page<Pool, Log> new_page_local(_pool, _log);
		vmem_map_page<T, Header>* new_container_page_local = nullptr;

		if (new_page_local.ptr() == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x1045f, "vmem_map::insert_page_after() Could not create page");
			}
		}

		if (ok) {
			new_container_page_local = reinterpret_cast<vmem_map_page<T, Header>*>(new_page_local.ptr());

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
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::insert_page_after() Done. ok=%d, after_page_pos=0x%llx, new_page_pos=0x%llx",
				ok, (long long)after_page_pos, (long long)new_page.pos());
		}

		return ok;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline bool vmem_map<Key, T, Pool, Log>::should_balance_insert(const_iterator itr, const vmem_map_page<T, Header>* container_page) const noexcept {
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


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::erase2(const_iterator itr) {
		if (!itr.can_deref()) {
			throw exception<std::logic_error, Log>("vmem_map::erase(itr)", __TAG__);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_map::erase() Begin. page_pos=0x%llx, item_pos=0x%x, edge=%u, total_item_count=%zu",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge(), (std::size_t)_state->total_item_count);
		}

		result2 result = erase_nostate(itr);

		if (result.iterator.is_valid()) {
			// Update the total item count.
			_state->total_item_count--;
		}
		else {
			result = result2(nullptr);
			result.iterator = end_itr();
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_map::erase() Done. result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, result.page_pos=0x%llx, total_item_count=%zu",
				(long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (long long)result.page_pos, (std::size_t)_state->total_item_count);
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::erase(const_iterator itr) {
		return erase2(itr).iterator;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::erase(const_iterator first, const_iterator last) {
		iterator item = first;

		while (item != last) {
			if (!item.can_deref()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::important, __TAG__, "vmem_map::erase() Breaking from the loop.");
				}

				break;
			}

			item = erase(item);
		}

		return item;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::erase_nostate(const_iterator itr) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::erase_nostate() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
				(long long)itr.page_pos(), itr.item_pos());
		}

		result2 result(nullptr);

		vmem_page<Pool, Log> page(_pool, itr.page_pos(), _log);

		if (page.ptr() == nullptr) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_map::erase() Could not load page pos=0x%llx", (long long)itr.page_pos());
			}
		}
		else {
			vmem_map_page<T, Header>* container_page = reinterpret_cast<vmem_map_page<T, Header>*>(page.ptr());

			if (container_page->item_count > 1) {
				// Determine whether we should balance before any inout parameter gets altered.
				bool balance = should_balance_erase(container_page, itr.item_pos());

				// There are many items on the page.
				result = erase_from_many(itr, container_page);

				// Balance if item count drops below half of capacity.
				if (balance && 2 * container_page->item_count <= page_capacity()) {
					result = balance_merge(result.iterator, page, container_page);
				}
			}
			else {
				// Erasing the only item on a page means erasing the page.
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_map::erase_nostate() Only.");
				}

				if (container_page->next_page_pos != vmem_page_pos_nil) {
					result.iterator = iterator(this, container_page->next_page_pos, 0, vmem_iterator_edge::none, _log);
				}
				else {
					result.iterator = end_itr();
				}

				result.page_pos = page.pos();
				std::memmove(&result.item_0, &container_page->items[0], sizeof(T));

				erase_page(page);
				container_page = nullptr;
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::erase_nostate() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, result.page_pos=0x%llx",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (long long)result.page_pos);
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::erase_from_many(const_iterator itr, vmem_map_page<T, Header>* container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::erase_from_many() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x",
				(long long)itr.page_pos(), itr.item_pos());
		}

		result2 result(nullptr);

		if (itr.item_pos() < container_page->item_count - 1) {
			// To delete an item before the last one, pull up the remaining elements.

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x1046a, "vmem_map::erase_from_many() Middle.");
			}

			std::size_t move_item_count = container_page->item_count - itr.item_pos() - 1;
			std::memmove(&container_page->items[itr.item_pos()], &container_page->items[itr.item_pos() + 1], move_item_count * sizeof(T));

			result.iterator = itr;
		}
		else {
			// To delete the last (back) item on a page, there is nothing to do.

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x1046b, "vmem_map::erase_from_many() Last.");
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
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1046c, "vmem_map::erase_from_many() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, result.page_pos=0x%llx",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (long long)result.page_pos);
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::balance_merge(const_iterator itr, vmem_page<Pool, Log>& page, vmem_map_page<T, Header>* container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1046d, "vmem_map::balance_merge_safe() Start. page_pos=0x%llx",
				(long long)page.pos());
		}

		result2 result(nullptr);
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
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1046e, "vmem_map::balance_merge_safe() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.page_pos=0x%llx",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), (long long)result.page_pos);
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::balance_merge_next(const_iterator itr, vmem_page<Pool, Log>& page, vmem_map_page<T, Header>* container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1046f, "vmem_map::balance_merge_next_safe() Start. page_pos=0x%llx",
				(long long)page.pos());
		}

		result2 result(nullptr);
		result.iterator = itr;

		vmem_page<Pool, Log> next_page(_pool, container_page->next_page_pos, _log);
		vmem_map_page<T, Header>* next_container_page = nullptr;

		if (next_page.ptr() == nullptr) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_map::balance_merge_next_safe() Could not load page pos=0x%llx",
					(long long)container_page->next_page_pos);
			}
		}
		else {
			next_container_page = reinterpret_cast<vmem_map_page<T, Header>*>(next_page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_map::balance_merge_next_safe() page_item_count=%u, next_page_pos=0x%llx, next_page_item_count=%u",
					container_page->item_count, (long long)next_page.pos(), next_container_page->item_count);
			}

			if (container_page->item_count + next_container_page->item_count <= page_capacity()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::balance_merge_next_safe() Do.");
				}

				result.page_pos = next_page.pos();
				std::memmove(&result.item_0, &next_container_page->items[0], sizeof(T));

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
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::balance_merge_next_safe() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, result.page_pos=0x%llx",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (long long)result.page_pos);
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::balance_merge_prev(const_iterator itr, vmem_page<Pool, Log>& page, vmem_map_page<T, Header>* container_page) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::balance_merge_prev() Start. page_pos=0x%llx",
				(long long)page.pos());
		}

		result2 result(nullptr);
		result.iterator = itr;


		vmem_page<Pool, Log> prev_page(_pool, container_page->prev_page_pos, _log);
		vmem_map_page<T, Header>* prev_container_page = nullptr;

		if (prev_page.ptr() == nullptr) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_map::balance_merge_prev() Could not load page pos=0x%llx",
					(long long)container_page->prev_page_pos);
			}
		}
		else {
			prev_container_page = reinterpret_cast<vmem_map_page<T, Header>*>(prev_page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_map::balance_merge_prev() page_pos=0x%llx, page_item_count=%u, prev_page_pos=0x%llx, prev_page_item_count=%u",
					(long long)page.pos(), container_page->item_count, (long long)prev_page.pos(), prev_container_page->item_count);
			}

			if (container_page->item_count + prev_container_page->item_count <= page_capacity()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::balance_merge_prev() Do.");
				}

				result.page_pos = prev_page.pos();
				std::memmove(&result.item_0, &prev_container_page->items[0], sizeof(T));

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
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::balance_merge_prev() Done. result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u, result.page_pos=0x%llx",
				result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge(), (long long)result.page_pos);
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline bool vmem_map<Key, T, Pool, Log>::erase_page(vmem_page<Pool, Log>& page) noexcept {
		vmem_page_pos_t page_pos = page.pos();

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_map::erase_page() Start. page_pos=0x%llx",
				(long long)page_pos);
		}

		bool ok = erase_page_pos(page_pos);

		if (ok) {
			page.free();
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1047a, "vmem_map::erase_page() Done. ok=%d, page_pos=0x%llx",
				ok, (long long)page_pos);
		}

		return ok;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline bool vmem_map<Key, T, Pool, Log>::erase_page_pos(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1047b, "vmem_map::erase_page_pos() Start. page_pos=0x%llx",
				(long long)page_pos);
		}

		vmem_linked<Pool, Log> linked(_state, _pool, _log);

		vmem_linked_iterator<Pool, Log> itr(&linked, page_pos, vmem_item_pos_nil, vmem_iterator_edge::none, _log);
		vmem_linked_iterator<Pool, Log> next_itr = linked.erase(itr);
		bool ok = true;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1047c, "vmem_map::erase_page_pos() Done. ok=%d, page_pos=0x%llx",
				ok, (long long)page_pos);
		}

		return ok;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline bool vmem_map<Key, T, Pool, Log>::should_balance_erase(const vmem_map_page<T, Header>* container_page, vmem_item_pos_t item_pos) const noexcept {
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


	template <typename Key, typename T, typename Pool, typename Log>
	inline void vmem_map<Key, T, Pool, Log>::clear() noexcept {
		vmem_linked<Pool, Log> linked(_state, _pool, _log);
		linked.clear();
	}


	// ..............................................................


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::next(const_iterator& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1047d, "vmem_map::next() Before itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
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
					_log->put_any(category::abc::vmem, severity::warning, 0x1047e, "vmem_map::next() Could not load page pos=0x%llx", (long long)itr.page_pos());
				}
			}
			else {
				vmem_map_page<T, Header>* container_page = reinterpret_cast<vmem_map_page<T, Header>*>(page.ptr());

				if (itr._item_pos < container_page->item_count - 1) {
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
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1047f, "vmem_map::next() After result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
				(long long)result.page_pos(), result.item_pos(), result.edge());
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::prev(const_iterator& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_map::prev() Before itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
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
					_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_map::prev() Could not load page pos=0x%llx", (long long)itr.page_pos());
				}
			}
			else {
				vmem_map_page<T, Header>* container_page = reinterpret_cast<vmem_map_page<T, Header>*>(page.ptr());

				if (itr._item_pos > 0) {
					result = iterator(this, itr.page_pos(), itr.item_pos() - 1, vmem_iterator_edge::none, _log);
				}
				else {
					if (container_page->prev_page_pos != vmem_page_pos_nil) {
						vmem_page<Pool, Log> prev_page(_pool, container_page->prev_page_pos, _log);

						if (prev_page.ptr() == nullptr) {
							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_map::prev() Could not load page pos=0x%llx", (long long)container_page->prev_page_pos);
							}
						}
						else {
							vmem_map_page<T, Header>* prev_container_page = reinterpret_cast<vmem_map_page<T, Header>*>(prev_page.ptr());
		
							result = iterator(this, container_page->prev_page_pos, prev_container_page->item_count - 1, vmem_iterator_edge::none, _log);
						}
					}
				}
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_map::prev() After result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
				(long long)result.page_pos(), result.item_pos(), result.edge());
		}

		return result;
	}
#endif


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::find_result2 vmem_map<Key, T, Pool, Log>::find2(const Key& key) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_map::find2() Start.");
		}

		vmem_page_pos_t page_pos = vmem_page_pos_nil;
		vmem_item_pos_t actual_item_pos = vmem_item_pos_nil;
		vmem_item_pos_t expected_item_pos = vmem_item_pos_nil;

		vmem_map_key_level_stack<Key, Pool, Log> key_stack(_state->keys, _pool, _log);

		if (!key_stack.empty()) {
			page_pos = key_stack.back().front_page_pos;

			for (std::size_t l = 0; page_pos != vmem_page_pos_nil && l < key_stack.size(); l++) {
				vmem_page<Pool, Log> page(page_pos, _pool, _log);

				if (page.ptr() == nullptr) {
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_map::find2() Could not load key page pos=0x%llx", (long long)page_pos);
					}

					page_pos = vmem_page_pos_nil;
					break;
				}
				else {
					vmem_map_key_page<Key>* key_page = reinterpret_cast<vmem_map_key_page<Key>*>(page.ptr());

					page_pos = key_page->items[0].page_pos;
					for (std::size_t i = 1; i < key_page->item_count && key < key_page->items[i].key; i++) {
						page_pos = key_page->items[i].page_pos;
					}
				}
			}

			if (page_pos != vmem_page_pos_nil) {
				vmem_page<Pool, Log> page(page_pos, _pool, _log);

				if (page.ptr() == nullptr) {
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_map::find2() Could not load value page pos=0x%llx", (long long)page_pos);
					}

					page_pos = vmem_page_pos_nil;
				}
				else {
					vmem_map_value_page<Key, T>* value_page = reinterpret_cast<vmem_map_value_page<Key, T>*>(page.ptr());

					actual_item_pos = vmem_item_pos_nil;
					expected_item_pos = 0;
					for (std::size_t i = 1; i < value_page->item_count && key <= value_page->items[i].key; i++) {
						expected_item_pos++;

						if (key == value_page->items[i].key) {
							actual_item_pos = i;
							break;
						}
					}
				}
			}
		}

		find_result2 result(nullptr);
		result.actual_iterator = end_itr();

		if (page_pos != vmem_item_pos_nil) {
			if (actual_item_pos != vmem_item_pos_nil) {
				result.actual_iterator = iterator(this, page_pos, actual_item_pos, _log);
			}

			result.expected_iterator = iterator(this, page_pos, expected_item_pos, _log);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_map::find2() Done. "
				"result.actual.valid=%d, result.actual.page_pos=0x%llx, result.actual.item_pos=0x%x, result.actual.edge=%u, "
				"result.expected.valid=%d, result.expected.page_pos=0x%llx, result.expected.item_pos=0x%x, result.expected.edge=%u",
				result.actual_iterator.is_valid(), (long long)result.actual_iterator.page_pos(), result.actual_iterator.item_pos(), result.actual_iterator.edge(),
				result.expected_iterator.is_valid(), (long long)result.expected_iterator.page_pos(), result.expected_iterator.item_pos(), result.expected_iterator.edge());
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::find(const Key& key) noexcept {
		return find2(key).actual_iterator;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::find(const Key& key) const noexcept {
		return const_cast<vmem_map<Key, T, Pool, Log>>(this)->find(key);
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline bool vmem_map<Key, T, Pool, Log>::contains(const Key& key) const noexcept {
		return find(key).can_deref();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::pointer vmem_map<Key, T, Pool, Log>::operator [](const Key& key) noexcept {
		return find(key).operator->();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_pointer vmem_map<Key, T, Pool, Log>::operator [](const Key& key) const noexcept {
		return const_cast<vmem_map<Key, T, Pool, Log>>(this)->operator[](key);
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::pointer vmem_map<Key, T, Pool, Log>::at(const_iterator& itr) noexcept {
		vmem_map_value_level<Key, T, Pool, Log> values(_state->values, _pool, _log);

		typename vmem_map_value_level<Key, T, Pool, Log>::iterator values_itr(&values, itr.page_pos(), itr.item_pos(), itr.edge(), _log);
		return values_itr.operator->();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_pointer vmem_map<Key, T, Pool, Log>::at(const_iterator& itr) const noexcept {
		return const_cast<vmem_map<Key, T, Pool, Log>>(this)->at(itr);
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::begin_itr() const noexcept {
		vmem_map_value_level<Key, T, Pool, Log> values(_state->values, _pool, _log);

		return itr_from_values(values.begin());
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::rbegin_itr() const noexcept {
		vmem_map_value_level<Key, T, Pool, Log> values(_state->values, _pool, _log);

		return itr_from_values(values.rbegin());
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::end_itr() const noexcept {
		vmem_map_value_level<Key, T, Pool, Log> values(_state->values, _pool, _log);

		return itr_from_values(values.end());
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::rend_itr() const noexcept {
		vmem_map_value_level<Key, T, Pool, Log> values(_state->values, _pool, _log);

		return itr_from_values(values.rend());
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::itr_from_values(typename vmem_map_value_level<Key, T, Pool, Log>::iterator values_itr) const noexcept {
		iterator itr(this, values_itr.page_pos(), values_itr.item_pos(), values_itr.edge(), _log);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_map::itr_from_values() page_pos=0x%llx, item_pos=0x%x, edge=%u",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge());
		}

		return itr;
	}

}
