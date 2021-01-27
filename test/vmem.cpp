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


#include "inc/vmem.h"


namespace abc { namespace test { namespace vmem {

	using Log = abc::test::log;
	using PoolMin = abc::vmem_pool<3, Log>;
	using PoolFit = abc::vmem_pool<4, Log>;
	using PoolExceed = abc::vmem_pool<3, Log>;
	using PoolFree = abc::vmem_pool<5, Log>;


	struct ItemMany {
		std::uint64_t					data;
		std::array<std::uint8_t, 900>	dummy;
	};


	template <typename List>
	bool insert_vmem_list_items(test_context<abc::test::log>& context, List& list, std::size_t count);

	template <typename Pool>
	bool create_vmem_pool(test_context<abc::test::log>& context, Pool* pool, bool fit);

	bool verify_bytes(test_context<abc::test::log>& context, const void* buffer, std::size_t begin_pos, std::size_t end_pos, std::uint8_t b);


	bool test_vmem_pool_fit(test_context<abc::test::log>& context) {
		using Pool = PoolFit;

		bool passed = true;

		Pool pool("out/test/pool_fit.vmem", context.log);
		passed = create_vmem_pool(context, &pool, true) && passed;

		return passed;
	}


	bool test_vmem_pool_exceed(test_context<abc::test::log>& context) {
		using Pool = PoolExceed;

		bool passed = true;

		Pool pool("out/test/pool_exceed.vmem", context.log);
		passed = create_vmem_pool(context, &pool, false) && passed;

		return passed;
	}


	bool test_vmem_pool_reopen(test_context<abc::test::log>& context) {
		using Pool = PoolFit;

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
			passed = context.are_equal<int>(cmp, 0, 0x103bd, "%d") && passed;

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

		// Page 5
		{
			abc::vmem_page<Pool, Log> page(&pool, 5, context.log);
			passed = verify_bytes(context, page.ptr(), 0, abc::vmem_page_size, 0x55) && passed;
		}

		return passed;
	}


	bool test_vmem_pool_freepages(test_context<abc::test::log>& context) {
		using Pool = PoolFree;

		bool passed = true;

		Pool pool("out/test/pool_freepages.vmem", context.log);

		{
			// Page 2
			abc::vmem_page<Pool, Log> page2(&pool, context.log);
			passed = context.are_equal(page2.ptr() != nullptr, true, 0x103be, "%d") && passed;
			passed = context.are_equal((long long)page2.pos(), 2LL, 0x103bf, "0x%llx") && passed;

			// Page 3
			abc::vmem_page<Pool, Log> page3(&pool, context.log);
			passed = context.are_equal(page3.ptr() != nullptr, true, 0x103c0, "%d") && passed;
			passed = context.are_equal((long long)page3.pos(), 3LL, 0x103c1, "0x%llx") && passed;

			// Page 4
			abc::vmem_page<Pool, Log> page4(&pool, context.log);
			passed = context.are_equal(page4.ptr() != nullptr, true, 0x103c2, "%d") && passed;
			passed = context.are_equal((long long)page4.pos(), 4LL, 0x103c3, "0x%llx") && passed;

			// Page 5
			abc::vmem_page<Pool, Log> page5(&pool, context.log);
			passed = context.are_equal(page5.ptr() != nullptr, true, 0x103c4, "%d") && passed;
			passed = context.are_equal((long long)page5.pos(), 5LL, 0x103c5, "0x%llx") && passed;

			page2.free();
			page3.free();
			page4.free();
			page5.free();
		}

		{
			// Page 5
			abc::vmem_page<Pool, Log> page5(&pool, context.log);
			passed = context.are_equal(page5.ptr() != nullptr, true, 0x103c6, "%d") && passed;
			passed = context.are_equal((long long)page5.pos(), 5LL, 0x103c7, "0x%llx") && passed;

			// Page 4
			abc::vmem_page<Pool, Log> page4(&pool, context.log);
			passed = context.are_equal(page4.ptr() != nullptr, true, 0x103c8, "%d") && passed;
			passed = context.are_equal((long long)page4.pos(), 4LL, 0x103c9, "0x%llx") && passed;

			// Page 3
			abc::vmem_page<Pool, Log> page3(&pool, context.log);
			passed = context.are_equal(page3.ptr() != nullptr, true, 0x103ca, "%d") && passed;
			passed = context.are_equal((long long)page3.pos(), 3LL, 0x103cb, "0x%llx") && passed;

			// Page 2
			abc::vmem_page<Pool, Log> page2(&pool, context.log);
			passed = context.are_equal(page2.ptr() != nullptr, true, 0x103cc, "%d") && passed;
			passed = context.are_equal((long long)page2.pos(), 2LL, 0x103cd, "0x%llx") && passed;
		}

		return passed;
	}


