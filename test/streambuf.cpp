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


#include "streambuf.h"


namespace abc { namespace test { namespace streambuf {

	static bool test_buffer_streambuf(test_context<abc::test_log_ptr>& context, const char* text) ;


	bool test_buffer_streambuf_1_char(test_context<abc::test_log_ptr>& context) {
		return test_buffer_streambuf(context, "x");
	}


	bool test_buffer_streambuf_N_chars(test_context<abc::test_log_ptr>& context) {
		return test_buffer_streambuf(context, "This is a slightly longer text");
	}


	bool test_buffer_streambuf(test_context<abc::test_log_ptr>& context, const char* text) {
		char expected[200];
		std::strncpy(expected, text, sizeof(expected));

		char actual[200];
		std::memset(actual, 0, sizeof(actual));

		abc::buffer_streambuf sb(expected, 0, std::strlen(expected), actual, 0, sizeof(actual));

		std::istream in(&sb);
		std::ostream out(&sb);

		while (!in.eof()) {
			char ch = in.get();
			if (!in.eof()) {
				out.put(ch);
			}
		}

		return context.are_equal(actual, expected, 0x1003a);
	}

}}}

