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

#include "../util.h"
#include "../diag/diag_ready.h"
#include "page.h"
//// TODO: #include "linked.h"
#include "i/layout.i.h"
#include "i/pool.i.h"


namespace abc { namespace vmem {

    inline constexpr const char* pool::origin() noexcept {
        return "abc::vmem::pool";
    }


    inline pool::pool(pool_config&& config, diag::log_ostream* log)
        : diag_base(abc::copy(origin()), log)
        , _config(std::move(config))
        , _ready(false)
        , _fd(-1)
        , _mapped_pages{ }
        , _stats{ } {

        constexpr const char* suborigin = "pool()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: file_path='%s', max_mapped_page_count=%zu", _config.file_path.c_str(), _config.max_mapped_page_count);

        diag_base::expect(suborigin, !_config.file_path.empty(), __TAG__, "!_config.file_path.empty()");

        bool is_init = open();

        if (!is_init) {
            init();
        }

        verify();
        diag_base::ensure(suborigin, _ready, __TAG__, "_ready");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10390, "End:");
    }


    inline pool::pool(pool&& other) noexcept 
        : diag_base(other)
        , _config(std::move(other._config))
        , _ready(other._ready)
        , _fd(other._fd)
        , _mapped_pages(std::move(other._mapped_pages))
        , _stats(std::move(other._stats)) {

        constexpr const char* suborigin = "pool(move)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: fd=%d, max_mapped_page_count=%zu", _fd, _config.max_mapped_page_count);

        other._ready = false;
        other._fd = -1;

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline pool::~pool() noexcept {
        constexpr const char* suborigin = "~pool()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: fd=%d, max_mapped_page_count=%zu", _fd, _config.max_mapped_page_count);

        if (_ready) {
            if (_fd >= 0) {
                // Unmap all mapped pages.
                while (!_mapped_pages.empty()) {
                    unmap_page(_mapped_pages.begin());
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

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline const pool_config& pool::config() const noexcept {
        return _config;
    }


    inline bool pool::open() {
        constexpr const char* suborigin = "open()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1037c, "Begin: file_path='%s'", _config.file_path.c_str());

        _fd = ::open(_config.file_path.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        diag_base::ensure(suborigin, _fd >= 0, 0x1037e, "_fd >= 0, errno=%d", errno);

        page_pos_t file_size = ::lseek(_fd, 0, SEEK_END);
        diag_base::ensure(suborigin, (file_size & (page_size - 1)) == 0, 0x10380, "(file_size & (page_size - 1)) == 0");

        bool is_init = (file_size / page_size >= 2);

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
        diag_base::expect(suborigin, page.ptr() != nullptr, 0x10388, "page.ptr() != nullptr");

        vmem::root_page* root_page = reinterpret_cast<vmem::root_page*>(page.ptr());
        diag_base::put_any(suborigin, diag::severity::debug, 0x10389, "Root page: pos=0x%llx, ptr=%p, version=%u, signature='%s', page_size=%u",
                (unsigned long long)page.pos(), page.ptr(), root_page->version, root_page->signature, (unsigned)root_page->page_size);

        vmem::root_page root_page_layout;
        diag_base::expect(suborigin, root_page->version == root_page_layout.version, 0x1038a, "root_page->version == root_page_layout.version");
        diag_base::expect(suborigin, std::strcmp(root_page->signature, root_page_layout.signature) == 0, 0x1038b, "std::strcmp(root_page->signature, root_page_layout.signature) == 0");
        diag_base::expect(suborigin, root_page->page_size == page_size, 0x1038c, "root_page->page_size == page_size");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104af, "End:");
    }


    inline void pool::verify_start_page() {
        constexpr const char* suborigin = "verify_start_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1038d, "Begin:");

        vmem::page page(this, page_pos_start, diag_base::log());
        diag_base::expect(suborigin, page.ptr() != nullptr, 0x1038e, "page.ptr() != nullptr");

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

#if 0 //// TODO: Linked
        // Get the root page to get the free pages linked state.
        vmem::page page(this, page_pos_root, diag_base::log());
        diag_base::expect(suborigin, page.ptr() != nullptr, 0x10392, "page.ptr() != nullptr");

        vmem::root_page* root_page = reinterpret_cast<vmem::root_page*>(page.ptr());
        vmem::linked free_pages_linked(&root_page->free_pages, this, diag_base::log());

        if (!free_pages_linked.empty()) {
            diag_base::put_any(suborigin, diag::severity::optional, 0x10393, "!empty");

            page_pos = free_pages_linked.back();
            free_pages_linked.pop_back();

            diag_base::put_any(suborigin, diag::severity::optional, 0x10394, "page_pos=0x%llx", (unsigned long long)page_pos);
        }
#endif

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104b4, "End: page_pos=0x%llx", (unsigned long long)page_pos);

        return page_pos;
    }


    inline void pool::push_free_page_pos(page_pos_t page_pos) {
        constexpr const char* suborigin = "push_free_page_pos()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x104b5, "Begin: page_pos=0x%llx", (unsigned long long)page_pos);

#if 0 //// TODO: Linked
        // Get the root page to get the free pages linked state.
        vmem:page page(this, page_pos_root, _log);
        diag_base::expect(suborigin, page.ptr() != nullptr, 0x1039a, "page.ptr() != nullptr");

        vmem::root_page* root_page = reinterpret_cast<root_page*>(page.ptr());
        vmem::linked free_pages_linked(&root_page->free_pages, this, _log);

        free_pages_linked.push_back(page_pos);
#endif

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

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104b8, "End: page_pos=0x%llx", (unsigned long long)page_pos);

        return page_pos;
    }


    // ..............................................................


    inline void* pool::lock_page(page_pos_t page_pos) {
        constexpr const char* suborigin = "lock_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1039b, "Begin: page_pos=0x%llx", (unsigned long long)page_pos);

        vmem::mapped_page* mapped_page = map_page(page_pos);
        diag_base::ensure(suborigin, mapped_page != nullptr, __TAG__, "mapped_page != nullptr");

        if (mapped_page->lock_count == 0) {
            // Move lock_count and keep_count from unlocked to locked.
            _stats.unlocked_page_count--;
            _stats.unlocked_page_keep_count -= mapped_page->keep_count;

            _stats.locked_page_count++;
            _stats.locked_page_keep_count += mapped_page->keep_count + 1;
        }

        mapped_page->lock_count++;
        mapped_page->keep_count++;

        log_stats();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1039b, "End: lock_count=%u", (unsigned)mapped_page->lock_count);

        return mapped_page->ptr;
    }


    inline void pool::unlock_page(page_pos_t page_pos) {
        constexpr const char* suborigin = "unlock_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x103aa, "Begin: page_pos=0x%llx", (unsigned long long)page_pos);

        // The page must be mapped.
        mapped_page_container::iterator mapped_page_itr = _mapped_pages.find(page_pos);
        diag_base::expect(suborigin, mapped_page_itr != _mapped_pages.end(), 0x103ad, "mapped_page_itr != _mapped_pages.end()");

        // The page's lock count must be strictly bigger than 0.
        diag_base::expect(suborigin, mapped_page_itr->second.lock_count > 0, __TAG__, "mapped_page_itr->second.lock_count > 0");
        mapped_page_itr->second.lock_count--;

        if (mapped_page_itr->second.lock_count == 0) {
            // Move lock_count and keep_count from locked to unlocked.
            _stats.locked_page_count--;
            _stats.locked_page_keep_count -= mapped_page_itr->second.keep_count;

            _stats.unlocked_page_count++;
            _stats.unlocked_page_keep_count += mapped_page_itr->second.keep_count;

            if (_config.sync_pages_on_unlock) {
                // Sync the OS page.
                int sn = msync(mapped_page_itr->second.ptr, page_size, MS_ASYNC);
                diag_base::ensure(suborigin, sn == 0, 0x103ab, "sn == 0, page_pos=0x%llx, ptr=%p, sn=%d, errno=%d", (unsigned long long)page_pos, mapped_page_itr->second.ptr, sn, errno);
            }
        }

        log_stats();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: lock_count=%u", (unsigned)mapped_page_itr->second.lock_count);
    }


    inline vmem::mapped_page* pool::map_page(page_pos_t page_pos) {
        constexpr const char* suborigin = "map_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: page_pos=0x%llx", (unsigned long long)page_pos);

        mapped_page_container::iterator mapped_page_itr = _mapped_pages.find(page_pos);
        if (mapped_page_itr != _mapped_pages.end()) {
            // The page is already mapped.
            _stats.map_hit_count++;
        }
        else {
            // The page has to be mapped.
            _stats.map_miss_count++;

            // Make sure there is capacity.
            ensure_mapping_capacity();
            diag_base::expect(suborigin, _mapped_pages.size() < _config.max_mapped_page_count, __TAG__, "_mapped_pages.size() < _config.max_mapped_page_count, mapped_page_count=%zu, max_mapped_page_count=%zu", _mapped_pages.size(), _config.max_mapped_page_count);

            // Map the OS page.
            off_t page_off = static_cast<off_t>(page_pos * vmem::page_size);
            void* ptr = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, page_off);
            diag_base::ensure(suborigin, ptr != MAP_FAILED, __TAG__, "ptr != MAP_FAILED, ptr=%p, errno=%d", ptr, errno);

            // Init a mapped_page entry.
            std::pair<page_pos_t, mapped_page> mapped_page_kvp { };
            mapped_page_kvp.first = page_pos;
            mapped_page_kvp.second.pos = page_pos;
            mapped_page_kvp.second.ptr = ptr;

            std::pair<mapped_page_container::iterator, bool> inserted_mapped_page = _mapped_pages.insert(std::move(mapped_page_kvp));
            diag_base::ensure(suborigin, inserted_mapped_page.second, __TAG__, "inserted_mapped_page.second");
            mapped_page_itr = std::move(inserted_mapped_page.first);            

            // There is one more unlocked page in the container.
            _stats.unlocked_page_count++;
        }

        diag_base::ensure(suborigin, mapped_page_itr != _mapped_pages.end(), __TAG__, "mapped_page_itr != _mapped_pages.end()");

        log_stats();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: mapped_page=%p", &*mapped_page_itr);

        return &mapped_page_itr->second;
    }


    inline pool::mapped_page_container::iterator pool::unmap_page(const mapped_page_container::iterator& mapped_page_itr) {
        constexpr const char* suborigin = "unmap_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x103a0, "Begin:");

        diag_base::expect(suborigin, mapped_page_itr != _mapped_pages.end(), __TAG__, "mapped_page_itr != _mapped_pages.end()");
        diag_base::expect(suborigin, mapped_page_itr->second.ptr != nullptr, __TAG__, "mapped_page_itr->second.ptr != nullptr");

        if (!_config.sync_pages_on_unlock || (_config.sync_locked_pages_on_destroy && mapped_page_itr->second.lock_count > 0)) {
            // Sync the OS page. 
            int sn = msync(mapped_page_itr->second.ptr, page_size, MS_ASYNC);
            diag_base::ensure(suborigin, sn == 0, __TAG__, "sn == 0, page_pos=0x%llx, ptr=%p, sn=%d, errno=%d", (unsigned long long)mapped_page_itr->second.pos, mapped_page_itr->second.ptr, sn, errno);
        }

        // Unmap the OS page.
        int um = munmap(mapped_page_itr->second.ptr, page_size);
        diag_base::ensure(suborigin, um == 0, __TAG__, "um == 0");

        if (mapped_page_itr->second.lock_count > 0) {
            _stats.locked_page_keep_count -= mapped_page_itr->second.keep_count;
            _stats.locked_page_count--;
        }
        else {
            _stats.unlocked_page_keep_count -= mapped_page_itr->second.keep_count;
            _stats.unlocked_page_count--;
        }

        diag_base::put_any(suborigin, diag::severity::optional, __TAG__, "pos=0x%llx, ptr=%p", (unsigned long long)mapped_page_itr->second.pos, mapped_page_itr->second.ptr);

        // Remove the mapped page entry from the container.
        mapped_page_container::iterator ret_itr = _mapped_pages.erase(mapped_page_itr);

        log_stats();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104c2, "End:");

        return ret_itr;
    }


    inline void pool::ensure_mapping_capacity() {
        constexpr const char* suborigin = "ensure_mapping_capacity()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: count=%zu, max_count=%zu", _mapped_pages.size(), _config.max_mapped_page_count);

        diag_base::expect(suborigin, _mapped_pages.size() <= _config.max_mapped_page_count, __TAG__, "_mapped_pages.size() <= _config.max_mapped_page_count, mapped_page_count=%zu, max_mapped_page_count=%zu", _mapped_pages.size(), _config.max_mapped_page_count);

        if (_mapped_pages.size() == _config.max_mapped_page_count) {
            _stats.free_capacity_count++;

            diag_base::put_any(suborigin, diag::severity::verbose, __TAG__, "Trying to free capacity.");
            log_stats();

            //// TODO: A combination of a container and algorithms should be used so that the following requirements are met:
            // 1. A mapped page entry should not be moved from one container to another when its "locked" state changes.
            // 2. Ditto, especially when an already locked page is being re-locked.
            // 3. No situation should lead to full, sequential, traversal of container.

            // If there are no unlocked pages, then nothing can be freed.
            if (_stats.unlocked_page_count == 0) {
                diag_base::throw_exception<std::runtime_error>(suborigin, __TAG__, "No mapping capacity. max_page_count=%zu, locked_page_count=%u, unlocked_page_count=%u", _config.max_mapped_page_count, (unsigned)_stats.locked_page_count, (unsigned)_stats.unlocked_page_count);
            }

            // Remove all unlocked pages with a keep count not higher than the average.
            count_t avg_keep_count = (_stats.unlocked_page_keep_count + _stats.unlocked_page_count - 1) / _stats.unlocked_page_count;
            diag_base::put_any(suborigin, diag::severity::optional, __TAG__, "avg_keep_count=%u", (unsigned)avg_keep_count);

            mapped_page_container::iterator mapped_page_itr = _mapped_pages.begin();
            while (mapped_page_itr != _mapped_pages.end()) {
                if (mapped_page_itr->second.lock_count == 0 && mapped_page_itr->second.keep_count <= avg_keep_count) {
                    mapped_page_itr = unmap_page(mapped_page_itr);
                }
                else {
                    mapped_page_itr++;
                }
            }
        }

        diag_base::ensure(suborigin, _mapped_pages.size() < _config.max_mapped_page_count, __TAG__, "_mapped_pages.size() < _config.max_mapped_page_count, mapped_page_count=%zu, max_mapped_page_count=%zu", _mapped_pages.size(), _config.max_mapped_page_count);

        log_stats();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: count=%zu, max_count=%zu", _mapped_pages.size(), _config.max_mapped_page_count);
    }



