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


#include <memory>
#include <atomic>
#include <new>

#include "heap.h"


namespace abc { namespace test { namespace heap {

	bool verify_heap_allocation(test_context<abc::test::log>& context, tag_t tag);


	using counter_t = std::int32_t;

	static counter_t	instance_unaligned_throw_count		= 0;
	static counter_t	instance_aligned_throw_count		= 0;
	static counter_t	instance_unaligned_nothrow_count	= 0;
	static counter_t	instance_aligned_nothrow_count		= 0;
	static counter_t	array_unaligned_throw_count			= 0;
	static counter_t	array_aligned_throw_count			= 0;
	static counter_t	array_unaligned_nothrow_count		= 0;
	static counter_t	array_aligned_nothrow_count			= 0;


	bool start_heap_allocation(test_context<abc::test::log>& context) {
		instance_unaligned_throw_count		= 0;
		instance_aligned_throw_count		= 0;
		instance_unaligned_nothrow_count	= 0;
		instance_aligned_nothrow_count		= 0;
		array_unaligned_throw_count			= 0;
		array_aligned_throw_count			= 0;
		array_unaligned_nothrow_count		= 0;
		array_aligned_nothrow_count			= 0;

		return true;
	}


	bool test_heap_allocation(test_context<abc::test::log>& context) {
		return verify_heap_allocation(context, 0x10069);
	}


	bool ignore_heap_allocation(test_context<abc::test::log>& context, tag_t tag) {
		instance_unaligned_throw_count--;

		return verify_heap_allocation(context, tag);
	}


	bool verify_heap_allocation(test_context<abc::test::log>& context, tag_t tag) {
		bool passed = true;

		passed = context.are_equal(instance_unaligned_throw_count,		0, tag, "%ld") && passed;
		passed = context.are_equal(instance_aligned_throw_count,		0, tag, "%ld") && passed;
		passed = context.are_equal(instance_unaligned_nothrow_count,	0, tag, "%ld") && passed;
		passed = context.are_equal(instance_aligned_nothrow_count,		0, tag, "%ld") && passed;
		passed = context.are_equal(array_unaligned_throw_count,			0, tag, "%ld") && passed;
		passed = context.are_equal(array_aligned_throw_count,			0, tag, "%ld") && passed;
		passed = context.are_equal(array_unaligned_nothrow_count,		0, tag, "%ld") && passed;
		passed = context.are_equal(array_aligned_nothrow_count,			0, tag, "%ld") && passed;

		return passed;
	}


	void* alloc_nothrow(std::size_t size, counter_t& counter) noexcept {
		counter++;

		return malloc(size);
	}


	void* alloc_throw(std::size_t size, counter_t& counter) {
		void* ptr = alloc_nothrow(size, counter);

		if (ptr == nullptr) {
			throw std::bad_alloc();
		}

		return ptr;
	}

}}}


void* operator new(std::size_t size) {
	return abc::test::heap::alloc_throw(size, abc::test::heap::instance_unaligned_throw_count);
}

#if (__cplusplus >= 201700L)
void* operator new(std::size_t size, std::align_val_t) {
	return abc::test::heap::alloc_throw(size, abc::test::heap::instance_aligned_throw_count);
}
#endif


void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
	return abc::test::heap::alloc_nothrow(size, abc::test::heap::instance_unaligned_nothrow_count);
}

#if (__cplusplus >= 201700L)
void* operator new(std::size_t size, std::align_val_t, const std::nothrow_t&) noexcept {
	return abc::test::heap::alloc_nothrow(size, abc::test::heap::instance_aligned_nothrow_count);
}
#endif


void* operator new[](std::size_t size) {
	return abc::test::heap::alloc_throw(size, abc::test::heap::array_unaligned_throw_count);
}

#if (__cplusplus >= 201700L)
void* operator new[](std::size_t size, std::align_val_t) {
	return abc::test::heap::alloc_throw(size, abc::test::heap::array_aligned_throw_count);
}
#endif


void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
	return abc::test::heap::alloc_nothrow(size, abc::test::heap::array_unaligned_nothrow_count);
}

#if (__cplusplus >= 201700L)
void* operator new[](std::size_t size, std::align_val_t, const std::nothrow_t&) noexcept {
	return abc::test::heap::alloc_nothrow(size, abc::test::heap::array_aligned_nothrow_count);
}
#endif
