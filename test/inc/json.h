/*
MIT License

Copyright (c) 2018-2024 Zlatko Michailov 

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

#include "../../src/net/json.h"

#include "test.h"
#include "stream.h"


bool test_json_value_empty(test_context& context);
bool test_json_value_null(test_context& context);
bool test_json_value_boolean(test_context& context);
bool test_json_value_number(test_context& context);
bool test_json_value_string(test_context& context);
bool test_json_value_array_simple(test_context& context);
bool test_json_value_object_simple(test_context& context);
bool test_json_value_array_complex(test_context& context);
bool test_json_value_object_complex(test_context& context);

bool test_json_istream_null(test_context& context);
bool test_json_istream_boolean_01(test_context& context);
bool test_json_istream_boolean_02(test_context& context);
bool test_json_istream_number_01(test_context& context);
bool test_json_istream_number_02(test_context& context);
bool test_json_istream_number_03(test_context& context);
bool test_json_istream_number_04(test_context& context);
bool test_json_istream_number_05(test_context& context);
bool test_json_istream_string_01(test_context& context);
bool test_json_istream_string_02(test_context& context);
bool test_json_istream_string_03(test_context& context);
bool test_json_istream_string_04(test_context& context);
bool test_json_istream_array_01(test_context& context);
bool test_json_istream_array_02(test_context& context);
bool test_json_istream_array_03(test_context& context);
bool test_json_istream_object_01(test_context& context);
bool test_json_istream_object_02(test_context& context);
bool test_json_istream_object_03(test_context& context);
bool test_json_istream_mixed_01(test_context& context);
bool test_json_istream_mixed_02(test_context& context);

bool test_json_reader_null(test_context& context);
bool test_json_reader_boolean_01(test_context& context);
bool test_json_reader_boolean_02(test_context& context);
bool test_json_reader_number_01(test_context& context);
bool test_json_reader_number_02(test_context& context);
bool test_json_reader_number_03(test_context& context);
bool test_json_reader_number_04(test_context& context);
bool test_json_reader_number_05(test_context& context);
bool test_json_reader_string_01(test_context& context);
bool test_json_reader_string_02(test_context& context);
bool test_json_reader_string_03(test_context& context);
bool test_json_reader_string_04(test_context& context);
bool test_json_reader_array_01(test_context& context);
bool test_json_reader_array_02(test_context& context);
bool test_json_reader_array_03(test_context& context);
bool test_json_reader_object_01(test_context& context);
bool test_json_reader_object_02(test_context& context);
bool test_json_reader_object_03(test_context& context);
bool test_json_reader_mixed_01(test_context& context);
bool test_json_reader_mixed_02(test_context& context);

bool test_json_ostream_null(test_context& context);
bool test_json_ostream_boolean_01(test_context& context);
bool test_json_ostream_boolean_02(test_context& context);
bool test_json_ostream_number_01(test_context& context);
bool test_json_ostream_number_02(test_context& context);
bool test_json_ostream_number_03(test_context& context);
bool test_json_ostream_string_01(test_context& context);
bool test_json_ostream_string_02(test_context& context);
bool test_json_ostream_array_01(test_context& context);
bool test_json_ostream_array_02(test_context& context);
bool test_json_ostream_array_03(test_context& context);
bool test_json_ostream_object_01(test_context& context);
bool test_json_ostream_object_02(test_context& context);
bool test_json_ostream_object_03(test_context& context);
bool test_json_ostream_mixed_01(test_context& context);
bool test_json_ostream_mixed_02(test_context& context);

bool test_json_writer_null(test_context& context);
bool test_json_writer_boolean_01(test_context& context);
bool test_json_writer_boolean_02(test_context& context);
bool test_json_writer_number_01(test_context& context);
bool test_json_writer_number_02(test_context& context);
bool test_json_writer_number_03(test_context& context);
bool test_json_writer_string_01(test_context& context);
bool test_json_writer_string_02(test_context& context);
bool test_json_writer_array_01(test_context& context);
bool test_json_writer_array_02(test_context& context);
bool test_json_writer_array_03(test_context& context);
bool test_json_writer_object_01(test_context& context);
bool test_json_writer_object_02(test_context& context);
bool test_json_writer_object_03(test_context& context);
bool test_json_writer_mixed_01(test_context& context);
bool test_json_writer_mixed_02(test_context& context);

bool test_json_istream_move(test_context& context);
bool test_json_reader_move(test_context& context);
bool test_json_ostream_move(test_context& context);
bool test_json_writer_move(test_context& context);

