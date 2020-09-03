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

#include "test.h"
#include "timestamp.h"
#include "streambuf.h"
#include "table.h"
#include "socket.h"
#include "http.h"
#include "json.h"
#include "heap.h"
#include "clock.h"

#include "../src/multifile.h"


int main() {
	abc::test::log_filter filter(abc::severity::critical);
	abc::test::log log(std::cout.rdbuf(), &filter);

	abc::test_suite<abc::test::log> test_suite( {
			{ "pre-tests", {
				{ "start_heap_allocation",							abc::test::heap::start_heap_allocation },
			} },

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
			{ "table", {
				{ "test_table_line_debug",							abc::test::table::test_table_line_debug },
				{ "test_table_line_diag",							abc::test::table::test_table_line_diag },
				{ "test_table_line_test",							abc::test::table::test_table_line_test },
			} },
			{ "http", {
				{ "test_http_request_istream_extraspaces",			abc::test::http::test_http_request_istream_extraspaces },
				{ "test_http_request_istream_bodytext",				abc::test::http::test_http_request_istream_bodytext },
				{ "test_http_request_istream_bodybinary",			abc::test::http::test_http_request_istream_bodybinary },
				{ "test_http_request_istream_realworld_01",			abc::test::http::test_http_request_istream_realworld_01 },
				{ "test_http_request_ostream_bodytext",				abc::test::http::test_http_request_ostream_bodytext },
				{ "test_http_request_ostream_bodybinary",			abc::test::http::test_http_request_ostream_bodybinary },
				{ "test_http_response_istream_extraspaces",			abc::test::http::test_http_response_istream_extraspaces },
				{ "test_http_response_istream_realworld_01",		abc::test::http::test_http_response_istream_realworld_01 },
				{ "test_http_response_istream_realworld_02",		abc::test::http::test_http_response_istream_realworld_02 },
				{ "test_http_response_ostream_bodytext",			abc::test::http::test_http_response_ostream_bodytext },
			} },
			{ "json", {
				{ "test_json_istream_null",							abc::test::json::test_json_istream_null },
				{ "test_json_istream_boolean_01",					abc::test::json::test_json_istream_boolean_01 },
				{ "test_json_istream_boolean_02",					abc::test::json::test_json_istream_boolean_02 },
				{ "test_json_istream_number_01",					abc::test::json::test_json_istream_number_01 },
				{ "test_json_istream_number_02",					abc::test::json::test_json_istream_number_02 },
				{ "test_json_istream_number_03",					abc::test::json::test_json_istream_number_03 },
				{ "test_json_istream_number_04",					abc::test::json::test_json_istream_number_04 },
				{ "test_json_istream_number_05",					abc::test::json::test_json_istream_number_05 },
				{ "test_json_istream_string_01",					abc::test::json::test_json_istream_string_01 },
				{ "test_json_istream_string_02",					abc::test::json::test_json_istream_string_02 },
				{ "test_json_istream_string_03",					abc::test::json::test_json_istream_string_03 },
				{ "test_json_istream_string_04",					abc::test::json::test_json_istream_string_04 },
				{ "test_json_istream_array_01",						abc::test::json::test_json_istream_array_01 },
				{ "test_json_istream_array_02",						abc::test::json::test_json_istream_array_02 },
				{ "test_json_istream_array_03",						abc::test::json::test_json_istream_array_03 },
				{ "test_json_istream_object_01",					abc::test::json::test_json_istream_object_01 },
				{ "test_json_istream_object_02",					abc::test::json::test_json_istream_object_02 },
				{ "test_json_istream_object_03",					abc::test::json::test_json_istream_object_03 },
				{ "test_json_istream_mixed_01",						abc::test::json::test_json_istream_mixed_01 },
				{ "test_json_istream_mixed_02",						abc::test::json::test_json_istream_mixed_02 },
				{ "test_json_istream_skip",							abc::test::json::test_json_istream_skip },
				{ "test_json_ostream_null",							abc::test::json::test_json_ostream_null },
				{ "test_json_ostream_boolean_01",					abc::test::json::test_json_ostream_boolean_01 },
				{ "test_json_ostream_boolean_02",					abc::test::json::test_json_ostream_boolean_02 },
				{ "test_json_ostream_number_01",					abc::test::json::test_json_ostream_number_01 },
				{ "test_json_ostream_number_02",					abc::test::json::test_json_ostream_number_02 },
				{ "test_json_ostream_number_03",					abc::test::json::test_json_ostream_number_03 },
				{ "test_json_ostream_string_01",					abc::test::json::test_json_ostream_string_01 },
				{ "test_json_ostream_string_02",					abc::test::json::test_json_ostream_string_02 },
				{ "test_json_ostream_array_01",						abc::test::json::test_json_ostream_array_01 },
				{ "test_json_ostream_array_02",						abc::test::json::test_json_ostream_array_02 },
				{ "test_json_ostream_array_03",						abc::test::json::test_json_ostream_array_03 },
				{ "test_json_ostream_object_01",					abc::test::json::test_json_ostream_object_01 },
				{ "test_json_ostream_object_02",					abc::test::json::test_json_ostream_object_02 },
				{ "test_json_ostream_object_03",					abc::test::json::test_json_ostream_object_03 },
				{ "test_json_ostream_mixed_01",						abc::test::json::test_json_ostream_mixed_01 },
				{ "test_json_ostream_mixed_02",						abc::test::json::test_json_ostream_mixed_02 },
			} },
			{ "socket", {
				{ "test_udp_sync_socket",							abc::test::socket::test_udp_sync_socket },
				{ "test_tcp_sync_socket",							abc::test::socket::test_tcp_sync_socket },
				{ "test_tcp_socket_stream",							abc::test::socket::test_tcp_socket_stream },
				{ "test_http_json_socket_stream",					abc::test::socket::test_http_json_socket_stream },
			} },

			{ "post-tests", {
				{ "test_heap_allocation",							abc::test::heap::test_heap_allocation },
			} },
		},
		&log,
		0);

	bool passed = test_suite.run();

	return passed ? 0 : 1;
}
