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

#include <unordered_map>

#include "../../root/size.h"
#include "../../diag/i/diag_ready.i.h"
#include "page.i.h"


namespace abc { namespace vmem {

    /**
     * @brief `pool` settings.
     */
    struct pool_config {

        /**
         * @brief                              Constructor. Properties can only be set at construction.
         * @param file_path                    Path to the pool file.
         * @param max_mapped_page_count        Maximum number of mapped pages at the same time. Default: `abc::size::max`, i.e. no limit.
         * @param sync_pages_on_unlock         When `true`, pages get synced to disk when their lock count drops to `0`. Default: `false`.
         * @param sync_locked_pages_on_destroy When `true`, locked pages get synced to disk when the pool is destroyed. Default: `false`.
         */
        pool_config(const char* file_path, std::size_t max_mapped_page_count = size::max, bool sync_pages_on_unlock = false, bool sync_locked_pages_on_destroy = false);

        /**
         * @brief Path to the pool file.
         */
        const std::string file_path;

        /**
         * @brief   Maximum number of mapped pages at the same time.
         * @details Limits the maximum physical memory the pool can use.
         */
        const std::size_t max_mapped_page_count;

        /**
         * @brief   When `true`, pages get synced to disk when their lock count drops to `0`. Otherwise, pages get synced to disk only when unmapped.
         * @details `true` improves performance at the risk of losing data in case of a process crash.
         */
        const bool sync_pages_on_unlock;

        /**
         * @brief   When `true`, locked pages get synced to disk when the pool is destroyed.
         * @details Having locked pages when the pool is destroyed is a program error. Either way could lead to a loss of data integrity.
         */
        const bool sync_locked_pages_on_destroy;
    };


    // --------------------------------------------------------------


    /**
     * @brief Pool performance stats.
     */
    struct pool_stats {
        count_t map_hit_count;
        count_t map_miss_count;

        count_t locked_page_count;
        count_t locked_page_keep_count;

        count_t unlocked_page_count;
        count_t unlocked_page_keep_count;

        count_t free_capacity_count;
    };


    // --------------------------------------------------------------


    class linked;


    // --------------------------------------------------------------


    /**
     * @brief   Virtual memory (vmem) pool.
     * @details Every pool is persisted to a file, and thus could be reopened later.
     */
    class pool
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;
        using mapped_page_container = std::unordered_map<page_pos_t, mapped_page>;

    private:
        static constexpr const char* origin() noexcept;

    public:
        /**
         * @brief          Returns `true` if the page at the given position is required for the pool to function properly.
         * @param page_pos Page position.
         */
        static constexpr bool is_required_page(page_pos_t page_pos) noexcept;

    public:
        /**
         * @brief        Constructor.
         * @param config `pool_config` instance.
         * @param log    Pointer to a `log_ostream` instance.
         */
        pool(pool_config&& config, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        pool(pool&& other) noexcept;

        /**
         * @brief Deleted.
         */
        pool(const pool& other) = delete;

        /**
         * @brief Destructor.
         */
        ~pool() noexcept;

    public:
        const pool_config& config() const noexcept;

    private:
        friend page;

        /**
         * @brief   Allocates a page.
         * @details Tries to reuse a free page if there are any. Otherwise, adds a new page to the pool. 
         * @return  The position of the allocated page. 
         */
        page_pos_t alloc_page();

        /**
         * @brief          Frees a page by adding it to the list of free pages.
         * @param page_pos Page position.
         */
        void free_page(page_pos_t page_pos);

        /**
         * @brief          Locks a page in memory
         * @details        If the page is not mapped, maps it. In either case, increments the page's lock count.
         *                 Once a page is locked, its contents can be addressed by regular pointers.
         * @param page_pos Page position.
         * @return         Pointer to the first byte of the page.
         */
        void* lock_page(page_pos_t page_pos);

        /**
         * @brief          Unlocks a page in memory.
         * @details        As a page may be locked multiple times, decrements the page's lock count.
         *                 Once the last lock is removed, the contents of the page can no longer be addressed by regular pointers.
         *                 The page may or may not be unmapped.
         * @param page_pos Page position.
         */
        void unlock_page(page_pos_t page_pos);

    private:
        friend linked;

        /**
         * @brief        Frees up all pages of the `linked` struct at once.
         * @param linked A `linked` instance.
         */
        void clear_linked(linked& linked);


    // Constructor helpers
    private:
        /**
         * @brief  Opens the pool file, and verifies its essential pages. An empty file is acceptable.
         * @return `true` = the pool file is already initialized; `false` = the pool file needs initialization.
         */
        bool open();

        /**
         * @brief Initializes an empty pool file.
         */
        void init();

        /**
         * @brief Creates the root page on an empty pool file.
         */
        void create_root_page();

        /**
         * @briefCreates the start page on an empty pool file.
         */
        void create_start_page();

        /**
         * @brief Verifies the essential pages on an existing pool file.
         */
        void verify();

        /**
         * @brief Verifies the root page on an existing pool file.
         */
        void verify_root_page();

        /**
         * @brief Verifies the start page on an existing pool file.
         */
        void verify_start_page();


    // alloc_page() / free_page() helpers
    private:
        /**
         * @brief  Pops a free page from the pool's list of free pages, and returns its position.
         * @return The position of the popped page.
         */
        page_pos_t pop_free_page_pos();

        /**
         * @brief          Pushes a page to the pool's list of free pages.
         * @param page_pos Page position.
         */
        void push_free_page_pos(page_pos_t page_pos);

        /**
         * @brief  Unconditionally creates a new page on the pool file, and returns its position.
         * @return The position of the new page.
         */
        page_pos_t create_page();


    // lock_page() / unlock_page() helpers
    private:
        /**
         * @brief          Ensures a page is mapped in memory
         * @details        If the page is already mapped, it simply returns a pointer to the entry.
         *                 Does not modify the lock count of the page.
         * @param page_pos Page position.
         * @return         Pointer to the `mapped_page` entry.
         */
        vmem::mapped_page* map_page(page_pos_t page_pos);

        /**
         * @brief                 Unconditionally unmaps a mapped page.
         * @param mapped_page_itr Iterator on the mapped pages container.
         * @return                An iterator pointing "after" the erased item.
         */
        mapped_page_container::iterator unmap_page(const mapped_page_container::iterator& mapped_page_itr);

        /**
         * @brief Ensures that `_mapped_page_count` is less than `_max_mapped_page_count`, so that a new page can be mapped.
         */
        void ensure_mapping_capacity();

        /**
         * @brief Logs performance stats.
         */
        void log_stats() noexcept;

    private:
        /**
         * @brief The config settings passed in to the constructor.
         */
        pool_config _config;

        /**
         * @brief Whether this instance is properly initialized.
         */
        bool _ready;

        /**
         * @brief Descriptor of the pool file.
         */
        int _fd;

        /**
         * @brief Mapped page container.
         */
        mapped_page_container _mapped_pages;

        /**
         * @brief Perf stats.
         */
        pool_stats _stats;
    };


    // --------------------------------------------------------------

} }
