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


	bool test_json_istream_string_01(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"\"\"";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		const std::size_t size = sizeof(abc::json::item_t) + std::strlen("");

		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::string, istream, __TAG__, "%x", size) && passed;
		passed = verify_string(context, token->value.string, "", istream, __TAG__) && passed;

		return passed;
	}


	bool test_json_istream_string_02(test_context<abc::test_log_ptr>& context) {
		char content[] =
			" \r  \"abc xyz\" \n  ";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		const std::size_t size = sizeof(abc::json::item_t) + std::strlen("abc xyz");

		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::string, istream, __TAG__, "%x", size) && passed;
		passed = verify_string(context, token->value.string, "abc xyz", istream, __TAG__) && passed;

		return passed;
	}


	bool test_json_istream_string_03(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"\n\"a\\nb\\rc\\txyz\"";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		const std::size_t size = sizeof(abc::json::item_t) + std::strlen("a\nb\rc\txyz");

		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::string, istream, __TAG__, "%x", size) && passed;
		passed = verify_string(context, token->value.string, "a\nb\rc\txyz", istream, __TAG__) && passed;

		return passed;
	}


	bool test_json_istream_string_04(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"\n   \"абв\\u0020юя\"  ";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		const std::size_t size = sizeof(abc::json::item_t) + std::strlen("абв юя");

		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::string, istream, __TAG__, "%x", size) && passed;
		passed = verify_string(context, token->value.string, "абв юя", istream, __TAG__) && passed;

		return passed;
	}


	bool test_json_istream_array_01(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"[]";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		std::size_t size;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

		return passed;
	}


	bool test_json_istream_array_02(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"\n[\n\t12.34,\r\n\tnull,\n    true,\r\n    \"abc\"]";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		std::size_t size;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t) + sizeof(double);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
			passed = verify_value(context, token->value.number, 12.34, istream, __TAG__, "%f", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::null, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t) + sizeof(bool);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::boolean, istream, __TAG__, "%x", size) && passed;
			passed = verify_value(context, token->value.boolean, true, istream, __TAG__, "%u", size) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("abc");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::string, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.string, "abc", istream, __TAG__) && passed;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

		return passed;
	}


	bool test_json_istream_array_03(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"[ 1, 2, [[3], [4]], [[[5]]] ]";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		std::size_t size;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t) + sizeof(double);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
			passed = verify_value(context, token->value.number, 1.0, istream, __TAG__, "%f", size) && passed;

			size = sizeof(abc::json::item_t) + sizeof(double);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
			passed = verify_value(context, token->value.number, 2.0, istream, __TAG__, "%f", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t) + sizeof(double);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
					passed = verify_value(context, token->value.number, 3.0, istream, __TAG__, "%f", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t) + sizeof(double);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
					passed = verify_value(context, token->value.number, 4.0, istream, __TAG__, "%f", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

						size = sizeof(abc::json::item_t) + sizeof(double);
						istream.get_token(token, sizeof(buffer));
						passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
						passed = verify_value(context, token->value.number, 5.0, istream, __TAG__, "%f", size) && passed;

					size = sizeof(abc::json::item_t);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

		return passed;
	}


	bool test_json_istream_object_01(test_context<abc::test_log_ptr>& context) {
		char content[] =
			"{}";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		std::size_t size;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

		return passed;
	}


	bool test_json_istream_object_02(test_context<abc::test_log_ptr>& context) {
		char content[] = R"####(

 { 
   		  "a":12.34, 
	  	"bb" : null, 
     
		  "ccc": true, 
     
	   "dddd" 		  
	    : "abc"}
)####";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		std::size_t size;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("a");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.property, "a", istream, __TAG__) && passed;

			size = sizeof(abc::json::item_t) + sizeof(double);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
			passed = verify_value(context, token->value.number, 12.34, istream, __TAG__, "%f", size) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("bb");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.property, "bb", istream, __TAG__) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::null, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("ccc");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.property, "ccc", istream, __TAG__) && passed;

			size = sizeof(abc::json::item_t) + sizeof(bool);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::boolean, istream, __TAG__, "%x", size) && passed;
			passed = verify_value(context, token->value.boolean, true, istream, __TAG__, "%u", size) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("dddd");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.property, "dddd", istream, __TAG__) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("abc");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::string, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.string, "abc", istream, __TAG__) && passed;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

		return passed;
	}


	bool test_json_istream_object_03(test_context<abc::test_log_ptr>& context) {
		char content[] = R"####(
{
	"a1": 1,
	"a2": 2,
	"a3": {
		"a31": {
			"a313": 3
	},
		"a32": {
			"a324": 4
		}
	},
	"a5": {
		"a51": {
			"a512": {
				"a5123": 5
			}
		}
	}
}
)####";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		std::size_t size;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("a1");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.property, "a1", istream, __TAG__) && passed;

			size = sizeof(abc::json::item_t) + sizeof(double);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
			passed = verify_value(context, token->value.number, 1.0, istream, __TAG__, "%f", size) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("a2");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.property, "a2", istream, __TAG__) && passed;

			size = sizeof(abc::json::item_t) + sizeof(double);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
			passed = verify_value(context, token->value.number, 2.0, istream, __TAG__, "%f", size) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("a3");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.property, "a3", istream, __TAG__) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t) + std::strlen("a31");
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
				passed = verify_string(context, token->value.property, "a31", istream, __TAG__) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t) + std::strlen("a313");
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
					passed = verify_string(context, token->value.property, "a313", istream, __TAG__) && passed;

					size = sizeof(abc::json::item_t) + sizeof(double);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
					passed = verify_value(context, token->value.number, 3.0, istream, __TAG__, "%f", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t) + std::strlen("a32");
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
				passed = verify_string(context, token->value.property, "a32", istream, __TAG__) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t) + std::strlen("a324");
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
					passed = verify_string(context, token->value.property, "a324", istream, __TAG__) && passed;

					size = sizeof(abc::json::item_t) + sizeof(double);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
					passed = verify_value(context, token->value.number, 4.0, istream, __TAG__, "%f", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("a5");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.property, "a5", istream, __TAG__) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t) + std::strlen("a51");
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
				passed = verify_string(context, token->value.property, "a51", istream, __TAG__) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t) + std::strlen("a512");
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
					passed = verify_string(context, token->value.property, "a512", istream, __TAG__) && passed;

					size = sizeof(abc::json::item_t);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

						size = sizeof(abc::json::item_t) + std::strlen("a5123");
						istream.get_token(token, sizeof(buffer));
						passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
						passed = verify_string(context, token->value.property, "a5123", istream, __TAG__) && passed;

						size = sizeof(abc::json::item_t) + sizeof(double);
						istream.get_token(token, sizeof(buffer));
						passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
						passed = verify_value(context, token->value.number, 5.0, istream, __TAG__, "%f", size) && passed;

					size = sizeof(abc::json::item_t);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

		return passed;
	}


	bool test_json_istream_mixed_01(test_context<abc::test_log_ptr>& context) {
		char content[] = R"####(
[
	{
		"a11": [ 1, true ],
		"a12": [ "abc", 2 ]
	},
	[
		{
			"a211": [ 4, "def", false ],
			"a212": [ null ]
		}
	]
]
)####";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		std::size_t size;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t) + std::strlen("a11");
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
				passed = verify_string(context, token->value.property, "a11", istream, __TAG__) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t) + sizeof(double);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
					passed = verify_value(context, token->value.number, 1.0, istream, __TAG__, "%f", size) && passed;

					size = sizeof(abc::json::item_t) + sizeof(bool);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::boolean, istream, __TAG__, "%x", size) && passed;
					passed = verify_value(context, token->value.boolean, true, istream, __TAG__, "%d", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t) + std::strlen("a12");
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
				passed = verify_string(context, token->value.property, "a12", istream, __TAG__) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t) + std::strlen("abc");
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::string, istream, __TAG__, "%x", size) && passed;
					passed = verify_string(context, token->value.string, "abc", istream, __TAG__) && passed;

					size = sizeof(abc::json::item_t) + sizeof(double);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
					passed = verify_value(context, token->value.number, 2.0, istream, __TAG__, "%f", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t) + std::strlen("a211");
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
					passed = verify_string(context, token->value.property, "a211", istream, __TAG__) && passed;

					size = sizeof(abc::json::item_t);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

						size = sizeof(abc::json::item_t) + sizeof(double);
						istream.get_token(token, sizeof(buffer));
						passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
						passed = verify_value(context, token->value.number, 4.0, istream, __TAG__, "%f", size) && passed;

						size = sizeof(abc::json::item_t) + std::strlen("def");
						istream.get_token(token, sizeof(buffer));
						passed = verify_value(context, token->item, abc::json::item::string, istream, __TAG__, "%x", size) && passed;
						passed = verify_string(context, token->value.string, "def", istream, __TAG__) && passed;

						size = sizeof(abc::json::item_t) + sizeof(bool);
						istream.get_token(token, sizeof(buffer));
						passed = verify_value(context, token->item, abc::json::item::boolean, istream, __TAG__, "%x", size) && passed;
						passed = verify_value(context, token->value.boolean, false, istream, __TAG__, "%d", size) && passed;

					size = sizeof(abc::json::item_t);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t) + std::strlen("a212");
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
					passed = verify_string(context, token->value.property, "a212", istream, __TAG__) && passed;

					size = sizeof(abc::json::item_t);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

						size = sizeof(abc::json::item_t);
						istream.get_token(token, sizeof(buffer));
						passed = verify_value(context, token->item, abc::json::item::null, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

		return passed;
	}


	bool test_json_istream_mixed_02(test_context<abc::test_log_ptr>& context) {
		char content[] = R"####(
{
	"a1": {
		"a11": [ 1, true ],
		"a12": [ "abc", 2 ]
	},
	"a2": [
		{
			"a211": [ 4, "def", false ],
			"a212": [ null ]
		}
	]
}
)####";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		std::size_t size;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("a1");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.property, "a1", istream, __TAG__) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t) + std::strlen("a11");
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
				passed = verify_string(context, token->value.property, "a11", istream, __TAG__) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t) + sizeof(double);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
					passed = verify_value(context, token->value.number, 1.0, istream, __TAG__, "%f", size) && passed;

					size = sizeof(abc::json::item_t) + sizeof(bool);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::boolean, istream, __TAG__, "%x", size) && passed;
					passed = verify_value(context, token->value.boolean, true, istream, __TAG__, "%d", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t) + std::strlen("a12");
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
				passed = verify_string(context, token->value.property, "a12", istream, __TAG__) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t) + std::strlen("abc");
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::string, istream, __TAG__, "%x", size) && passed;
					passed = verify_string(context, token->value.string, "abc", istream, __TAG__) && passed;

					size = sizeof(abc::json::item_t) + sizeof(double);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
					passed = verify_value(context, token->value.number, 2.0, istream, __TAG__, "%f", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("a2");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.property, "a2", istream, __TAG__) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t) + std::strlen("a211");
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
					passed = verify_string(context, token->value.property, "a211", istream, __TAG__) && passed;

					size = sizeof(abc::json::item_t);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

						size = sizeof(abc::json::item_t) + sizeof(double);
						istream.get_token(token, sizeof(buffer));
						passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
						passed = verify_value(context, token->value.number, 4.0, istream, __TAG__, "%f", size) && passed;

						size = sizeof(abc::json::item_t) + std::strlen("def");
						istream.get_token(token, sizeof(buffer));
						passed = verify_value(context, token->item, abc::json::item::string, istream, __TAG__, "%x", size) && passed;
						passed = verify_string(context, token->value.string, "def", istream, __TAG__) && passed;

						size = sizeof(abc::json::item_t) + sizeof(bool);
						istream.get_token(token, sizeof(buffer));
						passed = verify_value(context, token->item, abc::json::item::boolean, istream, __TAG__, "%x", size) && passed;
						passed = verify_value(context, token->value.boolean, false, istream, __TAG__, "%d", size) && passed;

					size = sizeof(abc::json::item_t);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t) + std::strlen("a212");
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
					passed = verify_string(context, token->value.property, "a212", istream, __TAG__) && passed;

					size = sizeof(abc::json::item_t);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

						size = sizeof(abc::json::item_t);
						istream.get_token(token, sizeof(buffer));
						passed = verify_value(context, token->item, abc::json::item::null, istream, __TAG__, "%x", size) && passed;

					size = sizeof(abc::json::item_t);
					istream.get_token(token, sizeof(buffer));
					passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

		return passed;
	}


	bool test_json_istream_skip(test_context<abc::test_log_ptr>& context) {
		char content[] = R"####(
{
	"a1": {
		"a11": [ 1, true ],
		"a12": [ "abc", 2 ]
	},
	"a2": [
		{
			"a211": [ 4, "def", false ],
			"a212": [ null ]
		},
		42,
	]
}
)####";

		abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

		abc::json_istream<abc::test_log_ptr> istream(&sb, context.log_ptr);

		char buffer[101];
		abc::json::token_t* token = (abc::json::token_t*)buffer;
		bool passed = true;
		std::size_t size;
		abc::json::item_t item;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::begin_object, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("a1");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.property, "a1", istream, __TAG__) && passed;

				size = sizeof(abc::json::item_t);
				item = istream.skip_value();
				passed = verify_value(context, item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

			size = sizeof(abc::json::item_t) + std::strlen("a2");
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::property, istream, __TAG__, "%x", size) && passed;
			passed = verify_string(context, token->value.property, "a2", istream, __TAG__) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::begin_array, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t);
				item = istream.skip_value();
				passed = verify_value(context, item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

				size = sizeof(abc::json::item_t) + sizeof(double);
				istream.get_token(token, sizeof(buffer));
				passed = verify_value(context, token->item, abc::json::item::number, istream, __TAG__, "%x", size) && passed;
				passed = verify_value(context, token->value.number, 42.0, istream, __TAG__, "%f", size) && passed;

			size = sizeof(abc::json::item_t);
			istream.get_token(token, sizeof(buffer));
			passed = verify_value(context, token->item, abc::json::item::end_array, istream, __TAG__, "%x", size) && passed;

		size = sizeof(abc::json::item_t);
		istream.get_token(token, sizeof(buffer));
		passed = verify_value(context, token->item, abc::json::item::end_object, istream, __TAG__, "%x", size) && passed;

		return passed;
	}


	// --------------------------------------------------------------


	template <typename StdStream, std::size_t MaxLevels>
	static bool verify_string(test_context<abc::test_log_ptr>& context, const char* actual, const char* expected, const abc::_json_stream<StdStream, abc::test_log_ptr, MaxLevels>& istream, tag_t tag) {
		bool passed = true;

		passed = context.are_equal(actual, expected, tag) && passed;
		passed = verify_stream(context, istream, sizeof(abc::json::item_t) + std::strlen(expected), tag) && passed;

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