	bool test_vmem_list_insert(test_context<abc::test::log>& context) {
		using Pool = PoolMin;

		using Item = std::array<std::uint8_t, 900>;
		using List = abc::vmem_list<Item, Pool, Log>;
		using Iterator = abc::vmem_list_iterator<Item, Pool, Log>;

		bool passed = true;

		Pool pool("out/test/list_insert.vmem", context.log);

		abc::vmem_list_state list_state;
		List list(&list_state, &pool, context.log);
		Item item;

		item.fill(0x21);
		Iterator actual_itr = list.insert(list.end(), item);
		Iterator expected_itr = Iterator(&list, 2U, 0U, abc::vmem_iterator_edge::none, context.log);
		Iterator itr21 = actual_itr;
		passed = context.are_equal(actual_itr == expected_itr, true, __TAG__, "%d") && passed;
		passed = context.are_equal(actual_itr == list.begin(), true, 0x103ce, "%d") && passed;
		passed = context.are_equal(actual_itr == list.rend(), true, 0x103cf, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 1, 0x103d0, "%zu") && passed;
		// | (2)
		// | 21 __ __ __ |

		item.fill(0x22);
		actual_itr = list.insert(list.end(), item);
		expected_itr = Iterator(&list, 2U, 1U, abc::vmem_iterator_edge::none, context.log);
		passed = context.are_equal(actual_itr == expected_itr, true, __TAG__, "%d") && passed;
		passed = context.are_equal(actual_itr == list.rend(), true, 0x103d1, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 2, 0x103d2, "%zu") && passed;
		// | (2)
		// | 21 22 __ __ |

		item.fill(0x23);
		actual_itr = list.insert(itr21, item);
		expected_itr = Iterator(&list, 2U, 0U, abc::vmem_iterator_edge::none, context.log);
		passed = context.are_equal(actual_itr == expected_itr, true, __TAG__, "%d") && passed;
		passed = context.are_equal(actual_itr == itr21, true, 0x103d3, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 3, 0x103d4, "%zu") && passed;
		// | (2)
		// | 23 21 22 __ |

		item.fill(0x24);
		actual_itr = list.insert(list.begin(), item);
		expected_itr = Iterator(&list, 2U, 0U, abc::vmem_iterator_edge::none, context.log);
		passed = context.are_equal(actual_itr == expected_itr, true, __TAG__, "%d") && passed;
		passed = context.are_equal(actual_itr == list.begin(), true, 0x103d5, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 4, 0x103d6, "%zu") && passed;
		// | (2)
		// | 24 23 21 22 |

		itr21 = Iterator(&list, 2U, 2U, abc::vmem_iterator_edge::none, context.log);
		item.fill(0x25);
		actual_itr = list.insert(itr21, item);
		expected_itr = Iterator(&list, 2U, 2U, abc::vmem_iterator_edge::none, context.log);
		passed = context.are_equal(actual_itr == expected_itr, true, __TAG__, "%d") && passed;
		Iterator rend_itr = Iterator(&list, 3U, 1U, abc::vmem_iterator_edge::none, context.log);
		passed = context.are_equal(list.rend() == rend_itr, true, __TAG__, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 5, 0x103d8, "%zu") && passed;
		// | (2)         | (3)
		// | 24 23 25 __ | 21 22 __ __ |

		item.fill(0x26);
		actual_itr = list.insert(list.end(), item);
		expected_itr = Iterator(&list, 3U, 2U, abc::vmem_iterator_edge::none, context.log);
		passed = context.are_equal(actual_itr == expected_itr, true, __TAG__, "%d") && passed;
		passed = context.are_equal(actual_itr == list.rend(), true, 0x103d9, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 6, 0x103da, "%zu") && passed;
		// | (2)         | (3)
		// | 24 23 25 __ | 21 22 26 __ |

		item.fill(0x27);
		actual_itr = list.insert(list.begin(), item);
		expected_itr = Iterator(&list, 2U, 0U, abc::vmem_iterator_edge::none, context.log);
		passed = context.are_equal(actual_itr == expected_itr, true, __TAG__, "%d") && passed;
		passed = context.are_equal(actual_itr == list.begin(), true, 0x103db, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 7, 0x103dc, "%zu") && passed;
		// | (2)         | (3)
		// | 27 24 23 25 | 21 22 26 __ |

		item.fill(0x28);
		actual_itr = list.insert(list.begin(), item);
		expected_itr = Iterator(&list, 2U, 0U, abc::vmem_iterator_edge::none, context.log);
		passed = context.are_equal(actual_itr == expected_itr, true, __TAG__, "%d") && passed;
		passed = context.are_equal(actual_itr == list.begin(), true, 0x103dd, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 8, 0x103de, "%zu") && passed;
		// | (2)         | (4)         | (3)
		// | 28 27 24 __ | 23 25 __ __ | 21 22 26 __ |

		item.fill(0x29);
		actual_itr = list.insert(list.end(), item);
		expected_itr = Iterator(&list, 3U, 3U, abc::vmem_iterator_edge::none, context.log);
		passed = context.are_equal(actual_itr == expected_itr, true, __TAG__, "%d") && passed;
		passed = context.are_equal(actual_itr == list.rend(), true, __TAG__, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 9, __TAG__, "%zu") && passed;
		// | (2)         | (4)         | (3)
		// | 28 27 24 __ | 23 25 __ __ | 21 22 26 29 |

		item.fill(0x2a);
		actual_itr = list.insert(list.end(), item);
		expected_itr = Iterator(&list, 5U, 0U, abc::vmem_iterator_edge::none, context.log);
		passed = context.are_equal(actual_itr == expected_itr, true, __TAG__, "%d") && passed;
		passed = context.are_equal(actual_itr == list.rend(), true, __TAG__, "%d") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 10, __TAG__, "%zu") && passed;
		// | (2)         | (4)         | (3)         | (5)
		// | 28 27 24 __ | 23 25 __ __ | 21 22 26 29 | 2a __ __ __ |

