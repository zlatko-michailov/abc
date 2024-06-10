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


#include "../diag/diag_ready.h"
#include "page.h"
#include "i/ptr.i.h"


namespace abc { namespace vmem {

	template <typename T>
	inline ptr<T>::ptr(vmem::pool* pool, page_pos_t page_pos, item_pos_t byte_pos, diag::log_ostream* log)
		: diag_base(abc::copy(_origin), log)
        , _page(page_pos != page_pos_nil ? page(pool, page_pos, log) : page(nullptr))
		, _byte_pos(byte_pos) {

        constexpr const char* suborigin = "ptr()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: pool=%p, page_pos=%lu, byte_pos=%u, page_ptr=%p", _page.pool(), (unsigned long)_page.pos(), _byte_pos, page.ptr());

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
	}


	template <typename T>
	inline ptr<T>::ptr(std::nullptr_t, diag::log_ostream* log) noexcept
		: ptr<T>(nullptr, page_pos_nil, item_pos_nil, log) {
	}


	template <typename T>
	inline vmem::pool* ptr<T>::pool() const noexcept {
		return _page.pool();
	}


	template <typename T>
	inline page_pos_t ptr<T>::page_pos() const noexcept {
		return _page.pos();
	}


	template <typename T>
	inline item_pos_t ptr<T>::byte_pos() const noexcept {
		return _byte_pos;
	}


	template <typename T>
	inline ptr<T>::operator T*() noexcept {
		return p();
	}


	template <typename T>
	inline ptr<T>::operator const T*() const noexcept {
		return p();
	}


	template <typename T>
	inline T* ptr<T>::operator ->() noexcept {
		return p();
	}


	template <typename T>
	inline const T* ptr<T>::operator ->() const noexcept {
		return p();
	}


	template <typename T>
	inline T& ptr<T>::operator *() {
		return deref();
	}


	template <typename T>
	inline const T& ptr<T>::operator *() const {
		return deref();
	}


	template <typename T>
	inline T* ptr<T>::p() const noexcept {
		const char* page_ptr = reinterpret_cast<const char*>(_page.ptr());

		if (page_ptr == nullptr || _byte_pos == item_pos_nil) {
			return nullptr;
		}

		return const_cast<T*>(reinterpret_cast<const T*>(page_ptr + _byte_pos));
	}


	template <typename T>
	inline T& ptr<T>::deref() const {
		T* p = p();

        diag_base::expect(suborigin, p != nullptr, 0x103b5, "p != nullptr");

		return *p;
	}

} }
