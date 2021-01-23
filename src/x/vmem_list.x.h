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
			throw exception<std::logic_error, Log>("state", 0x1034c);
		}

		if (pool == nullptr) {
			throw exception<std::logic_error, Log>("pool", 0x1034d);
		}

		if (sizeof(T) > max_item_size()) {
			throw exception<std::logic_error, Log>("size excess", 0x1034e);
		}

		if (is_uninit(state)) {
			_state->front_page_pos = vmem_page_pos_nil;
			_state->back_page_pos = vmem_page_pos_nil;
			_state->item_size = sizeof(T);
		}

		if (sizeof(T) != _state->item_size) {
			throw exception<std::logic_error, Log>("size mismatch", 0x1034f);
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
			throw exception<std::logic_error, Log>("itr.page", 0x10351);
		}

		if (itr._item_pos == vmem_item_pos_nil && (itr._page_pos != _state->back_page_pos && itr._edge != vmem_iterator_edge::end)) {
			throw exception<std::logic_error, Log>("itr.item", 0x10352);
		}

		bool ok = true;
		vmem_page_pos_t page_pos = vmem_page_pos_nil;
		vmem_item_pos_t item_pos = vmem_item_pos_nil;
		vmem_item_pos_t page_item_count = 0;

		// Copy the item to a local variable to make sure the reference is valid and copyable before we change any state.
		T item_copy(item);

		// IMPORTANT: There must be no exceptions from here to the end of the method!

		if (itr._page_pos == vmem_page_pos_nil) {
			// Empty list.

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10353, "vmem_list::insert() Empty");
			}

			vmem_page<Pool, Log> page(_pool, _log);
			
			if (page.ptr() == nullptr) {
				ok = false;
				_log->put_any(category::abc::vmem, severity::warning, 0x10354, "vmem_list::insert() Could not create page");
			}
			
			if (ok) {
				_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());

				// Insert page - only one.
				list_page->next_page_pos = vmem_page_pos_nil;
				list_page->prev_page_pos = vmem_page_pos_nil;
				list_page->item_count = 0;

				_state->front_page_pos = page.pos();
				_state->back_page_pos = page.pos();

				// Insert item - first/only one.
				page_pos = page.pos();
				item_pos = 0;
				page_item_count = ++list_page->item_count;
				std::memmove(&list_page->items[item_pos], &item_copy, sizeof(T));

				if (_log != nullptr) {
					_log->put_binary(category::abc::vmem, severity::abc::debug, 0x10355, &list_page->items[item_pos], std::min(sizeof(T), (std::size_t)16));
				}
			}
		}
		else {
			// Non-empty list.

			vmem_page<Pool, Log> page(_pool, itr._page_pos, _log);

			if (page.ptr() == nullptr) {
				ok = false;
				_log->put_any(category::abc::vmem, severity::warning, 0x10356, "vmem_list::insert() Could not load page pos=0x%llx", (long long)itr._page_pos);
			}

			if (ok) {
				_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, 0x10357, "vmem_list::insert() item_count=%u, page_capacity=%zu", list_page->item_count, (std::size_t)page_capacity());
				}

				if (list_page->item_count == page_capacity()) {
					// The page has no capacity.

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, 0x10358, "vmem_list::insert() No capacity");
					}

					vmem_page<Pool, Log> new_page(_pool, _log);

					if (new_page.ptr() == nullptr) {
						ok = false;
						_log->put_any(category::abc::vmem, severity::warning, 0x10359, "vmem_list::insert() Could not create page");
					}

					if (ok) {
						_vmem_list_page<T>* new_list_page = reinterpret_cast<_vmem_list_page<T>*>(new_page.ptr());

						// Insert page - after.
						new_list_page->next_page_pos = list_page->next_page_pos;
						new_list_page->prev_page_pos = itr._page_pos;
						new_list_page->item_count = 0;

						if (list_page->next_page_pos != vmem_page_pos_nil) {
							vmem_page<Pool, Log> next_page(_pool, list_page->next_page_pos, _log);

							if (next_page.ptr() == nullptr) {
								ok = false;
								_log->put_any(category::abc::vmem, severity::warning, 0x1035a, "vmem_list::insert() Could not load page pos=0x%llx", (long long)list_page->next_page_pos);
							}

							if (ok) {
								_vmem_list_page<T>* next_list_page = reinterpret_cast<_vmem_list_page<T>*>(next_page.ptr());

								next_list_page->prev_page_pos = new_page.pos();
							}
						}

						if (ok) {
							list_page->next_page_pos = new_page.pos();

							if (_state->back_page_pos == page.pos()) {
								_state->back_page_pos = new_page.pos();
							}

							if (new_list_page->next_page_pos != vmem_page_pos_nil || itr._item_pos != vmem_item_pos_nil) {
								// Split the items evenly among the 2 pages unless we are inserting at the end.
								// This exception fills up pages fully when items keep being added at the end.
								if (_log != nullptr) {
									_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::insert() No capacity. Balancing. page_pos=0x%llx, new_page_pos=0x%llx",
										(long long)page.pos(), (long long)new_page.pos());
								}

								constexpr std::size_t new_page_item_count = page_capacity() / 2;
								page_item_count = page_capacity() - new_page_item_count;
								std::memmove(&new_list_page->items[0], &list_page->items[page_item_count], new_page_item_count * sizeof(T));
								new_list_page->item_count = new_page_item_count;
								list_page->item_count = page_item_count;
							}

							if (itr._item_pos == vmem_item_pos_nil) {
								// Inserting to the end of a full page.
								// Insert at the end of the new page.

								if (_log != nullptr) {
									_log->put_any(category::abc::vmem, severity::abc::debug, 0x1035d, "vmem_list::insert() No capacity. End. page_pos=0x%llx, item_pos=0x%llx",
										(long long)new_page.pos(), (long long)new_list_page->item_count);
								}

								// Insert item.
								page_pos = new_page.pos();
								item_pos = new_list_page->item_count;
								page_item_count = ++new_list_page->item_count;
								std::memmove(&new_list_page->items[item_pos], &item_copy, sizeof(T));

								if (_log != nullptr) {
									_log->put_binary(category::abc::vmem, severity::abc::debug, 0x1035e, &new_list_page->items[item_pos], std::min(sizeof(T), (std::size_t)16));
								}
							}
							else if (itr._item_pos <= list_page->item_count) {
								// Inserting to the first half of a full page.

								if (_log != nullptr) {
									_log->put_any(category::abc::vmem, severity::abc::debug, 0x1035b, "vmem_list::insert() No capacity. First half. page_pos=0x%llx, item_pos=0x%llx",
										(long long)page.pos(), (long long)itr._item_pos);
								}

								// Shift the items from the insertion position to free up a slot if necessary.
								if (itr._item_pos < list_page->item_count) {
									std::size_t move_item_count = list_page->item_count - itr._item_pos;
									std::memmove(&list_page->items[itr._item_pos + 1], &list_page->items[itr._item_pos], move_item_count * sizeof(T));
								}

								// Insert item.
								page_pos = page.pos();
								item_pos = itr._item_pos;
								page_item_count = ++list_page->item_count;
								std::memmove(&list_page->items[item_pos], &item_copy, sizeof(T));

								if (_log != nullptr) {
									_log->put_binary(category::abc::vmem, severity::abc::debug, 0x1035c, &list_page->items[item_pos], std::min(sizeof(T), (std::size_t)16));
								}
							}
							else {
								// Inserting to the second half of a full page.

								item_pos = itr._item_pos - list_page->item_count;

								if (_log != nullptr) {
									_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::insert() No capacity. Second half. page_pos=0x%llx, item_pos=0x%llx",
										(long long)new_page.pos(), (long long)item_pos);
								}

								// Shift the items from the insertion position to free up a slot if necessary.
								if (item_pos < new_list_page->item_count) {
									std::size_t move_item_count = new_list_page->item_count - item_pos;
									std::memmove(&new_list_page->items[item_pos + 1], &new_list_page->items[item_pos], move_item_count * sizeof(T));
								}

								// Insert item.
								page_pos = new_page.pos();
								// item_pos is already set
								page_item_count = ++new_list_page->item_count;
								std::memmove(&new_list_page->items[item_pos], &item_copy, sizeof(T));

								if (_log != nullptr) {
									_log->put_binary(category::abc::vmem, severity::abc::debug, __TAG__, &new_list_page->items[item_pos], std::min(sizeof(T), (std::size_t)16));
								}
							}
						}
					}
				}
				else {
					// The page has capacity.

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, 0x1035f, "vmem_list::insert() Capacity");
					}

					if (itr._item_pos != vmem_item_pos_nil) {
						// Inserting to the middle of a page with capacity.

						if (_log != nullptr) {
							_log->put_any(category::abc::vmem, severity::abc::debug, 0x10360, "vmem_list::insert() Capacity. Middle.");
						}

						// Shift the items from the insertion position to free up a slot.
						std::size_t move_item_count = list_page->item_count - itr._item_pos;
						std::memmove(&list_page->items[itr._item_pos + 1], &list_page->items[itr._item_pos], move_item_count * sizeof(T));

						// Insert item - middle.
						page_pos = itr._page_pos;
						item_pos = itr._item_pos;
						page_item_count = ++list_page->item_count;
						std::memmove(&list_page->items[item_pos], &item_copy, sizeof(T));

						if (_log != nullptr) {
							_log->put_binary(category::abc::vmem, severity::abc::debug, 0x10361, &list_page->items[item_pos], std::min(sizeof(T), (std::size_t)16));
						}
					}
					else {
						// Inserting to the end of a page with capacity.

						if (_log != nullptr) {
							_log->put_any(category::abc::vmem, severity::abc::debug, 0x10362, "vmem_list::insert() Capacity. End.");
						}

						// Insert item - last one.
						page_pos = itr._page_pos;
						item_pos = list_page->item_count;
						page_item_count = ++list_page->item_count;
						std::memmove(&list_page->items[item_pos], &item_copy, sizeof(T));

						if (_log != nullptr) {
							_log->put_binary(category::abc::vmem, severity::abc::debug, 0x10363, &list_page->items[item_pos], std::min(sizeof(T), (std::size_t)16));
						}
					}
				}
			}
		}

		vmem_iterator_edge_t edge = vmem_iterator_edge::none;

		if (ok) {
			// We have inserted successfully.

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
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10364, "vmem_list::insert() Done. page_pos=0x%llx, item_pos=0x%x, edge=%u, page_item_count=%u, total_item_count=%zu",
				(long long)page_pos, item_pos, edge, (unsigned)page_item_count, (std::size_t)_state->total_item_count);
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
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::erase(const_iterator itr) {
		if (!itr.can_deref()) {
			throw exception<std::logic_error, Log>("itr", 0x10366);
		}

		bool ok = true;
		vmem_page_pos_t page_pos = itr._page_pos;
		vmem_item_pos_t item_pos = itr._item_pos;
		vmem_iterator_edge_t edge = vmem_iterator_edge::none;

		// IMPORTANT: There must be no exceptions from here to the end of the method!

		vmem_page<Pool, Log> page(_pool, itr._page_pos, _log);

		if (page.ptr() == nullptr) {
			ok = false;
			_log->put_any(category::abc::vmem, severity::warning, 0x10367, "vmem_list::erase() Could not load page pos=0x%llx", (long long)itr._page_pos);
		}
		else {
			_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, 0x10368, "vmem_list::erase() Start. page_pos=0x%llx, item_pos=0x%x, page_item_count=%u, total_item_count=%zu",
					(long long)page_pos, item_pos, list_page->item_count, (std::size_t)_state->total_item_count);
			}

			if (list_page->item_count > 1) {
				// The page has multiple items.

				// To delete an item, bring up the remaining elements, if there are any.
				if (itr._item_pos < list_page->item_count - 1) {
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, 0x10369, "vmem_list::erase() Multiple. Middle.");
					}

					std::size_t move_item_count = list_page->item_count - itr._item_pos - 1;
					std::memmove(&list_page->items[itr._item_pos], &list_page->items[itr._item_pos + 1], move_item_count * sizeof(T));
				}
				else {
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, 0x1036a, "vmem_list::erase() Multiple. Last.");
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

				list_page->item_count--;
			}
			else {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, 0x1036b, "vmem_list::erase() Only.");
				}

				// The page has no other items.

				// We can free the current page now.
				vmem_page_pos_t prev_page_pos = list_page->prev_page_pos;
				vmem_page_pos_t next_page_pos = list_page->next_page_pos;
				list_page = nullptr;
				page.free();

				// Connect the two adjaceent pages, and free this page.
				// The next item is item 0 on the next page or end().
				if (prev_page_pos != vmem_page_pos_nil) {
					vmem_page<Pool, Log> prev_page(_pool, prev_page_pos, _log);

					if (prev_page.ptr() == nullptr) {
						ok = false;
						_log->put_any(category::abc::vmem, severity::warning, 0x1036c, "vmem_list::erase() Could not load prev page pos=0x%llx", (long long)prev_page_pos);
					}
					else {
						_vmem_list_page<T>* prev_list_page = reinterpret_cast<_vmem_list_page<T>*>(prev_page.ptr());

						prev_list_page->next_page_pos = next_page_pos;
					}
				}
				else {
					_state->front_page_pos = next_page_pos;
				}

				if (ok) {
					if (next_page_pos != vmem_page_pos_nil) {
						vmem_page<Pool, Log> next_page(_pool, next_page_pos, _log);

						if (next_page.ptr() == nullptr) {
							ok = false;
							_log->put_any(category::abc::vmem, severity::warning, 0x1036d, "vmem_list::erase() Could not load next page pos=0x%llx", (long long)next_page_pos);
						}
						else {
							_vmem_list_page<T>* next_list_page = reinterpret_cast<_vmem_list_page<T>*>(next_page.ptr());

							next_list_page->prev_page_pos = prev_page_pos;

							page_pos = next_page_pos;
							item_pos = 0;
						}
					}
					else {
						_state->back_page_pos = prev_page_pos;

						end_pos(page_pos, item_pos);
						edge = vmem_iterator_edge::end;
					}
				}
			}
		}

		if (ok) {
			// Update the total item count.
			_state->total_item_count--;
		}
		else {
			end_pos(page_pos, item_pos);
			edge = vmem_iterator_edge::end;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1036e, "vmem_list::erase() Done. page_pos=0x%llx, item_pos=0x%x, edge=%u, total_item_count=%zu",
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
	inline void vmem_list<T, Pool, Log>::clear() noexcept {
		erase(begin(), end());
	}


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
				_log->put_any(category::abc::vmem, severity::warning, 0x10371, "vmem_list::move_next() Could not load page pos=0x%llx", (long long)itr._page_pos);

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
				_log->put_any(category::abc::vmem, severity::warning, 0x10374, "vmem_list::move_prev() Could not load page pos=0x%llx", (long long)itr._page_pos);

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
							_log->put_any(category::abc::vmem, severity::warning, 0x10375, "vmem_list::move_prev() Could not load page pos=0x%llx", (long long)list_page->prev_page_pos);

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
				_log->put_any(category::abc::vmem, severity::warning, 0x1037a, "vmem_list::rend_pos() Could not load page pos=0x%llx", (long long)_state->back_page_pos);

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
