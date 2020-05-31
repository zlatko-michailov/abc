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
	static bool verify_binary(test_context<abc::test_log_ptr>& context, const void* actual, const void* expected, std::size_t size, const abc::_http_istream<abc::test_log_ptr>& istream);
	static bool verify_stream(test_context<abc::test_log_ptr>& context, const abc::_http_istream<abc::test_log_ptr>& istream, std::size_t expected_gcount);


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


	bool test_http_request_istream_bodytext(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"POST http://a.com/b?c=d HTTP/1.1\r\n"
			"\r\n"
			"{\r\n"
			"  \"foo\": 42,\r\n"
			"  \"bar\": \"qwerty\"\r\n"
			"}";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::http_request_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		bool passed = true;

		const std::size_t binary_line_size		= 10;
		const std::size_t binary_line_remainder	= 7;

		istream.get_method(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "POST", istream);

		istream.get_resource(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "http://a.com/b?c=d", istream);

		istream.get_protocol(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "HTTP/1.1", istream);

		istream.get_headername(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "", istream);

		istream.get_body(buffer, binary_line_size);
		passed &= verify_binary(context, buffer, "{\r\n  \"foo\"", binary_line_size, istream);

		istream.get_body(buffer, binary_line_size);
		passed &= verify_binary(context, buffer, ": 42,\r\n  \"", binary_line_size, istream);

		istream.get_body(buffer, binary_line_size);
		passed &= verify_binary(context, buffer, "bar\": \"qwe", binary_line_size, istream);

		istream.get_body(buffer, binary_line_remainder);
		passed &= verify_binary(context, buffer, "rty\"\r\n}", binary_line_remainder, istream);

		return passed;
	}


	bool test_http_request_istream_bodybinary(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"POST http://a.com/b?c=d HTTP/1.1\r\n"
			"\r\n"
			"\x01\x05\x10 text \x02\x03\x12 mixed \x04\x18\x19 with \x7f\x80 bytes \xaa\xff";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::http_request_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		bool passed = true;

		const std::size_t binary_line_size		= 16;
		const std::size_t binary_line_remainder	= 7;

		istream.get_method(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "POST", istream);

		istream.get_resource(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "http://a.com/b?c=d", istream);

		istream.get_protocol(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "HTTP/1.1", istream);

		istream.get_headername(buffer, sizeof(buffer));
		passed &= verify_string(context, buffer, "", istream);

		istream.get_body(buffer, binary_line_size);
		passed &= verify_binary(context, buffer, "\x01\x05\x10 text \x02\x03\x12 mix", binary_line_size, istream);

		istream.get_body(buffer, binary_line_size);
		passed &= verify_binary(context, buffer, "ed \x04\x18\x19 with \x7f\x80 b", binary_line_size, istream);

		istream.get_body(buffer, binary_line_remainder);
		passed &= verify_binary(context, buffer, "ytes \xaa\xff", binary_line_remainder, istream);

		return passed;
	}


	static bool verify_string(test_context<abc::test_log_ptr>& context, const char* actual, const char* expected, const abc::_http_istream<abc::test_log_ptr>& istream) {
		bool passed = true;

		passed &= context.are_equal(actual, expected, __TAG__);
		passed &= verify_stream(context, istream, std::strlen(expected));

		return passed;
	}


	static bool verify_binary(test_context<abc::test_log_ptr>& context, const void* actual, const void* expected, std::size_t size, const abc::_http_istream<abc::test_log_ptr>& istream) {
		bool passed = true;

		passed &= context.are_equal(actual, expected, size, __TAG__);
		passed &= verify_stream(context, istream, size);

		return passed;
	}


	static bool verify_stream(test_context<abc::test_log_ptr>& context, const abc::_http_istream<abc::test_log_ptr>& istream, std::size_t expected_gcount) {
		bool passed = true;

		passed &= context.are_equal(istream.gcount(), expected_gcount, __TAG__, "%u");
		passed &= context.are_equal(istream.good(), true, __TAG__, "%u");
		passed &= context.are_equal(istream.eof(), false, __TAG__, "%u");
		passed &= context.are_equal(istream.fail(), false, __TAG__, "%u");
		passed &= context.are_equal(istream.bad(), false, __TAG__, "%u");

		return passed;
	}

}}}

