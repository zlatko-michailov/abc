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

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../root/util.h"
#include "../diag/diag_ready.h"
#include "ptr.h"
#include "linked.h"
#include "i/layout.i.h"
#include "i/pool.i.h"


namespace abc { namespace vmem {

    inline constexpr const char* pool::origin() noexcept {
        return "abc::vmem::pool";
    }


    inline constexpr bool pool::is_required_page(page_pos_t page_pos) noexcept {
        return page_pos == page_pos_root || page_pos == page_pos_start;
    }


    inline pool::pool(pool_config&& config, diag::log_ostream* log)
        : diag_base(abc::copy(origin()), log)
        , _config(std::move(config))
        , _ready(false)
        , _fd(-1)
        , _mapped_pages{ }
        , _stats{ } {

        constexpr const char* suborigin = "pool()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a7b, "Begin: file_path='%s', max_mapped_page_count=%zu", _config.file_path.c_str(), _config.max_mapped_page_count);

        diag_base::expect(suborigin, !_config.file_path.empty(), 0x10a7c, "!_config.file_path.empty()");

        bool is_init = open();

        if (!is_init) {
            init();
        }

        verify();
        diag_base::ensure(suborigin, _ready, 0x10a7d, "_ready");

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
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a7e, "Begin: fd=%d, max_mapped_page_count=%zu", _fd, _config.max_mapped_page_count);

