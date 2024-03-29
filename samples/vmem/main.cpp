/*
MIT License

Copyright (c) 2018-2023 Zlatko Michailov 

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
#include <cstring>
#include <array>

#include "../../src/log.h"
#include "../../src/vmem.h"


using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;

// Max 8 pages = 32KB in memory.
using vmem_pool = abc::vmem_pool<8, log_ostream>;

using vmem_page = abc::vmem_page<vmem_pool, log_ostream>;

// IMPORTANT: Ensure a predictable layout of the data on disk!
#pragma pack(push, 1)

// Max 4 items per vmem_page.
using vmem_list_item = struct {
	std::uint64_t					data; // Use types with predictable size!
	std::array<std::uint8_t, 900>	dummy;
};

using vmem_start_page = struct {
	abc::vmem_list_state	list1;
	abc::vmem_list_state	list2;
	abc::vmem_list_state	list3;
	abc::vmem_string_state	str1;
	abc::vmem_string_state	str2;
};

#pragma pack(pop)

using vmem_list = abc::vmem_list<vmem_list_item, vmem_pool, log_ostream>;
using vmem_string = abc::vmem_string<vmem_pool, log_ostream>;
using vmem_string_streambuf = abc::vmem_string_streambuf<vmem_pool, log_ostream>;
using vmem_string_iterator = abc::vmem_string_iterator<vmem_pool, log_ostream>;


void work_with_list(abc::vmem_list_state* list_state, vmem_pool* pool, log_ostream* log, const char* list_name, std::size_t items_to_add);


int main(int /*argc*/, const char* argv[]) {
	// Create a log.
	abc::log_filter filter(abc::severity::optional);
	log_ostream log(std::cout.rdbuf(), &filter);


	// Use the path to this program to build the path to the pool file.
	constexpr std::size_t max_path = abc::size::k1;
	char path[max_path];
	path[0] = '\0';

	constexpr const char pool_path[] = "pool.vmem";
	std::size_t pool_path_len = std::strlen(pool_path); 

	const char* prog_last_separator = std::strrchr(argv[0], '/');
	std::size_t prog_path_len = 0;

	if (prog_last_separator != nullptr) {
		prog_path_len = prog_last_separator - argv[0] + 1;
		std::size_t full_path_len = prog_path_len + pool_path_len;

		if (full_path_len >= max_path) {
			log.put_any(abc::category::abc::samples, abc::severity::critical, 0x102f4,
				"This sample allows paths up to %zu chars. The path to this process is %zu chars. To continue, either move the current dir closer to the process, or increase the path limit in main.cpp.",
				max_path, full_path_len);

			return 1;
		}

		std::strncpy(path, argv[0], prog_path_len);
	}

	std::strcpy(path + prog_path_len, pool_path);
	log.put_any(abc::category::abc::samples, abc::severity::optional, 0x10340, "path='%s'", path);


	// Construct a pool instance.
	// If the file doesn't exist, the pool will be initialized.
	// If the fie exists, it should be a valid pool.
	vmem_pool pool(path, &log);

	// Map and lock the start page in memory.
	vmem_page start_page(&pool, abc::vmem_page_pos_start, &log);
	vmem_start_page* start_page_ptr = reinterpret_cast<vmem_start_page*>(start_page.ptr());

	work_with_list(&start_page_ptr->list1, &pool, &log, "list1", 1);
	work_with_list(&start_page_ptr->list2, &pool, &log, "list2", 5);

	// Compare vmem_ptr instances.
	abc::vmem_ptr<std::uint8_t, vmem_pool, log_ostream> p1(&pool, abc::vmem_page_pos_start, 12, &log);
	abc::vmem_ptr<std::uint8_t, vmem_pool, log_ostream> p2(&pool, abc::vmem_page_pos_start, 12, &log);
	abc::vmem_ptr<std::uint8_t, vmem_pool, log_ostream> p3(&pool, abc::vmem_page_pos_start, 34, &log);
	abc::vmem_ptr<std::uint8_t, vmem_pool, log_ostream> p4(nullptr);
	log.put_any(abc::category::abc::samples, abc::severity::important, 0x107a6, "(p1 == p2) = %d", p1 == p2);
	log.put_any(abc::category::abc::samples, abc::severity::important, 0x107a7, "(p1 == p3) = %d", p1 == p3);
	log.put_any(abc::category::abc::samples, abc::severity::important, 0x107a8, "(p1 == nullptr) = %d", p1 == nullptr);
	log.put_any(abc::category::abc::samples, abc::severity::important, 0x107a9, "(p4 == nullptr) = %d", p4 == nullptr);

	// List iterator
	abc::vmem_list<int, vmem_pool, log_ostream> list3(&start_page_ptr->list3, &pool, &log);
	list3.push_back(42);
	list3.push_back(43);
	list3.push_back(44);
	for (auto itr = list3.begin(); itr != list3.end(); itr++) {
		log.put_any(abc::category::abc::samples, abc::severity::important, 0x107aa, "%d", *itr);
	}

	// String iterator
	vmem_string str1(&start_page_ptr->str1, &pool, &log);
	str1.push_back('x');
	str1.push_back('y');
	str1.push_back('z');
	for (vmem_string_iterator itr = str1.begin(); itr != str1.end(); ) {
		log.put_any(abc::category::abc::samples, abc::severity::important, 0x107ab, "%c", *itr++);
	}

	// Work with streams over vmem_string.
	vmem_string str2(&start_page_ptr->str2, &pool, &log);
	vmem_string_streambuf sb(&str2, &log);
	std::ostream ostrm(&sb);
	ostrm << "abc" << 12 << "xyz";

	for (vmem_string_iterator itr = str2.begin(); itr != str2.end(); itr++) {
		log.put_any(abc::category::abc::samples, abc::severity::important, 0x107ac, "%c", *itr);
	}

	std::istream istrm(&sb);
	std::string stdstr;
	istrm >> stdstr;
	log.put_any(abc::category::abc::samples, abc::severity::important, 0x107ad, "'%s'", stdstr.c_str());

	return 0;
}


void work_with_list(abc::vmem_list_state* list_state, vmem_pool* pool, log_ostream* log, const char* list_name, std::size_t items_to_add) {
	log->put_any(abc::category::abc::samples, abc::severity::important, 0x10341, "---------- %s ----------", list_name);

	// Construct a list for the given state.
	vmem_list list(list_state, pool, log);

	// Print the initial size of the list.
	std::size_t size = list.size();
	log->put_any(abc::category::abc::samples, abc::severity::important, 0x10342, "Initial size=%zu", size);

	// Print the elements
	for (vmem_list::iterator itr = list.begin(); itr != list.end(); itr++) {
		log->put_any(abc::category::abc::samples, abc::severity::important, 0x10343, "%lu", (long)itr->data);
	}

	// Add more items.
	log->put_any(abc::category::abc::samples, abc::severity::important, 0x10344, "Adding...");
	for (std::size_t i = 0; i < items_to_add; i++) {
		vmem_list_item item { (std::uint64_t)(size + i), { } };

		list.insert(list.end(), item);
		log->put_any(abc::category::abc::samples, abc::severity::important, 0x10345, "%llu", (long long)item.data);
	}

	// Print the final size of the list.
	size = list.size();
	log->put_any(abc::category::abc::samples, abc::severity::important, 0x10346, "Final size=%zu", size);
}