		using Pair = std::pair<std::uint8_t, Iterator>;
		const Pair exp[] = {
			{ 0x28, Iterator(&list, 2U, 0U, abc::vmem_iterator_edge::none, context.log) },
			{ 0x27, Iterator(&list, 2U, 1U, abc::vmem_iterator_edge::none, context.log) },
			{ 0x24, Iterator(&list, 2U, 2U, abc::vmem_iterator_edge::none, context.log) },
			{ 0x23, Iterator(&list, 4U, 0U, abc::vmem_iterator_edge::none, context.log) },
			{ 0x25, Iterator(&list, 4U, 1U, abc::vmem_iterator_edge::none, context.log) },
			{ 0x21, Iterator(&list, 3U, 0U, abc::vmem_iterator_edge::none, context.log) },
			{ 0x22, Iterator(&list, 3U, 1U, abc::vmem_iterator_edge::none, context.log) },
			{ 0x26, Iterator(&list, 3U, 2U, abc::vmem_iterator_edge::none, context.log) },
			{ 0x29, Iterator(&list, 3U, 3U, abc::vmem_iterator_edge::none, context.log) },
			{ 0x2a, Iterator(&list, 5U, 0U, abc::vmem_iterator_edge::none, context.log) },
		};
		constexpr std::size_t exp_len = sizeof(exp) / sizeof(Pair); 

