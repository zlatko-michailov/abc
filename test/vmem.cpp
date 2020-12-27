/*
MIT License

Copyright (c) 2018-2020 Zlatko Michailov 

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
	using Item = std::array<std::uint8_t, 900>;


	template <typename Pool>
	bool test_vmem_pool(test_context<abc::test::log>& context, Pool* pool, bool fit) {
		bool passed = true;

		context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "--- page2");
		abc::vmem_page<Pool, Log> page2(pool, context.log);
		context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "--- page2 pos=%llu, ptr=%p", (unsigned long long)page2.pos(), page2.ptr());
		passed = context.are_equal(2ULL, (unsigned long long)page2.pos(), __TAG__, "%llu") && passed;
		std::memset(page2.ptr(), 0x22, abc::vmem_page_size);

		{
			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "--- page3a");
			abc::vmem_page<Pool, Log> page3a(pool, context.log);
			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "--- page3a pos=%llu, ptr=%p", (unsigned long long)page3a.pos(), page3a.ptr());
			passed = context.are_equal(3ULL, (unsigned long long)page3a.pos(), __TAG__, "%llu") && passed;
			std::memset(page3a.ptr(), 0x33, abc::vmem_page_size);

			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "--- page3b");
			abc::vmem_page<Pool, Log> page3b(pool, page3a.pos(), context.log);
			context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "--- page3b pos=%llu, ptr=%p", page3b.pos(), page3b.ptr());
			passed = context.are_equal(3ULL, (unsigned long long)page3b.pos(), __TAG__, "%llu") && passed;

			try {
				context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "--- page4");
				abc::vmem_page<Pool, Log> page4(pool, context.log);
				context.log->put_any(abc::category::abc::vmem, abc::severity::abc::important, __TAG__, "--- page4 pos=%llu, ptr=%p", page4.pos(), page4.ptr());
				passed = context.are_equal(4ULL, (unsigned long long)page4.pos(), __TAG__, "%llu") && passed;
				std::memset(page4.ptr(), 0x44, abc::vmem_page_size);

				passed = context.are_equal(true, fit, __TAG__, "%d") && passed;
			}
			catch (abc::exception<std::runtime_error, Log>& ex) {
				passed = context.are_equal(false, fit, __TAG__, "%d") && passed;
				passed = abc::test::heap::ignore_heap_allocation(context, __TAG__) && passed; // std::exception allocates state
			}
		}

		return passed;
	}


	bool test_vmem_pool_fit(test_context<abc::test::log>& context) {
		bool passed = true;

		abc::vmem_pool<3, abc::test::log> pool("out/test/pool_fit.vmem", context.log);

		passed = test_vmem_pool(context, &pool, true) && passed;

		return passed;
	}


	bool test_vmem_pool_exceed(test_context<abc::test::log>& context) {
		bool passed = true;

		abc::vmem_pool<2, abc::test::log> pool("out/test/pool_exceed.vmem", context.log);

		passed = test_vmem_pool(context, &pool, false) && passed;

		return passed;
	}

}}}

