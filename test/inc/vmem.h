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

#include "../../src/log.h"
#include "../../src/vmem.h"

#include "test.h"


namespace abc { namespace test { namespace vmem {

	bool test_vmem_pool_fit(test_context<abc::test::log>& context);
	bool test_vmem_pool_exceed(test_context<abc::test::log>& context);
	bool test_vmem_pool_reopen(test_context<abc::test::log>& context);
	bool test_vmem_pool_freepages(test_context<abc::test::log>& context);

	bool test_vmem_linked_mixedone(test_context<abc::test::log>& context);
	bool test_vmem_linked_mixedmany(test_context<abc::test::log>& context);
	bool test_vmem_linked_splice(test_context<abc::test::log>& context);
	bool test_vmem_linked_clear(test_context<abc::test::log>& context);

	bool test_vmem_list_insert(test_context<abc::test::log>& context);
	bool test_vmem_list_insertmany(test_context<abc::test::log>& context);
	bool test_vmem_list_erase(test_context<abc::test::log>& context);

	bool test_vmem_temp_destructor(test_context<abc::test::log>& context);

	bool test_vmem_map_insert(test_context<abc::test::log>& context);
	bool test_vmem_map_insertmany(test_context<abc::test::log>& context);
	bool test_vmem_map_erase(test_context<abc::test::log>& context);
	bool test_vmem_map_clear(test_context<abc::test::log>& context);

	bool test_vmem_string_iterator(test_context<abc::test::log>& context);
	bool test_vmem_string_stream(test_context<abc::test::log>& context);

	bool test_vmem_pool_move(test_context<abc::test::log>& context);
	bool test_vmem_page_move(test_context<abc::test::log>& context);
}}}

