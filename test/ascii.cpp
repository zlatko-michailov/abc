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


#include "ascii.h"


namespace abc { namespace test { namespace ascii {

	bool test_ascii_equal(test_context<abc::test::log>& context) {
		bool passed = true;

		passed = context.are_equal(abc::ascii::are_equal(nullptr, nullptr), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal(nullptr, ""), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal(nullptr, "abc"), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("", nullptr), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("abc", nullptr), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("", ""), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("", "abc"), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("abc", ""), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("abc", "abc"), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("aBc", "aBc"), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("abc", "abcd"), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("abcd", "abc"), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("aBc", "abc"), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("abc", "aBc"), false, __TAG__, "%d") && passed;

		return passed;
	}

	bool test_ascii_equal_n(test_context<abc::test::log>& context) {
		bool passed = true;

		passed = context.are_equal(abc::ascii::are_equal_n(nullptr, nullptr, 3), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n(nullptr, "", 3), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n(nullptr, "abc", 0), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("", nullptr, 3), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", nullptr, 0), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("", "", 3), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("", "abc", 5), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("", "abc", 0), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "", 5), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "", 0), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "abc", 5), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("aBc", "aBc", 2), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "abcd", 5), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "abcd", 3), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abcd", "abc", 5), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abcd", "abc", 3), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("aBc", "abc", 5), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("aBc", "abc", 2), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("aBc", "abc", 1), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("aBc", "abc", 0), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "aBc", 5), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "aBc", 2), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "aBc", 1), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "aBc", 0), true, __TAG__, "%d") && passed;

		return passed;
	}

	bool test_ascii_equal_i(test_context<abc::test::log>& context) {
		bool passed = true;

		passed = context.are_equal(abc::ascii::are_equal_i(nullptr, nullptr), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i(nullptr, ""), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i(nullptr, "abc"), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("", nullptr), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("abc", nullptr), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("", ""), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("abc", "abc"), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("aBc", "aBc"), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("abc", "abcd"), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("abcd", "abc"), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("aBc", "abc"), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("abc", "aBc"), true, __TAG__, "%d") && passed;

		return passed;
	}

	bool test_ascii_equal_i_n(test_context<abc::test::log>& context) {
		bool passed = true;

		passed = context.are_equal(abc::ascii::are_equal_i_n(nullptr, nullptr, 3), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n(nullptr, "", 3), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n(nullptr, "abc", 0), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("", nullptr, 3), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", nullptr, 0), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("", "", 3), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("", "abc", 5), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("", "abc", 0), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "", 5), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "", 0), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "abc", 5), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("aBc", "aBc", 2), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "abcd", 5), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "abcd", 3), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abcd", "abc", 5), false, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abcd", "abc", 3), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("aBc", "abc", 5), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("aBc", "abc", 2), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("aBc", "abc", 1), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("aBc", "abc", 0), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "aBc", 5), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "aBc", 2), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "aBc", 1), true, __TAG__, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "aBc", 0), true, __TAG__, "%d") && passed;

		return passed;
	}

}}}

