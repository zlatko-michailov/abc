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
	vmem_map_find_result2<Key, T, Pool, Log>::vmem_map_find_result2(Pool* pool, Log* log) noexcept
		: vmem_map_result2<Key, T, Pool, Log>(nullptr)
		, path(&_path_state, pool, log) {
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
	inline vmem_map<Key, T, Pool, Log>::vmem_map(vmem_map_state* state, Pool* pool, Log* log)
		: _state(state)
		, _pool(pool)
		, _log(log)
		, _key_stack(&state->keys, pool, log)
		, _values(&state->values, pool, log) {

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1050a, "vmem_map::vmem_map() state=%p, pool=%p", state, pool);
		}

		if (state == nullptr) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(state)", 0x1050b);
		}

		if (pool == nullptr) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(pool)", 0x1050c);
		}

		if (sizeof(vmem_map_key<Key>) > max_key_item_size()) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(key size) excess", 0x1050d);
		}

		if (sizeof(vmem_map_value<Key, T>) > max_value_item_size()) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(value size) excess", 0x1050e);
		}

		if (key_page_capacity() < 2) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(key page capacity) insufficient", 0x1050f);
		}

		if (Pool::max_mapped_pages() < vmem_min_mapped_pages) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(pool<MaxMappedPages>)", 0x10510);
		}

		if (sizeof(vmem_container_state) != _state->keys.item_size) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(key size) mismatch", 0x10511);
		}

		if (sizeof(vmem_map_value<Key, T>) != _state->values.item_size) {
			throw exception<std::logic_error, Log>("vmem_map::vmem_map(value size) mismatch", 0x10512);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10513, "vmem_map::vmem_map() keys.front_page_pos=0x%llx, keys.back_page_pos=0x%llx,  values.front_page_pos=0x%llx, values.back_page_pos=0x%llx", 
				(long long)_state->keys.front_page_pos, (long long)_state->keys.back_page_pos, (long long)_state->values.front_page_pos, (long long)_state->values.back_page_pos);
		}
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::begin() noexcept {
		return begin_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::begin() const noexcept {
		return begin_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::cbegin() const noexcept {
		return begin_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::end() noexcept {
		return end_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::end() const noexcept {
		return end_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::cend() const noexcept {
		return end_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::rend() noexcept {
		return rend_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::rend() const noexcept {
		return rend_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::crend() const noexcept {
		return rend_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::rbegin() noexcept {
		return rbegin_itr();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_iterator vmem_map<Key, T, Pool, Log>::rbegin() const noexcept {
		return rbegin_itr();
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
			_log->put_any(category::abc::vmem, severity::abc::important, 0x10514, "vmem_map::insert2(item) Start.");
		}

		find_result2 find_result = find2(item.key);
		result2 result(nullptr);

		if (!find_result.ok) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::important, 0x10515, "vmem_map::insert2(item) Not found.");
			}

			result = insert2(std::move(find_result), item);
		}
		else {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::important, 0x10516, "vmem_map::insert2(item) Found.");
			}

			result.iterator = find_result.iterator;
			result.ok = false;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x10517, "vmem_map::insert2() Done. result.ok=%d, result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
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
					_log->put_any(category::abc::vmem, severity::important, 0x10518, "vmem_map::insert() Breaking from the loop.");
				}
				break;
			}
		}
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::insert2(find_result2&& find_result, const_reference item) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x10519, "vmem_map::insert2() Start.");
		}

		value_level_iterator values_itr(&_values, find_result.iterator.page_pos(), find_result.iterator.item_pos(), find_result.iterator.edge(), _log);

		value_level_result2 values_result = _values.insert2(values_itr, item);

		result2 result(nullptr);
		if (values_result.iterator.is_valid()) {
			result = update_key_levels(true, std::move(find_result), std::move(values_result));
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1051a, "vmem_map::insert2() Done. ok=%d, iterator.valid=%d, iterator.page_pos=0x%llx, iterator.item_pos=0x%x, iterator.edge=%d",
				result.ok, result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge());
		}

		return result;
	}


	// ..............................................................


	template <typename Key, typename T, typename Pool, typename Log>
	inline std::size_t vmem_map<Key, T, Pool, Log>::erase(const Key& key) {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1051b, "vmem_map::erase(key) Start.");
		}

		std::size_t result = 0;

		find_result2 find_result = find2(key);

		if (find_result.ok) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x1051c, "vmem_map::erase(key) Found. itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%d",
					(long long)find_result.iterator.page_pos(), find_result.iterator.item_pos(), find_result.iterator.edge());
			}

			result = erase2(std::move(find_result));
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1051d, "vmem_map::erase(key) Done. result=%zu", result);
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	template <typename InputItr>
	inline void vmem_map<Key, T, Pool, Log>::erase(InputItr first, InputItr last) {
		for (InputItr item_itr = first; item_itr != last; item_itr++) {
			erase(*item_itr);
		}
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline std::size_t vmem_map<Key, T, Pool, Log>::erase2(find_result2&& find_result) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1051e, "vmem_map::erase(find_result2) Start.");
		}

		value_level_iterator values_itr(&_values, find_result.iterator.page_pos(), find_result.iterator.item_pos(), find_result.iterator.edge(), _log);

		value_level_result2 values_result = _values.erase2(values_itr);

		result2 result(nullptr);
		if (values_result.iterator.is_valid()) {
			result = update_key_levels(false, std::move(find_result), std::move(values_result));
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1051f, "vmem_map::erase(find_result2) Done. iterator.page_pos=0x%llx, iterator.item_pos=0x%x, iterator.edge=%d",
				(long long)values_result.iterator.page_pos(), values_result.iterator.item_pos(), values_result.iterator.edge());
		}

		return values_result.iterator.is_valid() ? 1 : 0;
	}


	// ..............................................................


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::result2 vmem_map<Key, T, Pool, Log>::update_key_levels(bool is_insert, find_result2&& find_result, value_level_result2&& values_result) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10520, "vmem_map::update_key_levels() Start.");
		}

		bool ok = true;

		if (values_result.iterator.is_valid()
			&& (values_result.page_leads[0].operation != vmem_container_page_lead_operation::none || values_result.page_leads[1].operation != vmem_container_page_lead_operation::none)) {
			if (_key_stack.size() != find_result.path.size()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, 0x10521, "vmem_map::update_key_levels() Mismatch key_stack.size=%zu, path.size=%zu",
						_key_stack.size(), find_result.path.size());
				}

				ok = false;
			}
			else {
				key_level_stack_iterator key_stack_itr = _key_stack.begin();
				path_reverse_iterator path_itr = find_result.path.rend();

				page_lead page_leads[] = { values_result.page_leads[0], values_result.page_leads[1] };

				// While there is rebalance, keep going back the path (and up the levels).
				while ((page_leads[0].operation != vmem_container_page_lead_operation::none || page_leads[1].operation != vmem_container_page_lead_operation::none)
						&& key_stack_itr != _key_stack.end()
						&& path_itr != find_result.path.rbegin()) {
					// IMPORTANT: Save the vmem_ptr instance to keep the page locked.
					vmem_ptr<vmem_container_state, Pool, Log> key_level_state_ptr = key_stack_itr.operator->();

					vmem_map_key_level<Key, Pool, Log> parent_keys(key_level_state_ptr.operator->(), _pool, _log);
					vmem_page_pos_t parent_page_pos = *path_itr;

					key_level_result2 keys_result;
					if (is_insert) {
						// page_leads[0] - insert; new page
						// page_leads[1] - supplemental; used only when a new level is created
						
						vmem_item_pos_t parent_item_pos = key_item_pos(parent_page_pos, page_leads[0].items[0].key);
						if (parent_item_pos == vmem_item_pos_nil) {
							ok = false;
							break;
						}

						key_level_iterator parent_keys_itr(&parent_keys, parent_page_pos, parent_item_pos, vmem_iterator_edge::none, _log);

						vmem_map_key<Key> key_item;
						vmem_copy(key_item.key, page_leads[0].items[0].key);
						key_item.page_pos = page_leads[0].page_pos;

						keys_result = parent_keys.insert2(parent_keys_itr, key_item);
					}
					else {
						// page_leads[0] - replace or none; doesn't create new leads
						// page_leads[1] - erase

						if (page_leads[0].operation == vmem_container_page_lead_operation::replace) {
							vmem_item_pos_t parent_item_pos = key_item_pos(parent_page_pos, page_leads[0].items[0].key);
							if (parent_item_pos == vmem_item_pos_nil) {
								ok = false;
								break;
							}

							key_level_iterator parent_keys_itr(&parent_keys, parent_page_pos, parent_item_pos, vmem_iterator_edge::none, _log);
							if (parent_keys_itr.can_deref()) {
								vmem_ptr<vmem_map_key<Key>, Pool, Log> key_ptr = parent_keys_itr.operator->();
								vmem_copy(key_ptr->key, page_leads[0].items[1].key);
							}
						}

						vmem_item_pos_t parent_item_pos = key_item_pos(parent_page_pos, page_leads[1].items[0].key);
						if (parent_item_pos == vmem_item_pos_nil) {
							ok = false;
							break;
						}

						key_level_iterator parent_keys_itr(&parent_keys, parent_page_pos, parent_item_pos, vmem_iterator_edge::none, _log);

						keys_result = parent_keys.erase2(parent_keys_itr);
					}

					if (!keys_result.iterator.is_valid()) {
						if (_log != nullptr) {
							_log->put_any(category::abc::vmem, severity::warning, 0x10522, "vmem_map::update_key_levels() Could not insert key to page pos=0x%llx", (long long)parent_page_pos);
						}

						ok = false;
						break;
					}

					if (page_leads[0].operation != vmem_container_page_lead_operation::none || page_leads[1].operation != vmem_container_page_lead_operation::none) {
						page_leads[0] = keys_result.page_leads[0];
						page_leads[1] = keys_result.page_leads[1];
					}

					key_stack_itr++;
					path_itr--;
				} // while (new_page_pos != nil)

				// If we still have a new_page_pos, then we have to add/remove a key level at the top.
				if (ok) {
					if (is_insert) {
						if (page_leads[0].page_pos != vmem_page_pos_nil) {
							vmem_container_state new_keys_state;
							vmem_map_key_level<Key, Pool, Log> new_keys(&new_keys_state, _pool, _log);

							// items[0]
							vmem_map_key<Key> other_key_item;
							vmem_copy(other_key_item.key, page_leads[1].items[0].key);
							other_key_item.page_pos = page_leads[1].page_pos;
							new_keys.push_back(other_key_item);

							// items[1]
							vmem_map_key<Key> new_key_item;
							vmem_copy(new_key_item.key, page_leads[0].items[0].key);
							new_key_item.page_pos = page_leads[0].page_pos;
							new_keys.push_back(new_key_item);

							_key_stack.push_back(new_keys_state);
						}
					}
					else {
						if (!_key_stack.empty()) {
							std::size_t top_keys_size = 2;
							{
								vmem_container_state top_keys_state = _key_stack.back();
								vmem_map_key_level<Key, Pool, Log> top_keys(&top_keys_state, _pool, _log);
								top_keys_size = top_keys.size();
							}

							if (top_keys_size == 1) {
								_key_stack.pop_back();
							}
						}
					}

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, 0x10523, "vmem_map::update_key_levels() key_stack.size=%zu", _key_stack.size());
					}
				}
			}
		}

		result2 result(nullptr);

		if (ok) {
			result.iterator = iterator(this, values_result.iterator.page_pos(), values_result.iterator.item_pos(), values_result.iterator.edge(), _log);
			result.ok = true;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10524, "vmem_map::update_key_levels() Done. ok=%d, iterator.valid=%d, iterator.page_pos=0x%llx, iterator.item_pos=0x%x, iterator.edge=%d",
				result.ok, result.iterator.is_valid(), (long long)result.iterator.page_pos(), result.iterator.item_pos(), result.iterator.edge());
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline vmem_item_pos_t vmem_map<Key, T, Pool, Log>::key_item_pos(vmem_page_pos_t key_page_pos, const Key& key) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10525, "vmem_map::key_item_pos() Start. key_page_pos=0x%llx..., key=0x%llx...",
				(long long)key_page_pos, *(long long*)&key);
		}

		vmem_item_pos_t item_pos = vmem_item_pos_nil;
		vmem_page<Pool, Log> page(_pool, key_page_pos, _log);

		if (page.ptr() == nullptr) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x10526, "vmem_map::key_item_pos() Could not load key page pos=0x%llx", (long long)key_page_pos);
			}
		}
		else {
			vmem_map_key_page<Key>* key_page = reinterpret_cast<vmem_map_key_page<Key>*>(page.ptr());

			item_pos = 0;
			for (std::size_t i = 0; i < key_page->item_count && vmem_is_less(key_page->items[i].key, key); i++) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, 0x10527, "vmem_map::key_item_pos() item[%zu]=0x%llx..., key=0x%llx...",
						i, *(long long*)&key_page->items[i].key, *(long long*)&key);
				}

				item_pos++;
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10528, "vmem_map::key_item_pos() Done. item_pos=0x%x", item_pos);
		}

		return item_pos;
	}


	// ..............................................................


	template <typename Key, typename T, typename Pool, typename Log>
	inline void vmem_map<Key, T, Pool, Log>::clear() noexcept {
		for (key_level_stack_iterator key_stack_itr = _key_stack.rend(); key_stack_itr != _key_stack.rbegin(); key_stack_itr--) {
			// IMPORTANT: Save the vmem_ptr instance to keep the page locked.
			vmem_ptr<vmem_container_state, Pool, Log> key_level_state_ptr = key_stack_itr.operator->();

			vmem_map_key_level<Key, Pool, Log> keys(key_level_state_ptr.operator->(), _pool, _log);
			keys.clear();
		}

		_key_stack.clear();

		_values.clear();
	}


	// ..............................................................


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::next(const iterator_state& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10529, "vmem_map::next() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge());
		}

		value_level_iterator values_itr(&_values, itr.page_pos(), itr.item_pos(), itr.edge(), _log);

		values_itr++;

		iterator result = iterator(this, values_itr.page_pos(), values_itr.item_pos(), values_itr.edge(), _log);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1052a, "vmem_map::next() Done. result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
				(long long)result.page_pos(), result.item_pos(), result.edge());
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::prev(const iterator_state& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1052b, "vmem_map::prev() Start. itr.page_pos=0x%llx, itr.item_pos=0x%x, itr.edge=%u",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge());
		}

		value_level_iterator values_itr(&_values, itr.page_pos(), itr.item_pos(), itr.edge(), _log);

		values_itr--;

		iterator result = iterator(this, values_itr.page_pos(), values_itr.item_pos(), values_itr.edge(), _log);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1052c, "vmem_map::prev() Done. result.page_pos=0x%llx, result.item_pos=0x%x, result.edge=%u",
				(long long)result.page_pos(), result.item_pos(), result.edge());
		}

		return result;
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::find_result2 vmem_map<Key, T, Pool, Log>::find2(const Key& key) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1052d, "vmem_map::find2() Start.");
		}

		vmem_page_pos_t page_pos = vmem_page_pos_nil;
		vmem_item_pos_t item_pos = vmem_item_pos_nil;
		bool found = false;

		find_result2 result(_pool, _log);

		if (!_key_stack.empty()) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x1052e, "vmem_map::find2() %zu key levels. root page_pos=0x%llx",
					_key_stack.size(), (long long)_key_stack.back().front_page_pos);
			}

			// There are key levels.
			page_pos = _key_stack.back().front_page_pos;
			result.path.push_back(page_pos);

			for (std::size_t lev = 0; page_pos != vmem_page_pos_nil && lev < _key_stack.size(); lev++) {
				vmem_page<Pool, Log> page(_pool, page_pos, _log);

				if (page.ptr() == nullptr) {
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::warning, 0x1052f, "vmem_map::find2() Could not load key page pos=0x%llx", (long long)page_pos);
					}

					page_pos = vmem_page_pos_nil;
					break;
				}
				else {
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, 0x10530, "vmem_map::find2() Examine key lev=%zu, page_pos=0x%llx", lev, (long long)page.pos());
					}

					vmem_map_key_page<Key>* key_page = reinterpret_cast<vmem_map_key_page<Key>*>(page.ptr());

					page_pos = key_page->items[0].page_pos;
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, 0x10531, "vmem_map::find2() Item i=0 page_pos=0x%llx", (long long)page_pos);
					}

					for (std::size_t i = 1; i < key_page->item_count && vmem_is_less_or_equal(key_page->items[i].key, key); i++) {
						page_pos = key_page->items[i].page_pos;
						if (_log != nullptr) {
							_log->put_any(category::abc::vmem, severity::abc::debug, 0x10532, "vmem_map::find2() Item i=%zu page_pos=0x%llx", i, (long long)page_pos);
						}
					}

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, 0x10533, "vmem_map::find2() Child page_pos=0x%llx", (long long)page_pos);
					}

					if (lev != _key_stack.size() - 1) {
						result.path.push_back(page_pos);
					}
				}
			}
		}
		else {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10534, "vmem_map::find2() No key levels. value page_pos=0x%llx",
					(long long)_state->values.front_page_pos);
			}

			// There are no key levels. There must be at most 1 value page.
			page_pos = _state->values.front_page_pos;
		}

		// page_pos must be the pos of a value page.
		if (page_pos != vmem_page_pos_nil) {
			vmem_page<Pool, Log> page(_pool, page_pos, _log);

			if (page.ptr() == nullptr) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, 0x10535, "vmem_map::find2() Could not load value page pos=0x%llx", (long long)page_pos);
				}

				page_pos = vmem_page_pos_nil;
			}
			else {
				vmem_map_value_page<Key, T>* value_page = reinterpret_cast<vmem_map_value_page<Key, T>*>(page.ptr());

				item_pos = 0;
				for (std::size_t i = 0; i < value_page->item_count && vmem_is_less(value_page->items[i].key, key); i++) {
					item_pos++;
				}

				found = vmem_are_equal(value_page->items[item_pos].key, key);
			}
		}

		result.ok = found;

		if (page_pos != vmem_item_pos_nil && item_pos != vmem_item_pos_nil) {
			result.iterator = iterator(this, page_pos, item_pos, vmem_iterator_edge::none, _log);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x10536, "vmem_map::find2() Done. result.ok=%d, result.iterator.valid=%d, result.iterator.page_pos=0x%llx, result.iterator.item_pos=0x%x, result.iterator.edge=%u",
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
		return const_cast<vmem_map<Key, T, Pool, Log>*>(this)->operator[](key);
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::pointer vmem_map<Key, T, Pool, Log>::at(const iterator_state& itr) noexcept {
		value_level_iterator values_itr(&_values, itr.page_pos(), itr.item_pos(), itr.edge(), _log);
		return values_itr.operator->();
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::const_pointer vmem_map<Key, T, Pool, Log>::at(const iterator_state& itr) const noexcept {
		return const_cast<vmem_map<Key, T, Pool, Log>*>(this)->at(itr);
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::begin_itr() const noexcept {
		return itr_from_values(_values.begin());
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::end_itr() const noexcept {
		return itr_from_values(_values.end());
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::reverse_iterator vmem_map<Key, T, Pool, Log>::rend_itr() const noexcept {
		return itr_from_values(_values.rend());
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::reverse_iterator vmem_map<Key, T, Pool, Log>::rbegin_itr() const noexcept {
		return itr_from_values(_values.rbegin());
	}


	template <typename Key, typename T, typename Pool, typename Log>
	inline typename vmem_map<Key, T, Pool, Log>::iterator vmem_map<Key, T, Pool, Log>::itr_from_values(value_level_iterator values_itr) const noexcept {
		iterator itr(this, values_itr.page_pos(), values_itr.item_pos(), values_itr.edge(), _log);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10537, "vmem_map::itr_from_values() page_pos=0x%llx, item_pos=0x%x, edge=%u",
				(long long)itr.page_pos(), itr.item_pos(), itr.edge());
		}

		return itr;
	}

}
