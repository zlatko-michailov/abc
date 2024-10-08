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


#include <array>

#include "inc/vmem.h"


static constexpr const char origin[] = "";

constexpr std::size_t max_mapped_page_count_min    = 4;
constexpr std::size_t max_mapped_page_count_fit    = 6;
constexpr std::size_t max_mapped_page_count_exceed = 5;
constexpr std::size_t max_mapped_page_count_free   = 6;
constexpr std::size_t max_mapped_page_count_linked = 5;
constexpr std::size_t max_mapped_page_count_list   = 5;
constexpr std::size_t max_mapped_page_count_map    = 5;

using LinkedPageData = unsigned long long;
struct LinkedPage : abc::vmem::linked_page {
    LinkedPageData                    data;
};

struct ItemMany {
    std::uint64_t                 data;
    std::array<std::uint8_t, 900> dummy;
};

struct Key {
    std::uint64_t                 data;
    std::array<std::uint8_t, 900> dummy;

    bool operator < (const Key& other) const noexcept {
        return data < other.data;
    }

    bool operator <= (const Key& other) const noexcept {
        return data <= other.data;
    }

    bool operator > (const Key& other) const noexcept {
        return data > other.data;
    }

    bool operator >= (const Key& other) const noexcept {
        return data >= other.data;
    }

    bool operator == (const Key& other) const noexcept {
        return data == other.data;
    }
};
using Value = std::uint64_t;
#if 0 //// TODO: TEMP
using MapItem = vmem_map_value<Key, Value>;
#endif


bool insert_list_items(test_context& context, abc::vmem::list<ItemMany>& list, std::size_t count);

template <typename Map>
bool insert_map_items(test_context& context, Map& map, std::size_t count);

bool insert_linked_page(test_context& context, abc::vmem::pool* pool, abc::vmem::linked& linked, abc::vmem::page_pos_t expected_page_pos, LinkedPageData data,
                        const abc::vmem::linked::const_iterator& itr, const abc::vmem::linked::const_iterator& expected_itr,
                        abc::vmem::linked::const_iterator& actual_itr);

bool verify_linked_pages(test_context& context, abc::vmem::pool* pool, abc::vmem::linked& linked,
                        const std::pair<LinkedPageData, abc::vmem::linked::const_iterator> expected[], std::size_t expected_len);

bool verify_bytes(test_context& context, const void* buffer, std::size_t begin_pos, std::size_t end_pos, std::uint8_t b, abc::diag::tag_t tag);

bool create_vmem_pool(test_context& context, abc::vmem::pool* pool, bool fit);


bool test_vmem_pool_fit(test_context& context) {
    bool passed = true;

    abc::vmem::pool_config config("out/test/pool_fit.vmem", max_mapped_page_count_fit);
    abc::vmem::pool pool(std::move(config), context.log());

    passed = create_vmem_pool(context, &pool, true) && passed;

    return passed;
}


bool test_vmem_pool_exceed(test_context& context) {
    bool passed = true;

    abc::vmem::pool_config config("out/test/pool_exceed.vmem", max_mapped_page_count_exceed);
    abc::vmem::pool pool(std::move(config), context.log());

    passed = create_vmem_pool(context, &pool, false) && passed;

    return passed;
}


bool test_vmem_pool_reopen(test_context& context) {
    bool passed = true;

    {
        abc::vmem::pool_config config("out/test/pool_reopen.vmem", max_mapped_page_count_fit);
        abc::vmem::pool pool(std::move(config), context.log());

        passed = create_vmem_pool(context, &pool, true) && passed;
    }

    abc::vmem::pool_config config("out/test/pool_reopen.vmem", max_mapped_page_count_fit);
    abc::vmem::pool pool(std::move(config), context.log());

    // Page 0 (root page)
    {
        abc::vmem::page page(&pool, 0UL, context.log());

        abc::vmem::root_page expected;
        int cmp = std::memcmp(&expected, page.ptr(), sizeof(abc::vmem::root_page));
        passed = context.are_equal<int>(cmp, 0, 0x103bd, "%d") && passed;

        passed = verify_bytes(context, page.ptr(), sizeof(abc::vmem::root_page), abc::vmem::page_size, 0x00, 0x104c6) && passed;
    }

    // Page 1 (start page)
    {
        abc::vmem::page page(&pool, 1UL, context.log());
        passed = verify_bytes(context, page.ptr(), sizeof(abc::vmem::root_page), abc::vmem::page_size, 0x00, 0x104c7) && passed;
    }

    // Page 2
    {
        abc::vmem::page page(&pool, 2UL, context.log());
        passed = verify_bytes(context, page.ptr(), 0, abc::vmem::page_size, 0x22, 0x104c8) && passed;
    }

    // Page 3
    {
        abc::vmem::page page(&pool, 3UL, context.log());
        passed = verify_bytes(context, page.ptr(), 0, abc::vmem::page_size, 0x33, 0x104c9) && passed;
    }

    // Page 4
    {
        abc::vmem::page page(&pool, 4UL, context.log());
        passed = verify_bytes(context, page.ptr(), 0, abc::vmem::page_size, 0x44, 0x104ca) && passed;
    }

    // Page 5
    {
        abc::vmem::page page(&pool, 5UL, context.log());
        passed = verify_bytes(context, page.ptr(), 0, abc::vmem::page_size, 0x55, 0x104cb) && passed;
    }

    return passed;
}


bool test_vmem_pool_freepages(test_context& context) {
    bool passed = true;

    abc::vmem::pool_config config("out/test/pool_freepages.vmem", max_mapped_page_count_free);
    abc::vmem::pool pool(std::move(config), context.log());

    {
        // Page 2
        abc::vmem::page page2(&pool, context.log());
        passed = context.are_equal(page2.ptr() != nullptr, true, 0x103be, "%d") && passed;
        passed = context.are_equal((long long)page2.pos(), 2LL, 0x103bf, "0x%llx") && passed;

        // Page 3
        abc::vmem::page page3(&pool, context.log());
        passed = context.are_equal(page3.ptr() != nullptr, true, 0x103c0, "%d") && passed;
        passed = context.are_equal((long long)page3.pos(), 3LL, 0x103c1, "0x%llx") && passed;

        // Page 4
        abc::vmem::page page4(&pool, context.log());
        passed = context.are_equal(page4.ptr() != nullptr, true, 0x103c2, "%d") && passed;
        passed = context.are_equal((long long)page4.pos(), 4LL, 0x103c3, "0x%llx") && passed;

        // Page 5
        abc::vmem::page page5(&pool, context.log());
        passed = context.are_equal(page5.ptr() != nullptr, true, 0x103c4, "%d") && passed;
        passed = context.are_equal((long long)page5.pos(), 5LL, 0x103c5, "0x%llx") && passed;

        page2.free();
        page3.free();
        page4.free();
        page5.free();
    }

    {
        // Page 5
        abc::vmem::page page5(&pool, context.log());
        passed = context.are_equal(page5.ptr() != nullptr, true, 0x103c6, "%d") && passed;
        passed = context.are_equal((long long)page5.pos(), 5LL, 0x103c7, "0x%llx") && passed;

        // Page 4
        abc::vmem::page page4(&pool, context.log());
        passed = context.are_equal(page4.ptr() != nullptr, true, 0x103c8, "%d") && passed;
        passed = context.are_equal((long long)page4.pos(), 4LL, 0x103c9, "0x%llx") && passed;

        // Page 3
        abc::vmem::page page3(&pool, context.log());
        passed = context.are_equal(page3.ptr() != nullptr, true, 0x103ca, "%d") && passed;
        passed = context.are_equal((long long)page3.pos(), 3LL, 0x103cb, "0x%llx") && passed;

        // Page 2
        abc::vmem::page page2(&pool, context.log());
        passed = context.are_equal(page2.ptr() != nullptr, true, 0x103cc, "%d") && passed;
        passed = context.are_equal((long long)page2.pos(), 2LL, 0x103cd, "0x%llx") && passed;
    }

    return passed;
}


bool test_vmem_linked_mixedone(test_context& context) {
    bool passed = true;

    abc::vmem::pool_config config("out/test/linked_mixedone.vmem", max_mapped_page_count_min);
    abc::vmem::pool pool(std::move(config), context.log());

    abc::vmem::linked_state linked_state;
    abc::vmem::linked linked(&linked_state, &pool, context.log());

    // Allocate and insert
    {
        // Page 2
        abc::vmem::linked::const_iterator actual_itr = linked.end();
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&linked, 2U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        passed = insert_linked_page(context, &pool, linked, 2U, 0x0052, linked.end(), expected_itr, actual_itr) && passed;
        passed = context.are_equal(actual_itr == linked.cbegin(), true, 0x104cc, "%d") && passed;
        passed = context.are_equal(actual_itr == linked.crend(), true, 0x104cd, "%d") && passed;
    }

    // Iterate
    {
        using Pair = std::pair<unsigned long long, abc::vmem::linked::const_iterator>;
        const Pair expected[] = {
            { 0x0052ULL, abc::vmem::linked::const_iterator(&linked, 2U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log()) },
        };
        constexpr std::size_t expected_len = sizeof(expected) / sizeof(Pair); 

        passed = verify_linked_pages(context, &pool, linked, expected, expected_len) && passed;
    }

    // Erase
    {
        abc::vmem::linked::const_iterator expected_itr = linked.end();
        abc::vmem::linked::const_iterator actual_itr = linked.erase(linked.begin());
        passed = context.are_equal(actual_itr == expected_itr, true, 0x104ce, "%d") && passed;
    }

    // Iterate
    {
        passed = context.are_equal(linked.cbegin() == linked.cend(), true, 0x104cf, "%d") && passed;
    }

    // Allocate again
    {
        // Page 2
        abc::vmem::page page2(&pool, context.log());
        LinkedPage* linked_page2 = reinterpret_cast<LinkedPage*>(page2.ptr());
        passed = context.are_equal(linked_page2 != nullptr, true, 0x104d0, "%d") && passed;
        passed = context.are_equal((long long)page2.pos(), 2LL, 0x104d1, "0x%llx") && passed;
    }

    return passed;
}


