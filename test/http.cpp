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

	template <typename StdStream>
	static bool verify_string(test_context<abc::test_log_ptr>& context, const char* actual, const char* expected, const abc::_http_stream<StdStream, abc::test_log_ptr>& istream);

	template <typename StdStream>
	static bool verify_binary(test_context<abc::test_log_ptr>& context, const void* actual, const void* expected, std::size_t size, const abc::_http_stream<StdStream, abc::test_log_ptr>& istream);

	template <typename StdStream>
	static bool verify_stream(test_context<abc::test_log_ptr>& context, const abc::_http_stream<StdStream, abc::test_log_ptr>& istream, std::size_t expected_gcount);


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
		passed = verify_string(context, buffer, "GET", istream) && passed;

		istream.get_resource(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "http://a.com/b?c=d", istream) && passed;

		istream.get_protocol(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "HTTP/12.345", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Name", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Value", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Multi_Word-Name", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Value with spaces inside", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Multi-Line", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "First line Second line Third line", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Trailing-Spaces", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "3 spaces", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "", istream) && passed;

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
		passed = verify_string(context, buffer, "POST", istream) && passed;

		istream.get_resource(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "http://a.com/b?c=d", istream) && passed;

		istream.get_protocol(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "HTTP/1.1", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "", istream) && passed;

		istream.get_body(buffer, binary_line_size);
		passed = verify_binary(context, buffer, "{\r\n  \"foo\"", binary_line_size, istream) && passed;

		istream.get_body(buffer, binary_line_size);
		passed = verify_binary(context, buffer, ": 42,\r\n  \"", binary_line_size, istream) && passed;

		istream.get_body(buffer, binary_line_size);
		passed = verify_binary(context, buffer, "bar\": \"qwe", binary_line_size, istream) && passed;

		istream.get_body(buffer, binary_line_remainder);
		passed = verify_binary(context, buffer, "rty\"\r\n}", binary_line_remainder, istream) && passed;

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
		passed = verify_string(context, buffer, "POST", istream) && passed;

		istream.get_resource(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "http://a.com/b?c=d", istream) && passed;

		istream.get_protocol(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "HTTP/1.1", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "", istream) && passed;

		istream.get_body(buffer, binary_line_size);
		passed = verify_binary(context, buffer, "\x01\x05\x10 text \x02\x03\x12 mix", binary_line_size, istream) && passed;

		istream.get_body(buffer, binary_line_size);
		passed = verify_binary(context, buffer, "ed \x04\x18\x19 with \x7f\x80 b", binary_line_size, istream) && passed;

		istream.get_body(buffer, binary_line_remainder);
		passed = verify_binary(context, buffer, "ytes \xaa\xff", binary_line_remainder, istream) && passed;

		return passed;
	}


	bool test_http_request_istream_realworld_01(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"GET https://en.cppreference.com/w/cpp/io/basic_streambuf HTTP/1.1\r\n"
			"Host: en.cppreference.com\r\n"
			"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0\r\n"
			"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
			"Accept-Language: en-US,en;q=0.5\r\n"
			"Accept-Encoding: gzip, deflate, br\r\n"
			"Connection: keep-alive\r\n"
			"Cookie: __utma=165123437.761011328.1578550293.1590821219.1590875063.126; __utmz=165123437.1581492299.50.2.utmcsr=bing|utmccn=(organic)|utmcmd=organic|utmctr=(not%20provided); _bsap_daycap=407621%2C407621%2C408072%2C408072%2C408072%2C408072; _bsap_lifecap=407621%2C407621%2C408072%2C408072%2C408072%2C408072; __utmc=165123437\r\n"
			"Upgrade-Insecure-Requests: 1\r\n"
			"Cache-Control: max-age=0\r\n"
			"\r\n";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::http_request_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[1024];
		bool passed = true;

		istream.get_method(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "GET", istream) && passed;

		istream.get_resource(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "https://en.cppreference.com/w/cpp/io/basic_streambuf", istream) && passed;

		istream.get_protocol(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "HTTP/1.1", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Host", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "en.cppreference.com", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "User-Agent", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Accept", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Accept-Language", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "en-US,en;q=0.5", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Accept-Encoding", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "gzip, deflate, br", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Connection", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "keep-alive", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Cookie", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "__utma=165123437.761011328.1578550293.1590821219.1590875063.126; __utmz=165123437.1581492299.50.2.utmcsr=bing|utmccn=(organic)|utmcmd=organic|utmctr=(not%20provided); _bsap_daycap=407621%2C407621%2C408072%2C408072%2C408072%2C408072; _bsap_lifecap=407621%2C407621%2C408072%2C408072%2C408072%2C408072; __utmc=165123437", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Upgrade-Insecure-Requests", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "1", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Cache-Control", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "max-age=0", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "", istream) && passed;

		return passed;
	}


	// --------------------------------------------------------------


	bool test_http_request_ostream_bodytext(test_context<abc::test_log_ptr>& context) {
		const char expected[] =
			"POST http://a.com/b?c=d HTTP/1.1\r\n"
			"Simple-Header-Name: Simple-Header-Value\r\n"
			"List: items separated by a single space\r\n"
			"Multi-Line: first line second line third line\r\n"
			"\r\n"
			"{\r\n"
			"  \"foo\": 42,\r\n"
			"  \"bar\": \"qwerty\"\r\n"
			"}";

		char actual [1024 + 1];

		abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

		abc::http_request_ostream<abc::test_log_ptr> ostream(&sb, context.log_ptr);

		bool passed = true;
		const char* input;

		input = "POST";
		ostream.put_method(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "http://a.com/b?c=d";
		ostream.put_resource(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "HTTP/1.1";
		ostream.put_protocol(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "Simple-Header-Name";
		ostream.put_header_name(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "Simple-Header-Value";
		ostream.put_header_value(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "List";
		ostream.put_header_name(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = " \t items  \t\t  separated   by \t  a\t\tsingle space\t";
		ostream.put_header_value(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "Multi-Line";
		ostream.put_header_name(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "first line \r\n  \t  second  line\t \r\n\tthird line\t";
		ostream.put_header_value(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		ostream.end_headers();

		input = "{\r\n";
		ostream.put_body(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "  \"foo\": 42,\r\n";
		ostream.put_body(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "  \"bar\": \"qwerty\"\r\n";
		ostream.put_body(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "}";
		ostream.put_body(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		passed = context.are_equal(actual, expected, std::strlen(expected), __TAG__) && passed;

		return passed;
	}


	bool test_http_request_ostream_bodybinary(test_context<abc::test_log_ptr>& context) {
		const char expected[] =
			"GET http://a.com/b?c=d HTTP/1.1\r\n"
			"Multi-Line: second line third line\r\n"
			"\r\n"
			"\x01\x04\x10\x1f"
			"\x20\x70\x7f"
			"\x80\xa5\xb8\xcc\xdd\xff";

		char actual [1024 + 1];

		abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

		abc::http_request_ostream<abc::test_log_ptr> ostream(&sb, context.log_ptr);

		bool passed = true;
		const char* input;

		input = "GET";
		ostream.put_method(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "http://a.com/b?c=d";
		ostream.put_resource(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "HTTP/1.1";
		ostream.put_protocol(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "Multi-Line";
		ostream.put_header_name(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "\r\n\tsecond line\t\r\n third  line      ";
		ostream.put_header_value(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		ostream.end_headers();

		input = "\x01\x04\x10\x1f";
		ostream.put_body(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "\x20\x70\x7f";
		ostream.put_body(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "\x80\xa5\xb8\xcc\xdd\xff";
		ostream.put_body(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		passed = context.are_equal(actual, expected, std::strlen(expected), __TAG__) && passed;

		return passed;
	}


	// --------------------------------------------------------------


	bool test_http_response_istream_extraspaces(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"HTTP/12.345  789  \t  Something went wrong  \r\n"
			"Header-Name:Header-Value\r\n"
			"\r\n";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::http_response_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		bool passed = true;

		istream.get_protocol(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "HTTP/12.345", istream) && passed;

		istream.get_status_code(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "789", istream) && passed;

		istream.get_reason_phrase(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Something went wrong  ", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Header-Name", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Header-Value", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "", istream) && passed;

		return passed;
	}


	bool test_http_response_istream_realworld_01(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"HTTP/1.1 302\r\n"
			"Set-Cookie: ADRUM_BTa=R:59|g:a2345a60-c557-41f0-8cd9-0ee876b70b76; Max-Age=30; Expires=Sun, 31-May-2020 01:27:14 GMT; Path=/\r\n"
			"Cache-Control: no-cache, no-store, max-age=0, must-revalidate\r\n"
			"Location: https://xerxes-sub.xerxessecure.com/xerxes-jwt/init?state=eyJlbmMiOiJBMTI4R0NNIiwiYWxnIjoiUlNBLU9BRVAtMjU2In0.\r\n"
			"Content-Length: 0\r\n"
			"Date: Sun, 31 May 2020 01:26:44 GMT\r\n"
			"\r\n";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::http_response_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[201];
		bool passed = true;

		istream.get_protocol(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "HTTP/1.1", istream) && passed;

		istream.get_status_code(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "302", istream) && passed;

		istream.get_reason_phrase(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Set-Cookie", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "ADRUM_BTa=R:59|g:a2345a60-c557-41f0-8cd9-0ee876b70b76; Max-Age=30; Expires=Sun, 31-May-2020 01:27:14 GMT; Path=/", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Cache-Control", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "no-cache, no-store, max-age=0, must-revalidate", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Location", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "https://xerxes-sub.xerxessecure.com/xerxes-jwt/init?state=eyJlbmMiOiJBMTI4R0NNIiwiYWxnIjoiUlNBLU9BRVAtMjU2In0.", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Content-Length", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "0", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Date", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Sun, 31 May 2020 01:26:44 GMT", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "", istream) && passed;

		return passed;
	}


	bool test_http_response_istream_realworld_02(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json; charset=utf-8\r\n"
			"Access-Control-Expose-Headers: X-Content-Type-Options,Cache-Control,Pragma,ContextId,Content-Length,Connection,MS-CV,Date\r\n"
			"Content-Length: 205\r\n"
			"\r\n"
			"{\"next\":\"https://centralus.notifications.teams.microsoft.com/users/8:orgid:66c7bbfd-e15c-4257-ad6b-867c195de604/endpoints/0bf687c1-c864-45df-891a-90f548dee242/events/poll?cursor=1590886559&epfs=srt&sca=2\"}\r\n"
			"\r\n";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::http_response_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[201];
		bool passed = true;

		istream.get_protocol(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "HTTP/1.1", istream) && passed;

		istream.get_status_code(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "200", istream) && passed;

		istream.get_reason_phrase(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "OK", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Content-Type", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "application/json; charset=utf-8", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Access-Control-Expose-Headers", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "X-Content-Type-Options,Cache-Control,Pragma,ContextId,Content-Length,Connection,MS-CV,Date", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "Content-Length", istream) && passed;

		istream.get_header_value(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "205", istream) && passed;

		istream.get_header_name(buffer, sizeof(buffer));
		passed = verify_string(context, buffer, "", istream) && passed;

		istream.get_body(buffer, 200);
		passed = verify_binary(context, buffer, "{\"next\":\"https://centralus.notifications.teams.microsoft.com/users/8:orgid:66c7bbfd-e15c-4257-ad6b-867c195de604/endpoints/0bf687c1-c864-45df-891a-90f548dee242/events/poll?cursor=1590886559&epfs=srt&sc", 200, istream) && passed;

		istream.get_body(buffer, 5);
		passed = verify_binary(context, buffer, "a=2\"}", 5, istream) && passed;

		return passed;
	}


	// --------------------------------------------------------------


	bool test_http_response_ostream_bodytext(test_context<abc::test_log_ptr>& context) {
		const char expected[] =
			"HTTP/1.1 200 OK\r\n"
			"Simple: simple\r\n"
			"List: foo bar foobar\r\n"
			"\r\n"
			"First line\r\n"
			"  Second line\r\n"
			"\tThird line";

		char actual [1024 + 1];

		abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

		abc::http_response_ostream<abc::test_log_ptr> ostream(&sb, context.log_ptr);

		bool passed = true;
		const char* input;

		input = "HTTP/1.1";
		ostream.put_protocol(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "200";
		ostream.put_status_code(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "OK";
		ostream.put_reason_phrase(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "Simple";
		ostream.put_header_name(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "simple";
		ostream.put_header_value(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "List";
		ostream.put_header_name(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "foo    bar\t\t\tfoobar   \t  \t \t ";
		ostream.put_header_value(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		ostream.end_headers();

		input = "First line\r\n";
		ostream.put_body(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "  Second line\r\n";
		ostream.put_body(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "\tThird line\r\n";
		ostream.put_body(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		passed = context.are_equal(actual, expected, std::strlen(expected), __TAG__) && passed;

		return passed;
	}


	bool test_http_response_ostream_bodybinary(test_context<abc::test_log_ptr>& context) {
		const char expected[] =
			"HTTP/1.1 789 Something went wrong \r\n"
			"Multi-Line-List: aaa bbbb ccc ddd\r\n"
			"\r\n"
			"\x03\x07\x13\x16\x19"
			"\x20\x24\x35\x46\x57\x71\x7f"
			"\x80\x89\xa5\xb6\xc7\xff";

		char actual [1024 + 1];

		abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

		abc::http_response_ostream<abc::test_log_ptr> ostream(&sb, context.log_ptr);

		bool passed = true;
		const char* input;

		input = "HTTP/1.1";
		ostream.put_protocol(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "789";
		ostream.put_status_code(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "Somethig went wrong ";
		ostream.put_reason_phrase(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "Multi-Line-List";
		ostream.put_header_name(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "\r\n  \r\n\taaa  \t bbb\r\n\t\t\tccc\tddd";
		ostream.put_header_value(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		ostream.end_headers();

		input = "\x03\x07\x13\x16\x19";
		ostream.put_body(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "\x20\x24\x35\x46\x57\x71\x7f";
		ostream.put_body(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		input = "\x80\x89\xa5\xb6\xc7\xff";
		ostream.put_body(input);
		passed = verify_stream(context, ostream, std::strlen(input)) && passed;

		passed = context.are_equal(actual, expected, std::strlen(expected), __TAG__) && passed;

		return passed;
	}


	// --------------------------------------------------------------


	template <typename StdStream>
	static bool verify_string(test_context<abc::test_log_ptr>& context, const char* actual, const char* expected, const abc::_http_stream<StdStream, abc::test_log_ptr>& istream) {
		bool passed = true;

		passed = context.are_equal(actual, expected, __TAG__) && passed;
		passed = verify_stream(context, istream, std::strlen(expected)) && passed;

		return passed;
	}


	template <typename StdStream>
	static bool verify_binary(test_context<abc::test_log_ptr>& context, const void* actual, const void* expected, std::size_t size, const abc::_http_stream<StdStream, abc::test_log_ptr>& istream) {
		bool passed = true;

		passed = context.are_equal(actual, expected, size, __TAG__) && passed;
		passed = verify_stream(context, istream, size) && passed;

		return passed;
	}


	template <typename StdStream>
	static bool verify_stream(test_context<abc::test_log_ptr>& context, const abc::_http_stream<StdStream, abc::test_log_ptr>& stream, std::size_t expected_gcount) {
		bool passed = true;

		passed = context.are_equal(stream.gcount(), expected_gcount, __TAG__, "%u") && passed;
		passed = context.are_equal(stream.good(), true, __TAG__, "%u") && passed;
		passed = context.are_equal(stream.eof(), false, __TAG__, "%u") && passed;
		passed = context.are_equal(stream.fail(), false, __TAG__, "%u") && passed;
		passed = context.are_equal(stream.bad(), false, __TAG__, "%u") && passed;

		return passed;
	}

}}}

