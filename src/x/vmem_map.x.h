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


	template <typename Key, typename Pool, typename Log>
	vmem_map_key_level<Key, Pool, Log>::vmem_map_key_level(vmem_container_state* state, Pool* pool, Log* log)
		: base(state, balance_insert, balance_erase, pool, log) {
	}


	template <typename Key, typename Pool, typename Log>
	vmem_map_key_level_stack<Key, Pool, Log>::vmem_map_key_level_stack(vmem_stack_state* state, Pool* pool, Log* log)
		: base(state, pool, log) {
	}


	template <typename Key, typename T, typename Pool, typename Log>
	vmem_map_value_level<Key, T, Pool, Log>::vmem_map_value_level(vmem_container_state* state, Pool* pool, Log* log)
		: base(state, balance_insert, balance_erase, pool, log) {
	}


	// --------------------------------------------------------------


	template <typename Key, typename T, typename Pool, typename Log>
	vmem_map_result2<Key, T, Pool, Log>::vmem_map_result2(nullptr_t) noexcept
		: iterator(nullptr)
		, ok(false) {
	}


	template <typename Key, typename T, typename Pool, typename Log>
	vmem_map_find_result2<Key, T, Pool, Log>::vmem_map_find_result2(nullptr_t) noexcept
		: vmem_map_result2<Key, T, Pool, Log>(nullptr) {
	}


	template <typename Key, typename T, typename Pool, typename Log>
	vmem_map_find_result2<Key, T, Pool, Log>::~vmem_map_find_result2() noexcept {
		//// TODO:
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

		if (key_page_capacity() < 2) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(key page capacity) insufficient", __TAG__);
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


	// ..............................................................


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::insert2(const_reference item) {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_map::insert2(item) Start.");
		}

		find_result2 find_result = find2(item.key);
		result2 result(nullptr);

		if (!find_result.ok) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_map::insert2(item) Not found.");
			}

			result = insert2(std::move(find_result), item);
		}
		else {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_map::insert2(item) Found.");
			}

			result.iterator = find_result.iterator;
			result.ok = false;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_map::insert2() Done. result.ok=%d, result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
				result.ok, result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge());
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator_bool vmem_map<Key, T, Pool, Log>::insert(const_reference item) {
		result2 result = insert2(item);

		return std::make_pair(result.iterator, result.ok);
	}


	template <typename Key, typename T, typename Pool, typename Log>
	template <typename InputItr>
	inline void vmem_map<Key, T, Pool, Log>::insert(InputItr first, InputItr last) {
		for (InputItr item_itr = first; item_itr != last; item_itr++) {
			if (!insert(*item_itr).second) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::important, __TAG__, "vmem_map::insert() Breaking from the loop.");
				}
				break;
			}
		}
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::insert2(find_result2&& find_result, const_reference item) {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_map::insert2() Start.");
		}

		// Insert into values.
		vmem_map_value_level<Key, T, Pool, Log> values(&_state->values, _pool, _log);
		typename vmem_map_value_level<Key, T, Pool, Log>::iterator values_itr(&values, find_result.iterator.page_pos(), find_result.iterator.item_pos(), find_result.iterator.edge(), _log);

		typename vmem_map_value_level<Key, T, Pool, Log>::result2 values_result = values.insert2(values_itr, item);

		result2 result(nullptr);
		result.iterator = find_result.iterator;
		result.ok = values_result.iterator.is_valid();

		if (values_result.iterator.is_valid() && values_result.page_pos != vmem_page_pos_nil) {
			vmem_map_key_level_stack<Key, Pool, Log> key_stack(&_state->keys, _pool, _log);
			vmem_stack<vmem_page_pos_t, Pool, Log> path(&find_result.path_state, _pool, _log);

			if (key_stack.size() != path.size()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_map::insert2() Mismatch key_stack.size=%zu, path.size=%zu",key_stack.size(), path.size());
				}

				result.iterator = iterator(nullptr);
				result.ok = false;
			}
			else {
				typename vmem_map_key_level_stack<Key, Pool, Log>::iterator key_stack_itr = key_stack.begin();
				typename vmem_stack<vmem_page_pos_t, Pool, Log>::reverse_iterator path_itr = path.rend();
				vmem_page_pos_t new_page_pos = values_result.page_pos;
				Key new_key = values_result.item_0.key;
				vmem_page_pos_t other_page_pos = values_result.other_page_pos;
				Key other_key = values_result.other_item_0.key;

				// While there is overflow, keep going back the path (and up the levels).
				while (new_page_pos != vmem_page_pos_nil && key_stack_itr != key_stack.end() && path_itr != path.rbegin()) {
					vmem_page_pos_t parent_page_pos = *path_itr;
					vmem_page<Pool, Log> parent_page(_pool, parent_page_pos, _log);

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_map::insert2() parent_page_pos=0x%llx", (long long)parent_page_pos);
					}

					if (parent_page.ptr() == nullptr) {
						if (_log != nullptr) {
							_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_map::insert2() Could not load key page pos=0x%llx", (long long)parent_page_pos);
						}

						result.iterator = iterator(nullptr);
						result.ok = false;
						break;
					}

					vmem_map_key_page<Key>* parent_key_page = reinterpret_cast<vmem_map_key_page<Key>*>(parent_page.ptr());

					// Find the item_pos for the new key.
					vmem_item_pos_t parent_item_pos = 0;
					for (std::size_t i = 1; i < parent_key_page->item_count && new_key < parent_key_page->items[i].key; i++) {
						parent_item_pos++;
					}

					vmem_map_key_level<Key, Pool, Log> parent_keys(key_stack_itr.operator->(), _pool, _log);
					typename vmem_map_key_level<Key, Pool, Log>::iterator parent_keys_itr(&parent_keys, parent_page_pos, parent_item_pos, vmem_iterator_edge::none, _log);
					vmem_map_key<Key> key_item;
					key_item.key = new_key;
					key_item.page_pos = new_page_pos;

					typename vmem_map_key_level<Key, Pool, Log>::result2 keys_result = parent_keys.insert2(parent_keys_itr, key_item);

					if (!keys_result.iterator.is_valid()) {
						if (_log != nullptr) {
							_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_map::insert2() Could not insert key to page pos=0x%llx", (long long)parent_page_pos);
						}

						result.iterator = iterator(nullptr);
						result.ok = false;
						break;
					}

					if (new_page_pos != vmem_page_pos_nil) {
						new_page_pos = keys_result.page_pos;
						new_key = keys_result.item_0.key;
						other_page_pos = keys_result.other_page_pos;
						other_key = keys_result.other_item_0.key;
					}
				} // while (new_page_pos != nil)

				// If we still have a new_page_pos, then we have to add a key level at the top.
				if (result.ok && new_page_pos != vmem_page_pos_nil) {
					vmem_container_state new_keys_state;
					vmem_map_key_level<Key, Pool, Log> new_keys(&new_keys_state, _pool, _log);

					vmem_map_key<Key> other_key_item;
					other_key_item.key = other_key;
					other_key_item.page_pos = other_page_pos;
					new_keys.push_back(other_key_item);

					vmem_map_key<Key> new_key_item;
					new_key_item.key = new_key;
					new_key_item.page_pos = new_page_pos;
					new_keys.push_back(new_key_item);

					key_stack.push_back(new_keys_state);

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_map::insert2() key_stack.size=%zu", key_stack.size());
					}
				}
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_map::insert2() Done. ok=&d", result.ok);
		}

		return result;
	}


	// ..............................................................


