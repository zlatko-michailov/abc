/*
MIT License

Copyright (c) 2018-2026 Zlatko Michailov 

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
#include "pool.h"
#include "i/page.i.h"


namespace abc { namespace vmem {

    inline constexpr const char* page::origin() noexcept {
        return "abc::vmem::page";
    }


    inline page::page(vmem::pool* pool, diag::log_ostream* log)
        : page(pool, page_pos_nil, log) {
    }


    inline page::page(vmem::pool* pool, page_pos_t pos, diag::log_ostream* log)
        : diag_base(abc::copy(origin()), log)
        , _pool(pool)
        , _pos(pos)
        , _ptr(nullptr) {

        constexpr const char* suborigin = "page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a5d, "Begin: pool=%p, pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);

        diag_base::expect(suborigin, _pool != nullptr, 0x103af, "_pool != nullptr");

        if (_pos == page_pos_nil) {
            alloc();
        }

        lock();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a5e, "End: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);
    }


    inline page::page(page&& other) noexcept
        : diag_base(other)
        , _pool(other._pool)
        , _pos(other._pos)
        , _ptr(other._ptr) {

        constexpr const char* suborigin = "page(move)";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a5f, "Begin: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);

        other.invalidate();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a60, "End: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);
    }


    inline page::page(const page& other) noexcept
        : diag_base(other)
        , _pool(other._pool)
        , _pos(other._pos)
        , _ptr(nullptr) {

        constexpr const char* suborigin = "page(copy)";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a61, "Begin: pool=%p, pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);

        if (_pool != nullptr && _pos != page_pos_nil) {
            lock();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a62, "End: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);
    }


    inline page::page(std::nullptr_t) noexcept
        : diag_base(abc::copy(origin()), nullptr)
        , _pool(nullptr)
        , _pos(page_pos_nil)
        , _ptr(nullptr) {

        constexpr const char* suborigin = "page(nullptr)";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a63, "Begin: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a64, "End: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);
    }


    inline page::~page() noexcept {
        constexpr const char* suborigin = "~page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a65, "Begin: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);

        unlock();
        invalidate();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a66, "End: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);
    }


    inline page& page::operator =(page&& other) noexcept {
        constexpr const char* suborigin = "=(move)";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a67, "Begin: pool=%p, page_pos=0x%llu, ptr=%p", other._pool, (unsigned long long)other._pos, other._ptr);

        unlock();

        _pool = other._pool;
        _pos = other._pos;
        _ptr = nullptr;

        if (_pool != nullptr && _pos != page_pos_nil) {
            lock();
        }

        other.unlock();
        other.invalidate();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a68, "End: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);

        return *this;
    }


    inline page& page::operator =(const page& other) noexcept {
        constexpr const char* suborigin = "=(copy)";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a69, "Begin: pool=%p, page_pos=0x%llu, ptr=%p", other._pool, (unsigned long long)other._pos, other._ptr);

        unlock();

        _pool = other._pool;
        _pos = other._pos;
        _ptr = nullptr;

        if (_pool != nullptr && _pos != page_pos_nil) {
            lock();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a6a, "End: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);

        return *this;
    }


    inline void page::free() noexcept {
        constexpr const char* suborigin = "free()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a6b, "Begin: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);

        unlock();

        if (_pool != nullptr && _pos != page_pos_nil) {
            _pool->free_page(_pos);
        }

        invalidate();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a6c, "End: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);
    }


    inline void page::alloc() {
        constexpr const char* suborigin = "alloc()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a6d, "Begin: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);

        diag_base::expect(suborigin, _pool != nullptr, 0x10a6e, "_pool != nullptr");
        diag_base::expect(suborigin, _pos == page_pos_nil, 0x10a6f, "_pos == page_pos_nil");
        diag_base::expect(suborigin, _ptr == nullptr, 0x10a70, "_ptr == nullptr");

        _pos = _pool->alloc_page();
        diag_base::ensure(suborigin, _pos != page_pos_nil, 0x10a71, "_pos != page_pos_nil");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a72, "End: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);
    }


    inline void page::lock() {
        constexpr const char* suborigin = "lock()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a73, "Begin: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);

        diag_base::expect(suborigin, _pool != nullptr, 0x10a74, "_pool != nullptr");
        diag_base::expect(suborigin, _pos != page_pos_nil, 0x10a75, "_pos != page_pos_nil");
        diag_base::expect(suborigin, _ptr == nullptr, 0x10a76, "_ptr == nullptr");

        _ptr = _pool->lock_page(_pos);
        diag_base::ensure(suborigin, _ptr != nullptr, 0x10a77, "_ptr != nullptr");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a78, "End: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);
    }


    inline void page::unlock() noexcept {
        constexpr const char* suborigin = "unlock()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a79, "Begin: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);

        if (_pool != nullptr && _pos != page_pos_nil && _ptr != nullptr)
        {
            _pool->unlock_page(_pos);
            _ptr = nullptr;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a7a, "End: pool=%p, page_pos=0x%llu, ptr=%p", _pool, (unsigned long long)_pos, _ptr);
    }


    inline void page::invalidate() noexcept {
        _pool = nullptr;
        _pos = page_pos_nil;
        _ptr = nullptr;
    }


    inline vmem::pool* page::pool() const noexcept {
        return _pool;
    }


    inline page_pos_t page::pos() const noexcept {
        return _pos;
    }


    inline void* page::ptr() noexcept {
        return _ptr;
    }


    inline const void* page::ptr() const noexcept {
        return _ptr;
    }

} }
