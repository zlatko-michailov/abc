/*
MIT License

Copyright (c) 2018-2023 Zlatko Michailov 

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

#include "../src/util.h"

#include "inc/test.h"
//// TODO: #include "inc/clock.h"
//// TODO: #include "inc/ascii.h"
//// TODO: #include "inc/timestamp.h"
//// TODO: #include "inc/buffer_streambuf.h"
//// TODO: #include "inc/multifile_streambuf.h"
//// TODO: #include "inc/table_stream.h"
//// TODO: #include "inc/socket.h"
//// TODO: #include "inc/http.h"
//// TODO: #include "inc/json.h"
//// TODO: #include "inc/vmem.h"


int main(int /*argc*/, const char* argv[]) {
    test_log_filter filter("", abc::diag::severity::critical);
    test_log log(std::cout.rdbuf(), &filter);

    test_suite suite(
        {
#if 0 //// TODO:
            { "ascii", {
                { "test_ascii_equal",                                abc::test::ascii::test_ascii_equal },
                { "test_ascii_equal_n",                              abc::test::ascii::test_ascii_equal_n },
                { "test_ascii_equal_i",                              abc::test::ascii::test_ascii_equal_i },
                { "test_ascii_equal_i_n",                            abc::test::ascii::test_ascii_equal_i_n },
            } },
            { "timestamp", {
                { "test_null_timestamp",                             abc::test::timestamp::test_null_timestamp },
                { "test_before_year_2000_before_mar_1_timestamp",    abc::test::timestamp::test_before_year_2000_before_mar_1_timestamp },
                { "test_before_year_2000_after_mar_1_timestamp",     abc::test::timestamp::test_before_year_2000_after_mar_1_timestamp },
                { "test_after_year_2000_before_mar_1_timestamp",     abc::test::timestamp::test_after_year_2000_before_mar_1_timestamp },
                { "test_after_year_2000_after_mar_1_timestamp",      abc::test::timestamp::test_after_year_2000_after_mar_1_timestamp },
            } },
            { "buffer_streambuf", {
                { "test_buffer_streambuf_1_char",                    abc::test::streambuf::test_buffer_streambuf_1_char },
                { "test_buffer_streambuf_N_chars",                   abc::test::streambuf::test_buffer_streambuf_N_chars },
                { "test_buffer_streambuf_move",                      abc::test::streambuf::test_buffer_streambuf_move },
            } },
            { "multifile", {
                { "test_multifile_streambuf_move",                   abc::test::multifile::test_multifile_streambuf_move },
                { "test_duration_multifile_streambuf_move",          abc::test::multifile::test_duration_multifile_streambuf_move },
                { "test_size_multifile_streambuf_move",              abc::test::multifile::test_size_multifile_streambuf_move },
            } },
            { "stream", {
                { "test_istream_move",                               abc::test::stream::test_istream_move },
                { "test_ostream_move",                               abc::test::stream::test_ostream_move },
            } },
            { "table_stream", {
                { "test_line_debug",                                 abc::test::table::test_line_debug },
                { "test_line_diag",                                  abc::test::table::test_line_diag },
                { "test_line_test",                                  abc::test::table::test_line_test },
                { "test_table_move",                                 abc::test::table::test_table_move },
                { "test_log_move",                                   abc::test::table::test_log_move },
                { "test_line_move",                                  abc::test::table::test_line_move },
                { "test_line_debug_move",                            abc::test::table::test_line_debug_move },
                { "test_line_diag_move",                             abc::test::table::test_line_diag_move },
                { "test_line_test_move",                             abc::test::table::test_line_test_move },
            } },
            { "http", {
                { "test_http_request_istream_extraspaces",           abc::test::http::test_http_request_istream_extraspaces },
                { "test_http_request_istream_bodytext",              abc::test::http::test_http_request_istream_bodytext },
                { "test_http_request_istream_bodybinary",            abc::test::http::test_http_request_istream_bodybinary },
                { "test_http_request_istream_realworld_01",          abc::test::http::test_http_request_istream_realworld_01 },
                { "test_http_request_istream_resource_01",           abc::test::http::test_http_request_istream_resource_01 },
                { "test_http_request_istream_resource_02",           abc::test::http::test_http_request_istream_resource_02 },
                { "test_http_request_istream_resource_03",           abc::test::http::test_http_request_istream_resource_03 },
                { "test_http_request_istream_resource_04",           abc::test::http::test_http_request_istream_resource_04 },
                { "test_http_request_ostream_bodytext",              abc::test::http::test_http_request_ostream_bodytext },
                { "test_http_request_ostream_bodybinary",            abc::test::http::test_http_request_ostream_bodybinary },
                { "test_http_response_istream_extraspaces",          abc::test::http::test_http_response_istream_extraspaces },
                { "test_http_response_istream_realworld_01",         abc::test::http::test_http_response_istream_realworld_01 },
                { "test_http_response_istream_realworld_02",         abc::test::http::test_http_response_istream_realworld_02 },
                { "test_http_response_ostream_bodytext",             abc::test::http::test_http_response_ostream_bodytext },
                { "test_http_request_istream_move",                  abc::test::http::test_http_request_istream_move },
                { "test_http_request_ostream_move",                  abc::test::http::test_http_request_ostream_move },
                { "test_http_response_istream_move",                 abc::test::http::test_http_response_istream_move },
                { "test_http_response_ostream_move",                 abc::test::http::test_http_response_ostream_move },
                { "test_http_client_stream_move",                    abc::test::http::test_http_client_stream_move },
                { "test_http_server_stream_move",                    abc::test::http::test_http_server_stream_move },
            } },
            { "json", {
                { "test_json_istream_null",                          abc::test::json::test_json_istream_null },
                { "test_json_istream_boolean_01",                    abc::test::json::test_json_istream_boolean_01 },
                { "test_json_istream_boolean_02",                    abc::test::json::test_json_istream_boolean_02 },
                { "test_json_istream_number_01",                     abc::test::json::test_json_istream_number_01 },
                { "test_json_istream_number_02",                     abc::test::json::test_json_istream_number_02 },
                { "test_json_istream_number_03",                     abc::test::json::test_json_istream_number_03 },
                { "test_json_istream_number_04",                     abc::test::json::test_json_istream_number_04 },
                { "test_json_istream_number_05",                     abc::test::json::test_json_istream_number_05 },
                { "test_json_istream_string_01",                     abc::test::json::test_json_istream_string_01 },
                { "test_json_istream_string_02",                     abc::test::json::test_json_istream_string_02 },
                { "test_json_istream_string_03",                     abc::test::json::test_json_istream_string_03 },
                { "test_json_istream_string_04",                     abc::test::json::test_json_istream_string_04 },
                { "test_json_istream_array_01",                      abc::test::json::test_json_istream_array_01 },
                { "test_json_istream_array_02",                      abc::test::json::test_json_istream_array_02 },
                { "test_json_istream_array_03",                      abc::test::json::test_json_istream_array_03 },
                { "test_json_istream_object_01",                     abc::test::json::test_json_istream_object_01 },
                { "test_json_istream_object_02",                     abc::test::json::test_json_istream_object_02 },
                { "test_json_istream_object_03",                     abc::test::json::test_json_istream_object_03 },
                { "test_json_istream_mixed_01",                      abc::test::json::test_json_istream_mixed_01 },
                { "test_json_istream_mixed_02",                      abc::test::json::test_json_istream_mixed_02 },
                { "test_json_istream_skip",                          abc::test::json::test_json_istream_skip },
                { "test_json_ostream_null",                          abc::test::json::test_json_ostream_null },
                { "test_json_ostream_boolean_01",                    abc::test::json::test_json_ostream_boolean_01 },
                { "test_json_ostream_boolean_02",                    abc::test::json::test_json_ostream_boolean_02 },
                { "test_json_ostream_number_01",                     abc::test::json::test_json_ostream_number_01 },
                { "test_json_ostream_number_02",                     abc::test::json::test_json_ostream_number_02 },
                { "test_json_ostream_number_03",                     abc::test::json::test_json_ostream_number_03 },
                { "test_json_ostream_string_01",                     abc::test::json::test_json_ostream_string_01 },
                { "test_json_ostream_string_02",                     abc::test::json::test_json_ostream_string_02 },
                { "test_json_ostream_array_01",                      abc::test::json::test_json_ostream_array_01 },
                { "test_json_ostream_array_02",                      abc::test::json::test_json_ostream_array_02 },
                { "test_json_ostream_array_03",                      abc::test::json::test_json_ostream_array_03 },
                { "test_json_ostream_object_01",                     abc::test::json::test_json_ostream_object_01 },
                { "test_json_ostream_object_02",                     abc::test::json::test_json_ostream_object_02 },
                { "test_json_ostream_object_03",                     abc::test::json::test_json_ostream_object_03 },
                { "test_json_ostream_mixed_01",                      abc::test::json::test_json_ostream_mixed_01 },
                { "test_json_ostream_mixed_02",                      abc::test::json::test_json_ostream_mixed_02 },
                { "test_json_istream_move",                          abc::test::json::test_json_istream_move },
                { "test_json_ostream_move",                          abc::test::json::test_json_ostream_move },
            } },
            { "socket", {
                { "test_udp_socket",                                 abc::test::socket::test_udp_socket },
                { "test_tcp_socket",                                 abc::test::socket::test_tcp_socket },
                { "test_tcp_socket_stream_move",                     abc::test::socket::test_tcp_socket_stream_move },
                { "test_tcp_socket_http_json_stream",                abc::test::socket::test_tcp_socket_http_json_stream },
                { "test_http_endpoint_json_stream",                  abc::test::socket::test_http_endpoint_json_stream },
#ifdef __ABC__OPENSSL
                { "test_openssl_tcp_socket",                         abc::test::socket::test_openssl_tcp_socket },
                { "test_openssl_tcp_socket_stream_move",             abc::test::socket::test_openssl_tcp_socket_stream_move },
                { "test_openssl_tcp_socket_http_json_stream",        abc::test::socket::test_openssl_tcp_socket_http_json_stream },
                { "test_https_endpoint_json_stream",                 abc::test::socket::test_https_endpoint_json_stream },
#endif
            } },
            { "vmem", {
                { "test_vmem_pool_fit",                              abc::test::vmem::test_vmem_pool_fit },
                { "test_vmem_pool_exceed",                           abc::test::vmem::test_vmem_pool_exceed },
                { "test_vmem_pool_reopen",                           abc::test::vmem::test_vmem_pool_reopen },
                { "test_vmem_pool_freepages",                        abc::test::vmem::test_vmem_pool_freepages },
                { "test_vmem_linked_mixedone",                       abc::test::vmem::test_vmem_linked_mixedone },
                { "test_vmem_linked_mixedmany",                      abc::test::vmem::test_vmem_linked_mixedmany },
                { "test_vmem_linked_splice",                         abc::test::vmem::test_vmem_linked_splice },
                { "test_vmem_linked_clear",                          abc::test::vmem::test_vmem_linked_clear },
                { "test_vmem_list_insert",                           abc::test::vmem::test_vmem_list_insert },
                { "test_vmem_list_insertmany",                       abc::test::vmem::test_vmem_list_insertmany },
                { "test_vmem_list_erase",                            abc::test::vmem::test_vmem_list_erase },
                { "test_vmem_temp_destructor",                       abc::test::vmem::test_vmem_temp_destructor },
                { "test_vmem_map_insert",                            abc::test::vmem::test_vmem_map_insert },
                { "test_vmem_map_insertmany",                        abc::test::vmem::test_vmem_map_insertmany },
                { "test_vmem_map_erase",                             abc::test::vmem::test_vmem_map_erase },
                { "test_vmem_map_clear",                             abc::test::vmem::test_vmem_map_clear },
                { "test_vmem_string_iterator",                       abc::test::vmem::test_vmem_string_iterator },
                { "test_vmem_string_stream",                         abc::test::vmem::test_vmem_string_stream },
                { "test_vmem_pool_move",                             abc::test::vmem::test_vmem_pool_move },
                { "test_vmem_page_move",                             abc::test::vmem::test_vmem_page_move },
            } },
#endif
        },
        &log,
        abc::test::seed::random,
        abc::copy(argv[0]));

    bool passed = suite.run();

    return passed ? 0 : 1;
}
