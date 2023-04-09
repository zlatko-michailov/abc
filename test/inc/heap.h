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


#pragma once

#include "../../src/exception.h"
#include "../../src/tag.h"

#include "test.h"


namespace abc { namespace test { namespace heap {

	using counter_t = std::int32_t;


	extern counter_t	instance_unaligned_throw_count;
	extern counter_t	instance_aligned_throw_count;
	extern counter_t	instance_unaligned_nothrow_count;
	extern counter_t	instance_aligned_nothrow_count;
	extern counter_t	array_unaligned_throw_count;
	extern counter_t	array_aligned_throw_count;
	extern counter_t	array_unaligned_nothrow_count;
	extern counter_t	array_aligned_nothrow_count;


	bool start_heap_allocation(test_context<abc::test::log>& context);
	bool test_heap_allocation(test_context<abc::test::log>& context);
	bool ignore_heap_allocations(counter_t& counter, counter_t delta, test_context<abc::test::log>& context, tag_t tag);

}}}

