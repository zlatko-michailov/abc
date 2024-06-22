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

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "../diag/diag_ready.h"
#include "page.h"
#include "linked.h"
#include "i/layout.i.h"
#include "i/pool.i.h"


namespace abc { namespace vmem {

    inline pool::pool(const char* file_path, std::size_t max_mapped_page_count = size::max, diag::log_ostream* log)
        : diag_base(abc::copy(_origin), log)
        , _ready(false)
        , _max_mapped_page_count(max_mapped_page_count)
        , _mapped_page_count(0)
        , _mapped_pages{ }
        , _stats{ } {

        constexpr const char* suborigin = "pool()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: file_path='%s', max_mapped_page_count=%zu", file_path, max_mapped_page_count);

        diag_base::expect(suborigin, file_path != nullptr, __TAG__, "file_path != nullptr");
        diag_base::expect(suborigin, min_mapped_page_count < max_mapped_page_count, __TAG__, "min_mapped_page_count < max_mapped_page_count");

        bool is_init = open(file_path);

        if (!is_init) {
            init();
        }

        verify();
        diag_base::ensure(suborigin, _ready, __TAG__, "_ready");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10390, "End:");
    }


    inline pool::pool(pool&& other) noexcept 
        : diag_base(other)
        , _ready(other._ready)
        , _fd(other._fd)
        , _max_mapped_page_count(other._max_mapped_page_count)
        , _mapped_page_count(other._mapped_page_count)
        , _mapped_pages(std::move(other._mapped_pages))
        , _stats(std::move(other._stats)) {

        constexpr const char* suborigin = "pool(move)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: fd=%d, max_mapped_page_count=%zu", _fd, _max_mapped_page_count);

        other._ready = false;
        other._fd = -1;
        other._mapped_page_count = 0;

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline pool::~pool() noexcept {
        constexpr const char* suborigin = "~pool()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: fd=%d, max_mapped_page_count=%zu", _fd, _max_mapped_page_count);

        if (_ready) {
            if (_fd >= 0) {
                for (std::size_t i = 0; i < _mapped_page_count; i++) {
                    std::size_t dummy_unmapped_count = 0;
                    std::size_t dummy_empty_i = _mapped_page_count;

                    diag_base::put_any(suborigin, diag::severity::optional, 0x10712, "Unmapping page i=%zu", i);
                    unmap_mapped_page(i, _mapped_pages[i].keep_count + 1, dummy_empty_i, dummy_unmapped_count); //// TODO: Unmap all pages?
                }

                diag_base::put_any(suborigin, diag::severity::optional, 0x10713, "Close file fd=%d", _fd);
                ::close(_fd);
            }
            else {
                diag_base::put_any(suborigin, diag::severity::optional, 0x10711, "No file - no unmapping.");
            }
        }

        _ready = false;
        _fd = -1;
        _mapped_page_count = 0;

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline bool pool::open(const char* file_path) {
        constexpr const char* suborigin = "open()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1037c, "Begin: file_path='%s'", file_path);

        _fd = ::open(file_path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        diag_base::ensure(suborigin, _fd >= 0, 0x1037e, "_fd >= 0, errno=%d", errno);

        page_pos_t file_size = ::lseek(_fd, 0, SEEK_END);
        diag_base::ensure(suborigin, (file_size & (page_size - 1)) == 0, 0x10380, "(file_size & (page_size - 1)) == 0");

        bool is_init = (file_size > 0);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104ae, "End: is_init=%d, file_size=%llu", is_init, (unsigned long long)file_size);

        return is_init;
    }


    inline void pool::init() {
        constexpr const char* suborigin = "init()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        // IMPORTANT! Keep this order:
        // root  (0)
        // start (1)
        create_root_page();
        create_start_page();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void pool::create_root_page() {
        constexpr const char* suborigin = "create_root_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10381, "Begin:");

        vmem::page page(this, diag_base::log());
        diag_base::ensure(suborigin, page.ptr() != nullptr, 0x10382, "page.ptr() != nullptr");

        std::memset(page.ptr(), 0, page_size);

        vmem::root_page root_page_layout;
        std::memmove(page.ptr(), &root_page_layout, sizeof(root_page_layout));

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10383, "Begin:");
    }


    inline void pool::create_start_page() {
        constexpr const char* suborigin = "create_start_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10384, "Begin:");

        vmem::page page(this, diag_base::log());
        diag_base::ensure(suborigin, page.ptr() != nullptr, 0x10385, "page.ptr() != nullptr");

        std::memset(page.ptr(), 0, page_size);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10386, "Begin:");
    }


    inline void pool::verify() {
        constexpr const char* suborigin = "verify()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        verify_root_page();
        verify_start_page();

        _ready = true;

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");
    }


    inline void pool::verify_root_page() {
        constexpr const char* suborigin = "verify_root_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10387, "Begin:");

        vmem::page page(this, page_pos_root, diag_base::log());
        diag_base::ensure(suborigin, page.ptr() != nullptr, 0x10388, "page.ptr() != nullptr");

        vmem::root_page* root_page = reinterpret_cast<vmem::root_page*>(page.ptr());
        diag_base::put_any(suborigin, diag::severity::debug, 0x10389, "Root page: pos=0x%llx, ptr=%p, version=%u, signature='%s', page_size=%u",
                (unsigned long long)page.pos(), page.ptr(), root_page->version, root_page->signature, (unsigned)root_page->page_size);

        vmem::root_page root_page_layout;
        diag_base::ensure(suborigin, root_page->version == root_page_layout.version, 0x1038a, "root_page->version == root_page_layout.version");
        diag_base::ensure(suborigin, std::strcmp(root_page->signature, root_page_layout.signature) == 0, 0x1038b, "std::strcmp(root_page->signature, root_page_layout.signature) == 0");
        diag_base::ensure(suborigin, root_page->page_size == page_size, 0x1038c, "root_page->page_size == page_size");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104af, "End:");
    }


    inline void pool::verify_start_page() {
        constexpr const char* suborigin = "verify_start_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1038d, "Begin:");

        vmem::page page(this, page_pos_start, diag_base::log());
        diag_base::ensure(suborigin, page.ptr() != nullptr, 0x1038e, "page.ptr() != nullptr");

        diag_base::put_any(suborigin, diag::severity::debug, 0x1038f, "Start page: pos=0x%llx, ptr=%p", (unsigned long long)page.pos(), page.ptr());

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104b0, "End:");
    }


    // ..............................................................


    inline page_pos_t pool::alloc_page() {
        constexpr const char* suborigin = "alloc_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10391, "Begin: ready=%d", _ready);

        page_pos_t page_pos = pop_free_page_pos();

        if (page_pos == page_pos_nil) {
            page_pos = create_page();
        }

        diag_base::ensure(suborigin, page_pos != page_pos_nil, 0x10396, "page_pos != page_pos_nil");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104b1, "End: page_pos=0x%llx", (unsigned long long)page_pos);

        return page_pos;
    }


    inline void pool::free_page(page_pos_t page_pos) {
        constexpr const char* suborigin = "free_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10399, "Begin: ready=%d, page_pos=0x%llx", _ready, (unsigned long long)page_pos);

        if (page_pos != page_pos_nil && _ready) {
            push_free_page_pos(page_pos);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104b2, "End:");
    }


    inline page_pos_t pool::pop_free_page_pos() {
        constexpr const char* suborigin = "pop_free_page_pos()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x104b3, "Begin:");

        page_pos_t page_pos = page_pos_nil;
        vmem::page page(this, page_pos_root, diag_base::log());
        diag_base::ensure(suborigin, page.ptr() != nullptr, 0x10392, "page.ptr() != nullptr");

        vmem::root_page* root_page = reinterpret_cast<vmem::root_page*>(page.ptr());
        vmem::linked free_pages_linked(&root_page->free_pages, this, diag_base::log());

        if (!free_pages_linked.empty()) {
            diag_base::put_any(suborigin, diag::severity::optional, 0x10393, "!empty");

            page_pos = free_pages_linked.back();
            free_pages_linked.pop_back();

            diag_base::put_any(suborigin, diag::severity::optional, 0x10394, "page_pos=0x%llx", (unsigned long long)page_pos);
        }

        diag_base::put_any(suborigin, diag::severity::callback, 0x104b4, "End: page_pos=0x%llx", (unsigned long long)page_pos);

        return page_pos;
    }


    inline void pool::push_free_page_pos(page_pos_t page_pos) {
        constexpr const char* suborigin = "push_free_page_pos()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x104b5, "Begin: page_pos=0x%llx", (unsigned long long)page_pos);

        vmem:page page(this, page_pos_root, _log);
        diag_base::ensure(suborigin, page.ptr() != nullptr, 0x1039a, "page.ptr() != nullptr");

        vmem::root_page* root_page = reinterpret_cast<root_page*>(page.ptr());
        vmem::linked free_pages_linked(&root_page->free_pages, this, _log);

        free_pages_linked.push_back(page_pos);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104b6, "End:");
    }


    inline page_pos_t pool::create_page() {
        constexpr const char* suborigin = "create_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x104b7, "Begin:");

        page_pos_t page_off = lseek(_fd, 0, SEEK_END);
        page_pos_t page_pos = page_off / page_size;
        diag_base::put_any(suborigin, diag::severity::optional,  0x10397, "pos=0x%llx off=0x%llx", (unsigned long long)page_pos, (unsigned long long)page_off);

        std::uint8_t blank_page[page_size] = { };
        ssize_t wb = write(_fd, blank_page, page_size);
        diag_base::ensure(suborigin, wb == page_size, 0x10398, "wb == page_size, wb=%l, errno=%d", (long)wb, errno);

        diag_base::put_any(suborigin, diag::severity::callback, 0x104b8, "End: page_pos=0x%llx", (unsigned long long)page_pos);

        return page_pos;
    }


    // ..............................................................


    template <std::size_t MaxMappedPages, typename Log>
    inline void* pool<MaxMappedPages, Log>::lock_page(page_pos_t page_pos) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::important, 0x1039b, "pool::lock_page() Start. page_pos=0x%llx", (long long)page_pos);
        }

        std::size_t i;
        bool is_found = find_mapped_page(page_pos, i);

        if (!is_found) {
            // The page is not mapped. We'll have to map it. We need capacity for that.
            if (!has_mapping_capacity()) {
                make_mapping_capacity();
            }

            if (has_mapping_capacity()) {
                // There is capacity to map one more page.
                if (_log != nullptr) {
                    _log->put_any(category::abc::vmem, severity::abc::debug, 0x103a4, "pool::lock_page() Capacity _mapped_page_count=%zu", _mapped_page_count);
                }

                i = _mapped_page_count;
            }
            else {
                // All the maped pages are locked. We cannot find a slot for the new page.
                if (_log != nullptr) {
                    _log->put_any(category::abc::vmem, severity::warning, 0x103a5, "pool::lock_page() Insufficient capacity. MaxedMappedPages=%zu", MaxMappedPages);
                }

                return nullptr;
            }
        } // Not found

        void* page_ptr = nullptr; 

        if (i < _mapped_page_count) {
            // The page is already mapped. Only re-lock it.
            page_ptr = lock_mapped_page(i);
        }
        else {
            // The page is not mapped. Map it. Then lock it.
            page_ptr = map_new_page(i, page_pos);
        }

        // Optimization: Swap this page forward (once) to keep pages sorted by keep_count.
        optimize_mapped_page(i);

        log_totals();

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::important, 0x104b9, "pool::lock_page() Done. page_pos=0x%llx, ptr=%p", (long long)page_pos, page_ptr);
        }

        return page_ptr;
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline bool pool<MaxMappedPages, Log>::unlock_page(page_pos_t page_pos) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x103aa, "pool::unlock_page() pos=0x%llx", (long long)page_pos);
        }

        _mapped_page_totals.unlock_count++;

        std::size_t i;
        bool is_found = find_mapped_page(page_pos, i);

        if (is_found) {
            unlock_mapped_page(i);
        }
        else {
            // The page was not found. This is a logic error.
            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::warning, 0x103ad, "pool::unlock_page() Trying to unlock a page that is not locked. page_pos=0x%llx",
                    (long long)page_pos);
            }

            return false;
        }

        log_totals();

        return true;
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline bool pool<MaxMappedPages, Log>::find_mapped_page(page_pos_t page_pos, std::size_t& i) noexcept {
        bool is_found = false;

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x104ba, "pool::find_mapped_page() Start. page_pos=0x%llx", (long long)page_pos);
        }

        for (i = 0; i < _mapped_page_count; i++) {
            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::abc::debug, 0x104bb, "pool::find_mapped_page() Examine i=%zu pos=0x%llx, lock_count=%d, keep_count=%d, ptr=%p",
                    i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].lock_count, (unsigned)_mapped_pages[i].keep_count, _mapped_pages[i].ptr);
            }

            if (_mapped_pages[i].pos == page_pos) {
                if (_log != nullptr) {
                    _log->put_any(category::abc::vmem, severity::abc::debug, 0x104bc, "pool::find_mapped_page() Found i=%zu pos=0x%llx, lock_count=%d, keep_count=%d, ptr=%p",
                        i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].lock_count, (unsigned)_mapped_pages[i].keep_count, _mapped_pages[i].ptr);
                }

                is_found = true;
                break;
            }
        }

        // Update the totals with the cost of this attempt.
        _mapped_page_totals.check_count += i + 1;

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x104bd, "pool::find_mapped_page() Done. page_pos=0x%llx, i=%zu", (long long)page_pos, i);
        }

        return is_found;
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline bool pool<MaxMappedPages, Log>::has_mapping_capacity() noexcept {
        return _mapped_page_count < MaxMappedPages;
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline std::size_t pool<MaxMappedPages, Log>::make_mapping_capacity() noexcept {
        // Record this run in the totals.
        _mapped_page_totals.unmap_count++;

        // Since this process is not cheap - it requires a scan of all the pages - we don't unmap a single page per run.
        // Instead, we unmap as many pages that match a certain condition.  
        // This is to avoid doing this process too frequently while still keeping frequently used pages mapped.
        // To enforce some fairness, we subtract the min_keep_count from the keep_count of each page that will be kept.

        // First try to unmap the pages with a keep_count below the average.
        page_hit_count_t avg_keep_count = _mapped_page_totals.keep_count / _mapped_page_count;
        std::size_t unmapped_count = make_mapping_capacity(avg_keep_count);

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x104be, "pool::make_mapping_capacity() First attempt. unmapped_count=%zu", unmapped_count);
        }

        // If the attempt makes some capacity, we are done.
        if (unmapped_count > 0) {
            return unmapped_count;
        }

        // If the first attempt, doesn't make any capacity, we try to unmap all pages that are not locked.
        avg_keep_count = _mapped_page_totals.keep_count  + 1;
        unmapped_count = make_mapping_capacity(avg_keep_count);

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x104bf, "pool::make_mapping_capacity() Second attempt. unmapped_count=%zu", unmapped_count);
        }

        return unmapped_count;
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline std::size_t pool<MaxMappedPages, Log>::make_mapping_capacity(page_hit_count_t min_keep_count) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x1039d, "pool::make_mapping_capacity() Start. min_keep_count=%u, mapped_page_count=%zu",
                (unsigned)min_keep_count, _mapped_page_count);
        }

        std::size_t unmapped_count = 0;
        std::size_t empty_i = MaxMappedPages;

        for (std::size_t i = 0; i < _mapped_page_count; i++) {
            if (should_keep_mapped_page(i, min_keep_count)) {
                keep_mapped_page(i, min_keep_count, empty_i);
            }
            else {
                unmap_mapped_page(i, min_keep_count, empty_i, unmapped_count);
            }
        }

        // Update the mapped page count.
        _mapped_page_count -= unmapped_count;

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x104c0, "pool::make_mapping_capacity() Done. min_keep_count=%u, mapped_page_count=%zu, unmapped_count=%zu",
                (unsigned)min_keep_count, _mapped_page_count, unmapped_count);
        }

        return unmapped_count;
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline bool pool<MaxMappedPages, Log>::should_keep_mapped_page(std::size_t i, page_hit_count_t min_keep_count) noexcept {
        return _mapped_pages[i].lock_count > 0 || _mapped_pages[i].keep_count > min_keep_count;
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline void pool<MaxMappedPages, Log>::keep_mapped_page(std::size_t i, page_hit_count_t min_keep_count, std::size_t& empty_i) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x1039e, "pool::keep_mapped_page() Start. i=%zu, pos=0x%llx, keep_count=%u, min_keep_count=%u",
                i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].keep_count, (unsigned)min_keep_count);
        }

        // Reduce the keep_count for fairness.
        if (_mapped_pages[i].keep_count > min_keep_count) {
            _mapped_page_totals.keep_count -= min_keep_count;
            _mapped_pages[i].keep_count -= min_keep_count;
        }
        else {
            _mapped_page_totals.keep_count -= _mapped_pages[i].keep_count;
            _mapped_pages[i].keep_count = 0;
        }

        // If there is already an empty slot, we move this element there, and zero out this slot.
        if (empty_i < _mapped_page_count) {
            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::abc::debug, 0x1039f, "pool::keep_mapped_page() Moving page empty_i=%zu, i=%zu, pos=0x%llx",
                    empty_i, i, (long long)_mapped_pages[i].pos);
            }

            // Move the element to the empty slot.
            _mapped_pages[empty_i] = _mapped_pages[i];

            // Zero out this slot.
            _mapped_pages[i] = { };
            _mapped_pages[i].ptr = nullptr;

            // Find the next empty slot.
            empty_i = next_empty_i(i, empty_i);
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x104c1, "pool::keep_mapped_page() Done. i=%zu, pos=0x%llx, keep_count=%u, min_keep_count=%u",
                i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].keep_count, (unsigned)min_keep_count);
        }
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline void pool<MaxMappedPages, Log>::unmap_mapped_page(std::size_t i, page_hit_count_t min_keep_count, std::size_t& empty_i, std::size_t& unmapped_count) noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x103a0, "pool::unmap_mapped_page() Start. i=%zu, pos=0x%llx, keep_count=%u, min_keep_count=%u",
                i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].keep_count, (unsigned)min_keep_count);
        }

        // Unmap the OS page.
        int um = munmap(_mapped_pages[i].ptr, page_size);
        void* ptr = _mapped_pages[i].ptr;

        // Zero out the slot.
        _mapped_pages[i] = { };
        _mapped_pages[i].ptr = nullptr;

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x103a1, "pool::unmap_mapped_page() Unmap. i=%zu, ptr=%p, um=%d, errno=%d", i, ptr, um, errno);
        }

        // If this is the first unmapped page, set empty_i.
        if (unmapped_count++ == 0) {
            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::abc::debug, 0x103a2, "pool::unmap_mapped_page() First empty slot i=%zu", i);
            }

            empty_i = i;
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x104c2, "pool::unmap_mapped_page() Done. i=%zu", i);
        }
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline void* pool<MaxMappedPages, Log>::lock_mapped_page(std::size_t i) noexcept {
        void* ptr = _mapped_pages[i].ptr;

        // Update the slot.
        _mapped_pages[i].lock_count++;
        _mapped_pages[i].keep_count++;

        // Update the stats.
        _mapped_page_totals.keep_count++;
        _mapped_page_totals.hit_count++;

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x103a6, "pool::lock_mapped_page() i=%zu, pos=0x%llx, lock_count=%d",
                i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].lock_count);
        }

        return ptr;
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline void pool<MaxMappedPages, Log>::unlock_mapped_page(std::size_t i) noexcept {
        _mapped_pages[i].lock_count--;

        if (_mapped_pages[i].lock_count == 0) {
            // When all locks on a page are released, we sync the OS page. 
            int sn = msync(_mapped_pages[i].ptr, page_size, MS_ASYNC);

            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::abc::optional, 0x103ab, "pool::unlock_mapped_page() msync i=%zu pos=0x%llx, ptr=%p, lock_count=%d, sn=%d, errno=%d",
                    i, (long long)_mapped_pages[i].pos, _mapped_pages[i].ptr, (unsigned)_mapped_pages[i].lock_count, sn, errno);
            }
        }
        else {
            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::abc::optional, 0x103ac, "pool::unlock_mapped_page() Used. i=%zu pos=0x%llx, ptr=%p, lock_count=%d",
                    i, (long long)_mapped_pages[i].pos, _mapped_pages[i].ptr, (unsigned)_mapped_pages[i].lock_count);
            }
        }
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline void* pool<MaxMappedPages, Log>::map_new_page(std::size_t i, page_pos_t page_pos) noexcept {
        // Map the OS page.
        off_t page_off = static_cast<off_t>(page_pos * page_size);
        void* ptr = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, page_off);

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x103a7, "pool::map_new_page() mmap i=%zu, pos=0x%llx, lock_count=1, ptr=%p, errno=%d",
                i, (long long)page_pos, ptr, errno);
        }

        _mapped_page_count++;

        // Set the slot.
        _mapped_pages[i].pos = page_pos;
        _mapped_pages[i].ptr = ptr;
        _mapped_pages[i].lock_count = 1;
        _mapped_pages[i].keep_count = 1;

        // Update the stats.
        _mapped_page_totals.keep_count++;
        _mapped_page_totals.miss_count++;

        return ptr;
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline void pool<MaxMappedPages, Log>::optimize_mapped_page(std::size_t i) noexcept {
        // The goal is to keep pages with a high keep_count close to the front.
        for (std::size_t j = 0; j < i; j++) {
            if (_mapped_pages[j].keep_count < _mapped_pages[i].keep_count) {
                if (_log != nullptr) {
                    _log->put_any(category::abc::vmem, severity::abc::debug, 0x103a8, "pool::optimize_mapped_page() Swapping j=%zu (pos=0x%llx), i=%zu (pos=0x%llx)",
                        j, (long long)_mapped_pages[j].pos, i, (long long)_mapped_pages[i].pos);
                }

                // Swap.
                mapped_page temp = _mapped_pages[j];
                _mapped_pages[j] = _mapped_pages[i];
                _mapped_pages[i] = temp;

                if (_log != nullptr) {
                    _log->put_any(category::abc::vmem, severity::abc::debug, 0x103a9, "pool::optimize_mapped_page() Swapped  j=%zu (pos=0x%llx), i=%zu (pos=0x%llx)",
                        j, (long long)_mapped_pages[j].pos, i, (long long)_mapped_pages[i].pos);
                }

                break;
            }
        }
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline std::size_t pool<MaxMappedPages, Log>::next_empty_i(std::size_t i, std::size_t empty_i) noexcept {
        std::size_t next_empty_i = empty_i + 1;

        while (next_empty_i < i && _mapped_pages[next_empty_i].ptr != nullptr) {
            next_empty_i++;
        }

        return next_empty_i;
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline void pool<MaxMappedPages, Log>::log_totals() noexcept {
        if (_log != nullptr) {
            page_hit_count_t total_lock_count = _mapped_page_totals.hit_count + _mapped_page_totals.miss_count;
            page_hit_count_t hit_percent = (_mapped_page_totals.hit_count * 100) / total_lock_count;
            page_hit_count_t miss_percent = (_mapped_page_totals.miss_count * 100) / total_lock_count;

            page_hit_count_t total_lookup_count = _mapped_page_totals.hit_count + _mapped_page_totals.miss_count + _mapped_page_totals.unlock_count;
            page_hit_count_t check_factor_x10 = (_mapped_page_totals.check_count * 10) / total_lookup_count;
            page_hit_count_t check_factor_percent = (check_factor_x10 * 10) / MaxMappedPages;

            _log->put_any(category::abc::vmem, severity::abc::optional, 0x103ae, "pool::log_totals() Pool Totals hits=%u (%u%%), misses=%u (%u%%), checks=%u (%u.%u, %u%%)", 
                (unsigned)_mapped_page_totals.hit_count, (unsigned)hit_percent,
                (unsigned)_mapped_page_totals.miss_count, (unsigned)miss_percent,
                (unsigned)_mapped_page_totals.check_count, (unsigned)(check_factor_x10 / 10), (unsigned)(check_factor_x10 % 10), (unsigned)check_factor_percent);
        }
    }


    template <std::size_t MaxMappedPages, typename Log>
    inline void pool<MaxMappedPages, Log>::clear_linked(linked<Pool, Log>& linked) {
        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x104c3, "pool::clear_linked() Start");
        }

        page<Pool, Log> page(this, page_pos_root, _log);

        if (page.ptr() == nullptr) {
            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::warning, 0x104c4, "pool::clear_linked() Could not check free_pages");
            }
        }
        else {
            root_page* root_page = reinterpret_cast<root_page*>(page.ptr());

            linked<Pool, Log> free_pages_linked(&root_page->free_pages, this, _log);
            free_pages_linked.splice(linked);
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::optional, 0x104c5, "pool::clear_linked() Done.");
        }
    }


    // --------------------------------------------------------------


    template <typename Pool, typename Log>
    inline page<Pool, Log>::page(Pool* pool, Log* log)
        : page<Pool, Log>(pool, page_pos_nil, log) {
    }


    template <typename Pool, typename Log>
    inline page<Pool, Log>::page(Pool* pool, page_pos_t page_pos, Log* log)
        : _pool(pool)
        , _pos(page_pos)
        , _ptr(nullptr)
        , _log(log) {

        if (pool == nullptr) {
            throw exception<std::logic_error, Log>("page::page(pool)", 0x103af);
        }

        if (page_pos == page_pos_nil) {
            if (!alloc()) {
                return;
            }
        }

        lock();
    }


    template <typename Pool, typename Log>
    inline page<Pool, Log>::page(const page<Pool, Log>& other) noexcept
        : _pool(other._pool)
        , _pos(other._pos)
        , _ptr(other._ptr)
        , _log(other._log) {

        if (_pool != nullptr && _pos != page_pos_nil) {
            lock();
        }
    }


    template <typename Pool, typename Log>
    inline page<Pool, Log>::page(page<Pool, Log>&& other) noexcept
        : _pool(other._pool)
        , _pos(other._pos)
        , _ptr(other._ptr)
        , _log(other._log) {

        other.invalidate();
    }


    template <typename Pool, typename Log>
    inline page<Pool, Log>::page(std::nullptr_t) noexcept
        : _pool(nullptr)
        , _pos(page_pos_nil)
        , _ptr(nullptr)
        , _log(nullptr) {
    }


    template <typename Pool, typename Log>
    inline page<Pool, Log>::~page() noexcept {
        unlock();
        invalidate();
    }


    template <typename Pool, typename Log>
    inline page<Pool, Log>& page<Pool, Log>::operator =(const page<Pool, Log>& other) noexcept {
        unlock();

        _pool = other._pool;
        _pos = other._pos;
        _ptr = other._ptr;
        _log = other._log;

        if (_pool != nullptr && _pos != page_pos_nil) {
            lock();
        }

        return *this;
    }


    template <typename Pool, typename Log>
    inline page<Pool, Log>& page<Pool, Log>::operator =(page<Pool, Log>&& other) noexcept {
        unlock();

        _pool = other._pool;
        _pos = other._pos;
        _ptr = other._ptr;
        _log = other._log;

        if (_pool != nullptr && _pos != page_pos_nil) {
            lock();
        }

        other.unlock();
        other.invalidate();

        return *this;
    }


    template <typename Pool, typename Log>
    inline bool page<Pool, Log>::alloc() noexcept {
        _pos = _pool->alloc_page();

        if (_pos == page_pos_nil) {
            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::warning, 0x103b0, "page::alloc() _pos=nil");
            }

            return false;
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x103b1, "page::alloc() _pos=0x%llx", (long long)_pos);
        }

        return true;
    }


    template <typename Pool, typename Log>
    inline void page<Pool, Log>::free() noexcept {
        unlock();

        if (_pos != page_pos_nil) {
            _pool->free_page(_pos);
        }

        invalidate();
    }


    template <typename Pool, typename Log>
    inline bool page<Pool, Log>::lock() noexcept {
        _ptr = _pool->lock_page(_pos);

        if (_ptr == nullptr) {
            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::warning, 0x103b2, "page::lock() _pos=0x%llx, _ptr=nullptr", (long long)_pos);
            }

            return false;
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::vmem, severity::abc::debug, 0x103b3, "page::lock() _pos=0x%llx, _ptr=%p", (long long)_pos, _ptr);
        }

        return  true;
    }


    template <typename Pool, typename Log>
    inline void page<Pool, Log>::unlock() noexcept {
        if (_pool != nullptr && _pos != page_pos_nil && _ptr != nullptr)
        {
            _pool->unlock_page(_pos);
            _ptr = nullptr;

            if (_log != nullptr) {
                _log->put_any(category::abc::vmem, severity::abc::debug, 0x103b4, "page::unlock() _pos=0x%llx", (long long)_pos);
            }
        }
    }


    template <typename Pool, typename Log>
    inline void page<Pool, Log>::invalidate() noexcept {
        _pool = nullptr;
        _pos = page_pos_nil;
        _ptr = nullptr;
        _log = nullptr;
    }


    template <typename Pool, typename Log>
    inline Pool* page<Pool, Log>::pool() const noexcept {
        return _pool;
    }


    template <typename Pool, typename Log>
    inline page_pos_t page<Pool, Log>::pos() const noexcept {
        return _pos;
    }


    template <typename Pool, typename Log>
    inline void* page<Pool, Log>::ptr() noexcept {
        return _ptr;
    }


    template <typename Pool, typename Log>
    inline const void* page<Pool, Log>::ptr() const noexcept {
        return _ptr;
    }


    // --------------------------------------------------------------


    template <typename T, typename Pool, typename Log>
    inline ptr<T, Pool, Log>::ptr(Pool* pool, page_pos_t page_pos, item_pos_t byte_pos, Log* log)
        : _page(page_pos != page_pos_nil ? page<Pool, Log>(pool, page_pos, log) : page<Pool, Log>(nullptr))
        , _byte_pos(byte_pos)
        , _log(log) {
    }


    template <typename T, typename Pool, typename Log>
    inline ptr<T, Pool, Log>::ptr(std::nullptr_t, Log* log) noexcept
        : ptr<T, Pool, Log>(nullptr, page_pos_nil, item_pos_nil, log) {
    }


    template <typename T, typename Pool, typename Log>
    inline Pool* ptr<T, Pool, Log>::pool() const noexcept {
        return _page.pool();
    }


    template <typename T, typename Pool, typename Log>
    inline page_pos_t ptr<T, Pool, Log>::page_pos() const noexcept {
        return _page.pos();
    }


    template <typename T, typename Pool, typename Log>
    item_pos_t ptr<T, Pool, Log>::byte_pos() const noexcept {
        return _byte_pos;
    }


    template <typename T, typename Pool, typename Log>
    ptr<T, Pool, Log>::operator T*() noexcept {
        return ptr();
    }


    template <typename T, typename Pool, typename Log>
    ptr<T, Pool, Log>::operator const T*() const noexcept {
        return ptr();
    }


    template <typename T, typename Pool, typename Log>
    T* ptr<T, Pool, Log>::operator ->() noexcept {
        return ptr();
    }


    template <typename T, typename Pool, typename Log>
    const T* ptr<T, Pool, Log>::operator ->() const noexcept {
        return ptr();
    }


    template <typename T, typename Pool, typename Log>
    T& ptr<T, Pool, Log>::operator *() {
        return deref();
    }


    template <typename T, typename Pool, typename Log>
    const T& ptr<T, Pool, Log>::operator *() const {
        return deref();
    }


    template <typename T, typename Pool, typename Log>
    T* ptr<T, Pool, Log>::ptr() const noexcept {
        const char* page_ptr = reinterpret_cast<const char*>(_page.ptr());

        if (page_ptr == nullptr || _byte_pos == item_pos_nil) {
            return nullptr;
        }

        return const_cast<T*>(reinterpret_cast<const T*>(page_ptr + _byte_pos));
    }


    template <typename T, typename Pool, typename Log>
    T& ptr<T, Pool, Log>::deref() const {
        T* p = ptr();

        if (p == nullptr) {
            throw exception<std::runtime_error, Log>("ptr::deref() Dereferencing invalid ptr", 0x103b5);
        }

        return *p;
    }

} }
