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


#include "inc/ascii.h"


namespace abc { namespace test { namespace ascii {

	bool test_ascii_equal(test_context<abc::test::log>& context) {
		bool passed = true;

		passed = context.are_equal(abc::ascii::are_equal(nullptr, nullptr), true, 0x102f6, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal(nullptr, ""), false, 0x102f7, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal(nullptr, "abc"), false, 0x102f8, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("", nullptr), false, 0x102f9, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("abc", nullptr), false, 0x102fa, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("", ""), true, 0x102fb, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("", "abc"), false, 0x102fc, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("abc", ""), false, 0x102fd, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("abc", "abc"), true, 0x102fe, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("aBc", "aBc"), true, 0x102ff, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("abc", "abcd"), false, 0x10300, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("abcd", "abc"), false, 0x10301, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("aBc", "abc"), false, 0x10302, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal("abc", "aBc"), false, 0x10303, "%d") && passed;

		return passed;
	}

	bool test_ascii_equal_n(test_context<abc::test::log>& context) {
		bool passed = true;

		passed = context.are_equal(abc::ascii::are_equal_n(nullptr, nullptr, 3), true, 0x10304, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n(nullptr, "", 3), false, 0x10305, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n(nullptr, "abc", 0), false, 0x10306, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("", nullptr, 3), false, 0x10307, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", nullptr, 0), false, 0x10308, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("", "", 3), true, 0x10309, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("", "abc", 5), false, 0x1030a, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("", "abc", 0), true, 0x1030b, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "", 5), false, 0x1030c, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "", 0), true, 0x1030d, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "abc", 5), true, 0x1030e, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("aBc", "aBc", 2), true, 0x1030f, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "abcd", 5), false, 0x10310, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "abcd", 3), true, 0x10311, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abcd", "abc", 5), false, 0x10312, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abcd", "abc", 3), true, 0x10313, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("aBc", "abc", 5), false, 0x10314, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("aBc", "abc", 2), false, 0x10315, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("aBc", "abc", 1), true, 0x10316, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("aBc", "abc", 0), true, 0x10317, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "aBc", 5), false, 0x10318, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "aBc", 2), false, 0x10319, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "aBc", 1), true, 0x1031a, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_n("abc", "aBc", 0), true, 0x1031b, "%d") && passed;

		return passed;
	}

	bool test_ascii_equal_i(test_context<abc::test::log>& context) {
		bool passed = true;

		passed = context.are_equal(abc::ascii::are_equal_i(nullptr, nullptr), true, 0x1031c, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i(nullptr, ""), false, 0x1031d, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i(nullptr, "abc"), false, 0x1031e, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("", nullptr), false, 0x1031f, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("abc", nullptr), false, 0x10320, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("", ""), true, 0x10321, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("abc", "abc"), true, 0x10322, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("aBc", "aBc"), true, 0x10323, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("abc", "abcd"), false, 0x10324, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("abcd", "abc"), false, 0x10325, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("aBc", "abc"), true, 0x10326, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i("abc", "aBc"), true, 0x10327, "%d") && passed;

		return passed;
	}

	bool test_ascii_equal_i_n(test_context<abc::test::log>& context) {
		bool passed = true;

		passed = context.are_equal(abc::ascii::are_equal_i_n(nullptr, nullptr, 3), true, 0x10328, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n(nullptr, "", 3), false, 0x10329, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n(nullptr, "abc", 0), false, 0x1032a, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("", nullptr, 3), false, 0x1032b, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", nullptr, 0), false, 0x1032c, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("", "", 3), true, 0x1032d, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("", "abc", 5), false, 0x1032e, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("", "abc", 0), true, 0x1032f, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "", 5), false, 0x10330, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "", 0), true, 0x10331, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "abc", 5), true, 0x10332, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("aBc", "aBc", 2), true, 0x10333, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "abcd", 5), false, 0x10334, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "abcd", 3), true, 0x10335, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abcd", "abc", 5), false, 0x10336, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abcd", "abc", 3), true, 0x10337, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("aBc", "abc", 5), true, 0x10338, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("aBc", "abc", 2), true, 0x10339, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("aBc", "abc", 1), true, 0x1033a, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("aBc", "abc", 0), true, 0x1033b, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "aBc", 5), true, 0x1033c, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "aBc", 2), true, 0x1033d, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "aBc", 1), true, 0x1033e, "%d") && passed;
		passed = context.are_equal(abc::ascii::are_equal_i_n("abc", "aBc", 0), true, 0x1033f, "%d") && passed;

		return passed;
	}

}}}

