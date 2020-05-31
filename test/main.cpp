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


#include <iostream>
#include <mutex>

#include "../src/test.h"
#include "../src/log.h"
#include "../src/http.h"

#include "timestamp.h"
#include "streambuf.h"
#include "socket.h"
#include "http.h"


int main() {
	abc::test_log test_log(
		std::move(abc::log_container::ostream(std::clog.rdbuf())),
		std::move(abc::log_view::test<>()),
		std::move(abc::log_filter::severity(abc::severity::abc)));

	abc::test_suite<> test_suite ( {
			{ "timestamp", {
				{ "test_null_timestamp",							abc::test::timestamp::test_null_timestamp },
				{ "test_before_year_2000_before_mar_1_timestamp",	abc::test::timestamp::test_before_year_2000_before_mar_1_timestamp },
				{ "test_before_year_2000_after_mar_1_timestamp",	abc::test::timestamp::test_before_year_2000_after_mar_1_timestamp },
				{ "test_after_year_2000_before_mar_1_timestamp",	abc::test::timestamp::test_after_year_2000_before_mar_1_timestamp },
				{ "test_after_year_2000_after_mar_1_timestamp",		abc::test::timestamp::test_after_year_2000_after_mar_1_timestamp },
			} },
			{ "streambuf", {
				{ "test_buffer_streambuf_1_char",					abc::test::streambuf::test_buffer_streambuf_1_char },
				{ "test_buffer_streambuf_N_chars",					abc::test::streambuf::test_buffer_streambuf_N_chars },
			} },
			{ "socket", {
				{ "test_udp_sync_socket",							abc::test::socket::test_udp_sync_socket },
				{ "test_tcp_sync_socket",							abc::test::socket::test_tcp_sync_socket },
				{ "test_tcp_socket_stream",							abc::test::socket::test_tcp_socket_stream },
			} },
			{ "http", {
				{ "test_http_request_istream_extraspaces",			abc::test::http::test_http_request_istream_extraspaces },
				{ "test_http_request_istream_bodytext",				abc::test::http::test_http_request_istream_bodytext },
				{ "test_http_request_istream_bodybinary",			abc::test::http::test_http_request_istream_bodybinary },
			} },
		},
		&test_log,
		0);

	bool passed = test_suite.run();

	return passed ? 0 : 1;
}