		// Iterate forward.
		actual_itr = list.cbegin();
		for (std::size_t i = 0; i < exp_len; i++) {
			context.log->put_any(abc::category::any, abc::severity::important, __TAG__, "forward[%zd]=0x%x", i, exp[i].first);
	
			passed = context.are_equal(actual_itr == exp[i].second, true, __TAG__, "%d") && passed;
			passed = verify_bytes(context, actual_itr->data(), 0, sizeof(Item), exp[i].first) && passed;
			actual_itr++;
		}
		passed = context.are_equal(actual_itr == list.cend(), true, 0x103df, "%d") && passed;

		// Iterate backwards.
		actual_itr = list.crend();
		for (std::size_t i = 0; i < exp_len; i++) {
			context.log->put_any(abc::category::any, abc::severity::important, __TAG__, "backward[%zd]=0x%x", i, exp[exp_len - i - 1].first);
	
			passed = context.are_equal(actual_itr == exp[exp_len - i - 1].second, true, __TAG__, "%d") && passed;
			passed = verify_bytes(context, actual_itr->data(), 0, sizeof(Item), exp[exp_len - i - 1].first) && passed;
			actual_itr--;
		}
		passed = context.are_equal(actual_itr == list.crbegin(), true, 0x103e0, "%d") && passed;

		return passed;
	}


	bool test_vmem_list_insertmany(test_context<abc::test::log>& context) {
		using Pool = PoolFit;
		using List = abc::vmem_list<ItemMany, Pool, Log>;

		bool passed = true;

		Pool pool("out/test/list_insertmany.vmem", context.log);

		abc::vmem_list_state list_state;
		List list(&list_state, &pool, context.log);

		passed = insert_vmem_list_items(context, list, 4000) && passed;

		return passed;
	}


	bool test_vmem_list_erase(test_context<abc::test::log>& context) {
		using Pool = PoolMin;
		using List = abc::vmem_list<ItemMany, Pool, Log>;
		using Iterator = abc::vmem_list_iterator<ItemMany, Pool, Log>;

		bool passed = true;

		Pool pool("out/test/list_erase.vmem", context.log);

		abc::vmem_list_state list_state;
		List list(&list_state, &pool, context.log);

		passed = insert_vmem_list_items(context, list, 16) && passed;
		// | (2)         | (3)         | (4)         | (5)
		// | 00 01 02 03 | 04 05 06 07 | 08 09 0a 0b | 0c 0d 0e 0f

		context.log->filter()->min_severity(abc::severity::optional); ////

		typename List::iterator itr_target = Iterator(&list, 4U, 3U, abc::vmem_iterator_edge::none, context.log);
		typename List::iterator itr_expected = Iterator(&list, 5U, 0U, abc::vmem_iterator_edge::none, context.log);
		typename List::iterator itr_actual = list.erase(itr_target);
		passed = context.are_equal<bool>(itr_actual == itr_expected, true, __TAG__, "%d") && passed;
		passed = context.are_equal<unsigned long long>(itr_actual->data, 0x0c, __TAG__, "0x%2.2llx") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 15, __TAG__, "%zu") && passed;
		// | (2)         | (3)         | (4)         | (5)
		// | 00 01 02 03 | 04 05 06 07 | 08 09 0a __ | 0c 0d 0e 0f

		itr_target = Iterator(&list, 4U, 0U, abc::vmem_iterator_edge::none, context.log);
		itr_expected = Iterator(&list, 4U, 0U, abc::vmem_iterator_edge::none, context.log);
		itr_actual = list.erase(itr_target);
		passed = context.are_equal<bool>(itr_actual == itr_expected, true, __TAG__, "%d") && passed;
		passed = context.are_equal<unsigned long long>(itr_actual->data, 0x09, __TAG__, "0x%2.2llx") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 14, __TAG__, "%zu") && passed;
		// | (2)         | (3)         | (4)         | (5)
		// | 00 01 02 03 | 04 05 06 07 | 09 0a __ __ | 0c 0d 0e 0f