bool test_vmem_linked_mixedmany(test_context& context) {
    bool passed = true;

    abc::vmem::pool_config config("out/test/linked_mixedmany.vmem", max_mapped_page_count_linked);
    abc::vmem::pool pool(std::move(config), context.log());

    abc::vmem::linked_state linked_state;
    abc::vmem::linked linked(&linked_state, &pool, context.log());

    // Insert four pages
    {
        // Page 2
        abc::vmem::linked::const_iterator actual_itr = linked.end();
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&linked, 2U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        passed = insert_linked_page(context, &pool, linked, 2U, 0x0062, linked.begin(), expected_itr, actual_itr) && passed;
        passed = context.are_equal(actual_itr == linked.cbegin(), true, 0x104d2, "%d") && passed;
        passed = context.are_equal(actual_itr == linked.crend(), true, 0x104d3, "%d") && passed;
    }
    // 2

    {
        // Page 3
        abc::vmem::linked::const_iterator actual_itr = linked.end();
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&linked, 3U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        passed = insert_linked_page(context, &pool, linked, 3U, 0x0063, linked.end(), expected_itr, actual_itr) && passed;
        passed = context.are_equal(actual_itr == linked.crend(), true, 0x104d4, "%d") && passed;
    }
    // 2 3

    {
        // Page 4
        abc::vmem::linked::const_iterator actual_itr = linked.end();
        abc::vmem::linked::const_iterator itr = abc::vmem::linked::const_iterator(&linked, 3U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&linked, 4U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        passed = insert_linked_page(context, &pool, linked, 4U, 0x0064, itr, expected_itr, actual_itr) && passed;
        passed = context.are_equal(actual_itr == expected_itr, true, 0x104d5, "%d") && passed;
    }
    // 2 4 3

    {
        // Page 5
        abc::vmem::linked::const_iterator actual_itr = linked.end();
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&linked, 5U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        passed = insert_linked_page(context, &pool, linked, 5U, 0x0065, linked.begin(), expected_itr, actual_itr) && passed;
        passed = context.are_equal(actual_itr == linked.cbegin(), true, 0x104d6, "%d") && passed;
    }
    // 5 2 4 3

    // Iterate after all inserts
    {
        using Pair = std::pair<LinkedPageData, abc::vmem::linked::const_iterator>;
        const Pair expected[] = {
            { 0x0065ULL, abc::vmem::linked::const_iterator(&linked, 5U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log()) },
            { 0x0062ULL, abc::vmem::linked::const_iterator(&linked, 2U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log()) },
            { 0x0064ULL, abc::vmem::linked::const_iterator(&linked, 4U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log()) },
            { 0x0063ULL, abc::vmem::linked::const_iterator(&linked, 3U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log()) },
        };
        constexpr std::size_t expected_len = sizeof(expected) / sizeof(Pair); 

        passed = verify_linked_pages(context, &pool, linked, expected, expected_len) && passed;
    }

    // 5 2 4 3
    // Erase

    {
        // erase(middle)
        abc::vmem::linked::const_iterator itr = abc::vmem::linked::const_iterator(&linked, 2U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&linked, 4U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        abc::vmem::linked::const_iterator actual_itr = linked.erase(itr);
        passed = context.are_equal(actual_itr == expected_itr, true, 0x104d7, "%d") && passed;
    }
    // 5 4 3

    {
        // erase(begin)
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&linked, 4U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        abc::vmem::linked::const_iterator actual_itr = linked.erase(linked.begin());
        passed = context.are_equal(actual_itr == expected_itr, true, 0x104d8, "%d") && passed;
        passed = context.are_equal(actual_itr == linked.cbegin(), true, 0x104d9, "%d") && passed;
    }
    // 4 3

    {
        // erase(rend)
        abc::vmem::linked::const_iterator actual_itr = linked.erase(linked.rend());
        abc::vmem::linked::const_iterator expected_itr = linked.end();
        passed = context.are_equal(actual_itr == expected_itr, true, 0x104da, "%d") && passed;
        passed = context.are_equal(actual_itr == linked.cend(), true, 0x104db, "%d") && passed;
    }
    // 4

    // Iterate after all erases
    {
        using Pair = std::pair<LinkedPageData, abc::vmem::linked::const_iterator>;
        const Pair expected[] = {
            { 0x0064ULL, abc::vmem::linked::const_iterator(&linked, 4U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log()) },
        };
        constexpr std::size_t expected_len = sizeof(expected) / sizeof(Pair); 

        passed = verify_linked_pages(context, &pool, linked, expected, expected_len) && passed;
    }

    // Allocate again
    {
        // Page 3
        abc::vmem::page page3(&pool, context.log());
        LinkedPage* linked_page3 = reinterpret_cast<LinkedPage*>(page3.ptr());
        passed = context.are_equal(linked_page3 != nullptr, true, 0x104dc, "%d") && passed;
        passed = context.are_equal((long long)page3.pos(), 3LL, 0x104dd, "0x%llx") && passed;
    }

    {
        // Page 5
        abc::vmem::page page5(&pool, context.log());
        LinkedPage* linked_page5 = reinterpret_cast<LinkedPage*>(page5.ptr());
        passed = context.are_equal(linked_page5 != nullptr, true, 0x104de, "%d") && passed;
        passed = context.are_equal((long long)page5.pos(), 5LL, 0x104df, "0x%llx") && passed;
    }

    {
        // Page 2
        abc::vmem::page page2(&pool, context.log());
        LinkedPage* linked_page2 = reinterpret_cast<LinkedPage*>(page2.ptr());
        passed = context.are_equal(linked_page2 != nullptr, true, 0x104e0, "%d") && passed;
        passed = context.are_equal((long long)page2.pos(), 2LL, 0x104e1, "0x%llx") && passed;
    }

    return passed;
}


bool test_vmem_linked_splice(test_context& context) {
    bool passed = true;

    abc::vmem::pool_config config("out/test/linked_splice.vmem", max_mapped_page_count_min);
    abc::vmem::pool pool(std::move(config), context.log());

    abc::vmem::linked_state linked_state;
    abc::vmem::linked linked(&linked_state, &pool, context.log());

    // Insert three pages
    {
        // Page 2
        abc::vmem::linked::const_iterator actual_itr = linked.end();
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&linked, 2U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        passed = insert_linked_page(context, &pool, linked, 2U, 0x0062, linked.begin(), expected_itr, actual_itr) && passed;
        passed = context.are_equal(actual_itr == linked.cbegin(), true, 0x104e2, "%d") && passed;
        passed = context.are_equal(actual_itr == linked.crend(), true, 0x104e3, "%d") && passed;
    }
    // 2

    {
        // Page 3
        abc::vmem::linked::const_iterator actual_itr = linked.end();
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&linked, 3U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        passed = insert_linked_page(context, &pool, linked, 3U, 0x0063, linked.begin(), expected_itr, actual_itr) && passed;
        passed = context.are_equal(actual_itr == linked.cbegin(), true, 0x104e4, "%d") && passed;
    }
    // 3 2

    {
        // Page 4
        abc::vmem::linked::const_iterator actual_itr = linked.end();
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&linked, 4U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        passed = insert_linked_page(context, &pool, linked, 4U, 0x0064, linked.begin(), expected_itr, actual_itr) && passed;
        passed = context.are_equal(actual_itr == linked.cbegin(), true, 0x104e5, "%d") && passed;
    }
    // 4 3 2

    abc::vmem::linked_state other_linked_state;
    abc::vmem::linked other_linked(&other_linked_state, &pool, context.log());

    // Insert two pages
    {
        // Page 5
        abc::vmem::linked::const_iterator actual_itr = other_linked.end();
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&other_linked, 5U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        passed = insert_linked_page(context, &pool, other_linked, 5U, 0x0065, other_linked.begin(), expected_itr, actual_itr) && passed;
        passed = context.are_equal(actual_itr == other_linked.cbegin(), true, 0x104e6, "%d") && passed;
        passed = context.are_equal(actual_itr == other_linked.crend(), true, 0x104e7, "%d") && passed;
    }
    // 5

    {
        // Page 6
        abc::vmem::linked::const_iterator actual_itr = other_linked.end();
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&other_linked, 6U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        passed = insert_linked_page(context, &pool, other_linked, 6U, 0x0066, other_linked.begin(), expected_itr, actual_itr) && passed;
        passed = context.are_equal(actual_itr == other_linked.cbegin(), true, 0x104e8, "%d") && passed;
    }
    // 6 5


    linked.splice(other_linked);

    passed = context.are_equal(other_linked.begin() == other_linked.end(), true, 0x104e9, "%d") && passed;
    passed = context.are_equal(other_linked.rend() == other_linked.rbegin(), true, 0x104ea, "%d") && passed;

    // Iterate
    {
        using Pair = std::pair<LinkedPageData, abc::vmem::linked::const_iterator>;
        const Pair expected[] = {
            { 0x0064ULL, abc::vmem::linked::const_iterator(&linked, 4U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log()) },
            { 0x0063ULL, abc::vmem::linked::const_iterator(&linked, 3U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log()) },
            { 0x0062ULL, abc::vmem::linked::const_iterator(&linked, 2U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log()) },
            { 0x0066ULL, abc::vmem::linked::const_iterator(&linked, 6U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log()) },
            { 0x0065ULL, abc::vmem::linked::const_iterator(&linked, 5U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log()) },
        };
        constexpr std::size_t expected_len = sizeof(expected) / sizeof(Pair); 

        passed = verify_linked_pages(context, &pool, linked, expected, expected_len) && passed;
    }

    return passed;
}


bool test_vmem_linked_clear(test_context& context) {
    bool passed = true;

    abc::vmem::pool_config config("out/test/linked_clear.vmem", max_mapped_page_count_min);
    abc::vmem::pool pool(std::move(config), context.log());

    abc::vmem::linked_state linked_state;
    abc::vmem::linked linked(&linked_state, &pool, context.log());

    // Allocate and insert
    {
        // Page 2
        abc::vmem::linked::const_iterator actual_itr = linked.end();
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&linked, 2U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        passed = insert_linked_page(context, &pool, linked, 2U, 0x0072, linked.end(), expected_itr, actual_itr) && passed;
        passed = context.are_equal(actual_itr == linked.begin(), true, 0x104eb, "%d") && passed;
        passed = context.are_equal(actual_itr == linked.rend(), true, 0x104ec, "%d") && passed;
    }

    {
        // Page 3
        abc::vmem::linked::const_iterator actual_itr = linked.end();
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&linked, 3U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        passed = insert_linked_page(context, &pool, linked, 3U, 0x0073, linked.end(), expected_itr, actual_itr) && passed;
        passed = context.are_equal(actual_itr == linked.rend(), true, 0x104ed, "%d") && passed;
    }

    {
        // Page 4
        abc::vmem::linked::const_iterator actual_itr = linked.end();
        abc::vmem::linked::const_iterator expected_itr = abc::vmem::linked::const_iterator(&linked, 4U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::none, context.log());
        passed = insert_linked_page(context, &pool, linked, 4U, 0x0074, linked.end(), expected_itr, actual_itr) && passed;
        passed = context.are_equal(actual_itr == linked.rend(), true, 0x104ee, "%d") && passed;
    }

    // Clear
    linked.clear();

    // Allocate again
    {
        // Page 4
        abc::vmem::page page4(&pool, context.log());
        LinkedPage* linked_page4 = reinterpret_cast<LinkedPage*>(page4.ptr());
        passed = context.are_equal(linked_page4 != nullptr, true, 0x104ef, "%d") && passed;
        passed = context.are_equal((long long)page4.pos(), 4LL, 0x104f0, "0x%llx") && passed;
    }
    {
        // Page 3
        abc::vmem::page page3(&pool, context.log());
        LinkedPage* linked_page3 = reinterpret_cast<LinkedPage*>(page3.ptr());
        passed = context.are_equal(linked_page3 != nullptr, true, 0x104f1, "%d") && passed;
        passed = context.are_equal((long long)page3.pos(), 3LL, 0x104f2, "0x%llx") && passed;
    }
    {
        // Page 2
        abc::vmem::page page2(&pool, context.log());
        LinkedPage* linked_page2 = reinterpret_cast<LinkedPage*>(page2.ptr());
        passed = context.are_equal(linked_page2 != nullptr, true, 0x104f3, "%d") && passed;
        passed = context.are_equal((long long)page2.pos(), 2LL, 0x104f4, "0x%llx") && passed;
    }

    return passed;
}


bool test_vmem_list_insert(test_context& context) {
    constexpr const char* suborigin = "test_vmem_list_insert";

    using Item = std::array<std::uint8_t, 900>;

    bool passed = true;

    abc::vmem::pool_config config("out/test/list_insert.vmem", max_mapped_page_count_list);
    abc::vmem::pool pool(std::move(config), context.log());

    abc::vmem::list_state list_state;
    abc::vmem::list<Item> list(&list_state, &pool, context.log());

    Item item;

    item.fill(0x21);
    abc::vmem::list_iterator<Item> actual_itr = list.insert(list.end(), item);
    abc::vmem::list_iterator<Item> expected_itr = abc::vmem::list_iterator<Item>(&list, 2U, 0U, abc::vmem::iterator_edge::none, context.log());
    abc::vmem::list_iterator<Item> itr21 = actual_itr;
    passed = context.are_equal(actual_itr == expected_itr, true, 0x1040e, "%d") && passed;
    passed = context.are_equal(actual_itr == list.begin(), true, 0x103ce, "%d") && passed;
    passed = context.are_equal(actual_itr == list.rend(), true, 0x103cf, "%d") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 1, 0x103d0, "%zu") && passed;
    // | (2)
    // | 21 __ __ __ |

    item.fill(0x22);
    actual_itr = list.insert(list.end(), item);
    expected_itr = abc::vmem::list_iterator<Item>(&list, 2U, 1U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr == expected_itr, true, 0x1040f, "%d") && passed;
    passed = context.are_equal(actual_itr == list.rend(), true, 0x103d1, "%d") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 2, 0x103d2, "%zu") && passed;
    // | (2)
    // | 21 22 __ __ |

    item.fill(0x23);
    actual_itr = list.insert(itr21, item);
    expected_itr = abc::vmem::list_iterator<Item>(&list, 2U, 0U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr == expected_itr, true, 0x10410, "%d") && passed;
    passed = context.are_equal(actual_itr == itr21, true, 0x103d3, "%d") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 3, 0x103d4, "%zu") && passed;
    // | (2)
    // | 23 21 22 __ |

    item.fill(0x24);
    actual_itr = list.insert(list.begin(), item);
    expected_itr = abc::vmem::list_iterator<Item>(&list, 2U, 0U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr == expected_itr, true, 0x10411, "%d") && passed;
    passed = context.are_equal(actual_itr == list.begin(), true, 0x103d5, "%d") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 4, 0x103d6, "%zu") && passed;
    // | (2)
    // | 24 23 21 22 |

    itr21 = abc::vmem::list_iterator<Item>(&list, 2U, 2U, abc::vmem::iterator_edge::none, context.log());
    item.fill(0x25);
    actual_itr = list.insert(itr21, item);
    expected_itr = abc::vmem::list_iterator<Item>(&list, 2U, 2U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr == expected_itr, true, 0x10412, "%d") && passed;
    abc::vmem::list_iterator<Item> rend_itr = abc::vmem::list_iterator<Item>(&list, 3U, 1U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(list.rend() == rend_itr, true, 0x10413, "%d") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 5, 0x103d8, "%zu") && passed;
    // | (2)         | (3)
    // | 24 23 25 __ | 21 22 __ __ |

    item.fill(0x26);
    actual_itr = list.insert(list.end(), item);
    expected_itr = abc::vmem::list_iterator<Item>(&list, 3U, 2U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr == expected_itr, true, 0x10414, "%d") && passed;
    passed = context.are_equal(actual_itr == list.rend(), true, 0x103d9, "%d") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 6, 0x103da, "%zu") && passed;
    // | (2)         | (3)
    // | 24 23 25 __ | 21 22 26 __ |

    item.fill(0x27);
    actual_itr = list.insert(list.begin(), item);
    expected_itr = abc::vmem::list_iterator<Item>(&list, 2U, 0U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr == expected_itr, true, 0x10415, "%d") && passed;
    passed = context.are_equal(actual_itr == list.begin(), true, 0x103db, "%d") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 7, 0x103dc, "%zu") && passed;
    // | (2)         | (3)
    // | 27 24 23 25 | 21 22 26 __ |

    item.fill(0x28);
    actual_itr = list.insert(list.begin(), item);
    expected_itr = abc::vmem::list_iterator<Item>(&list, 2U, 0U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr == expected_itr, true, 0x10416, "%d") && passed;
    passed = context.are_equal(actual_itr == list.begin(), true, 0x103dd, "%d") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 8, 0x103de, "%zu") && passed;
    // | (2)         | (4)         | (3)
    // | 28 27 24 __ | 23 25 __ __ | 21 22 26 __ |

    item.fill(0x29);
    actual_itr = list.insert(list.end(), item);
    expected_itr = abc::vmem::list_iterator<Item>(&list, 3U, 3U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr == expected_itr, true, 0x10417, "%d") && passed;
    passed = context.are_equal(actual_itr == list.rend(), true, 0x10418, "%d") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 9, 0x10419, "%zu") && passed;
    // | (2)         | (4)         | (3)
    // | 28 27 24 __ | 23 25 __ __ | 21 22 26 29 |

    item.fill(0x2a);
    actual_itr = list.insert(list.end(), item);
    expected_itr = abc::vmem::list_iterator<Item>(&list, 5U, 0U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr == expected_itr, true, 0x1041a, "%d") && passed;
    passed = context.are_equal(actual_itr == list.rend(), true, 0x1041b, "%d") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 10, 0x1041c, "%zu") && passed;
    // | (2)         | (4)         | (3)         | (5)
    // | 28 27 24 __ | 23 25 __ __ | 21 22 26 29 | 2a __ __ __ |

    using Pair = std::pair<std::uint8_t, abc::vmem::list_iterator<Item>>;
    const Pair exp[] = {
        { 0x28, abc::vmem::list_iterator<Item>(&list, 2U, 0U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x27, abc::vmem::list_iterator<Item>(&list, 2U, 1U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x24, abc::vmem::list_iterator<Item>(&list, 2U, 2U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x23, abc::vmem::list_iterator<Item>(&list, 4U, 0U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x25, abc::vmem::list_iterator<Item>(&list, 4U, 1U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x21, abc::vmem::list_iterator<Item>(&list, 3U, 0U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x22, abc::vmem::list_iterator<Item>(&list, 3U, 1U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x26, abc::vmem::list_iterator<Item>(&list, 3U, 2U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x29, abc::vmem::list_iterator<Item>(&list, 3U, 3U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x2a, abc::vmem::list_iterator<Item>(&list, 5U, 0U, abc::vmem::iterator_edge::none, context.log()) },
    };
    constexpr std::size_t exp_len = sizeof(exp) / sizeof(Pair); 

    // Iterate forward.
    actual_itr = list.cbegin();
    for (std::size_t i = 0; i < exp_len; i++) {
        context.log()->put_any(origin, suborigin, abc::diag::severity::optional, 0x1041d, "forward[%zu]=0x%x", i, exp[i].first);

        passed = context.are_equal(actual_itr == exp[i].second, true, 0x1041e, "%d") && passed;
        passed = verify_bytes(context, actual_itr->data(), 0, sizeof(Item), exp[i].first, 0x104f5) && passed;
        actual_itr++;
    }
    passed = context.are_equal(actual_itr == list.end(), true, 0x103df, "%d") && passed;

    // Iterate backwards.
    actual_itr = list.crend();
    for (std::size_t i = 0; i < exp_len; i++) {
        context.log()->put_any(origin, suborigin, abc::diag::severity::optional, 0x1041f, "backward[%zu]=0x%x", exp_len - i - 1, exp[exp_len - i - 1].first);

        passed = context.are_equal(actual_itr == exp[exp_len - i - 1].second, true, 0x10420, "%d") && passed;
        passed = verify_bytes(context, actual_itr->data(), 0, sizeof(Item), exp[exp_len - i - 1].first, 0x104f6) && passed;
        actual_itr--;
    }
    passed = context.are_equal(actual_itr == list.rbegin(), true, 0x103e0, "%d") && passed;

    return passed;
}


bool test_vmem_list_insertmany(test_context& context) {
    bool passed = true;

    abc::vmem::pool_config config("out/test/list_insertmany.vmem", max_mapped_page_count_list);
    abc::vmem::pool pool(std::move(config), context.log());

    abc::vmem::list_state list_state;
    abc::vmem::list<ItemMany> list(&list_state, &pool, context.log());

    passed = insert_list_items(context, list, 4000) && passed;

    return passed;
}


bool test_vmem_list_erase(test_context& context) {
    bool passed = true;

    abc::vmem::pool_config config("out/test/list_erase.vmem", max_mapped_page_count_list);
    abc::vmem::pool pool(std::move(config), context.log());

    abc::vmem::list_state list_state;
    abc::vmem::list<ItemMany> list(&list_state, &pool, context.log());

    passed = insert_list_items(context, list, 16) && passed;
    // | (2)         | (3)         | (4)         | (5)
    // | 00 01 02 03 | 04 05 06 07 | 08 09 0a 0b | 0c 0d 0e 0f

    abc::vmem::list<ItemMany>::iterator itr_target = abc::vmem::list<ItemMany>::iterator(&list, 4U, 3U, abc::vmem::iterator_edge::none, context.log());
    abc::vmem::list<ItemMany>::iterator itr_expected = abc::vmem::list<ItemMany>::iterator(&list, 5U, 0U, abc::vmem::iterator_edge::none, context.log());
    abc::vmem::list<ItemMany>::iterator itr_actual = list.erase(itr_target);
    passed = context.are_equal<bool>(itr_actual == itr_expected, true, 0x10421, "%d") && passed;
    passed = context.are_equal<unsigned long long>(itr_actual->data, 0x0c, 0x10422, "0x%2.2llx") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 15, 0x10423, "%zu") && passed;
    // | (2)         | (3)         | (4)         | (5)
    // | 00 01 02 03 | 04 05 06 07 | 08 09 0a __ | 0c 0d 0e 0f

    itr_target = abc::vmem::list<ItemMany>::iterator(&list, 4U, 0U, abc::vmem::iterator_edge::none, context.log());
    itr_expected = abc::vmem::list<ItemMany>::iterator(&list, 4U, 0U, abc::vmem::iterator_edge::none, context.log());
    itr_actual = list.erase(itr_target);
    passed = context.are_equal<bool>(itr_actual == itr_expected, true, 0x10424, "%d") && passed;
    passed = context.are_equal<unsigned long long>(itr_actual->data, 0x09, 0x10425, "0x%2.2llx") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 14, 0x10426, "%zu") && passed;
    // | (2)         | (3)         | (4)         | (5)
    // | 00 01 02 03 | 04 05 06 07 | 09 0a __ __ | 0c 0d 0e 0f

    itr_target = abc::vmem::list<ItemMany>::iterator(&list, 3U, 2U, abc::vmem::iterator_edge::none, context.log());
    itr_expected = abc::vmem::list<ItemMany>::iterator(&list, 3U, 2U, abc::vmem::iterator_edge::none, context.log());
    itr_actual = list.erase(itr_target);
    passed = context.are_equal<bool>(itr_actual == itr_expected, true, 0x10427, "%d") && passed;
    passed = context.are_equal<unsigned long long>(itr_actual->data, 0x07, 0x10428, "0x%2.2llx") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 13, 0x10429, "%zu") && passed;
    // | (2)         | (3)         | (4)         | (5)
    // | 00 01 02 03 | 04 05 07 __ | 09 0a __ __ | 0c 0d 0e 0f

    itr_target = abc::vmem::list<ItemMany>::iterator(&list, 3U, 1U, abc::vmem::iterator_edge::none, context.log());
    itr_expected = abc::vmem::list<ItemMany>::iterator(&list, 3U, 1U, abc::vmem::iterator_edge::none, context.log());
    itr_actual = list.erase(itr_target);
    passed = context.are_equal<bool>(itr_actual == itr_expected, true, 0x1042a, "%d") && passed;
    passed = context.are_equal<unsigned long long>(itr_actual->data, 0x07, 0x1042b, "0x%2.2llx") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 12, 0x1042c, "%zu") && passed;
    // | (2)         | (3)         | (5)
    // | 00 01 02 03 | 04 07 09 0a | 0c 0d 0e 0f

    itr_target = abc::vmem::list<ItemMany>::iterator(&list, 3U, 1U, abc::vmem::iterator_edge::none, context.log());
    itr_expected = abc::vmem::list<ItemMany>::iterator(&list, 3U, 1U, abc::vmem::iterator_edge::none, context.log());
    itr_actual = list.erase(itr_target);
    passed = context.are_equal<bool>(itr_actual == itr_expected, true, 0x1042d, "%d") && passed;
    passed = context.are_equal<unsigned long long>(itr_actual->data, 0x09, 0x1042e, "0x%2.2llx") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 11, 0x1042f, "%zu") && passed;
    // | (2)         | (3)         | (5)
    // | 00 01 02 03 | 04 09 0a __ | 0c 0d 0e 0f

    itr_target = abc::vmem::list<ItemMany>::iterator(&list, 2U, 0U, abc::vmem::iterator_edge::none, context.log());
    itr_expected = abc::vmem::list<ItemMany>::iterator(&list, 2U, 0U, abc::vmem::iterator_edge::none, context.log());
    itr_actual = list.erase(itr_target);
    passed = context.are_equal<bool>(itr_actual == itr_expected, true, 0x10430, "%d") && passed;
    passed = context.are_equal<unsigned long long>(itr_actual->data, 0x01, 0x10431, "0x%2.2llx") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 10, 0x10432, "%zu") && passed;
    // | (2)         | (3)         | (5)
    // | 01 02 03 __ | 04 09 0a __ | 0c 0d 0e 0f

    itr_target = abc::vmem::list<ItemMany>::iterator(&list, 2U, 2U, abc::vmem::iterator_edge::none, context.log());
    itr_expected = abc::vmem::list<ItemMany>::iterator(&list, 3U, 0U, abc::vmem::iterator_edge::none, context.log());
    itr_actual = list.erase(itr_target);
    passed = context.are_equal<bool>(itr_actual == itr_expected, true, 0x10433, "%d") && passed;
    passed = context.are_equal<unsigned long long>(itr_actual->data, 0x04, 0x10434, "0x%2.2llx") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 9, 0x10435, "%zu") && passed;
    // | (2)         | (3)         | (5)
    // | 01 02 __ __ | 04 09 0a __ | 0c 0d 0e 0f

    itr_target = abc::vmem::list<ItemMany>::iterator(&list, 3U, 0U, abc::vmem::iterator_edge::none, context.log());
    itr_expected = abc::vmem::list<ItemMany>::iterator(&list, 2U, 2U, abc::vmem::iterator_edge::none, context.log());
    itr_actual = list.erase(itr_target);
    passed = context.are_equal<bool>(itr_actual == itr_expected, true, 0x10436, "%d") && passed;
    passed = context.are_equal<unsigned long long>(itr_actual->data, 0x09, 0x10437, "0x%2.2llx") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 8, 0x10438, "%zu") && passed;
    // | (2)         | (5)
    // | 01 02 09 0a | 0c 0d 0e 0f

    itr_target = abc::vmem::list<ItemMany>::iterator(&list, 2U, 3U, abc::vmem::iterator_edge::none, context.log());
    itr_expected = abc::vmem::list<ItemMany>::iterator(&list, 5U, 0U, abc::vmem::iterator_edge::none, context.log());
    itr_actual = list.erase(itr_target);
    passed = context.are_equal<bool>(itr_actual == itr_expected, true, 0x10439, "%d") && passed;
    passed = context.are_equal<unsigned long long>(itr_actual->data, 0x0c, 0x1043a, "0x%2.2llx") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 7, 0x1043b, "%zu") && passed;
    // | (2)         | (5)
    // | 01 02 09 __ | 0c 0d 0e 0f

    itr_target = abc::vmem::list<ItemMany>::iterator(&list, 2U, 2U, abc::vmem::iterator_edge::none, context.log());
    itr_expected = abc::vmem::list<ItemMany>::iterator(&list, 5U, 0U, abc::vmem::iterator_edge::none, context.log());
    itr_actual = list.erase(itr_target);
    passed = context.are_equal<bool>(itr_actual == itr_expected, true, 0x104f7, "%d") && passed;
    passed = context.are_equal<unsigned long long>(itr_actual->data, 0x0c, 0x104f8, "0x%2.2llx") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 6, 0x104f9, "%zu") && passed;
    // | (2)         | (5)
    // | 01 02 __ __ | 0c 0d 0e 0f

    itr_target = abc::vmem::list<ItemMany>::iterator(&list, 5U, 3U, abc::vmem::iterator_edge::none, context.log());
    itr_expected = list.end();
    itr_actual = list.erase(itr_target);
    passed = context.are_equal<bool>(itr_actual == itr_expected, true, 0x104fa, "%d") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 5, 0x104fb, "%zu") && passed;
    // | (2)         | (5)
    // | 01 02 __ __ | 0c 0d 0e __

    itr_target = abc::vmem::list<ItemMany>::iterator(&list, 5U, 2U, abc::vmem::iterator_edge::none, context.log());
    itr_expected = abc::vmem::list<ItemMany>::iterator(&list, 2U, abc::vmem::item_pos_nil, abc::vmem::iterator_edge::end, context.log());
    itr_actual = list.erase(itr_target);
    passed = context.are_equal<bool>(itr_actual == itr_expected, true, 0x104fc, "%d") && passed;
    passed = context.are_equal<bool>(itr_actual == list.end(), true, 0x104fd, "%d") && passed;
    passed = context.are_equal<std::size_t>(list.size(), 4, 0x104fe, "%zu") && passed;
    // | (2)
    // | 01 02 0c 0d

    return passed;
}


