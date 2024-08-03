/*
MIT License

Copyright (c) 2018-2024 Zlatko Michailov 

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

#include "../util.h"
#include "../diag/diag_ready.h"
#include "ptr.h"
#include "i/linked.i.h"


namespace abc { namespace vmem {

	inline constexpr bool linked::is_uninit(const linked_state* state) noexcept {
		return
			// nil
			(
				state != nullptr
				&& state->front_page_pos == page_pos_nil
				&& state->back_page_pos == page_pos_nil
			)
			||
			// zero
			(
				state != nullptr
				&& state->front_page_pos == 0
				&& state->back_page_pos == 0
			);
	}


	inline linked::linked(linked_state* state, pool* pool, diag::log_ostream* log)
        : diag_base(abc::copy(_origin), log)
		, _state(state)
		, _pool(pool) {

        constexpr const char* suborigin = "linked()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1048a, "Begin: state=%p, pool=%p", state, pool);

        diag_base::expect(suborigin, state != nullptr, 0x1048b, "state != nullptr");
        diag_base::expect(suborigin, pool != nullptr, 0x1048c, "pool != nullptr");

		if (is_uninit(state)) {
			_state->front_page_pos = page_pos_nil;
			_state->back_page_pos = page_pos_nil;
		}

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: front_page_pos=0x%llx, back_page_pos=0x%llx", 
				(unsigned long long)_state->front_page_pos, (unsigned long long)_state->back_page_pos);
	}


	inline typename linked::iterator linked::begin() noexcept {
		return begin_itr();
	}


	inline typename linked::const_iterator linked::begin() const noexcept {
		return begin_itr();
	}


	inline typename linked::const_iterator linked::cbegin() const noexcept {
		return begin_itr();
	}


	inline typename linked::iterator linked::end() noexcept {
		return end_itr();
	}


	inline typename linked::const_iterator linked::end() const noexcept {
		return end_itr();
	}


	inline typename linked::const_iterator linked::cend() const noexcept {
		return end_itr();
	}


	inline typename linked::iterator linked::rend() noexcept {
		return rend_itr();
	}


	inline typename linked::const_iterator linked::rend() const noexcept {
		return rend_itr();
	}


	inline typename linked::const_iterator linked::crend() const noexcept {
		return rend_itr();
	}


	inline typename linked::iterator linked::rbegin() noexcept {
		return rbegin_itr();
	}


	inline typename linked::const_iterator linked::rbegin() const noexcept {
		return rbegin_itr();
	}


	inline typename linked::const_iterator linked::crbegin() const noexcept {
		return rbegin_itr();
	}


	inline bool linked::empty() const noexcept {
		return _state->front_page_pos == page_pos_nil
			|| _state->back_page_pos == page_pos_nil;
	}


	inline typename linked::reference linked::front() {
		return *at(begin());
	}


	inline typename linked::const_reference linked::front() const {
		return *at(begin());
	}


	inline typename linked::reference linked::back() {
		return *at(rend());
	}


	inline typename linked::const_reference linked::back() const {
		return *at(rend());
	}


	inline void linked::push_back(const_reference page_pos) {
		insert(end(), page_pos);
	}


	inline void linked::pop_back() {
		erase(rend());
	}


	inline void linked::push_front(const_reference page_pos) {
		insert(begin(), page_pos);
	}


	inline void linked::pop_front() {
		erase(begin());
	}


	// ..............................................................


	inline typename linked::iterator linked::insert(const_iterator itr, const_reference page_pos) {
        constexpr const char* suborigin = "insert()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10490, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, page_pos=0x%llx",
			(unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), (unsigned long long)page_pos);

        diag_base::expect(suborigin, itr.item_pos() == item_pos_nil, 0x1048e, "itr.item_pos() == item_pos_nil");
        diag_base::expect(suborigin, itr.page_pos() != page_pos_nil || itr.edge() == iterator_edge::end, 0x1048f, "itr.page_pos() != page_pos_nil || itr.edge() == iterator_edge::end");

		// Regardless of where we insert, the result will be this iterator upon success.
		iterator result(this, page_pos, item_pos_nil, iterator_edge::none, diag_base::log());

		// Insert without changing the state.
		insert_nostate(itr, page_pos, _state->back_page_pos);

		// Update the front page pos, if needed.
		if (_state->front_page_pos == page_pos_nil || _state->front_page_pos == itr.page_pos()) {
			_state->front_page_pos = page_pos;
		}

		// Update the back page pos, if needed.
		if (_state->back_page_pos == page_pos_nil || itr.edge() == iterator_edge::end) {
			_state->back_page_pos = page_pos;
		}

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10491, "End:");

		return result;
	}


	inline void linked::insert_nostate(const_iterator itr, const_reference page_pos, page_pos_t back_page_pos) {
        constexpr const char* suborigin = "insert_nostate()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10492, "Begin: itr.page_pos=0x%llx, itr.item_pos=0x%x, page_pos=0x%llx",
			(unsigned long long)itr.page_pos(), (unsigned)itr.item_pos(), (unsigned long long)page_pos);

		vmem::page page(_pool, page_pos, diag_base::log());
        diag_base::expect(suborigin, page.ptr() != nullptr, 0x10493, "page.ptr() != nullptr");

		// Init the page layout.
		vmem::linked_page* linked_page = reinterpret_cast<vmem::linked_page*>(page.ptr());
		linked_page->page_pos = page_pos;
		linked_page->prev_page_pos = page_pos_nil;
		linked_page->next_page_pos = page_pos_nil;

		if (empty()) {
			// Nothing to do.
		}
		else if (itr.page_pos() == page_pos_nil) {
			// Inserting at the end.

			vmem::page back_page(_pool, back_page_pos, diag_base::log());
	        diag_base::expect(suborigin, back_page.ptr() != nullptr, 0x10494, "page.ptr() != nullptr");

			vmem::linked_page* back_linked_page = reinterpret_cast<vmem::linked_page*>(back_page.ptr());
			back_linked_page->next_page_pos = page.pos();
			linked_page->prev_page_pos = back_page_pos;
		}
		else {
			// Inserting at the middle or at the front.
			// A previous page may or may not exist, but the next page does, and itr is pointing at it.

			vmem::page next_page(_pool, itr.page_pos(), diag_base::log());
	        diag_base::expect(suborigin, next_page.ptr() != nullptr, 0x10495, "page.ptr() != nullptr");

			vmem::linked_page* next_linked_page = reinterpret_cast<vmem::linked_page*>(next_page.ptr());

			if (next_linked_page->prev_page_pos == page_pos_nil) {
				// Inserting at the front.
				linked_page->next_page_pos = next_page.pos();
				next_linked_page->prev_page_pos = page.pos();
			}
			else {
				// Inserting at the middle.
				vmem::page prev_page(_pool, next_linked_page->prev_page_pos, diag_base::log());
		        diag_base::expect(suborigin, next_page.ptr() != nullptr, 0x10496, "page.ptr() != nullptr");
	
				vmem::linked_page* prev_linked_page = reinterpret_cast<vmem::linked_page*>(prev_page.ptr());
				prev_linked_page->next_page_pos = page.pos();
				linked_page->prev_page_pos = prev_page.pos();

				linked_page->next_page_pos = next_page.pos();
				next_linked_page->prev_page_pos = page.pos();
			}
		}

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10497, "End:");
	}


	// ..............................................................


	inline typename linked::iterator linked::erase(const_iterator itr) {
		if (itr.page_pos() == page_pos_nil || itr.edge() != iterator_edge::none) {
			throw exception<std::logic_error, Log>("vmem_linked::erase(itr)", 0x10498);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x10499, "vmem_linked::erase() Start. itr.page_pos=0x%llx", (long long)itr.page_pos());
		}

		// The result, upon success, is the next of itr.
		iterator result = next(itr);

		vmem_page_pos_t back_page_pos = page_pos_nil;
		bool ok = erase_nostate(itr, back_page_pos);

		if (ok) {
			// Update the front page pos.
			if (_state->front_page_pos == itr.page_pos()) {
				_state->front_page_pos = result.page_pos();
			}

			// Update the back page pos.
			if (_state->back_page_pos == itr.page_pos()) {
				_state->back_page_pos = back_page_pos;
			}
		}
		else {
			result = end();
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1049a, "vmem_linked::erase() Done. itr.page_pos=0x%llx, result.page_pos=0x%llx, result.edge=%u",
				(long long)itr.page_pos(), (long long)result.page_pos(), result.edge());
		}

		return result;
	}


	inline bool linked::erase_nostate(const_iterator itr, vmem_page_pos_t& back_page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1049b, "vmem_linked::erase_nostate() Start. itr.page_pos=0x%llx", (long long)itr.page_pos());
		}

		bool ok = true;

		vmem_page<Pool, Log> page(_pool, itr.page_pos(), _log);
		vmem_linked_page* linked_page = reinterpret_cast<vmem_linked_page*>(page.ptr());

		if (linked_page == nullptr) {
			ok = false;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x1049c, "vmem_linked::erase_nostate() Could not load page.");
			}
		}

		if (ok) {
			if (linked_page->prev_page_pos != page_pos_nil) {
				// There is a prev page.
				vmem_page<Pool, Log> prev_page(_pool, linked_page->prev_page_pos, _log);
				vmem_linked_page* prev_linked_page = reinterpret_cast<vmem_linked_page*>(prev_page.ptr());

				if (prev_linked_page == nullptr) {
					ok = false;

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::warning, 0x1049d, "vmem_linked::erase_nostate() Could not load prev page.");
					}
				}

				if (ok) {
					prev_linked_page->next_page_pos = linked_page->next_page_pos;
				}
			}
		}

		if (ok) {
			if (linked_page->next_page_pos != page_pos_nil) {
				// There is a next page.
				vmem_page<Pool, Log> next_page(_pool, linked_page->next_page_pos, _log);
				vmem_linked_page* next_linked_page = reinterpret_cast<vmem_linked_page*>(next_page.ptr());

				if (next_linked_page == nullptr) {
					ok = false;

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::warning, 0x1049e, "vmem_linked::erase_nostate() Could not load next page.");
					}
				}

				if (ok) {
					next_linked_page->prev_page_pos = linked_page->prev_page_pos;
				}
			}
			else {
				// There is no next page, which means we are deleting the back page.
				// Export back the new back page pos.
				back_page_pos = linked_page->prev_page_pos;
			}
		}

		if (ok) {
			linked_page = nullptr;
			page.free();
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1049f, "vmem_linked::erase_nostate() Done. ok=%d, itr.page_pos=0x%llx",
				ok, (long long)itr.page_pos());
		}

		return ok;
	}


	// ..............................................................


	inline void linked::clear() {
		_pool->clear_linked(*this);
	}


	// ..............................................................


	inline void linked::splice(linked& other) {
		splice(std::move(other));
	}


	inline void linked::splice(linked&& other) {
		if (_state == other._state) {
			throw exception<std::logic_error, Log>("vmem_linked::splice(other.state)", 0x104a0);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x104a1, "vmem_linked::splice() Start. front_page_pos=0x%llx, back_page_pos=0x%llx, other.front_page_pos=0x%llx, other.back_page_pos=0x%llx",
				(long long)_state->front_page_pos, (long long)_state->back_page_pos, (long long)other._state->front_page_pos, (long long)other._state->back_page_pos);
		}

		bool ok = true;

		if (other.empty()) {
			// Nothing to do.
		}
		else if (empty()) {
			// Take over the other state.
			*_state = *other._state;
		}
		else {
			// Connect the back page of this and the front page of the other.
			vmem_page<Pool, Log> back_page(_pool, _state->back_page_pos, _log);
			vmem_linked_page* back_linked_page = reinterpret_cast<vmem_linked_page*>(back_page.ptr());

			if (back_linked_page == nullptr) {
				ok = false;

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, 0x104a2, "vmem_linked::splice() Could not load back page.");
				}
			}

			if (ok) {
				vmem_page<Pool, Log> other_front_page(_pool, other._state->front_page_pos, _log);
				vmem_linked_page* other_front_linked_page = reinterpret_cast<vmem_linked_page*>(other_front_page.ptr());

				if (other_front_linked_page == nullptr) {
					ok = false;

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::warning, 0x104a3, "vmem_linked::splice() Could not load other.front page.");
					}
				}

				if (ok) {
					// Connect this back page and other front page.
					back_linked_page->next_page_pos = other._state->front_page_pos;
					other_front_linked_page->prev_page_pos = _state->back_page_pos;

					// Update this state.
					_state->back_page_pos = other._state->back_page_pos;

					// Empty other state.
					other._state->front_page_pos = page_pos_nil;
					other._state->back_page_pos = page_pos_nil;
				}
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x104a4, "vmem_linked::splice() Done. ok=%d, front_page_pos=0x%llx, back_page_pos=0x%llx",
				ok, (long long)_state->front_page_pos, (long long)_state->back_page_pos);
		}
	}


	// ..............................................................


	inline typename linked::iterator linked::next(const iterator_state& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x104a5, "vmem_linked::next() Start. itr.page_pos=0x%llx, itr.edge=%u",
				(long long)itr.page_pos(), itr.edge());
		}

		iterator result = end_itr();

		if (itr == static_cast<iterator_state>(end())) {
			// Nothing to do.
		}
		else if (itr == static_cast<iterator_state>(rbegin())) {
			result = begin_itr();
		}
		else if (itr == static_cast<iterator_state>(rend())) {
			// Nothing to do.
		}
		else if (itr.page_pos() != page_pos_nil) {
			vmem_page<Pool, Log> page(_pool, itr.page_pos(), _log);

			if (page.ptr() == nullptr) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, 0x104a6, "vmem_linked::next() Could not load page pos=0x%llx", (long long)itr.page_pos());
				}
			}
			else {
				vmem_linked_page* linked_page = reinterpret_cast<vmem_linked_page*>(page.ptr());

				iterator_edge_t edge = linked_page->next_page_pos == page_pos_nil ? iterator_edge::end : iterator_edge::none;
				result = iterator(this, linked_page->next_page_pos, item_pos_nil, edge, _log);
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x104a7, "vmem_linked::next() Done. result.page_pos=0x%llx, result.edge=%u",
				(long long)result.page_pos(), result.edge());
		}

		return result;
	}


	inline typename linked::iterator linked::prev(const iterator_state& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x104a8, "vmem_linked::prev() Start. itr.page_pos=0x%llx, itr.edge=%u",
				(long long)itr.page_pos(), itr.edge());
		}

		iterator result = rbegin_itr();

		if (itr == static_cast<iterator_state>(rbegin())) {
			// Nothing to do.
		}
		else if (itr == static_cast<iterator_state>(begin())) {
			// Nothing to do.
		}
		else if (itr == static_cast<iterator_state>(end())) {
			result = rend_itr();
		}
		else if (itr.page_pos() != page_pos_nil) {
			vmem_page<Pool, Log> page(_pool, itr.page_pos(), _log);

			if (page.ptr() == nullptr) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, 0x104a9, "vmem_linked::prev() Could not load page pos=0x%llx", (long long)itr.page_pos());
				}
			}
			else {
				vmem_linked_page* linked_page = reinterpret_cast<vmem_linked_page*>(page.ptr());

				iterator_edge_t edge = linked_page->prev_page_pos == page_pos_nil ? iterator_edge::rbegin : iterator_edge::none;
				result = iterator(this, linked_page->prev_page_pos, item_pos_nil, edge, _log);
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x104aa, "vmem_linked::prev() Done. result.page_pos=0x%llx, result.edge=%u",
				(long long)result.page_pos(), result.edge());
		}

		return result;
	}


	inline typename linked::pointer linked::at(const iterator_state& itr) const noexcept {
		return pointer(_pool, itr.page_pos(), 0, _log);
	}


	inline typename linked::iterator linked::begin_itr() const noexcept {
		if (_state->front_page_pos == page_pos_nil) {
			return end_itr();
		}

		return iterator(this, _state->front_page_pos, item_pos_nil, iterator_edge::none, _log);
	}


	inline typename linked::iterator linked::end_itr() const noexcept {
		return iterator(this, page_pos_nil, item_pos_nil, iterator_edge::end, _log);
	}


	inline typename linked::reverse_iterator linked::rend_itr() const noexcept {
		if (_state->back_page_pos == page_pos_nil) {
			return rbegin_itr();
		}

		return iterator(this, _state->back_page_pos, item_pos_nil, iterator_edge::none, _log);
	}


	inline typename linked::reverse_iterator linked::rbegin_itr() const noexcept {
		return vmem_linked_iterator<Pool, Log>(this, page_pos_nil, item_pos_nil, iterator_edge::rbegin, _log);
	}

} }