		itr_target = Iterator(&list, 3U, 2U, abc::vmem_iterator_edge::none, context.log);
		itr_expected = Iterator(&list, 3U, 2U, abc::vmem_iterator_edge::none, context.log);
		itr_actual = list.erase(itr_target);
		passed = context.are_equal<bool>(itr_actual == itr_expected, true, __TAG__, "%d") && passed;
		passed = context.are_equal<unsigned long long>(itr_actual->data, 0x07, __TAG__, "0x%2.2llx") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 13, __TAG__, "%zu") && passed;
		// | (2)         | (3)         | (4)         | (5)
		// | 00 01 02 03 | 04 05 07 __ | 09 0a __ __ | 0c 0d 0e 0f

		itr_target = Iterator(&list, 3U, 1U, abc::vmem_iterator_edge::none, context.log);
		itr_expected = Iterator(&list, 3U, 1U, abc::vmem_iterator_edge::none, context.log);
		//// context.log->filter()->min_severity(abc::severity::abc::debug); ////
		itr_actual = list.erase(itr_target);
		passed = context.are_equal<bool>(itr_actual == itr_expected, true, __TAG__, "%d") && passed;
		passed = context.are_equal<unsigned long long>(itr_actual->data, 0x07, __TAG__, "0x%2.2llx") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 12, __TAG__, "%zu") && passed;
		// | (2)         | (3)         | (5)
		// | 00 01 02 03 | 04 07 09 0a | 0c 0d 0e 0f

		itr_target = Iterator(&list, 3U, 1U, abc::vmem_iterator_edge::none, context.log);
		itr_expected = Iterator(&list, 3U, 1U, abc::vmem_iterator_edge::none, context.log);
		itr_actual = list.erase(itr_target);
		passed = context.are_equal<bool>(itr_actual == itr_expected, true, __TAG__, "%d") && passed;
		passed = context.are_equal<unsigned long long>(itr_actual->data, 0x09, __TAG__, "0x%2.2llx") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 11, __TAG__, "%zu") && passed;
		// | (2)         | (3)         | (5)
		// | 00 01 02 03 | 04 09 0a __ | 0c 0d 0e 0f

		context.log->filter()->min_severity(abc::severity::critical); ////

#ifdef REMOVE ////
		for (std::size_t i = 0; i < 4; i++) {
			itr = list.erase(itr);
			passed = context.are_equal<unsigned long long>(itr->data, 0x05 + i, 0x103e1, "%llu") && passed;
			passed = context.are_equal<std::size_t>(list.size(), 11 - i, 0x103e2, "%zu") && passed;
		}
		// | (2)         | (4)
		// | 00 01 02 03 | 08 09 0a 0b |

		for (std::size_t i = 0; i < 3; i++) {
			itr = list.erase(list.rend());
			passed = context.are_equal<bool>(itr == list.end(), true, 0x103e3, "%d") && passed;
			itr--;
			passed = context.are_equal<unsigned long long>(itr->data, 0x0a - i, 0x103e4, "%llu") && passed;
			passed = context.are_equal<std::size_t>(list.size(), 7 - i, 0x103e5, "%zu") && passed;
		}
		// | (2)         | (4)
		// | 00 01 02 03 | 08 __ __ __ |

		itr = list.erase(list.rend());
		itr--;
		passed = context.are_equal<unsigned long long>(itr->data, 0x03, 0x103e6, "%llu") && passed;
		passed = context.are_equal<std::size_t>(list.size(), 4, 0x103e7, "%zu") && passed;
		// | (2)
		// | 00 01 02 03 |

		for (std::size_t i = 0; i < 3; i++) {
			itr = list.erase(list.rend());
			itr--;
			passed = context.are_equal<unsigned long long>(itr->data, 0x02 - i, 0x103e8, "%llu") && passed;
			passed = context.are_equal<std::size_t>(list.size(), 3 - i, 0x103e9, "%zu") && passed;
		}
		// | (2)
		// | 00 __ __ __ |

		itr = list.erase(list.rend());
		passed = context.are_equal<std::size_t>(list.size(), 0, 0x103ea, "%zu") && passed;
		// <empty>

		//// passed = context.are_equal<bool>(list.begin() == list.end(), true, 0x103eb, "%d") && passed;
		//// passed = context.are_equal<bool>(list.rend() == list.rbegin(), true, 0x103ec, "%d") && passed;
