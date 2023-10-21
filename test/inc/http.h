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


#pragma once

#include "../../src/net/http.h"

#include "test.h"


bool test_http_request_istream_extraspaces(test_context& context);
bool test_http_request_istream_bodytext(test_context& context);
bool test_http_request_istream_bodybinary(test_context& context);
bool test_http_request_istream_realworld_01(test_context& context);
bool test_http_request_istream_resource_01(test_context& context);
bool test_http_request_istream_resource_02(test_context& context);
bool test_http_request_istream_resource_03(test_context& context);
bool test_http_request_istream_resource_04(test_context& context);
bool test_http_request_istream_resource_05(test_context& context);
bool test_http_request_istream_resource_06(test_context& context);
bool test_http_request_istream_resource_07(test_context& context);
bool test_http_request_istream_resource_08(test_context& context);
bool test_http_request_istream_resource_09(test_context& context);
bool test_http_request_istream_resource_10(test_context& context);

bool test_http_request_reader_none(test_context& context);
bool test_http_request_reader_headers(test_context& context);
bool test_http_request_reader_body(test_context& context);
bool test_http_request_reader_headers_body(test_context& context);

bool test_http_request_ostream_bodytext(test_context& context);
bool test_http_request_ostream_bodybinary(test_context& context);
bool test_http_request_ostream_resource_01(test_context& context);
bool test_http_request_ostream_resource_02(test_context& context);
bool test_http_request_ostream_resource_03(test_context& context);
bool test_http_request_ostream_resource_04(test_context& context);

bool test_http_request_writer_none(test_context& context);
bool test_http_request_writer_headers(test_context& context);
bool test_http_request_writer_body(test_context& context);
bool test_http_request_writer_headers_body(test_context& context);

bool test_http_response_istream_extraspaces(test_context& context);
bool test_http_response_istream_realworld_01(test_context& context);
bool test_http_response_istream_realworld_02(test_context& context);

//// TODO:
bool test_http_response_reader_01(test_context& context);
bool test_http_response_reader_02(test_context& context);
bool test_http_response_reader_03(test_context& context);
bool test_http_response_reader_04(test_context& context);

bool test_http_response_ostream_bodytext(test_context& context);
bool test_http_response_ostream_bodybinary(test_context& context);
bool test_http_response_ostream_bodynone(test_context& context);

//// TODO:
bool test_http_response_writer_01(test_context& context);
bool test_http_response_writer_02(test_context& context);
bool test_http_response_writer_03(test_context& context);
bool test_http_response_writer_04(test_context& context);

bool test_http_request_istream_move(test_context& context);
bool test_http_request_reader_move(test_context& context); //// TODO:
bool test_http_request_ostream_move(test_context& context);
bool test_http_request_writer_move(test_context& context); //// TODO:
bool test_http_response_istream_move(test_context& context);
bool test_http_response_reader_move(test_context& context); //// TODO:
bool test_http_response_ostream_move(test_context& context);
bool test_http_response_write_move(test_context& context); //// TODO:
bool test_http_client_stream_move(test_context& context);
bool test_http_server_stream_move(test_context& context);