#if 0 //// TODO: Remove
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
#endif


    inline void pool::log_stats() noexcept {
        constexpr const char* suborigin = "log_stat()";

        count_t map_count = _stats.map_hit_count + _stats.map_miss_count;
        count_t map_hit_percent = map_count == 0 ? 0 : 100 * _stats.map_hit_count / map_count;
        count_t map_miss_percent = map_count == 0 ? 0 : 100 * _stats.map_miss_count / map_count;

        diag_base::put_any(suborigin, diag::severity::verbose, __TAG__, "Pages: container=%zu, locked=%u, unlocked=%u", _mapped_pages.size(), (unsigned)_stats.locked_page_count, (unsigned)_stats.unlocked_page_count);
        diag_base::put_any(suborigin, diag::severity::verbose, __TAG__, "Map: hit=%u (%u%%), miss=%u (%u%%)", (unsigned)_stats.map_hit_count, (unsigned)map_hit_percent, (unsigned)_stats.map_miss_count, (unsigned)map_miss_percent);
        diag_base::put_any(suborigin, diag::severity::verbose, __TAG__, "Keep: locked=%u, unlocked=%u", (unsigned)_stats.locked_page_keep_count, (unsigned)_stats.unlocked_page_keep_count);
        diag_base::put_any(suborigin, diag::severity::verbose, __TAG__, "Capacity: count=%u", (unsigned)_stats.free_capacity_count);
    }


    // --------------------------------------------------------------


    inline pool_config::pool_config(const char* file_path, std::size_t max_mapped_page_count, bool sync_pages_on_unlock, bool sync_locked_pages_on_destroy)
        : file_path(file_path)
        , max_mapped_page_count(max_mapped_page_count)
        , sync_pages_on_unlock(sync_pages_on_unlock)
        , sync_locked_pages_on_destroy(sync_locked_pages_on_destroy) {
    }


    // --------------------------------------------------------------


#if 0 //// TODO: Remove
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
#endif

} }