#endif
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
			passed = context.are_equal<unsigned long long>(itr->data, i, 0x103ed, "%llu") && passed;
			itr++;
		}
		passed = context.are_equal(itr == list.cend(), true, 0x103ee, "%d") && passed;

		// Iterate backwards.
		itr = list.crend();
		for (std::size_t i = 0; i < count; i++) {
			passed = context.are_equal<unsigned long long>(itr->data, count - i - 1, 0x103ef, "%llu") && passed;
			itr--;
		}
		passed = context.are_equal(itr == list.crbegin(), true, 0x103f0, "%d") && passed;

		return passed;
	}


	template <typename Pool>
	bool create_vmem_pool(test_context<abc::test::log>& context, Pool* pool, bool fit) {
		bool passed = true;

		context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, 0x103f1, "--- page2");
		abc::vmem_page<Pool, Log> page2(pool, context.log);
		context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, 0x103f2, "page2 pos=0x%llx, ptr=%p", (long long)page2.pos(), page2.ptr());
		passed = context.are_equal(2LL, (long long)page2.pos(), 0x103f3, "0x%llx") && passed;
		std::memset(page2.ptr(), 0x22, abc::vmem_page_size);

		{
			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, 0x103f4, "--- page3a");
			abc::vmem_page<Pool, Log> page3a(pool, context.log);
			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, 0x103f5, "page3a pos=0x%llx, ptr=%p", (long long)page3a.pos(), page3a.ptr());
			passed = context.are_equal(3LL, (long long)page3a.pos(), 0x103f6, "0x%llx") && passed;
			std::memset(page3a.ptr(), 0x33, abc::vmem_page_size);

			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, 0x103f7, "--- page3b");
			abc::vmem_page<Pool, Log> page3b(pool, page3a.pos(), context.log);
			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, 0x103f8, "page3b pos=0x%llx, ptr=%p", (long long)page3b.pos(), page3b.ptr());
			passed = context.are_equal(3LL, (long long)page3b.pos(), 0x103f9, "0x%llx") && passed;

			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "--- page4");
			abc::vmem_page<Pool, Log> page4(pool, context.log);
			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "page4 pos=0x%llx, ptr=%p", (long long)page4.pos(), page4.ptr());
			passed = context.are_equal(4LL, (long long)page4.pos(), __TAG__, "0x%llx") && passed;
			std::memset(page4.ptr(), 0x44, abc::vmem_page_size);

			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, 0x103fa, "--- page5");
			abc::vmem_page<Pool, Log> page5(pool, context.log);
			passed = context.are_equal<bool>(page5.ptr() != nullptr, fit, 0x103fb, "%d") && passed;
			if (page5.ptr() != nullptr) {
				context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, 0x103fc, "page5 pos=0x%llx, ptr=%p", (long long)page5.pos(), page5.ptr());
				passed = context.are_equal(5LL, (long long)page5.pos(), 0x103fd, "0x%llx") && passed;
				std::memset(page5.ptr(), 0x55, abc::vmem_page_size);
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
					context.log->put_any(abc::category::any, abc::severity::debug, 0x103fe, "Verifying 0x%x", b);
				}

				context.log->put_any(abc::category::any, abc::severity::optional, 0x103ff, "i = %zu", i);
				passed = context.are_equal<std::uint8_t>(byte_buffer[i], b, 0x10400, "0x%x") && passed;
			}
		}

		return passed;
	}

}}}

