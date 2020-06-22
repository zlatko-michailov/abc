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


#include "json.h"


namespace abc { namespace test { namespace json {

	template <typename StdStream, std::size_t MaxLevels>
	static bool verify_string(test_context<abc::test_log_ptr>& context, const char* actual, const char* expected, const abc::_json_stream<StdStream, abc::test_log_ptr, MaxLevels>& istream, tag_t tag);

	template <typename StdStream, std::size_t MaxLevels, typename Value>
	static bool verify_value(test_context<abc::test_log_ptr>& context, const Value& actual, const Value& expected, const abc::_json_stream<StdStream, abc::test_log_ptr, MaxLevels>& istream, tag_t tag, const char* format, std::size_t expected_gcount);

	template <typename StdStream, std::size_t MaxLevels>
	static bool verify_stream(test_context<abc::test_log_ptr>& context, const abc::_json_stream<StdStream, abc::test_log_ptr, MaxLevels>& istream, std::size_t expected_gcount, tag_t tag);


	bool test_json_istream_null(test_context<abc::test_log_ptr>& context) {
		char content[] =
			" \r \t \n  null \t \r \n";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		const std::size_t size = sizeof(abc::json::item_t);

		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::null, istream, __TAG__, "%x", size) && passed;

		return passed;
	}


	bool test_json_istream_boolean_01(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"false";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		const std::size_t size = sizeof(abc::json::item_t) + sizeof(bool);

		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::boolean, istream, __TAG__, "%x", size) && passed;
		passed = verify_value(context, token->value.boolean, false, istream, __TAG__, "%u", size) && passed;

		return passed;
	}


	bool test_json_istream_boolean_02(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"\rtrue\n";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		const std::size_t size = sizeof(abc::json::item_t) + sizeof(bool);

		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::boolean, istream, __TAG__, "%x", size) && passed;
		passed = verify_value(context, token->value.boolean, true, istream, __TAG__, "%u", size) && passed;

		return passed;
	}


	bool test_json_istream_number_01(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"\t\t\t\t 42 \r\n";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		const std::size_t size = sizeof(abc::json::item_t) + sizeof(double);

		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
		passed = verify_value(context, token->value.number, 42.0, istream, __TAG__, "%f", size) && passed;

		return passed;
	}


	bool test_json_istream_number_02(test_context<abc::test_log_ptr>& context) {
		char content[] =
			" +1234.567 ";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		const std::size_t size = sizeof(abc::json::item_t) + sizeof(double);

		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
		passed = verify_value(context, token->value.number, 1234.567, istream, __TAG__, "%f", size) && passed;

		return passed;
	}


	bool test_json_istream_number_03(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"\t -56.0000 \t";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		const std::size_t size = sizeof(abc::json::item_t) + sizeof(double);

		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
		passed = verify_value(context, token->value.number, -56.0, istream, __TAG__, "%f", size) && passed;

		return passed;
	}


	bool test_json_istream_number_04(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"\n\r -67.899e23 \r\n";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		const std::size_t size = sizeof(abc::json::item_t) + sizeof(double);

		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
		passed = verify_value(context, token->value.number, -67.899e23, istream, __TAG__, "%f", size) && passed;

		return passed;
	}


	bool test_json_istream_number_05(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"\n\r -88776655443322.999E-5 \r\n";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		const std::size_t size = sizeof(abc::json::item_t) + sizeof(double);

		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
		passed = verify_value(context, token->value.number, -88776655443322.999E-5, istream, __TAG__, "%f", size) && passed;

		return passed;
	}




	// --------------------------------------------------------------


	template <typename StdStream, std::size_t MaxLevels>
	static bool verify_string(test_context<abc::test_log_ptr>& context, const char* actual, const char* expected, const abc::_json_stream<StdStream, abc::test_log_ptr, MaxLevels>& istream, tag_t tag) {
		bool passed = true;

		passed = context.are_equal(actual, expected, tag) && passed;
		passed = verify_stream(context, istream, std::strlen(expected), tag) && passed;

		return passed;
	}


	template <typename StdStream, std::size_t MaxLevels, typename Value>
	static bool verify_value(test_context<abc::test_log_ptr>& context, const Value& actual, const Value& expected, const abc::_json_stream<StdStream, abc::test_log_ptr, MaxLevels>& istream, tag_t tag, const char* format, std::size_t expected_gcount) {
		bool passed = true;

		passed = context.are_equal(actual, expected, tag, format) && passed;
		passed = verify_stream(context, istream, expected_gcount, tag) && passed;

		return passed;
	}


	template <typename StdStream, std::size_t MaxLevels>
	static bool verify_stream(test_context<abc::test_log_ptr>& context, const abc::_json_stream<StdStream, abc::test_log_ptr, MaxLevels>& stream, std::size_t expected_gcount, tag_t tag) {
		bool passed = true;

		passed = context.are_equal(stream.gcount(), expected_gcount, tag, "%u") && passed;
		passed = context.are_equal(stream.good(), true, tag, "%u") && passed;
		passed = context.are_equal(stream.eof(), false, tag, "%u") && passed;
		passed = context.are_equal(stream.fail(), false, tag, "%u") && passed;
		passed = context.are_equal(stream.bad(), false, tag, "%u") && passed;

		return passed;
	}

}}}

