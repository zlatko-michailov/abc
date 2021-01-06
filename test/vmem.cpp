/*
MIT License

Copyright (c) 2018-2021 Zlatko Michailov 

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


#include "vmem.h"
#include "heap.h"


namespace abc { namespace test { namespace vmem {

	using Log = abc::test::log;
	using Pool = abc::vmem_pool<3, Log>;


	struct ItemMany {
		std::uint64_t					data;
		std::array<std::uint8_t, 900>	dummy;
	};


	// IMPORTANT: Ensure a predictable layout of the data on disk!
	#pragma pack(push, 1)

	struct test_start_page {
		abc::vmem_list_state list_state;
	};

	#pragma pack(pop)


	template <typename List>
	bool insert_vmem_list_items(test_context<abc::test::log>& context, List& list, std::size_t count);

	template <typename Pool>
	bool create_vmem_pool(test_context<abc::test::log>& context, Pool* pool, bool fit);

	bool verify_bytes(test_context<abc::test::log>& context, const void* buffer, std::size_t begin_pos, std::size_t end_pos, std::uint8_t b);


	bool test_vmem_pool_fit(test_context<abc::test::log>& context) {
		bool passed = true;

		Pool pool("out/test/pool_fit.vmem", context.log);
		passed = create_vmem_pool(context, &pool, true) && passed;

		return passed;
	}


	bool test_vmem_pool_exceed(test_context<abc::test::log>& context) {
		using PoolExceed = abc::vmem_pool<2, Log>;

		bool passed = true;

		PoolExceed pool("out/test/pool_exceed.vmem", context.log);
		passed = create_vmem_pool(context, &pool, false) && passed;

		return passed;
	}


	bool test_vmem_pool_reopen(test_context<abc::test::log>& context) {
		bool passed = true;

		{
			Pool pool("out/test/pool_reopen.vmem", context.log);
			passed = create_vmem_pool(context, &pool, true) && passed;
		}

		Pool pool("out/test/pool_reopen.vmem", context.log);

		// Page 0 (root page)
		{
			abc::vmem_page<Pool, Log> page(&pool, 0, context.log);

			_vmem_root_page expected;
			expected.free_pages.item_size = static_cast<vmem_item_pos_t>(sizeof(vmem_page_pos_t));
			int cmp = std::memcmp(&expected, page.ptr(), sizeof(_vmem_root_page));
			passed = context.are_equal<int>(cmp, 0, __TAG__, "%d") && passed;

			passed = verify_bytes(context, page.ptr(), sizeof(_vmem_root_page), abc::vmem_page_size, 0x00) && passed;
		}

		// Page 1 (start page)
		{
			abc::vmem_page<Pool, Log> page(&pool, 1, context.log);
			passed = verify_bytes(context, page.ptr(), sizeof(_vmem_root_page), abc::vmem_page_size, 0x00) && passed;
		}

		// Page 2
		{
			abc::vmem_page<Pool, Log> page(&pool, 2, context.log);
			passed = verify_bytes(context, page.ptr(), 0, abc::vmem_page_size, 0x22) && passed;
		}

		// Page 3
		{
			abc::vmem_page<Pool, Log> page(&pool, 3, context.log);
			passed = verify_bytes(context, page.ptr(), 0, abc::vmem_page_size, 0x33) && passed;
		}

		// Page 4
		{
			abc::vmem_page<Pool, Log> page(&pool, 4, context.log);
			passed = verify_bytes(context, page.ptr(), 0, abc::vmem_page_size, 0x44) && passed;
		}

		return passed;
	}


	bool test_vmem_list_insert(test_context<abc::test::log>& context) {
		using Item = std::array<std::uint8_t, 900>;
		using List = abc::vmem_list<Item, Pool, Log>;
		using Iterator = abc::vmem_list_iterator<Item, Pool, Log>;

		bool passed = true;

		Pool pool("out/test/list_insert.vmem", context.log);

		abc::vmem_list_state list_state;
		List list(&list_state, &pool, context.log);
		Item item;
		
		item.fill(0x21);
		Iterator itr = list.insert(list.end(), item);
		Iterator itr21 = itr;
		passed = context.are_equal(itr == list.begin(), true, __TAG__, "%d") && passed;
		passed = context.are_equal(itr == list.rend(), true, __TAG__, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 1, __TAG__, "%zu") && passed;
		// | 21 __ __ __ |

		item.fill(0x22);
		itr = list.insert(list.end(), item);
		passed = context.are_equal(itr == list.rend(), true, __TAG__, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 2, __TAG__, "%zu") && passed;
		// | 21 22 __ __ |

		item.fill(0x23);
		itr = list.insert(itr21, item);
		passed = context.are_equal(itr == itr21, true, __TAG__, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 3, __TAG__, "%zu") && passed;
		// | 23 21 22 __ |

		item.fill(0x24);
		itr = list.insert(list.begin(), item);
		passed = context.are_equal(itr == list.begin(), true, __TAG__, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 4, __TAG__, "%zu") && passed;
		// | 24 23 21 22 |

		itr21 = list.begin();
		itr21++;
		itr21++;
		item.fill(0x25);
		itr = list.insert(itr21, item);
		passed = context.are_equal(itr == itr21, true, __TAG__, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 5, __TAG__, "%zu") && passed;
		// | 24 23 25 __ | 21 22 __ __ |

		item.fill(0x26);
		itr = list.insert(list.end(), item);
		passed = context.are_equal(itr == list.rend(), true, __TAG__, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 6, __TAG__, "%zu") && passed;
		// | 24 23 25 __ | 21 22 26 __ |

		item.fill(0x27);
		itr = list.insert(list.begin(), item);
		passed = context.are_equal(itr == list.begin(), true, __TAG__, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 7, __TAG__, "%zu") && passed;
		// | 27 24 23 25 | 21 22 26 __ |

		item.fill(0x28);
		itr = list.insert(list.begin(), item);
		passed = context.are_equal(itr == list.begin(), true, __TAG__, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 8, __TAG__, "%zu") && passed;
		// | 28 __ __ __ | 27 24 23 25 | 21 22 26 __ |

		std::uint8_t b[] = { 0x28, 0x27, 0x24, 0x23, 0x25, 0x21, 0x22, 0x26 };
		constexpr std::size_t b_len = sizeof(b) / sizeof(std::uint8_t); 

		// Iterate forward.
		itr = list.cbegin();
		for (std::size_t i = 0; i < b_len; i++) {
			passed = verify_bytes(context, itr->data(), 0, sizeof(Item), b[i]) && passed;
			itr++;
		}
		passed = context.are_equal(itr == list.cend(), true, __TAG__, "%d") && passed;

		// Iterate backwards.
		itr = list.crend();
		for (std::size_t i = 0; i < b_len; i++) {
			passed = verify_bytes(context, itr->data(), 0, sizeof(Item), b[b_len - i - 1]) && passed;
			itr--;
		}
		passed = context.are_equal(itr == list.crbegin(), true, __TAG__, "%d") && passed;

		return passed;
	}


	bool test_vmem_list_insertmany(test_context<abc::test::log>& context) {
		using List = abc::vmem_list<ItemMany, Pool, Log>;

		bool passed = true;

		Pool pool("out/test/list_insertmany.vmem", context.log);

		abc::vmem_list_state list_state;
		List list(&list_state, &pool, context.log);

		passed = insert_vmem_list_items(context, list, 4000) && passed;

		return passed;
	}


	bool test_vmem_list_erase(test_context<abc::test::log>& context) {
		using List = abc::vmem_list<ItemMany, Pool, Log>;

		bool passed = true;

		Pool pool("out/test/list_erase.vmem", context.log);

		abc::vmem_list_state list_state;
		List list(&list_state, &pool, context.log);

		passed = insert_vmem_list_items(context, list, 12) && passed;
		// | 00 01 02 03 | 04 05 06 07 | 08 09 0a 0b |

		typename List::iterator itr = list.begin();
		itr++; itr++; itr++; itr++;

		for (std::size_t i = 0; i < 4; i++) {
			itr = list.erase(itr);
			passed = context.are_equal<unsigned long long>(itr->data, 0x05 + i, __TAG__, "%llu") && passed;
			passed = context.are_equal<std::size_t>(list.size(), 11 - i, __TAG__, "%zu") && passed;
		}
		// | 00 01 02 03 | 08 09 0a 0b |

		for (std::size_t i = 0; i < 3; i++) {
			itr = list.erase(list.rend());
			passed = context.are_equal<bool>(itr == list.end(), true, __TAG__, "%d") && passed;
			itr--;
			passed = context.are_equal<unsigned long long>(itr->data, 0x0a - i, __TAG__, "%llu") && passed;
			passed = context.are_equal<std::size_t>(list.size(), 7 - i, __TAG__, "%zu") && passed;
		}
		// | 00 01 02 03 | 08 __ __ __ |

		itr = list.erase(list.rend());
		itr--;
		passed = context.are_equal<unsigned long long>(itr->data, 0x03, __TAG__, "%llu") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 4, __TAG__, "%zu") && passed;
		// | 00 01 02 03 |

		for (std::size_t i = 0; i < 3; i++) {
			itr = list.erase(list.rend());
			itr--;
			passed = context.are_equal<unsigned long long>(itr->data, 0x02 - i, __TAG__, "%llu") && passed;
			passed = context.are_equal<std::size_t>(list.size(), 3 - i, __TAG__, "%zu") && passed;
		}
		// | 00 __ __ __ |

		itr = list.erase(list.rend());
		passed = context.are_equal<std::size_t>(list.size(), 0, __TAG__, "%zu") && passed;
		// <empty>

		passed = context.are_equal<bool>(list.begin() == list.end(), true, __TAG__, "%d") && passed;
		passed = context.are_equal<bool>(list.rend() == list.rbegin(), true, __TAG__, "%d") && passed;

		return passed;
	}


	template <typename List>
	bool insert_vmem_list_items(test_context<abc::test::log>& context, List& list, std::size_t count) {
		bool passed = true;

		// Insert.
		for (std::size_t i = 0; i < count; i++) {
			ItemMany item = { i, { 0 } };
			list.insert(list.end(), item);
		}

		// Iterate forward.
		typename List::iterator itr = list.cbegin();
		for (std::size_t i = 0; i < count; i++) {
			passed = context.are_equal<unsigned long long>(itr->data, i, __TAG__, "%llu") && passed;
			itr++;
		}
		passed = context.are_equal(itr == list.cend(), true, __TAG__, "%d") && passed;

		// Iterate backwards.
		itr = list.crend();
		for (std::size_t i = 0; i < count; i++) {
			passed = context.are_equal<unsigned long long>(itr->data, count - i - 1, __TAG__, "%llu") && passed;
			itr--;
		}
		passed = context.are_equal(itr == list.crbegin(), true, __TAG__, "%d") && passed;

		return passed;
	}


	template <typename Pool>
	bool create_vmem_pool(test_context<abc::test::log>& context, Pool* pool, bool fit) {
		bool passed = true;

		context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "--- page2");
		abc::vmem_page<Pool, Log> page2(pool, context.log);
		context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "page2 pos=0x%llx, ptr=%p", (long long)page2.pos(), page2.ptr());
		passed = context.are_equal(2LL, (long long)page2.pos(), __TAG__, "0x%llx") && passed;
		std::memset(page2.ptr(), 0x22, abc::vmem_page_size);

		{
			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "--- page3a");
			abc::vmem_page<Pool, Log> page3a(pool, context.log);
			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "page3a pos=0x%llx, ptr=%p", (long long)page3a.pos(), page3a.ptr());
			passed = context.are_equal(3LL, (long long)page3a.pos(), __TAG__, "0x%llx") && passed;
			std::memset(page3a.ptr(), 0x33, abc::vmem_page_size);

			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "--- page3b");
			abc::vmem_page<Pool, Log> page3b(pool, page3a.pos(), context.log);
			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "page3b pos=0x%llx, ptr=%p", (long long)page3b.pos(), page3b.ptr());
			passed = context.are_equal(3LL, (long long)page3b.pos(), __TAG__, "0x%llx") && passed;

			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "--- page4");
			abc::vmem_page<Pool, Log> page4(pool, context.log);
			passed = context.are_equal<bool>(page4.ptr() != nullptr, fit, __TAG__, "%d") && passed;
			if (page4.ptr() != nullptr) {
				context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "page4 pos=0x%llx, ptr=%p", (long long)page4.pos(), page4.ptr());
				passed = context.are_equal(4LL, (long long)page4.pos(), __TAG__, "0x%llx") && passed;
				std::memset(page4.ptr(), 0x44, abc::vmem_page_size);
			}
		}

		return passed;
	}


	bool verify_bytes(test_context<abc::test::log>& context, const void* buffer, std::size_t begin_pos, std::size_t end_pos, std::uint8_t b) {
		bool passed = true;

		const std::uint8_t* byte_buffer = reinterpret_cast<const std::uint8_t*>(buffer);
		for (std::size_t i = begin_pos; i < end_pos; i++) {
			if (byte_buffer[i] != b) {
				if (i == begin_pos) {
					context.log->put_any(abc::category::any, abc::severity::debug, __TAG__, "Verifying 0x%x", b);
				}

				context.log->put_any(abc::category::any, abc::severity::optional, __TAG__, "i = %zu", i);
				passed = context.are_equal<std::uint8_t>(byte_buffer[i], b, __TAG__, "%d") && passed;
			}
		}

		return passed;
	}

}}}

