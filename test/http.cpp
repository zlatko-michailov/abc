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


#include "http.h"


namespace abc { namespace test { namespace http {

	static bool verify_string(test_context<abc::test_log_ptr>& context, const char* actual, const char* expected, const abc::_http_istream<abc::test_log_ptr>& istream);


	bool test_http_request_istream_extraspaces(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"GET   http://a.com/b?c=d    HTTP/12.345  \r\n"
			"Name:Value\r\n"
			"Multi_Word-Name:  Value  with   spaces   inside \t \r\n"
			"Multi-Line   :   First line\r\n"
			" Second  line  \r\n"
			"\t    \t  \t    Third  line   \r\n"
			"Trailing-Spaces  :  3 spaces   \r\n"
			"\r\n";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::http_request_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		bool passed = true;

		istream.get_method(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "GET", istream);

		istream.get_resource(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "http://a.com/b?c=d", istream);

		istream.get_protocol(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "HTTP/12.345", istream);

		istream.get_headername(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "Name", istream);

		istream.get_headervalue(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "Value", istream);

		istream.get_headername(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "Multi_Word-Name", istream);

		istream.get_headervalue(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "Value with spaces inside", istream);

		istream.get_headername(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "Multi-Line", istream);

		istream.get_headervalue(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "First line Second line Third line", istream);

		istream.get_headername(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "Trailing-Spaces", istream);

		istream.get_headervalue(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "3 spaces", istream);

		istream.get_headername(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "", istream);

		return passed;
	}


	static bool verify_string(test_context<abc::test_log_ptr>& context, const char* actual, const char* expected, const abc::_http_istream<abc::test_log_ptr>& istream) {
		bool passed = true;

		passed &= context.are_equal(actual, expected, __TAG__);
		passed &= context.are_equal(istream.gcount(), std::strlen(expected), __TAG__, "%u");
		passed &= context.are_equal(istream.good(), true, __TAG__, "%u");
		passed &= context.are_equal(istream.eof(), false, __TAG__, "%u");
		passed &= context.are_equal(istream.fail(), false, __TAG__, "%u");
		passed &= context.are_equal(istream.bad(), false, __TAG__, "%u");

		return passed;
	}

}}}

