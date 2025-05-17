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


#include <iostream>
#include <string>
#include <array>

#include "../../src/diag/diag_ready.h"
#include "../../src/vmem/all.h"


// IMPORTANT: Ensure a predictable layout of the data on disk!
#pragma pack(push, 1)

// Max 4 items per vmem page.
using list_item = struct {
    std::uint64_t                 data; // Use types with predictable size!
    std::array<std::uint8_t, 900> dummy;
};

using start_page_layout = struct {
    abc::vmem::list_state   list1;
    abc::vmem::list_state   list2;
    abc::vmem::list_state   list3;
    abc::vmem::string_state str1;
    abc::vmem::string_state str2;
};

#pragma pack(pop)


constexpr const char* origin = "sample_vmem";


void work_with_list(abc::vmem::list_state* list_state, abc::vmem::pool* pool, abc::diag::log_ostream* log, const char* list_name, std::size_t items_to_add);


int main(int /*argc*/, const char* argv[]) {
    constexpr const char* suborigin = "work_with_list()";

    // Create a log.
    abc::table_ostream table(std::cout.rdbuf());
    abc::diag::debug_line_ostream<> line(&table);
    abc::diag::str_log_filter<const char*> filter("", abc::diag::severity::important);
    abc::diag::log_ostream log(&line, &filter);

    // Use the path to this program to build the path to the pool file.
    std::string process_dir = abc::parent_path(argv[0]);
    std::string pool_path = process_dir + "/pool.vmem";

    // Construct a pool instance.
    // If the file doesn't exist, the pool will be initialized.
    // If the fie exists, it should be a valid pool.
    abc::vmem::pool_config config(pool_path.c_str(), 8);
    abc::vmem::pool pool(std::move(config), &log);

    // Map and lock the start page in memory.
    abc::vmem::page start_page(&pool, abc::vmem::page_pos_start, &log);
    start_page_layout* start_page_ptr = reinterpret_cast<start_page_layout*>(start_page.ptr());

    work_with_list(&start_page_ptr->list1, &pool, &log, "list1", 1);
    work_with_list(&start_page_ptr->list2, &pool, &log, "list2", 5);

    // Compare vmem_ptr instances.
    abc::vmem::ptr<std::uint8_t> p1(&pool, abc::vmem::page_pos_start, 12, &log);
    abc::vmem::ptr<std::uint8_t> p2(&pool, abc::vmem::page_pos_start, 12, &log);
    abc::vmem::ptr<std::uint8_t> p3(&pool, abc::vmem::page_pos_start, 34, &log);
    abc::vmem::ptr<std::uint8_t> p4(nullptr);
    log.put_any(origin, suborigin, abc::diag::severity::important, 0x107a6, "(p1 == p2) = %d", p1 == p2);
    log.put_any(origin, suborigin, abc::diag::severity::important, 0x107a7, "(p1 == p3) = %d", p1 == p3);
    log.put_any(origin, suborigin, abc::diag::severity::important, 0x107a8, "(p1 == nullptr) = %d", p1 == nullptr);
    log.put_any(origin, suborigin, abc::diag::severity::important, 0x107a9, "(p4 == nullptr) = %d", p4 == nullptr);

    // List iterator
    abc::vmem::list<int> list3(&start_page_ptr->list3, &pool, &log);
    list3.push_back(42);
    list3.push_back(43);
    list3.push_back(44);
    for (auto itr = list3.begin(); itr != list3.end(); itr++) {
        log.put_any(origin, suborigin, abc::diag::severity::important, 0x107aa, "%d", *itr);
    }

    // String iterator
    abc::vmem::string str1(&start_page_ptr->str1, &pool, &log);
    str1.push_back('x');
    str1.push_back('y');
    str1.push_back('z');
    for (abc::vmem::string::const_iterator itr = str1.begin(); itr != str1.end(); ) {
        log.put_any(origin, suborigin, abc::diag::severity::important, 0x107ab, "%c", *itr++);
    }

    // Work with streams over vmem_string.
    abc::vmem::string str2(&start_page_ptr->str2, &pool, &log);
    abc::vmem::string_streambuf sb(&str2, &log);
    std::ostream ostrm(&sb);
    ostrm << "abc" << 12 << "xyz";

    for (abc::vmem::string::const_iterator itr = str2.begin(); itr != str2.end(); itr++) {
        log.put_any(origin, suborigin, abc::diag::severity::important, 0x107ac, "%c", *itr);
    }

    std::istream istrm(&sb);
    std::string stdstr;
    istrm >> stdstr;
    log.put_any(origin, suborigin, abc::diag::severity::important, 0x107ad, "'%s'", stdstr.c_str());

    return 0;
}


void work_with_list(abc::vmem::list_state* list_state, abc::vmem::pool* pool, abc::diag::log_ostream* log, const char* list_name, std::size_t items_to_add) {
    constexpr const char* suborigin = "work_with_list()";

    log->put_any(origin, suborigin, abc::diag::severity::important, 0x10341, "---------- %s ----------", list_name);

    // Construct a list for the given state.
    abc::vmem::list<list_item> list(list_state, pool, log);

    // Print the initial size of the list.
    std::size_t size = list.size();
    log->put_any(origin, suborigin, abc::diag::severity::important, 0x10342, "Initial size=%zu", size);

    // Print the elements
    for (abc::vmem::list<list_item>::const_iterator itr = list.begin(); itr != list.end(); itr++) {
        log->put_any(origin, suborigin, abc::diag::severity::important, 0x10343, "%llu", (unsigned long long)itr->data);
    }

    // Add more items.
    log->put_any(origin, suborigin, abc::diag::severity::important, 0x10344, "Adding...");
    for (std::size_t i = 0; i < items_to_add; i++) {
        list_item item { (std::uint64_t)(size + i), { } };

        list.insert(list.end(), item);
        log->put_any(origin, suborigin, abc::diag::severity::important, 0x10345, "%llu", (unsigned long long)item.data);
    }

    // Print the final size of the list.
    size = list.size();
    log->put_any(origin, suborigin, abc::diag::severity::important, 0x10346, "Final size=%zu", size);
}

