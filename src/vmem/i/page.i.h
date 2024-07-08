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

#include "../../diag/i/diag_ready.i.h"
#include "base.i.h"


namespace abc { namespace vmem {

    // --------------------------------------------------------------


    class pool;


    // --------------------------------------------------------------


    /**
     * @brief Virtual memory (vmem) page.
     */
    class page
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    private:
        static constexpr const char* origin() noexcept;

    public:
        /**
         * @brief      Constructor.
         * @details    Maps a free page, if there is one. If there are no free pages, allocates a new page at the end of the pool. Locks the page.
         * @param pool Pointer to a `pool` instance.
         * @param log  Pointer to a `log_ostream` instance.
         */
        page(vmem::pool* pool, diag::log_ostream* log = nullptr);

        /**
         * @brief      Constructor.
         * @details    Maps and locks a specific page.
         * @param pool Pointer to a pool instance.
         * @param pos  Page position. If `page_pos_nil`, a free/new page is mapped.
         * @param log  Pointer to a `log_ostream` instance.
         */
        page(vmem::pool* pool, page_pos_t pos, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        page(page&& other) noexcept;

        /**
         * @brief   Copy constructor.
         * @details A page is locked by each instance that references it by incrementing its lock count.
         */
        page(const page& other) noexcept;

        /**
         * @brief   Constructor.
         * @details Constructs an invalid page. Does not map any page.
         */
        page(std::nullptr_t) noexcept;

        /**
         * @brief   Destructor.
         * @details Decrements the lock count of the page.
         */
        ~page() noexcept;

    public:
        page& operator =(page&& other) noexcept;
        page& operator =(const page& other) noexcept;

    public:
        /**
         * @brief Returns the pointer to the Pool instance passed in to the constructor.
         */
        vmem::pool* pool() const noexcept;

        /**
         * @brief Returns the page's position in the pool.
         */
        page_pos_t pos() const noexcept;

        /**
         * @brief Returns a raw pointer to the page's mapped area in memory.
         */
        void* ptr() noexcept;

        /**
         * @brief Returns a `const` raw pointer to the page's mapped area in memory.
         */
        const void* ptr() const noexcept;

    public:
        /**
         * @brief   Frees the page.
         * @details Adds the page to the list of free pages.
         */
        void free() noexcept;

    private:
        /**
         * @brief  Allocates a pool page for this instance.
         */
        void alloc();

        /**
         * @brief  Tries to allocate a pool page for this instance.
         * @return `true` = success; `false` = error.
         */
        bool try_alloc() noexcept; //// TODO: Is this needed?

        /**
         * @brief   Locks this page in memory.
         * @details A page's pointer may be used only after the page has been locked.
         *          A page may be called multiple times. It gets unlocked after the lock count goes down to `0`.
         */
        void lock();

        /**
         * @brief   Tries to lock this page in memory.
         * @details A page's pointer may be used only after the page has been locked.
         *          A page may be called multiple times. It gets unlocked after the lock count goes down to `0`.
         * @return  `true` = success; `false` = error.
         */
        bool try_lock() noexcept; //// TODO: Is this needed?

        /**
         * @brief   Unlocks this page.
         * @details Decrements the page's lock count. When the lock count drops down to `0`, the page's content is sync'd to the disk, and is no longer valid.
         */
        void unlock() noexcept;

        /**
         * @brief   Unconditionally invalidates the page.
         * @details If the instance had associated resources, they will remain orphan.
         */
        void invalidate() noexcept;

    protected:
        vmem::pool* _pool;
        page_pos_t  _pos;
        void*       _ptr;
    };


    // --------------------------------------------------------------

} }