bool test_vmem_temp_destructor(test_context& context) {
    bool passed = true;

    abc::vmem::pool_config config("out/test/temp_destructor.vmem", max_mapped_page_count_list);
    abc::vmem::pool pool(std::move(config), context.log());

    {
        abc::vmem::list_state list_state;
        abc::vmem::temp<abc::vmem::list<ItemMany>> temp_list(&list_state, &pool, context.log());

        passed = insert_list_items(context, temp_list, 8) && passed;
        // | (2)         | (3)
        // | 00 01 02 03 | 04 05 06 07
    }

    // Allocate again
    {
        // Page 3
        abc::vmem::page page3(&pool, context.log());
        LinkedPage* linked_page3 = reinterpret_cast<LinkedPage*>(page3.ptr());
        passed = context.are_equal(linked_page3 != nullptr, true, 0x10538, "%d") && passed;
        passed = context.are_equal((long long)page3.pos(), 3LL, 0x10539, "0x%llx") && passed;
    }
    {
        // Page 2
        abc::vmem::page page2(&pool, context.log());
        LinkedPage* linked_page2 = reinterpret_cast<LinkedPage*>(page2.ptr());
        passed = context.are_equal(linked_page2 != nullptr, true, 0x1053a, "%d") && passed;
        passed = context.are_equal((long long)page2.pos(), 2LL, 0x1053b, "0x%llx") && passed;
    }

    return passed;
}


