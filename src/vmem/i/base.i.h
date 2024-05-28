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

#include <cstdint>
#include <climits>

#include "../../size.h"


namespace abc { namespace vmem {

    using page_pos_t = std::uint64_t;
    using item_pos_t = std::uint16_t;
    using version_t  = std::uint16_t;
    using count_t    = std::uint32_t;


    constexpr std::size_t page_size        = size::k4;
    constexpr page_pos_t  page_pos_root    = 0;
    constexpr page_pos_t  page_pos_start   = 1;
    constexpr page_pos_t  page_pos_nil     = static_cast<page_pos_t>(ULLONG_MAX);
    constexpr item_pos_t  item_pos_nil     = static_cast<item_pos_t>(USHRT_MAX);
    constexpr std::size_t min_mapped_pages = 3;


    // --------------------------------------------------------------


    /**
     * @brief Information about a mapped vmem page.
     */
    struct mapped_page {
        page_pos_t pos;
        void*      ptr;
        count_t    lock_count;
        count_t    keep_count;
    };


    /**
     * @brief Performance counters of a vmem pool.
     */
    struct pool_stats {
        count_t keep_count;
        count_t hit_count;
        count_t miss_count;
        count_t unlock_count;
        count_t check_count;
        count_t unmap_count;
    };


    // --------------------------------------------------------------

} }