        other._ready = false;
        other._fd = -1;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a7f, "End:");
    }


    inline pool::~pool() noexcept {
        constexpr const char* suborigin = "~pool()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a80, "Begin: fd=%d, max_mapped_page_count=%zu", _fd, _config.max_mapped_page_count);

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

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a81, "End:");
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
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a82, "Begin:");

        // IMPORTANT! Keep this order:
        // root  (0)
        // start (1)
        create_root_page();
        create_start_page();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a83, "End:");
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
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a84, "Begin:");

        verify_root_page();
        verify_start_page();

        _ready = true;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a85, "Begin:");
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

        if (_ready) {
            // Get the root page to get the free pages linked state.
            vmem::page page(this, page_pos_root, diag_base::log());
            diag_base::expect(suborigin, page.ptr() != nullptr, 0x10392, "page.ptr() != nullptr");

            vmem::root_page* root_page = reinterpret_cast<vmem::root_page*>(page.ptr());
            vmem::linked free_pages_linked(&root_page->free_pages, this, diag_base::log(), true /*is_free_pages*/);

            if (!free_pages_linked.empty()) {
                diag_base::put_any(suborigin, diag::severity::optional, 0x10393, "!empty");

                page_pos = free_pages_linked.back();
                free_pages_linked.pop_back();

                diag_base::put_any(suborigin, diag::severity::optional, 0x10394, "page_pos=0x%llx", (unsigned long long)page_pos);
            }
        }
        else {
            diag_base::put_any(suborigin, diag::severity::optional, 0x10a86, "!_ready");
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104b4, "End: page_pos=0x%llx", (unsigned long long)page_pos);

        return page_pos;
    }


    inline void pool::push_free_page_pos(page_pos_t page_pos) {
        constexpr const char* suborigin = "push_free_page_pos()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x104b5, "Begin: page_pos=0x%llx", (unsigned long long)page_pos);

        if (_ready) {
            // Get the root page to get the free pages linked state.
            vmem::page page(this, page_pos_root, diag_base::log());
            diag_base::expect(suborigin, page.ptr() != nullptr, 0x1039a, "page.ptr() != nullptr");

            vmem::root_page* root_page = reinterpret_cast<vmem::root_page*>(page.ptr());
            vmem::linked free_pages_linked(&root_page->free_pages, this, diag_base::log(), true /*is_free_pages*/);

            free_pages_linked.push_back(page_pos);
        }
        else {
            diag_base::put_any(suborigin, diag::severity::optional, 0x10a87, "!_ready");
        }

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
        diag_base::ensure(suborigin, wb == page_size, 0x10398, "wb == page_size, wb=%ld, errno=%d", (long)wb, errno);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104b8, "End: page_pos=0x%llx", (unsigned long long)page_pos);

        return page_pos;
    }


    // ..............................................................


    inline void* pool::lock_page(page_pos_t page_pos) {
        constexpr const char* suborigin = "lock_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1039b, "Begin: page_pos=0x%llx", (unsigned long long)page_pos);

        vmem::mapped_page* mapped_page = map_page(page_pos);
        diag_base::ensure(suborigin, mapped_page != nullptr, 0x10a88, "mapped_page != nullptr");

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
        diag_base::expect(suborigin, mapped_page_itr->second.lock_count > 0, 0x10a89, "mapped_page_itr->second.lock_count > 0");
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

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a8a, "End: lock_count=%u", (unsigned)mapped_page_itr->second.lock_count);
    }


    inline vmem::mapped_page* pool::map_page(page_pos_t page_pos) {
        constexpr const char* suborigin = "map_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a8b, "Begin: page_pos=0x%llx", (unsigned long long)page_pos);

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
            diag_base::expect(suborigin, _mapped_pages.size() < _config.max_mapped_page_count, 0x10a8c, "_mapped_pages.size() < _config.max_mapped_page_count, mapped_page_count=%zu, max_mapped_page_count=%zu", _mapped_pages.size(), _config.max_mapped_page_count);

            // Map the OS page.
            off_t page_off = static_cast<off_t>(page_pos * vmem::page_size);
            void* ptr = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, page_off);
            diag_base::ensure(suborigin, ptr != MAP_FAILED, 0x10a8d, "ptr != MAP_FAILED, ptr=%p, errno=%d", ptr, errno);

            // Init a mapped_page entry.
            std::pair<page_pos_t, mapped_page> mapped_page_kvp { };
            mapped_page_kvp.first = page_pos;
            mapped_page_kvp.second.pos = page_pos;
            mapped_page_kvp.second.ptr = ptr;

            std::pair<mapped_page_container::iterator, bool> inserted_mapped_page = _mapped_pages.insert(std::move(mapped_page_kvp));
            diag_base::ensure(suborigin, inserted_mapped_page.second, 0x10a8e, "inserted_mapped_page.second");
            mapped_page_itr = std::move(inserted_mapped_page.first);            

            // There is one more unlocked page in the container.
            _stats.unlocked_page_count++;
        }

        diag_base::ensure(suborigin, mapped_page_itr != _mapped_pages.end(), 0x10a8f, "mapped_page_itr != _mapped_pages.end()");

        log_stats();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a90, "End: mapped_page=%p", &*mapped_page_itr);

        return &mapped_page_itr->second;
    }


    inline pool::mapped_page_container::iterator pool::unmap_page(const mapped_page_container::iterator& mapped_page_itr) {
        constexpr const char* suborigin = "unmap_page()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x103a0, "Begin:");

        diag_base::expect(suborigin, mapped_page_itr != _mapped_pages.end(), 0x10a91, "mapped_page_itr != _mapped_pages.end()");
        diag_base::expect(suborigin, mapped_page_itr->second.ptr != nullptr, 0x10a92, "mapped_page_itr->second.ptr != nullptr");

        if (!_config.sync_pages_on_unlock || (_config.sync_locked_pages_on_destroy && mapped_page_itr->second.lock_count > 0)) {
            // Sync the OS page. 
            int sn = msync(mapped_page_itr->second.ptr, page_size, MS_ASYNC);
            diag_base::ensure(suborigin, sn == 0, 0x10a93, "sn == 0, page_pos=0x%llx, ptr=%p, sn=%d, errno=%d", (unsigned long long)mapped_page_itr->second.pos, mapped_page_itr->second.ptr, sn, errno);
        }

        // Unmap the OS page.
        int um = munmap(mapped_page_itr->second.ptr, page_size);
        diag_base::ensure(suborigin, um == 0, 0x10a94, "um == 0");

        if (mapped_page_itr->second.lock_count > 0) {
            _stats.locked_page_keep_count -= mapped_page_itr->second.keep_count;
            _stats.locked_page_count--;
        }
        else {
            _stats.unlocked_page_keep_count -= mapped_page_itr->second.keep_count;
            _stats.unlocked_page_count--;
        }

        diag_base::put_any(suborigin, diag::severity::optional, 0x10a95, "pos=0x%llx, ptr=%p", (unsigned long long)mapped_page_itr->second.pos, mapped_page_itr->second.ptr);

        // Remove the mapped page entry from the container.
        mapped_page_container::iterator ret_itr = _mapped_pages.erase(mapped_page_itr);

        log_stats();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104c2, "End:");

        return ret_itr;
    }


    inline void pool::ensure_mapping_capacity() {
        constexpr const char* suborigin = "ensure_mapping_capacity()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a96, "Begin: count=%zu, max_count=%zu", _mapped_pages.size(), _config.max_mapped_page_count);

        diag_base::expect(suborigin, _mapped_pages.size() <= _config.max_mapped_page_count, 0x10a97, "_mapped_pages.size() <= _config.max_mapped_page_count, mapped_page_count=%zu, max_mapped_page_count=%zu", _mapped_pages.size(), _config.max_mapped_page_count);

        if (_mapped_pages.size() == _config.max_mapped_page_count) {
            _stats.free_capacity_count++;

            diag_base::put_any(suborigin, diag::severity::verbose, 0x10a98, "Trying to free capacity.");
            log_stats();

            //// TODO: A combination of a container and algorithms should be used so that the following requirements are met:
            // 1. A mapped page entry should not be moved from one container to another when its "locked" state changes.
            // 2. Ditto, especially when an already locked page is being re-locked.
            // 3. No situation should lead to full, sequential, traversal of container.

            // If there are no unlocked pages, then nothing can be freed.
            if (_stats.unlocked_page_count == 0) {
                diag_base::throw_exception<std::runtime_error>(suborigin, 0x10a99, "No mapping capacity. (1) max_page_count=%zu, locked_page_count=%u, unlocked_page_count=%u", _config.max_mapped_page_count, (unsigned)_stats.locked_page_count, (unsigned)_stats.unlocked_page_count);
            }

            // Pass 1: Remove all unlocked pages with a keep count not higher than the average.
            count_t avg_keep_count = (_stats.unlocked_page_keep_count + _stats.unlocked_page_count - 1) / _stats.unlocked_page_count;
            diag_base::put_any(suborigin, diag::severity::optional, 0x10a9a, "avg_keep_count=%u", (unsigned)avg_keep_count);

            mapped_page_container::iterator mapped_page_itr = _mapped_pages.begin();
            while (mapped_page_itr != _mapped_pages.end()) {
                // Don't unmap required pages.
                if (!is_required_page(mapped_page_itr->second.pos) && mapped_page_itr->second.lock_count == 0 && mapped_page_itr->second.keep_count <= avg_keep_count) {
                    mapped_page_itr = unmap_page(mapped_page_itr);
                }
                else {
                    mapped_page_itr++;
                }
            }

            // Pass 2: If Pass 1 didn't free up anything, free all unlocked pages.
            if (_mapped_pages.size() == _config.max_mapped_page_count) {
                mapped_page_itr = _mapped_pages.begin();
                while (mapped_page_itr != _mapped_pages.end()) {
                    // Don't unmap required pages.
                    if (!is_required_page(mapped_page_itr->second.pos) && mapped_page_itr->second.lock_count == 0) {
                        mapped_page_itr = unmap_page(mapped_page_itr);
                    }
                    else {
                        mapped_page_itr++;
                    }
                }
            }

            if (_mapped_pages.size() == _config.max_mapped_page_count) {
                diag_base::throw_exception<std::runtime_error>(suborigin, 0x10a9b, "No mapping capacity. (2) max_page_count=%zu, locked_page_count=%u, unlocked_page_count=%u", _config.max_mapped_page_count, (unsigned)_stats.locked_page_count, (unsigned)_stats.unlocked_page_count);
            }
        }

        diag_base::ensure(suborigin, _mapped_pages.size() < _config.max_mapped_page_count, 0x10a9c, "_mapped_pages.size() < _config.max_mapped_page_count, mapped_page_count=%zu, max_mapped_page_count=%zu", _mapped_pages.size(), _config.max_mapped_page_count);

        log_stats();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10a9d, "End: count=%zu, max_count=%zu", _mapped_pages.size(), _config.max_mapped_page_count);
    }


    inline void pool::clear_linked(linked& linked) {
        constexpr const char* suborigin = "clear_linked()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x104c3, "Begin:");

        vmem::page root_page(this, page_pos_root, diag_base::log());
        diag_base::expect(suborigin, root_page.ptr() != nullptr, 0x104c4, "root_page.ptr() != nullptr");

        vmem::root_page* root_linked_page = reinterpret_cast<vmem::root_page*>(root_page.ptr());
        vmem::linked free_pages_linked(&root_linked_page->free_pages, this, diag_base::log(), true /*is_free_pages*/);
        free_pages_linked.splice(linked);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x104c5, "End:");
    }


    inline void pool::log_stats() noexcept {
        constexpr const char* suborigin = "log_stat()";

        count_t map_count = _stats.map_hit_count + _stats.map_miss_count;
        count_t map_hit_percent = map_count == 0 ? 0 : 100 * _stats.map_hit_count / map_count;
        count_t map_miss_percent = map_count == 0 ? 0 : 100 * _stats.map_miss_count / map_count;

        diag_base::put_any(suborigin, diag::severity::verbose, 0x10a9e, "Pages: container=%zu, locked=%u, unlocked=%u", _mapped_pages.size(), (unsigned)_stats.locked_page_count, (unsigned)_stats.unlocked_page_count);
        diag_base::put_any(suborigin, diag::severity::verbose, 0x10a9f, "Map: hit=%u (%u%%), miss=%u (%u%%)", (unsigned)_stats.map_hit_count, (unsigned)map_hit_percent, (unsigned)_stats.map_miss_count, (unsigned)map_miss_percent);
        diag_base::put_any(suborigin, diag::severity::verbose, 0x10aa0, "Keep: locked=%u, unlocked=%u", (unsigned)_stats.locked_page_keep_count, (unsigned)_stats.unlocked_page_keep_count);
        diag_base::put_any(suborigin, diag::severity::verbose, 0x10aa1, "Capacity: count=%u", (unsigned)_stats.free_capacity_count);
    }


    // --------------------------------------------------------------


    inline pool_config::pool_config(const char* file_path, std::size_t max_mapped_page_count, bool sync_pages_on_unlock, bool sync_locked_pages_on_destroy)
        : file_path(file_path)
        , max_mapped_page_count(max_mapped_page_count)
        , sync_pages_on_unlock(sync_pages_on_unlock)
        , sync_locked_pages_on_destroy(sync_locked_pages_on_destroy) {
    }


    // --------------------------------------------------------------

} }
