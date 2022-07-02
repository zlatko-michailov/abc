/*
MIT License

Copyright (c) 2018-2022 Zlatko Michailov 

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


#include "inc/multifile.h"
#include "inc/heap.h"


namespace abc { namespace test { namespace multifile {

	template <typename Streambuf>
	bool test_move(Streambuf& sb1, test_context<abc::test::log>& context) {
		abc::test::heap::ignore_heap_allocations(abc::test::heap::array_unaligned_throw_count, 1, context, __TAG__); // std::filebuf::open()
		std::ostream out1(&sb1);
		out1.write("one ", 4);
		out1.flush();

		char path[abc::size::k2 + 1];
		std::strcpy(path, sb1.path());

		Streambuf sb2(std::move(sb1));
		std::ostream out2(&sb2);
		out2.write("two ", 4);
		out2.flush();

		char actual[abc::size::_256 + 1] = { };

		std::filebuf sbin;
		sbin.open(path, std::ios_base::in);
		abc::test::heap::ignore_heap_allocations(abc::test::heap::array_unaligned_throw_count, 1, context, __TAG__); // std::filebuf::open()
		std::istream in(&sbin);
		in.read(actual, 8);

		bool passed = true;

		passed = context.are_equal(actual, "one two ", __TAG__) && passed;

		return passed;
	}


	bool test_multifile_move(test_context<abc::test::log>& context) {
		abc::multifile_streambuf<abc::size::k1, std::chrono::system_clock, abc::test::log> sb1("out/test", std::ios_base::out, context.log);
		return test_move(sb1, context);
	}


	bool test_duration_multifile_move(test_context<abc::test::log>& context) {
		abc::duration_multifile_streambuf<abc::size::k1, std::chrono::system_clock, abc::test::log> sb1(std::chrono::minutes(1), "out/test", std::ios_base::out, context.log);
		return test_move(sb1, context);
	}


	bool test_size_multifile_move(test_context<abc::test::log>& context) {
		abc::size_multifile_streambuf<abc::size::k1, std::chrono::system_clock, abc::test::log> sb1(abc::size::k1, "out/test", std::ios_base::out, context.log);
		return test_move(sb1, context);
	}

}}}

