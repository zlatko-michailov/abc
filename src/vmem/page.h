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
#include "pool.h"
#include "i/page.i.h"


namespace abc { namespace vmem {

    inline page::page(vmem::pool* pool, diag::log_ostream* log)
        : page(pool, page_pos_nil, log) {
    }


    inline page::page(vmem::pool* pool, page_pos_t pos, diag::log_ostream* log)
        : diag_base(abc::copy(_origin), log)
        , _pool(pool)
        , _pos(pos)
        , _ptr(nullptr) {

        constexpr const char* suborigin = "page()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: pool=%p, pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);

        diag_base::expect(suborigin, _pool != nullptr, 0x103af, "_pool != nullptr");

        if (_pos == page_pos_nil) {
            alloc();
        }

        lock();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);
    }


    inline page::page(page&& other) noexcept
        : diag_base(other)
        , _pool(other._pool)
        , _pos(other._pos)
        , _ptr(other._ptr) {

        constexpr const char* suborigin = "page(move)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);

        other.invalidate();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);
    }


    inline page::page(const page& other) noexcept
        : diag_base(other)
        , _pool(other._pool)
        , _pos(other._pos)
        , _ptr(nullptr) {

        constexpr const char* suborigin = "page(copy)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: pool=%p, pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);

        if (_pool != nullptr && _pos != page_pos_nil) {
            lock();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);
    }


    inline page::page(std::nullptr_t) noexcept
        : diag_base(abc::copy(_origin), nullptr)
        , _pool(nullptr)
        , _pos(page_pos_nil)
        , _ptr(nullptr) {

        constexpr const char* suborigin = "page(nullptr)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);
    }


    inline page::~page() noexcept {
        constexpr const char* suborigin = "~page()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);

        unlock();
        invalidate();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);
    }


    inline page& page::operator =(page&& other) noexcept {
        constexpr const char* suborigin = "=(move)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: pool=%p, page_pos=%lu, ptr=%p", other._pool, (unsigned long)other._pos, other._ptr);

        unlock();

        _pool = other._pool;
        _pos = other._pos;
        _ptr = nullptr;

        if (_pool != nullptr && _pos != page_pos_nil) {
            lock();
        }

        other.unlock();
        other.invalidate();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);

        return *this;
    }


    inline page& page::operator =(const page& other) noexcept {
        constexpr const char* suborigin = "=(copy)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: pool=%p, page_pos=%lu, ptr=%p", other._pool, (unsigned long)other._pos, other._ptr);

        unlock();

        _pool = other._pool;
        _pos = other._pos;
        _ptr = nullptr;

        if (_pool != nullptr && _pos != page_pos_nil) {
            lock();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);

        return *this;
    }


    inline void page::free() noexcept {
        constexpr const char* suborigin = "free()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);

        unlock();

        if (_pool != nullptr && _pos != page_pos_nil) {
            _pool->free_page(_pos);
        }

        invalidate();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);
    }


    inline void page::alloc() {
        constexpr const char* suborigin = "alloc()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);

        diag_base::expect(suborigin, _pool != nullptr, __TAG__, "_pool != nullptr");
        diag_base::expect(suborigin, _pos == page_pos_nil, __TAG__, "_pos == page_pos_nil");
        diag_base::expect(suborigin, _ptr == nullptr, __TAG__, "_ptr == nullptr");

        _pos = _pool->alloc_page();
        diag_base::ensure(suborigin, _pos != page_pos_nil, __TAG__, "_pos != page_pos_nil");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);
    }


    inline void page::lock() {
        constexpr const char* suborigin = "lock()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);

        diag_base::expect(suborigin, _pool != nullptr, __TAG__, "_pool != nullptr");
        diag_base::expect(suborigin, _pos != page_pos_nil, __TAG__, "_pos != page_pos_nil");
        diag_base::expect(suborigin, _ptr == nullptr, __TAG__, "_ptr == nullptr");

        _ptr = _pool->lock_page(_pos);
        diag_base::ensure(suborigin, _ptr != nullptr, __TAG__, "_ptr != nullptr");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);
    }


    inline void page::unlock() noexcept {
        constexpr const char* suborigin = "unlock()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);

        if (_pool != nullptr && _pos != page_pos_nil && _ptr != nullptr)
        {
            _pool->unlock_page(_pos);
            _ptr = nullptr;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pool=%p, page_pos=%lu, ptr=%p", _pool, (unsigned long)_pos, _ptr);
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