#ifdef REMOVE ////
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
		vmem_item_pos_t item_pos = vmem_item_pos_nil;
		bool found = false;

		find_result2 result(nullptr);
		vmem_stack<vmem_page_pos_t, Pool, Log> path(&result.path_state, _pool, _log);

		vmem_map_key_level_stack<Key, Pool, Log> key_stack(&_state->keys, _pool, _log);

		if (!key_stack.empty()) {
			// There are key levels.
			page_pos = key_stack.back().front_page_pos;
			path.push_back(page_pos);

			for (std::size_t lev = 0; page_pos != vmem_page_pos_nil && lev < key_stack.size(); lev++) {
				vmem_page<Pool, Log> page(_pool, page_pos, _log);

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

					if (lev != key_stack.size() - 1) {
						path.push_back(page_pos);
					}
				}
			}
		}
		else {
			// There are no key levels. There must be at most 1 value page.
			page_pos = _state->values.front_page_pos;
		}

		// page_pos must be the pos of a value page.
		if (page_pos != vmem_page_pos_nil) {
			vmem_page<Pool, Log> page(_pool, page_pos, _log);

			if (page.ptr() == nullptr) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_map::find2() Could not load value page pos=0x%llx", (long long)page_pos);
				}

				page_pos = vmem_page_pos_nil;
			}
			else {
				vmem_map_value_page<Key, T>* value_page = reinterpret_cast<vmem_map_value_page<Key, T>*>(page.ptr());

				item_pos = 0;
				for (std::size_t i = 1; i < value_page->item_count && key <= value_page->items[i].key; i++) {
					item_pos++;

					if (key == value_page->items[i].key) {
						found = true;
						break;
					}
				}
			}
		}

		result.ok = found;

		if (page_pos != vmem_item_pos_nil && item_pos != vmem_item_pos_nil) {
			result.iterator = iterator(this, page_pos, item_pos, vmem_iterator_edge::none, _log);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_map::find2() Done. result.ok=%d, result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
				result.ok, result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge());
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::find(const Key& key) noexcept {
		result2 result = find2(key);
		return result.ok ? result.iterator : end_itr();
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
