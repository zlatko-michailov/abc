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

#include "../../size.h"
#include "../../diag/i/diag_ready.i.h"
#include "page.i.h"


namespace abc { namespace vmem {

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
        using mapped_pages_container = std::unordered_map<page_pos_t, mapped_page>;

        static constexpr const char* _origin = "abc::vmem::pool";

    public:
        /**
         * @brief                       Constructor.
         * @param file_path             Path to the pool file.
         * @param max_mapped_page_count Maximum number of mapped pages at the same time.
         * @param log                   Pointer to a `log_ostream` instance.
         */
        pool(const char* file_path, std::size_t max_mapped_page_count = size::max, diag::log_ostream* log = nullptr);

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
         * @param linked Pointer to a `linked` instance.
         */
        void clear_linked(linked* linked);


    // Constructor helpers
    private:
        /**
         * @brief           Opens the pool file, and verifies its essential pages. An empty file is acceptable.
         * @param file_path Pool file path.
         * @return          `true` = the pool file is already initialized; `false` = the pool file needs initialization.
         */
        bool open(const char* file_path);

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
         * @brief          Tries to find a mapped page.
         * @param page_pos Page position.
         * @return         The position of the page on the array of mapped pages if found; `size::invalid` if not found. 
         */
        std::size_t find_mapped_page(page_pos_t page_pos) noexcept;

        /**
         * @brief Checks whether there is capacity for at least one more page on the array of mapped pages.
         */
        bool has_mapping_capacity() noexcept;

        /**
         * @brief   Unmaps unlocked mapped pages.
         * @details First tries to unmap unlocked mapped pages with a keep count below the current average.
         *          If no page gets unmapped, tries to unmap all unlocked mapped pages.
         * @return  The count pages that have been unmapped.
         */
        std::size_t make_mapping_capacity();

        /**
         * @brief                Unmaps unlocked mapped pages with a keep count below the given minimum.
         * @param min_keep_count Minimum keep count for an unlocked mapped page to be kept.
         * @return               The count of pages that have been unmapped.
         */
        std::size_t make_mapping_capacity(count_t min_keep_count);

        /**
         * @brief                Checks weather an unlocked mapped page meets the required minimum keep count.
         * @param i              Position on the array of mapped pages.
         * @param min_keep_count Minimum keep count.
         */
        bool should_keep_mapped_page(std::size_t i, count_t min_keep_count) noexcept;

        /**
         * @brief                Keeps an unlocked mapped page.
         * @details              Subtracts the minimum keep count from the mapped page's keep count, so that eventually the page would get unmapped after a few rounds.
         *                       Separately, if a valid position of an empty slot is provided, the kept page is moved to that slot, and this slot becomes the new empty one.
         *                       This process bubbles empty slots to the end of the array of mapped pages.
         * @param i              Position on the array of mapped pages.
         * @param min_keep_count Minimum keep count.
         * @param empty_i        Input/output. The lowest position of an empty item on the array of mapped pages.
         */
        void keep_mapped_page(std::size_t i, count_t min_keep_count, std::size_t& empty_i);

        /**
         * @brief                Unconditionally unmaps a mapped page.
         * @param i              Position on the array of mapped pages.
         * @param min_keep_count Minimum keep count.
         * @param empty_i        Output. If this is the first unmapped page, this argument is set to `i`.
         * @param unmapped_count Input/output. Current count of unmapped pages. This counter gets incremented once.
         */
        void unmap_mapped_page(std::size_t i, count_t min_keep_count, std::size_t& empty_i, std::size_t& unmapped_count);

        /**
         * @brief   Locks a mapped page, and returns a pointer to the page's content.
         * @details Increments both the lock count and the keep count of the mapped page.
         * @param i Position on the array of mapped pages.
         * @return  Pointer to the beginning of the page.
         */
        void* lock_mapped_page(std::size_t i) noexcept;

        /**
         * @brief   Unlocks a mapped page.
         * @details Decrements the lock count of the mapped page.
         *          If the lock count becomes zero, i.e. if all locks have been released, the page is synced to the file.
         * @param i Position on the array of mapped pages.
         */
        void unlock_mapped_page(std::size_t i);

        /**
         * @brief          Maps and locks a page in memory.
         * @param i        Position on the array of mapped pages.
         * @param page_pos Page position.
         * @return         Pointer to the beginning of the page.
         */
        void* map_new_page(std::size_t i, page_pos_t page_pos);

        /**
         * @brief   Swaps a mapped page with another one that has a lower position on the array of mapped pages and a lower keep count.
         * @details This process keeps mapped pages with high keep counts, i.e. frequently used pages, closer to the beginning of the array,
         *          so that they can be found using fewer comparisons.
         * @param i Position on the array of mapped pages.
         */
        void optimize_mapped_page(std::size_t i) noexcept;

        /**
         * @brief         Tries to find an empty slot on the array of mapped pages before a given position `i`.
         * @param i       Position on the array of mapped pages.
         * @param empty_i Lower bound (exclusive) for the sought empty position.
         * @return        The lowest position of an empty slot on the array of mapped pages, or `i` if no such empty slot exists.
         */
        std::size_t next_empty_i(std::size_t i, std::size_t empty_i) noexcept;

        /**
         * @brief Logs performance stats.
         */
        void log_stats() noexcept;

    private:
        bool                   _ready;
        int                    _fd;
        const std::size_t      _max_mapped_page_count;
        std::size_t            _mapped_page_count;
        mapped_pages_container _mapped_pages;
        pool_stats             _stats;
    };


    // --------------------------------------------------------------

} }