bool test_vmem_map_insert(test_context& context) {
    using Iterator = abc::vmem::map<Key, Value>::const_iterator;
    using IteratorBool = std::pair<Iterator, bool>;

    constexpr const char* suborigin = "test_vmem_map_insert";

    bool passed = true;

    abc::vmem::pool_config config("out/test/map_insert.vmem", max_mapped_page_count_map);
    abc::vmem::pool pool(std::move(config), context.log());

    abc::vmem::map_state map_state;
    abc::vmem::map<Key, Value> map(&map_state, &pool, context.log());
    abc::vmem::map<Key, Value>::value_type item;

    item.key.data = 0x20;
    item.value = 0x900 + item.key.data;
    IteratorBool actual_itr = map.insert(item);
    Iterator expected_itr = Iterator(&map, 2U, 0U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr.second, true, 0x1053c, "%d") && passed;
    passed = context.are_equal(actual_itr.first == expected_itr, true, 0x1053d, "%d") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 1, 0x1053e, "%zu") && passed;
    // | (2)
    // | 20 __ __ __ |

    item.key.data = 0x50;
    item.value = 0x900 + item.key.data;
    actual_itr = map.insert(item);
    expected_itr = Iterator(&map, 2U, 1U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr.second, true, 0x1053f, "%d") && passed;
    passed = context.are_equal(actual_itr.first == expected_itr, true, 0x10540, "%d") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 2, 0x10541, "%zu") && passed;
    // | (2)
    // | 20 50 __ __ |

    item.key.data = 0x30;
    item.value = 0x900 + item.key.data;
    actual_itr = map.insert(item);
    expected_itr = Iterator(&map, 2U, 1U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr.second, true, 0x10542, "%d") && passed;
    passed = context.are_equal(actual_itr.first == expected_itr, true, 0x10543, "%d") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 3, 0x10544, "%zu") && passed;
    // | (2)
    // | 20 30 50 __ |

    item.key.data = 0x40;
    item.value = 0x900 + item.key.data;
    actual_itr = map.insert(item);
    expected_itr = Iterator(&map, 2U, 2U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr.second, true, 0x10545, "%d") && passed;
    passed = context.are_equal(actual_itr.first == expected_itr, true, 0x10546, "%d") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 4, 0x10547, "%zu") && passed;
    // | (2)
    // | 20 30 40 50 |

    item.key.data = 0x60;
    item.value = 0x900 + item.key.data;
    actual_itr = map.insert(item);
    expected_itr = Iterator(&map, 3U, 2U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr.second, true, 0x10548, "%d") && passed;
    passed = context.are_equal(actual_itr.first == expected_itr, true, 0x10549, "%d") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 5, 0x1054a, "%zu") && passed;
    // | (2)         | (3)
    // | 20 30 __ __ | 40 50 60 __ |

    item.key.data = 0x70;
    item.value = 0x900 + item.key.data;
    actual_itr = map.insert(item);
    expected_itr = Iterator(&map, 3U, 3U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr.second, true, 0x1054b, "%d") && passed;
    passed = context.are_equal(actual_itr.first == expected_itr, true, 0x1054c, "%d") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 6, 0x1054d, "%zu") && passed;
    // | (2)         | (3)
    // | 20 30 __ __ | 40 50 60 70 |

    item.key.data = 0x58;
    item.value = 0x900 + item.key.data;
    actual_itr = map.insert(item);
    expected_itr = Iterator(&map, 3U, 2U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr.second, true, 0x1054e, "%d") && passed;
    passed = context.are_equal(actual_itr.first == expected_itr, true, 0x1054f, "%d") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 7, 0x10550, "%zu") && passed;
    // | (2)         | (3)         | (7)
    // | 20 30 __ __ | 40 50 58 __ | 60 70 __ __ |

    item.key.data = 0x80;
    item.value = 0x900 + item.key.data;
    actual_itr = map.insert(item);
    expected_itr = Iterator(&map, 7U, 2U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr.second, true, 0x10551, "%d") && passed;
    passed = context.are_equal(actual_itr.first == expected_itr, true, 0x10552, "%d") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 8, 0x10553, "%zu") && passed;
    // | (2)         | (3)         | (7)
    // | 20 30 __ __ | 40 50 58 __ | 60 70 80 __ |

    item.key.data = 0x90;
    item.value = 0x900 + item.key.data;
    actual_itr = map.insert(item);
    expected_itr = Iterator(&map, 7U, 3U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr.second, true, 0x10554, "%d") && passed;
    passed = context.are_equal(actual_itr.first == expected_itr, true, 0x10555, "%d") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 9, 0x10556, "%zu") && passed;
    // | (2)         | (3)         | (7)
    // | 20 30 __ __ | 40 50 58 __ | 60 70 80 90 |

    item.key.data = 0x88;
    item.value = 0x900 + item.key.data;
    actual_itr = map.insert(item);
    expected_itr = Iterator(&map, 0x8, 1U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr.second, true, 0x10557, "%d") && passed;
    passed = context.are_equal(actual_itr.first == expected_itr, true, 0x10558, "%d") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 10, 0x10559, "%zu") && passed;
    // | (2)         | (3)         | (7)         | (8)
    // | 20 30 __ __ | 40 50 58 __ | 60 70 __ __ | 80 88 90 __ |

    item.key.data = 0xa0;
    item.value = 0x900 + item.key.data;
    actual_itr = map.insert(item);
    expected_itr = Iterator(&map, 0x8, 3U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr.second, true, 0x1055a, "%d") && passed;
    passed = context.are_equal(actual_itr.first == expected_itr, true, 0x1055b, "%d") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 11, 0x1055c, "%zu") && passed;
    // | (2)         | (3)         | (7)         | (8)
    // | 20 30 __ __ | 40 50 58 __ | 60 70 __ __ | 80 88 90 a0 |

    item.key.data = 0xb0;
    item.value = 0x900 + item.key.data;
    actual_itr = map.insert(item);
    expected_itr = Iterator(&map, 0x9, 2U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(actual_itr.second, true, 0x1055d, "%d") && passed;
    passed = context.are_equal(actual_itr.first == expected_itr, true, 0x1055e, "%d") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 12, 0x1055f, "%zu") && passed;
    // | (2)         | (3)         | (7)         | (8)         | (9)
    // | 20 30 __ __ | 40 50 58 __ | 60 70 __ __ | 80 88 __ __ | 90 a0 b0 __ |

    Key key;
    key.data = 0x70;
    Iterator itr = map.find(key);
    expected_itr = Iterator(&map, 7U, 1U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(itr == expected_itr, true, 0x10560, "%d") && passed;
    passed = context.are_equal(itr->value == 0x970, true, 0x10561, "%d") && passed;

    key.data = 0x40;
    itr = map.find(key);
    expected_itr = Iterator(&map, 3U, 0U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(itr == expected_itr, true, 0x10562, "%d") && passed;
    passed = context.are_equal(itr->value == 0x940, true, 0x10563, "%d") && passed;

    key.data = 0xa0;
    itr = map.find(key);
    expected_itr = Iterator(&map, 9U, 1U, abc::vmem::iterator_edge::none, context.log());
    passed = context.are_equal(itr == expected_itr, true, 0x10564, "%d") && passed;
    passed = context.are_equal(itr->value == 0x9a0, true, 0x10565, "%d") && passed;

    key.data = 0x20;
    item = *map[key];
    passed = context.are_equal(item.value == 0x920, true, 0x10566, "%d") && passed;

    key.data = 0x50;
    item = *map[key];
    passed = context.are_equal(item.value == 0x950, true, 0x10567, "%d") && passed;

    key.data = 0xb0;
    item = *map[key];
    passed = context.are_equal(item.value == 0x9b0, true, 0x10568, "%d") && passed;

    using Pair = std::pair<std::uint64_t, Iterator>;
    const Pair exp[] = {
        { 0x20, Iterator(&map, 2U, 0U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x30, Iterator(&map, 2U, 1U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x40, Iterator(&map, 3U, 0U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x50, Iterator(&map, 3U, 1U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x58, Iterator(&map, 3U, 2U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x60, Iterator(&map, 7U, 0U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x70, Iterator(&map, 7U, 1U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x80, Iterator(&map, 8U, 0U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x88, Iterator(&map, 8U, 1U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x90, Iterator(&map, 9U, 0U, abc::vmem::iterator_edge::none, context.log()) },
        { 0xa0, Iterator(&map, 9U, 1U, abc::vmem::iterator_edge::none, context.log()) },
        { 0xb0, Iterator(&map, 9U, 2U, abc::vmem::iterator_edge::none, context.log()) },
    };
    constexpr std::size_t exp_len = sizeof(exp) / sizeof(Pair); 

    // Iterate forward.
    itr = map.cbegin();
    for (std::size_t i = 0; i < exp_len; i++) {
        context.log()->put_any(origin, suborigin, abc::diag::severity::optional, 0x10569, "forward[%zu]=0x%llx", i, exp[i].first);

        passed = context.are_equal(itr == exp[i].second, true, 0x1056a, "%d") && passed;
        passed = context.are_equal<std::uint64_t>(itr->key.data, exp[i].first, 0x1056b, "%d") && passed;
        passed = context.are_equal<std::uint64_t>(itr->value, 0x900 + exp[i].first, 0x1056c, "%d") && passed;
        itr++;
    }
    passed = context.are_equal(itr == map.cend(), true, 0x1056d, "%d") && passed;

    // Iterate backwards.
    itr = map.crend();
    for (std::size_t i = 0; i < exp_len; i++) {
        context.log()->put_any(origin, suborigin, abc::diag::severity::optional, 0x1056e, "backward[%zu]=0x%llx", exp_len - i - 1, exp[exp_len - i - 1].first);

        passed = context.are_equal(itr == exp[exp_len - i - 1].second, true, 0x1056f, "%d") && passed;
        passed = context.are_equal<std::uint64_t>(itr->key.data, exp[exp_len - i - 1].first, 0x10570, "%d") && passed;
        passed = context.are_equal<std::uint64_t>(itr->value, 0x900 + exp[exp_len - i - 1].first, 0x10571, "%d") && passed;
        itr--;
    }
    passed = context.are_equal(itr == map.crbegin(), true, 0x10572, "%d") && passed;

    return passed;
}


#if 0 //// TODO: TEMP
bool test_vmem_map_insertmany(test_context<abc::test::log>& context) {
    using Pool = PoolMin;
    using Map = abc::vmem_map<Key, Value, Pool, Log>;

    bool passed = true;

    Pool pool("out/test/map_insertmany.vmem", context.log());

    abc::vmem_map_state map_state;
    Map map(&map_state, &pool, context.log());

    passed = insert_vmem_map_items(context, map, 4000) && passed;

    return passed;
}


bool test_vmem_map_erase(test_context<abc::test::log>& context) {
    using Pool = PoolMap;
    using Map = abc::vmem_map<Key, Value, Pool, Log>;
    using Iterator = abc::vmem_map_const_iterator<Key, Value, Pool, Log>;

    bool passed = true;

    Pool pool("out/test/map_erase.vmem", context.log());

    abc::vmem_map_state map_state;
    Map map(&map_state, &pool, context.log());
    Key key;

    passed = insert_vmem_map_items(context, map, 11) && passed;
    // | (2)         | (3)         | (7)         | (8)         | (9)
    // | 00 01 __ __ | 02 03 __ __ | 04 05 __ __ | 06 07 __ __ | 08 09 0a __ |

    key.data = 0x09;
    std::size_t one = map.erase(key);
    passed = context.are_equal<std::size_t>(one, 1, 0x10573, "%zu") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 10, 0x10574, "%zu") && passed;
    // | (2)         | (3)         | (7)         | (8)
    // | 00 01 __ __ | 02 03 __ __ | 04 05 __ __ | 06 07 08 0a |

    // Test that key levels have been updated.
    key.data = 0x0a;
    Iterator itr = map.find(key);
    passed = context.are_equal<long long>(itr.page_pos(), 8LL, 0x10575, "0x%llx") && passed;
    passed = context.are_equal<unsigned>(itr.item_pos(), 3U, 0x10576, "0x%x") && passed;

    key.data = 0x04;
    one = map.erase(key);
    passed = context.are_equal<std::size_t>(one, 1, 0x10577, "%zu") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 9, 0x10578, "%zu") && passed;
    // | (2)         | (3)         | (8)
    // | 00 01 __ __ | 02 03 05 __ | 06 07 08 0a |

    key.data = 0x01;
    one = map.erase(key);
    passed = context.are_equal<std::size_t>(one, 1, 0x10579, "%zu") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 8, 0x1057a, "%zu") && passed;
    // | (2)         | (8)
    // | 00 02 03 05 | 06 07 08 0a |

    // Test that key levels have been updated.
    key.data = 0x05;
    itr = map.find(key);
    passed = context.are_equal<long long>(itr.page_pos(), 2LL, 0x1057b, "0x%llx") && passed;
    passed = context.are_equal<unsigned>(itr.item_pos(), 3U, 0x1057c, "0x%x") && passed;

    using Pair = std::pair<std::uint64_t, Iterator>;
    const Pair exp[] = {
        { 0x00, Iterator(&map, 2U, 0U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x02, Iterator(&map, 2U, 1U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x03, Iterator(&map, 2U, 2U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x05, Iterator(&map, 2U, 3U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x06, Iterator(&map, 8U, 0U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x07, Iterator(&map, 8U, 1U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x08, Iterator(&map, 8U, 2U, abc::vmem::iterator_edge::none, context.log()) },
        { 0x0a, Iterator(&map, 8U, 3U, abc::vmem::iterator_edge::none, context.log()) },
    };
    constexpr std::size_t exp_len = sizeof(exp) / sizeof(Pair); 

    // Iterate forward.
    itr = map.cbegin();
    for (std::size_t i = 0; i < exp_len; i++) {
        context.log->put_any(abc::category::any, abc::severity::abc::important, 0x1057d, "forward[%zu]=0x%llx", i, exp[i].first);

        passed = context.are_equal(itr == exp[i].second, true, 0x1057e, "%d") && passed;
        passed = context.are_equal<std::uint64_t>(itr->key.data, exp[i].first, 0x1057f, "%d") && passed;
        passed = context.are_equal<std::uint64_t>(itr->value, 0x90000000 + exp[i].first, 0x10580, "%d") && passed;
        itr++;
    }
    passed = context.are_equal(itr == map.cend(), true, 0x10581, "%d") && passed;

    // Iterate backwards.
    itr = map.crend();
    for (std::size_t i = 0; i < exp_len; i++) {
        context.log->put_any(abc::category::any, abc::severity::abc::important, 0x10582, "backward[%zu]=0x%llx", exp_len - i - 1, exp[exp_len - i - 1].first);

        passed = context.are_equal(itr == exp[exp_len - i - 1].second, true, 0x10583, "%d") && passed;
        passed = context.are_equal<std::uint64_t>(itr->key.data, exp[exp_len - i - 1].first, 0x10584, "%d") && passed;
        passed = context.are_equal<std::uint64_t>(itr->value, 0x90000000 + exp[exp_len - i - 1].first, 0x10585, "%d") && passed;
        itr--;
    }
    passed = context.are_equal(itr == map.crbegin(), true, 0x10586, "%d") && passed;

    key.data = 0x09;
    one = map.erase(key);
    passed = context.are_equal<std::size_t>(one, 0, 0x10587, "%zu") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 8, 0x10588, "%zu") && passed;

    key.data = 0x04;
    one = map.erase(key);
    passed = context.are_equal<std::size_t>(one, 0, 0x10589, "%zu") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 8, 0x1058a, "%zu") && passed;

    key.data = 0x01;
    one = map.erase(key);
    passed = context.are_equal<std::size_t>(one, 0, 0x1058b, "%zu") && passed;
    passed = context.are_equal<std::size_t>(map.size(), 8, 0x1058c, "%zu") && passed;

    // Iterate forward.
    itr = map.cbegin();
    for (std::size_t i = 0; i < exp_len; i++) {
        context.log->put_any(abc::category::any, abc::severity::abc::important, 0x1058d, "forward[%zu]=0x%llx", i, exp[i].first);

        passed = context.are_equal(itr == exp[i].second, true, 0x1058e, "%d") && passed;
        passed = context.are_equal<std::uint64_t>(itr->key.data, exp[i].first, 0x1058f, "%d") && passed;
        passed = context.are_equal<std::uint64_t>(itr->value, 0x90000000 + exp[i].first, 0x10590, "%d") && passed;
        itr++;
    }
    passed = context.are_equal(itr == map.cend(), true, 0x10591, "%d") && passed;

    // Iterate backwards.
    itr = map.crend();
    for (std::size_t i = 0; i < exp_len; i++) {
        context.log->put_any(abc::category::any, abc::severity::abc::important, 0x10592, "backward[%zu]=0x%llx", exp_len - i - 1, exp[exp_len - i - 1].first);

        passed = context.are_equal(itr == exp[exp_len - i - 1].second, true, 0x10593, "%d") && passed;
        passed = context.are_equal<std::uint64_t>(itr->key.data, exp[exp_len - i - 1].first, 0x10594, "%d") && passed;
        passed = context.are_equal<std::uint64_t>(itr->value, 0x90000000 + exp[exp_len - i - 1].first, 0x10595, "%d") && passed;
        itr--;
    }
    passed = context.are_equal(itr == map.crbegin(), true, 0x10596, "%d") && passed;

    return passed;
}


bool test_vmem_map_clear(test_context<abc::test::log>& context) {
    using Pool = PoolMap;
    using Map = abc::vmem_map<Key, Value, Pool, Log>;
    using Linked = abc::vmem_linked<Pool, Log>;

    bool passed = true;

    Pool pool("out/test/map_clear.vmem", context.log());

    abc::vmem_map_state map_state;
    Map map(&map_state, &pool, context.log());

    passed = insert_vmem_map_items(context, map, 11) && passed;
    // | (2)         | (3)         | (7)         | (8)         | (9)
    // | 00 01 __ __ | 02 03 __ __ | 04 05 __ __ | 06 07 __ __ | 08 09 0a __ |

    constexpr vmem_page_pos_t max_page_pos = 0x0c;

    // This is only used to find out the above max_page_pos.
    {
        // Page 06
        abc::vmem::page page_06(&pool, context.log());
        passed = context.are_equal(page_06.ptr() != nullptr, true, 0x10597, "%d") && passed;

        // Page 0c
        abc::vmem::page page_0c(&pool, context.log());
        passed = context.are_equal(page_0c.ptr() != nullptr, true, 0x10598, "%d") && passed;
        passed = context.are_equal(page_0c.pos(), max_page_pos, 0x10599, "0x%llx") && passed;

        page_06.free();
        page_0c.free();
    }

    map.clear();

    // Verify all pages 2..max_page_pos are free.
    {
        unsigned long long bits = 0LLU;
        abc::vmem_linked_state linked_state;
        Linked linked(&linked_state, &pool, context.log());

        for (std::size_t i = 2; i <= max_page_pos; i++) {
            abc::vmem::page page(&pool, context.log());

            unsigned long long b = 1LLU << page.pos();
            passed = context.are_equal(page.pos() <= max_page_pos, true, 0x1059a, "%d") && passed;
            passed = context.are_equal(bits & b, 0LLU, 0x1059b, "%d") && passed;

            bits |= b;
            linked.push_back(page.pos());
        }

        passed = context.are_equal(bits, 0x01ffcLLU, 0x1059c, "0x%llx") && passed;

        linked.clear();
    }

    return passed;
}


bool test_vmem_string_iterator(test_context<abc::test::log>& context) {
    using Pool = PoolMin;
    using String = abc::vmem_string<Pool, Log>;
    using ConstIterator = abc::vmem_string_const_iterator<Pool, Log>; 

    bool passed = true;

    Pool pool("out/test/string_iterator.vmem", context.log());

    abc::vmem_string_state string_state;
    String str(&string_state, &pool, context.log());

    const char* expected = "abc 12 xyz";

    for (const char* ch = expected; *ch != '\0'; ch++) {
        str.push_back(*ch);
    }

    const char* ch = expected;
    for (ConstIterator itr = str.cbegin(); itr != str.cend(); itr++, ch++) {
        passed = context.are_equal(*itr, *ch, 0x107b4, "%c") && passed;
    }
    passed = context.are_equal('\0', *ch, 0x107b5, "\\x%2.2x") && passed;

    return passed;
}


bool test_vmem_string_stream(test_context<abc::test::log>& context) {
    using Pool = PoolMin;
    using String = abc::vmem_string<Pool, Log>;
    using Streambuf = abc::vmem_string_streambuf<Pool, Log>;

    bool passed = true;

    Pool pool("out/test/string_stream.vmem", context.log());

    abc::vmem_string_state string_state;
    String str(&string_state, &pool, context.log());

    int expected[] = {
        42,
        53,
        99,
        0
    };

    Streambuf sb(&str, context.log());
    std::ostream ostrm(&sb);

    for (const int* exp = expected; *exp != 0; exp++) {
        ostrm << *exp << " ";
    }

    std::istream istrm(&sb);
    for (int* exp = expected; *exp != 0; exp++) {
        int actual = -1;
        istrm >> actual;
        passed = context.are_equal(actual, *exp, 0x107b6, "%d") && passed;
    }

    return passed;
}


bool test_vmem_pool_move(test_context<abc::test::log>& context) {
    using Pool = PoolFree;

    bool passed = true;

    Pool pool1("out/test/pool_move.vmem", context.log());
    Pool pool2(std::move(pool1));

    {
        // Page Fail
        abc::vmem::page pageFail(&pool1, context.log());
        passed = context.are_equal(pageFail.ptr() == nullptr, true, 0x10739, "%d") && passed;
    }

    {
        // Page 2
        abc::vmem::page page2(&pool2, context.log());
        passed = context.are_equal(page2.ptr() != nullptr, true, 0x1073a, "%d") && passed;
        passed = context.are_equal((long long)page2.pos(), 2LL, 0x1073b, "0x%llx") && passed;

        page2.free();
    }

    return passed;
}


bool test_vmem_page_move(test_context<abc::test::log>& context) {
    using Pool = PoolFree;

    bool passed = true;

    Pool pool("out/test/page_move.vmem", context.log());

    // Page 2
    abc::vmem::page page2(&pool, context.log());
    passed = context.are_equal(page2.ptr() != nullptr, true, 0x1073c, "%d") && passed;
    passed = context.are_equal((long long)page2.pos(), 2LL, 0x1073d, "0x%llx") && passed;

    int* ptrActual = reinterpret_cast<int*>(page2.ptr());
    *ptrActual = 42; 

    abc::vmem::page page2Moved(std::move(page2));
    passed = context.are_equal(page2Moved.ptr() != nullptr, true, 0x1073e, "%d") && passed;
    passed = context.are_equal((long long)page2Moved.pos(), 2LL, 0x1073f, "0x%llx") && passed;
    passed = context.are_equal(*(int*)page2Moved.ptr(), 42, 0x10740, "%d") && passed;

    passed = context.are_equal(page2.ptr() == nullptr, true, 0x10741, "%d") && passed;
    passed = context.are_equal((long long)page2.pos(), -1LL, 0x10742, "0x%llx") && passed;

    page2Moved.free();

    return passed;
}
#endif


bool insert_list_items(test_context& context, abc::vmem::list<ItemMany>& list, std::size_t count) {
    bool passed = true;

    // Insert.
    for (std::size_t i = 0; i < count; i++) {
        ItemMany item = { i, { 0 } };
        list.insert(list.end(), item);
    }

    // Iterate forward.
    abc::vmem::list<ItemMany>::iterator itr = list.cbegin();
    for (std::size_t i = 0; i < count; i++) {
        passed = context.are_equal<unsigned long long>(itr->data, i, 0x103ed, "%llu") && passed;
        itr++;
    }
    passed = context.are_equal(itr == list.end(), true, 0x103ee, "%d") && passed;

    // Iterate backwards.
    itr = list.crend();
    for (std::size_t i = 0; i < count; i++) {
        passed = context.are_equal<unsigned long long>(itr->data, count - i - 1, 0x103ef, "%llu") && passed;
        itr--;
    }
    passed = context.are_equal(itr == list.rbegin(), true, 0x103f0, "%d") && passed;

    return passed;
}


#if 0 //// TODO: TEMP
template <typename Map>
bool insert_vmem_map_items(test_context<abc::test::log>& context, Map& map, std::size_t count) {
    constexpr std::size_t base_value = 0x90000000;

    bool passed = true;

    // Insert.
    for (std::size_t i = 0; i < count; i++) {
        MapItem item;
        item.key.data = i;
        item.value = base_value + i;

        map.insert(item);
    }

    // Iterate forward.
    typename Map::const_iterator itr = map.cbegin();
    for (std::size_t i = 0; i < count; i++) {
        passed = context.are_equal<unsigned long long>(itr->key.data, i, 0x1059d, "%llu") && passed;
        passed = context.are_equal<unsigned long long>(itr->value, base_value + i, 0x1059e, "%llu") && passed;
        itr++;
    }
    passed = context.are_equal(itr == map.cend(), true, 0x1059f, "%d") && passed;

    // Iterate backwards.
    itr = map.crend();
    for (std::size_t i = 0; i < count; i++) {
        passed = context.are_equal<unsigned long long>(itr->key.data, count - i - 1, 0x105a0, "%llu") && passed;
        passed = context.are_equal<unsigned long long>(itr->value, base_value + (count - i - 1), 0x105a1, "%llu") && passed;
        itr--;
    }
    passed = context.are_equal(itr == map.crbegin(), true, 0x105a2, "%d") && passed;

    return passed;
}
#endif


bool insert_linked_page(test_context& context, abc::vmem::pool* pool, abc::vmem::linked& linked, abc::vmem::page_pos_t expected_page_pos, LinkedPageData data,
                        const abc::vmem::linked::const_iterator& itr, const abc::vmem::linked::const_iterator& expected_itr,
                        abc::vmem::linked::const_iterator& actual_itr) {
    bool passed = true;

    // alloc page
    abc::vmem::page page(pool, context.log());
    LinkedPage* linked_page = reinterpret_cast<LinkedPage*>(page.ptr());
    passed = context.are_equal(linked_page != nullptr, true, 0x104ff, "%d") && passed;
    passed = context.are_equal((unsigned long long)page.pos(), (unsigned long long)expected_page_pos, 0x10500, "0x%llx") && passed;
    linked_page->data = data;

    // insert
    actual_itr = linked.insert(itr, page.pos());
    passed = context.are_equal(actual_itr == expected_itr, true, 0x10501, "%d") && passed;

    return passed;
}


bool verify_linked_pages(test_context& context, abc::vmem::pool* pool, abc::vmem::linked& linked,
                        const std::pair<LinkedPageData, abc::vmem::linked::const_iterator> expected[], std::size_t expected_len) {
    constexpr const char* suborigin = "verify_linked_pages()";

    bool passed = true;

    // Iterate forward.
    abc::vmem::linked::const_iterator actual_itr = linked.begin();
    for (std::size_t i = 0; i < expected_len; i++) {
        context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x10502, "forward[%zd]=0x%x", i, expected[i].first);

        abc::vmem::page page(pool, *actual_itr, context.log());
        LinkedPage* linked_page = static_cast<LinkedPage*>(page.ptr());

        passed = context.are_equal(actual_itr == expected[i].second, true, 0x10503, "%d") && passed;
        passed = context.are_equal(linked_page->data, expected[i].first, 0x10504, "0x%llx") && passed;

        actual_itr++;
    }
    passed = context.are_equal(actual_itr == linked.cend(), true, 0x10505, "%d") && passed;

    // Iterate backward.
    actual_itr = linked.rend();
    for (std::size_t i = 0; i < expected_len; i++) {
        context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x10506, "forward[%zd]=0x%x", expected_len - i - 1, expected[expected_len - i - 1].first);

        abc::vmem::page page(pool, *actual_itr, context.log());
        LinkedPage* linked_page = static_cast<LinkedPage*>(page.ptr());

        passed = context.are_equal(actual_itr == expected[expected_len - i - 1].second, true, 0x10507, "%d") && passed;
        passed = context.are_equal(linked_page->data, expected[expected_len - i - 1].first, 0x10508, "0x%llx") && passed;

        actual_itr--;
    }
    passed = context.are_equal(actual_itr == linked.crbegin(), true, 0x10509, "%d") && passed;

    return passed;
}


bool create_vmem_pool(test_context& context, abc::vmem::pool* pool, bool fit) {
    constexpr const char* suborigin = "create_vmem_pool()";

    bool passed = true;

    context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x103f1, "--- page2");
    abc::vmem::page page2(pool, context.log());
    context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x103f2, "page2 pos=0x%llx, ptr=%p", (unsigned long long)page2.pos(), page2.ptr());
    passed = context.are_equal(2ULL, (unsigned long long)page2.pos(), 0x103f3, "0x%llx") && passed;
    std::memset(page2.ptr(), 0x22, abc::vmem::page_size);

    {
        context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x103f4, "--- page3a");
        abc::vmem::page page3a(pool, context.log());
        context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x103f5, "page3a pos=0x%llx, ptr=%p", (unsigned long long)page3a.pos(), page3a.ptr());
        passed = context.are_equal(3ULL, (unsigned long long)page3a.pos(), 0x103f6, "0x%llx") && passed;
        std::memset(page3a.ptr(), 0x33, abc::vmem::page_size);

        context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x103f7, "--- page3b");
        abc::vmem::page page3b(pool, page3a.pos(), context.log());
        context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x103f8, "page3b pos=0x%llx, ptr=%p", (unsigned long long)page3b.pos(), page3b.ptr());
        passed = context.are_equal(3ULL, (unsigned long long)page3b.pos(), 0x103f9, "0x%llx") && passed;

        context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x1043c, "--- page4");
        abc::vmem::page page4(pool, context.log());
        context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x1043d, "page4 pos=0x%llx, ptr=%p", (unsigned long long)page4.pos(), page4.ptr());
        passed = context.are_equal(4ULL, (unsigned long long)page4.pos(), 0x1043e, "0x%llx") && passed;
        std::memset(page4.ptr(), 0x44, abc::vmem::page_size);

        try {
            context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x103fa, "--- page5");
            abc::vmem::page page5(pool, context.log());
            context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x103fc, "page5 pos=0x%llx, ptr=%p", (unsigned long long)page5.pos(), page5.ptr());
            passed = context.are_equal(5ULL, (unsigned long long)page5.pos(), 0x103fd, "0x%llx") && passed;
            std::memset(page5.ptr(), 0x55, abc::vmem::page_size);
            passed = context.are_equal<bool>(fit, true, __TAG__, "%d") && passed;
        }
        catch (const abc::diag::exception<std::runtime_error>& ex) {
            passed = context.are_equal<bool>(fit, false, __TAG__, "%d") && passed;
        }
    }

    return passed;
}


bool verify_bytes(test_context& context, const void* buffer, std::size_t begin_pos, std::size_t end_pos, std::uint8_t b, abc::diag::tag_t tag) {
    constexpr const char* suborigin = "verify_bytes()";

    bool passed = true;

    const std::uint8_t* byte_buffer = reinterpret_cast<const std::uint8_t*>(buffer);
    for (std::size_t i = begin_pos; i < end_pos; i++) {
        if (byte_buffer[i] != b) {
            if (i == begin_pos) {
                context.log()->put_any(origin, suborigin, abc::diag::severity::debug, tag, "Verifying 0x%x", b);
            }

            context.log()->put_any(origin, suborigin, abc::diag::severity::optional, tag, "i = %zu", i);
            passed = context.are_equal<std::uint8_t>(byte_buffer[i], b, tag, "0x%x") && passed;
        }
    }

    return passed;
}

