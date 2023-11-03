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


#include <cstring>
#include <functional>

#include "inc/json.h"


using json_literal_verifier = std::function<bool(test_context&, const abc::net::json::value<test_log*>&)>;
bool json_value(test_context& context, abc::net::json::value<test_log*>&& value, abc::net::json::value_type type, json_literal_verifier&& verify_literal);


bool test_json_value_empty(test_context& context) {
    abc::net::json::value<test_log*> value(context.log());

    bool passed = true;
    passed = context.are_equal(value.type(), abc::net::json::value_type::empty, __TAG__, "%u");
    
    abc::net::json::value<test_log*> value_copy_ctr(value);
    passed = context.are_equal(value_copy_ctr.type(), abc::net::json::value_type::empty, __TAG__, "%u");
    passed = context.are_equal(value.type(), abc::net::json::value_type::empty, __TAG__, "%u");

    abc::net::json::value<test_log*> value_copy_assign(context.log());
    value_copy_assign = value;
    passed = context.are_equal(value_copy_assign.type(), abc::net::json::value_type::empty, __TAG__, "%u");
    passed = context.are_equal(value.type(), abc::net::json::value_type::empty, __TAG__, "%u");

    abc::net::json::value<test_log*> value_move_ctr(std::move(value_copy_ctr));
    passed = context.are_equal(value_move_ctr.type(), abc::net::json::value_type::empty, __TAG__, "%u");
    passed = context.are_equal(value_copy_ctr.type(), abc::net::json::value_type::empty, __TAG__, "%u");

    abc::net::json::value<test_log*> value_move_assign(context.log());
    value_move_assign = std::move(value_copy_assign);
    passed = context.are_equal(value_move_assign.type(), abc::net::json::value_type::empty, __TAG__, "%u");
    passed = context.are_equal(value_copy_assign.type(), abc::net::json::value_type::empty, __TAG__, "%u");

    return passed;
}


bool test_json_value_null(test_context& context) {
    abc::net::json::value<test_log*> value(nullptr, context.log());

    bool passed = true;
    passed = context.are_equal(value.type(), abc::net::json::value_type::null, __TAG__, "%u");
    
    abc::net::json::value<test_log*> value_copy_ctr(value);
    passed = context.are_equal(value_copy_ctr.type(), abc::net::json::value_type::null, __TAG__, "%u");
    passed = context.are_equal(value.type(), abc::net::json::value_type::null, __TAG__, "%u");

    abc::net::json::value<test_log*> value_copy_assign(context.log());
    value_copy_assign = value;
    passed = context.are_equal(value_copy_assign.type(), abc::net::json::value_type::null, __TAG__, "%u");
    passed = context.are_equal(value.type(), abc::net::json::value_type::null, __TAG__, "%u");

    abc::net::json::value<test_log*> value_move_ctr(std::move(value_copy_ctr));
    passed = context.are_equal(value_move_ctr.type(), abc::net::json::value_type::null, __TAG__, "%u");
    passed = context.are_equal(value_copy_ctr.type(), abc::net::json::value_type::empty, __TAG__, "%u");

    abc::net::json::value<test_log*> value_move_assign(context.log());
    value_move_assign = std::move(value_copy_assign);
    passed = context.are_equal(value_move_assign.type(), abc::net::json::value_type::null, __TAG__, "%u");
    passed = context.are_equal(value_copy_assign.type(), abc::net::json::value_type::empty, __TAG__, "%u");

    return passed;
}


bool test_json_value_boolean(test_context& context);
bool test_json_value_number(test_context& context);
bool test_json_value_string(test_context& context);
bool test_json_value_array_simple(test_context& context);
bool test_json_value_object_simple(test_context& context);
bool test_json_value_array_complex(test_context& context);
bool test_json_value_object_complex(test_context& context);


#if 0 //// TODO:
template <typename JsonStream>
static bool verify_string(test_context<abc::test::log>& context, const char* actual, const char* expected, const JsonStream& stream, tag_t tag);

template <typename JsonStream, typename Value>
static bool verify_value(test_context<abc::test::log>& context, const Value& actual, const Value& expected, const JsonStream& stream, tag_t tag, const char* format, std::size_t expected_gcount);


bool test_json_istream_null(test_context<abc::test::log>& context) {
    char content[] =
        " \r \t \n  null \t \r \n";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    const std::size_t size = sizeof(abc::json::item_t);

    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::null, istream, 0x10123, "%x", size) && passed;

    return passed;
}


bool test_json_istream_boolean_01(test_context<abc::test::log>& context) {
    char content[] =
        "false";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    const std::size_t size = sizeof(abc::json::item_t) + sizeof(bool);

    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::boolean, istream, 0x10124, "%x", size) && passed;
    passed = verify_value(context, token->value.boolean, false, istream, 0x10125, "%u", size) && passed;

    return passed;
}


bool test_json_istream_boolean_02(test_context<abc::test::log>& context) {
    char content[] =
        "\rtrue\n";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    const std::size_t size = sizeof(abc::json::item_t) + sizeof(bool);

    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::boolean, istream, 0x10126, "%x", size) && passed;
    passed = verify_value(context, token->value.boolean, true, istream, 0x10127, "%u", size) && passed;

    return passed;
}


bool test_json_istream_number_01(test_context<abc::test::log>& context) {
    char content[] =
        "\t\t\t\t 42 \r\n";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    const std::size_t size = sizeof(abc::json::item_t) + sizeof(double);

    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::number, istream, 0x10128, "%x", size) && passed;
    passed = verify_value(context, token->value.number, 42.0, istream, 0x10129, "%f", size) && passed;

    return passed;
}


bool test_json_istream_number_02(test_context<abc::test::log>& context) {
    char content[] =
        " +1234.567 ";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    const std::size_t size = sizeof(abc::json::item_t) + sizeof(double);

    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::number, istream, 0x1012a, "%x", size) && passed;
    passed = verify_value(context, token->value.number, 1234.567, istream, 0x1012b, "%f", size) && passed;

    return passed;
}


bool test_json_istream_number_03(test_context<abc::test::log>& context) {
    char content[] =
        "\t -56.0000 \t";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    const std::size_t size = sizeof(abc::json::item_t) + sizeof(double);

    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::number, istream, 0x1012c, "%x", size) && passed;
    passed = verify_value(context, token->value.number, -56.0, istream, 0x1012d, "%f", size) && passed;

    return passed;
}


bool test_json_istream_number_04(test_context<abc::test::log>& context) {
    char content[] =
        "\n\r -67.899e23 \r\n";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    const std::size_t size = sizeof(abc::json::item_t) + sizeof(double);

    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::number, istream, 0x1012e, "%x", size) && passed;
    passed = verify_value(context, token->value.number, -67.899e23, istream, 0x1012f, "%f", size) && passed;

    return passed;
}


bool test_json_istream_number_05(test_context<abc::test::log>& context) {
    char content[] =
        "\n\r -88776655443322.999E-5 \r\n";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    const std::size_t size = sizeof(abc::json::item_t) + sizeof(double);

    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::number, istream, 0x10130, "%x", size) && passed;
    passed = verify_value(context, token->value.number, -88776655443322.999E-5, istream, 0x10131, "%f", size) && passed;

    return passed;
}


bool test_json_istream_string_01(test_context<abc::test::log>& context) {
    char content[] =
        "\"\"";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    const std::size_t size = sizeof(abc::json::item_t) + std::strlen("");

    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::string, istream, 0x10132, "%x", size) && passed;
    passed = verify_string(context, token->value.string, "", istream, 0x10133) && passed;

    return passed;
}


bool test_json_istream_string_02(test_context<abc::test::log>& context) {
    char content[] =
        " \r  \"abc xyz\" \n  ";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    const std::size_t size = sizeof(abc::json::item_t) + std::strlen("abc xyz");

    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::string, istream, 0x10134, "%x", size) && passed;
    passed = verify_string(context, token->value.string, "abc xyz", istream, 0x10135) && passed;

    return passed;
}


bool test_json_istream_string_03(test_context<abc::test::log>& context) {
    char content[] =
        "\n\"a\\nb\\rc\\txyz\"";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    const std::size_t size = sizeof(abc::json::item_t) + std::strlen("a\nb\rc\txyz");

    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::string, istream, 0x10136, "%x", size) && passed;
    passed = verify_string(context, token->value.string, "a\nb\rc\txyz", istream, 0x10137) && passed;

    return passed;
}


bool test_json_istream_string_04(test_context<abc::test::log>& context) {
    char content[] =
        "\n   \"абв\\u0020юя\"  ";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    const std::size_t size = sizeof(abc::json::item_t) + std::strlen("абв юя");

    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::string, istream, 0x10138, "%x", size) && passed;
    passed = verify_string(context, token->value.string, "абв юя", istream, 0x10139) && passed;

    return passed;
}


bool test_json_istream_array_01(test_context<abc::test::log>& context) {
    char content[] =
        "[]";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    std::size_t size;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x1013a, "%x", size) && passed;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x1013b, "%x", size) && passed;

    return passed;
}


bool test_json_istream_array_02(test_context<abc::test::log>& context) {
    char content[] =
        "\n[\n\t12.34,\r\n\tnull,\n    true,\r\n    \"abc\"]";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    std::size_t size;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x1013c, "%x", size) && passed;

        size = sizeof(abc::json::item_t) + sizeof(double);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::number, istream, 0x1013d, "%x", size) && passed;
        passed = verify_value(context, token->value.number, 12.34, istream, 0x1013e, "%f", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::null, istream, 0x1013f, "%x", size) && passed;

        size = sizeof(abc::json::item_t) + sizeof(bool);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::boolean, istream, 0x10140, "%x", size) && passed;
        passed = verify_value(context, token->value.boolean, true, istream, 0x10141, "%u", size) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("abc");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::string, istream, 0x10142, "%x", size) && passed;
        passed = verify_string(context, token->value.string, "abc", istream, 0x10143) && passed;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x10144, "%x", size) && passed;

    return passed;
}


bool test_json_istream_array_03(test_context<abc::test::log>& context) {
    char content[] =
        "[ 1, 2, [[3], [4]], [[[5]]] ]";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    std::size_t size;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x10145, "%x", size) && passed;

        size = sizeof(abc::json::item_t) + sizeof(double);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::number, istream, 0x10146, "%x", size) && passed;
        passed = verify_value(context, token->value.number, 1.0, istream, 0x10147, "%f", size) && passed;

        size = sizeof(abc::json::item_t) + sizeof(double);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::number, istream, 0x10148, "%x", size) && passed;
        passed = verify_value(context, token->value.number, 2.0, istream, 0x10149, "%f", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x1014a, "%x", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x1014b, "%x", size) && passed;

                size = sizeof(abc::json::item_t) + sizeof(double);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::number, istream, 0x1014c, "%x", size) && passed;
                passed = verify_value(context, token->value.number, 3.0, istream, 0x1014d, "%f", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x1014e, "%x", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x1014f, "%x", size) && passed;

                size = sizeof(abc::json::item_t) + sizeof(double);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::number, istream, 0x10150, "%x", size) && passed;
                passed = verify_value(context, token->value.number, 4.0, istream, 0x10151, "%f", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x10152, "%x", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x10153, "%x", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x10154, "%x", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x10155, "%x", size) && passed;

                size = sizeof(abc::json::item_t);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x10156, "%x", size) && passed;

                    size = sizeof(abc::json::item_t) + sizeof(double);
                    istream.get_token(token, sizeof(buffer));
                    passed = verify_value(context, token->item, abc::json::item::number, istream, 0x10157, "%x", size) && passed;
                    passed = verify_value(context, token->value.number, 5.0, istream, 0x10158, "%f", size) && passed;

                size = sizeof(abc::json::item_t);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x10159, "%x", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x1015a, "%x", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x1015b, "%x", size) && passed;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x1015c, "%x", size) && passed;

    return passed;
}


bool test_json_istream_object_01(test_context<abc::test::log>& context) {
    char content[] =
        "{}";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    std::size_t size;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x1015d, "%x", size) && passed;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x1015e, "%x", size) && passed;

    return passed;
}


bool test_json_istream_object_02(test_context<abc::test::log>& context) {
    char content[] = R"####(

{ 
            "a":12.34, 
        "bb" : null, 
    
        "ccc": true, 
    
    "dddd"           
    : "abc"}
)####";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    std::size_t size;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x1015f, "%x", size) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("a");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::property, istream, 0x10160, "%x", size) && passed;
        passed = verify_string(context, token->value.property, "a", istream, 0x10161) && passed;

        size = sizeof(abc::json::item_t) + sizeof(double);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::number, istream, 0x10162, "%x", size) && passed;
        passed = verify_value(context, token->value.number, 12.34, istream, 0x10163, "%f", size) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("bb");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::property, istream, 0x10164, "%x", size) && passed;
        passed = verify_string(context, token->value.property, "bb", istream, 0x10165) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::null, istream, 0x10166, "%x", size) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("ccc");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::property, istream, 0x10167, "%x", size) && passed;
        passed = verify_string(context, token->value.property, "ccc", istream, 0x10168) && passed;

        size = sizeof(abc::json::item_t) + sizeof(bool);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::boolean, istream, 0x10169, "%x", size) && passed;
        passed = verify_value(context, token->value.boolean, true, istream, 0x1016a, "%u", size) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("dddd");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::property, istream, 0x1016b, "%x", size) && passed;
        passed = verify_string(context, token->value.property, "dddd", istream, 0x1016c) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("abc");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::string, istream, 0x1016d, "%x", size) && passed;
        passed = verify_string(context, token->value.string, "abc", istream, 0x1016e) && passed;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x1016f, "%x", size) && passed;

    return passed;
}


bool test_json_istream_object_03(test_context<abc::test::log>& context) {
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

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    std::size_t size;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x10170, "%x", size) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("a1");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::property, istream, 0x10171, "%x", size) && passed;
        passed = verify_string(context, token->value.property, "a1", istream, 0x10172) && passed;

        size = sizeof(abc::json::item_t) + sizeof(double);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::number, istream, 0x10173, "%x", size) && passed;
        passed = verify_value(context, token->value.number, 1.0, istream, 0x10174, "%f", size) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("a2");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::property, istream, 0x10175, "%x", size) && passed;
        passed = verify_string(context, token->value.property, "a2", istream, 0x10176) && passed;

        size = sizeof(abc::json::item_t) + sizeof(double);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::number, istream, 0x10177, "%x", size) && passed;
        passed = verify_value(context, token->value.number, 2.0, istream, 0x10178, "%f", size) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("a3");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::property, istream, 0x10179, "%x", size) && passed;
        passed = verify_string(context, token->value.property, "a3", istream, 0x1017a) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x1017b, "%x", size) && passed;

            size = sizeof(abc::json::item_t) + std::strlen("a31");
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::property, istream, 0x1017c, "%x", size) && passed;
            passed = verify_string(context, token->value.property, "a31", istream, 0x1017d) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x1017e, "%x", size) && passed;

                size = sizeof(abc::json::item_t) + std::strlen("a313");
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::property, istream, 0x1017f, "%x", size) && passed;
                passed = verify_string(context, token->value.property, "a313", istream, 0x10180) && passed;

                size = sizeof(abc::json::item_t) + sizeof(double);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::number, istream, 0x10181, "%x", size) && passed;
                passed = verify_value(context, token->value.number, 3.0, istream, 0x10182, "%f", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x10183, "%x", size) && passed;

            size = sizeof(abc::json::item_t) + std::strlen("a32");
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::property, istream, 0x10184, "%x", size) && passed;
            passed = verify_string(context, token->value.property, "a32", istream, 0x10185) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x10186, "%x", size) && passed;

                size = sizeof(abc::json::item_t) + std::strlen("a324");
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::property, istream, 0x10187, "%x", size) && passed;
                passed = verify_string(context, token->value.property, "a324", istream, 0x10188) && passed;

                size = sizeof(abc::json::item_t) + sizeof(double);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::number, istream, 0x10189, "%x", size) && passed;
                passed = verify_value(context, token->value.number, 4.0, istream, 0x1018a, "%f", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x1018b, "%x", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x1018c, "%x", size) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("a5");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::property, istream, 0x1018d, "%x", size) && passed;
        passed = verify_string(context, token->value.property, "a5", istream, 0x1018e) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x1018f, "%x", size) && passed;

            size = sizeof(abc::json::item_t) + std::strlen("a51");
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::property, istream, 0x10190, "%x", size) && passed;
            passed = verify_string(context, token->value.property, "a51", istream, 0x10191) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x10192, "%x", size) && passed;

                size = sizeof(abc::json::item_t) + std::strlen("a512");
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::property, istream, 0x10193, "%x", size) && passed;
                passed = verify_string(context, token->value.property, "a512", istream, 0x10194) && passed;

                size = sizeof(abc::json::item_t);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x10195, "%x", size) && passed;

                    size = sizeof(abc::json::item_t) + std::strlen("a5123");
                    istream.get_token(token, sizeof(buffer));
                    passed = verify_value(context, token->item, abc::json::item::property, istream, 0x10196, "%x", size) && passed;
                    passed = verify_string(context, token->value.property, "a5123", istream, 0x10197) && passed;

                    size = sizeof(abc::json::item_t) + sizeof(double);
                    istream.get_token(token, sizeof(buffer));
                    passed = verify_value(context, token->item, abc::json::item::number, istream, 0x10198, "%x", size) && passed;
                    passed = verify_value(context, token->value.number, 5.0, istream, 0x10199, "%f", size) && passed;

                size = sizeof(abc::json::item_t);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x1019a, "%x", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x1019b, "%x", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x1019c, "%x", size) && passed;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x1019d, "%x", size) && passed;

    return passed;
}


bool test_json_istream_mixed_01(test_context<abc::test::log>& context) {
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

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    std::size_t size;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x1019e, "%x", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x1019f, "%x", size) && passed;

            size = sizeof(abc::json::item_t) + std::strlen("a11");
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::property, istream, 0x101a0, "%x", size) && passed;
            passed = verify_string(context, token->value.property, "a11", istream, 0x101a1) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x101a2, "%x", size) && passed;

                size = sizeof(abc::json::item_t) + sizeof(double);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::number, istream, 0x101a3, "%x", size) && passed;
                passed = verify_value(context, token->value.number, 1.0, istream, 0x101a4, "%f", size) && passed;

                size = sizeof(abc::json::item_t) + sizeof(bool);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::boolean, istream, 0x101a5, "%x", size) && passed;
                passed = verify_value(context, token->value.boolean, true, istream, 0x101a6, "%d", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x101a7, "%x", size) && passed;

            size = sizeof(abc::json::item_t) + std::strlen("a12");
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::property, istream, 0x101a8, "%x", size) && passed;
            passed = verify_string(context, token->value.property, "a12", istream, 0x101a9) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x101aa, "%x", size) && passed;

                size = sizeof(abc::json::item_t) + std::strlen("abc");
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::string, istream, 0x101ab, "%x", size) && passed;
                passed = verify_string(context, token->value.string, "abc", istream, 0x101ac) && passed;

                size = sizeof(abc::json::item_t) + sizeof(double);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::number, istream, 0x101ad, "%x", size) && passed;
                passed = verify_value(context, token->value.number, 2.0, istream, 0x101ae, "%f", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x101af, "%x", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x101b0, "%x", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x101b1, "%x", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x101b2, "%x", size) && passed;

                size = sizeof(abc::json::item_t) + std::strlen("a211");
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::property, istream, 0x101b3, "%x", size) && passed;
                passed = verify_string(context, token->value.property, "a211", istream, 0x101b4) && passed;

                size = sizeof(abc::json::item_t);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x101b5, "%x", size) && passed;

                    size = sizeof(abc::json::item_t) + sizeof(double);
                    istream.get_token(token, sizeof(buffer));
                    passed = verify_value(context, token->item, abc::json::item::number, istream, 0x101b6, "%x", size) && passed;
                    passed = verify_value(context, token->value.number, 4.0, istream, 0x101b7, "%f", size) && passed;

                    size = sizeof(abc::json::item_t) + std::strlen("def");
                    istream.get_token(token, sizeof(buffer));
                    passed = verify_value(context, token->item, abc::json::item::string, istream, 0x101b8, "%x", size) && passed;
                    passed = verify_string(context, token->value.string, "def", istream, 0x101b9) && passed;

                    size = sizeof(abc::json::item_t) + sizeof(bool);
                    istream.get_token(token, sizeof(buffer));
                    passed = verify_value(context, token->item, abc::json::item::boolean, istream, 0x101ba, "%x", size) && passed;
                    passed = verify_value(context, token->value.boolean, false, istream, 0x101bb, "%d", size) && passed;

                size = sizeof(abc::json::item_t);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x101bc, "%x", size) && passed;

                size = sizeof(abc::json::item_t) + std::strlen("a212");
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::property, istream, 0x101bd, "%x", size) && passed;
                passed = verify_string(context, token->value.property, "a212", istream, 0x101be) && passed;

                size = sizeof(abc::json::item_t);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x101bf, "%x", size) && passed;

                    size = sizeof(abc::json::item_t);
                    istream.get_token(token, sizeof(buffer));
                    passed = verify_value(context, token->item, abc::json::item::null, istream, 0x101c0, "%x", size) && passed;

                size = sizeof(abc::json::item_t);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x101c1, "%x", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x101c2, "%x", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x101c3, "%x", size) && passed;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x101c4, "%x", size) && passed;

    return passed;
}


bool test_json_istream_mixed_02(test_context<abc::test::log>& context) {
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

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    std::size_t size;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x101c5, "%x", size) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("a1");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::property, istream, 0x101c6, "%x", size) && passed;
        passed = verify_string(context, token->value.property, "a1", istream, 0x101c7) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x101c8, "%x", size) && passed;

            size = sizeof(abc::json::item_t) + std::strlen("a11");
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::property, istream, 0x101c9, "%x", size) && passed;
            passed = verify_string(context, token->value.property, "a11", istream, 0x101ca) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x101cb, "%x", size) && passed;

                size = sizeof(abc::json::item_t) + sizeof(double);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::number, istream, 0x101cc, "%x", size) && passed;
                passed = verify_value(context, token->value.number, 1.0, istream, 0x101cd, "%f", size) && passed;

                size = sizeof(abc::json::item_t) + sizeof(bool);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::boolean, istream, 0x101ce, "%x", size) && passed;
                passed = verify_value(context, token->value.boolean, true, istream, 0x101cf, "%d", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x101d0, "%x", size) && passed;

            size = sizeof(abc::json::item_t) + std::strlen("a12");
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::property, istream, 0x101d1, "%x", size) && passed;
            passed = verify_string(context, token->value.property, "a12", istream, 0x101d2) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x101d3, "%x", size) && passed;

                size = sizeof(abc::json::item_t) + std::strlen("abc");
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::string, istream, 0x101d4, "%x", size) && passed;
                passed = verify_string(context, token->value.string, "abc", istream, 0x101d5) && passed;

                size = sizeof(abc::json::item_t) + sizeof(double);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::number, istream, 0x101d6, "%x", size) && passed;
                passed = verify_value(context, token->value.number, 2.0, istream, 0x101d7, "%f", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x101d8, "%x", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x101d9, "%x", size) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("a2");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::property, istream, 0x101da, "%x", size) && passed;
        passed = verify_string(context, token->value.property, "a2", istream, 0x101db) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x101dc, "%x", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x101dd, "%x", size) && passed;

                size = sizeof(abc::json::item_t) + std::strlen("a211");
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::property, istream, 0x101de, "%x", size) && passed;
                passed = verify_string(context, token->value.property, "a211", istream, 0x101df) && passed;

                size = sizeof(abc::json::item_t);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x101e0, "%x", size) && passed;

                    size = sizeof(abc::json::item_t) + sizeof(double);
                    istream.get_token(token, sizeof(buffer));
                    passed = verify_value(context, token->item, abc::json::item::number, istream, 0x101e1, "%x", size) && passed;
                    passed = verify_value(context, token->value.number, 4.0, istream, 0x101e2, "%f", size) && passed;

                    size = sizeof(abc::json::item_t) + std::strlen("def");
                    istream.get_token(token, sizeof(buffer));
                    passed = verify_value(context, token->item, abc::json::item::string, istream, 0x101e3, "%x", size) && passed;
                    passed = verify_string(context, token->value.string, "def", istream, 0x101e4) && passed;

                    size = sizeof(abc::json::item_t) + sizeof(bool);
                    istream.get_token(token, sizeof(buffer));
                    passed = verify_value(context, token->item, abc::json::item::boolean, istream, 0x101e5, "%x", size) && passed;
                    passed = verify_value(context, token->value.boolean, false, istream, 0x101e6, "%d", size) && passed;

                size = sizeof(abc::json::item_t);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x101e7, "%x", size) && passed;

                size = sizeof(abc::json::item_t) + std::strlen("a212");
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::property, istream, 0x101e8, "%x", size) && passed;
                passed = verify_string(context, token->value.property, "a212", istream, 0x101e9) && passed;

                size = sizeof(abc::json::item_t);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x101ea, "%x", size) && passed;

                    size = sizeof(abc::json::item_t);
                    istream.get_token(token, sizeof(buffer));
                    passed = verify_value(context, token->item, abc::json::item::null, istream, 0x101eb, "%x", size) && passed;

                size = sizeof(abc::json::item_t);
                istream.get_token(token, sizeof(buffer));
                passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x101ec, "%x", size) && passed;

            size = sizeof(abc::json::item_t);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x101ed, "%x", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x101ee, "%x", size) && passed;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x101ef, "%x", size) && passed;

    return passed;
}


bool test_json_istream_skip(test_context<abc::test::log>& context) {
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

    abc::json_istream<abc::size::_64, abc::test::log> istream(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;
    std::size_t size;
    abc::json::item_t item;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::begin_object, istream, 0x101f0, "%x", size) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("a1");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::property, istream, 0x101f1, "%x", size) && passed;
        passed = verify_string(context, token->value.property, "a1", istream, 0x101f2) && passed;

            size = sizeof(abc::json::item_t);
            item = istream.skip_value();
            passed = verify_value(context, item, abc::json::item::end_object, istream, 0x101f3, "%x", size) && passed;

        size = sizeof(abc::json::item_t) + std::strlen("a2");
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::property, istream, 0x101f4, "%x", size) && passed;
        passed = verify_string(context, token->value.property, "a2", istream, 0x101f5) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::begin_array, istream, 0x101f6, "%x", size) && passed;

            size = sizeof(abc::json::item_t);
            item = istream.skip_value();
            passed = verify_value(context, item, abc::json::item::end_object, istream, 0x101f7, "%x", size) && passed;

            size = sizeof(abc::json::item_t) + sizeof(double);
            istream.get_token(token, sizeof(buffer));
            passed = verify_value(context, token->item, abc::json::item::number, istream, 0x101f8, "%x", size) && passed;
            passed = verify_value(context, token->value.number, 42.0, istream, 0x101f9, "%f", size) && passed;

        size = sizeof(abc::json::item_t);
        istream.get_token(token, sizeof(buffer));
        passed = verify_value(context, token->item, abc::json::item::end_array, istream, 0x101fa, "%x", size) && passed;

    size = sizeof(abc::json::item_t);
    istream.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::end_object, istream, 0x101fb, "%x", size) && passed;

    return passed;
}


// --------------------------------------------------------------


bool test_json_ostream_null(test_context<abc::test::log>& context) {
    char expected[] =
        " \r\t\n null \t\r\n";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    ostream.put_space();
    passed = verify_stream(context, ostream, 0x101fc) && passed;
    ostream.put_cr();
    passed = verify_stream(context, ostream, 0x101fd) && passed;
    ostream.put_tab();
    passed = verify_stream(context, ostream, 0x101fe) && passed;
    ostream.put_lf();
    passed = verify_stream(context, ostream, 0x101ff) && passed;
    ostream.put_space();
    passed = verify_stream(context, ostream, 0x10200) && passed;

    token->item = abc::json::item::null;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10201) && passed;

    ostream.put_space();
    passed = verify_stream(context, ostream, 0x10202) && passed;
    ostream.put_tab();
    passed = verify_stream(context, ostream, 0x10203) && passed;
    ostream.put_cr();
    passed = verify_stream(context, ostream, 0x10204) && passed;
    ostream.put_lf();
    passed = verify_stream(context, ostream, 0x10205) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10206) && passed;

    return passed;
}


bool test_json_ostream_boolean_01(test_context<abc::test::log>& context) {
    char expected[] =
        "\n\nfalse\r\n";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    ostream.put_lf();
    passed = verify_stream(context, ostream, 0x10207) && passed;
    ostream.put_lf();
    passed = verify_stream(context, ostream, 0x10208) && passed;

    token->item = abc::json::item::boolean;
    token->value.boolean = false;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10209) && passed;

    ostream.put_cr();
    passed = verify_stream(context, ostream, 0x1020a) && passed;
    ostream.put_lf();
    passed = verify_stream(context, ostream, 0x1020b) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x1020c) && passed;

    return passed;
}


bool test_json_ostream_boolean_02(test_context<abc::test::log>& context) {
    char expected[] =
        "true";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::boolean;
    token->value.boolean = true;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x1020d) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x1020e) && passed;

    return passed;
}


bool test_json_ostream_number_01(test_context<abc::test::log>& context) {
    char expected[] =
        "42";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::number;
    token->value.number = 42;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x1020f) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10210) && passed;

    return passed;
}


bool test_json_ostream_number_02(test_context<abc::test::log>& context) {
    char expected[] =
        "12345.6789012345";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::number;
    token->value.number = 12345.6789012345;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10211) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10212) && passed;

    return passed;
}


bool test_json_ostream_number_03(test_context<abc::test::log>& context) {
    char expected[] =
        "-8.87766554433221e-10";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::number;
    token->value.number = -8.87766554433221e-10;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10213) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10214) && passed;

    return passed;
}


bool test_json_ostream_string_01(test_context<abc::test::log>& context) {
    char expected[] =
        "\"\"";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::string;
    std::strcpy(token->value.string, "");
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10215) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10216) && passed;

    return passed;
}


bool test_json_ostream_string_02(test_context<abc::test::log>& context) {
    char expected[] =
        "\"qwerty\"";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::string;
    std::strcpy(token->value.string, "qwerty");
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10217) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10218) && passed;

    return passed;
}


bool test_json_ostream_array_01(test_context<abc::test::log>& context) {
    char expected[] =
        "[]";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::begin_array;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10219) && passed;

    token->item = abc::json::item::end_array;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x1021a) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x1021b) && passed;

    return passed;
}


bool test_json_ostream_array_02(test_context<abc::test::log>& context) {
    char expected[] =
        "[ 12.34,null,true,\"abc\" ]";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::begin_array;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x1021c) && passed;
    ostream.put_space();

        token->item = abc::json::item::number;
        token->value.number = 12.34;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1021d) && passed;

        token->item = abc::json::item::null;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1021e) && passed;

        token->item = abc::json::item::boolean;
        token->value.boolean = true;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1021f) && passed;

        token->item = abc::json::item::string;
        std::strcpy(token->value.string, "abc");
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10220) && passed;

    ostream.put_space();
    token->item = abc::json::item::end_array;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10221) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10222) && passed;

    return passed;
}


bool test_json_ostream_array_03(test_context<abc::test::log>& context) {
    char expected[] =
        "[ 1,2,[[3],[4]],[[[5]]] ]";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::begin_array;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10223) && passed;
    ostream.put_space();

        token->item = abc::json::item::number;
        token->value.number = 1;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10224) && passed;

        token->item = abc::json::item::number;
        token->value.number = 2;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10225) && passed;

        token->item = abc::json::item::begin_array;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10226) && passed;

            token->item = abc::json::item::begin_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10227) && passed;

                token->item = abc::json::item::number;
                token->value.number = 3;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10228) && passed;

            token->item = abc::json::item::end_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10229) && passed;

            token->item = abc::json::item::begin_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1022a) && passed;

                token->item = abc::json::item::number;
                token->value.number = 4;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x1022b) && passed;

            token->item = abc::json::item::end_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1022c) && passed;

        token->item = abc::json::item::end_array;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1022d) && passed;

        token->item = abc::json::item::begin_array;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1022e) && passed;

            token->item = abc::json::item::begin_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1022f) && passed;

                token->item = abc::json::item::begin_array;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10230) && passed;

                    token->item = abc::json::item::number;
                    token->value.number = 5;
                    ostream.put_token(token);
                    passed = verify_stream(context, ostream, 0x10231) && passed;

                token->item = abc::json::item::end_array;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10232) && passed;

            token->item = abc::json::item::end_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10233) && passed;

        token->item = abc::json::item::end_array;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10234) && passed;

    ostream.put_space();
    token->item = abc::json::item::end_array;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10235) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10236) && passed;

    return passed;
}


bool test_json_ostream_object_01(test_context<abc::test::log>& context) {
    char expected[] =
        "{}";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::begin_object;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10237) && passed;

    token->item = abc::json::item::end_object;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10238) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10239) && passed;

    return passed;
}


bool test_json_ostream_object_02(test_context<abc::test::log>& context) {
    char expected[] =
        "{"
            "\"a\":12.34,"
            "\"bb\":null,"
            "\"ccc\":true,"
            "\"dddd\":\"abc\""
        "}";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::begin_object;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x1023a) && passed;

        token->item = abc::json::item::property;
        std::strcpy(token->value.property, "a");
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1023b) && passed;

        token->item = abc::json::item::number;
        token->value.number = 12.34;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1023c) && passed;

        token->item = abc::json::item::property;
        std::strcpy(token->value.property, "bb");
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1023d) && passed;

        token->item = abc::json::item::null;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1023e) && passed;

        token->item = abc::json::item::property;
        std::strcpy(token->value.property, "ccc");
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1023f) && passed;

        token->item = abc::json::item::boolean;
        token->value.boolean = true;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10240) && passed;

        token->item = abc::json::item::property;
        std::strcpy(token->value.property, "dddd");
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10241) && passed;

        token->item = abc::json::item::string;
        std::strcpy(token->value.string, "abc");
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10242) && passed;

    token->item = abc::json::item::end_object;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10243) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10244) && passed;

    return passed;
}


bool test_json_ostream_object_03(test_context<abc::test::log>& context) {
    char expected[] = 
        "{"
            "\"a1\":1,"
            "\"a2\":2,"
            "\"a3\":{"
                "\"a31\":{"
                    "\"a313\":3"
                "},"
                "\"a32\":{"
                    "\"a324\":4"
                "}"
            "},"
            "\"a5\":{"
                "\"a51\":{"
                    "\"a512\":{"
                        "\"a5123\":5"
                    "}"
                "}"
            "}"
        "}";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::begin_object;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10245) && passed;

        token->item = abc::json::item::property;
        std::strcpy(token->value.property, "a1");
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10246) && passed;

        token->item = abc::json::item::number;
        token->value.number = 1;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10247) && passed;

        token->item = abc::json::item::property;
        std::strcpy(token->value.property, "a2");
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10248) && passed;

        token->item = abc::json::item::number;
        token->value.number = 2;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10249) && passed;

        token->item = abc::json::item::property;
        std::strcpy(token->value.property, "a3");
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1024a) && passed;

        token->item = abc::json::item::begin_object;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1024b) && passed;

            token->item = abc::json::item::property;
            std::strcpy(token->value.property, "a31");
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1024c) && passed;

            token->item = abc::json::item::begin_object;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1024d) && passed;

                token->item = abc::json::item::property;
                std::strcpy(token->value.property, "a313");
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x1024e) && passed;

                token->item = abc::json::item::number;
                token->value.number = 3;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x1024f) && passed;

            token->item = abc::json::item::end_object;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10250) && passed;

            token->item = abc::json::item::property;
            std::strcpy(token->value.property, "a32");
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10251) && passed;

            token->item = abc::json::item::begin_object;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10252) && passed;

                token->item = abc::json::item::property;
                std::strcpy(token->value.property, "a324");
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10253) && passed;

                token->item = abc::json::item::number;
                token->value.number = 4;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10254) && passed;

            token->item = abc::json::item::end_object;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10255) && passed;

        token->item = abc::json::item::end_object;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10256) && passed;

        token->item = abc::json::item::property;
        std::strcpy(token->value.property, "a5");
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10257) && passed;

        token->item = abc::json::item::begin_object;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10258) && passed;

            token->item = abc::json::item::property;
            std::strcpy(token->value.property, "a51");
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10259) && passed;

            token->item = abc::json::item::begin_object;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1025a) && passed;

                token->item = abc::json::item::property;
                std::strcpy(token->value.property, "a512");
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x1025b) && passed;

                token->item = abc::json::item::begin_object;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x1025c) && passed;

                    token->item = abc::json::item::property;
                    std::strcpy(token->value.property, "a5123");
                    ostream.put_token(token);
                    passed = verify_stream(context, ostream, 0x1025d) && passed;

                    token->item = abc::json::item::number;
                    token->value.number = 5;
                    ostream.put_token(token);
                    passed = verify_stream(context, ostream, 0x1025e) && passed;

                token->item = abc::json::item::end_object;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x1025f) && passed;

            token->item = abc::json::item::end_object;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10260) && passed;

        token->item = abc::json::item::end_object;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10261) && passed;

    token->item = abc::json::item::end_object;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10262) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10263) && passed;

    return passed;
}


bool test_json_ostream_mixed_01(test_context<abc::test::log>& context) {
    char expected[] =
        "["
            "{"
                "\"a11\":[1,true],"
                "\"a12\":[\"abc\",2]"
            "},"
            "["
                "{"
                    "\"a211\":[4,\"def\",false],"
                    "\"a212\":[null]"
                "}"
            "]"
        "]";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::begin_array;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10264) && passed;

        token->item = abc::json::item::begin_object;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10265) && passed;

            token->item = abc::json::item::property;
            std::strcpy(token->value.property, "a11");
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10266) && passed;

            token->item = abc::json::item::begin_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10267) && passed;

                token->item = abc::json::item::number;
                token->value.number = 1;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10268) && passed;

                token->item = abc::json::item::boolean;
                token->value.boolean = true;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10269) && passed;

            token->item = abc::json::item::end_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1026a) && passed;

            token->item = abc::json::item::property;
            std::strcpy(token->value.property, "a12");
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1026b) && passed;

            token->item = abc::json::item::begin_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1026c) && passed;

                token->item = abc::json::item::string;
                std::strcpy(token->value.string, "abc");
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x1026d) && passed;

                token->item = abc::json::item::number;
                token->value.number = 2;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x1026e) && passed;

            token->item = abc::json::item::end_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1026f) && passed;

        token->item = abc::json::item::end_object;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10270) && passed;

        token->item = abc::json::item::begin_array;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10271) && passed;

            token->item = abc::json::item::begin_object;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10272) && passed;

                token->item = abc::json::item::property;
                std::strcpy(token->value.property, "a211");
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10273) && passed;

                token->item = abc::json::item::begin_array;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10274) && passed;

                    token->item = abc::json::item::number;
                    token->value.number = 4;
                    ostream.put_token(token);
                    passed = verify_stream(context, ostream, 0x10275) && passed;

                    token->item = abc::json::item::string;
                    std::strcpy(token->value.string, "def");
                    ostream.put_token(token);
                    passed = verify_stream(context, ostream, 0x10276) && passed;

                    token->item = abc::json::item::boolean;
                    token->value.boolean = false;
                    ostream.put_token(token);
                    passed = verify_stream(context, ostream, 0x10277) && passed;

                token->item = abc::json::item::end_array;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10278) && passed;

                token->item = abc::json::item::property;
                std::strcpy(token->value.property, "a212");
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10279) && passed;

                token->item = abc::json::item::begin_array;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x1027a) && passed;

                    token->item = abc::json::item::null;
                    ostream.put_token(token);
                    passed = verify_stream(context, ostream, 0x1027b) && passed;

                token->item = abc::json::item::end_array;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x1027c) && passed;

            token->item = abc::json::item::end_object;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1027d) && passed;

        token->item = abc::json::item::end_array;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1027e) && passed;

    token->item = abc::json::item::end_array;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x1027f) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10280) && passed;

    return passed;
}


bool test_json_ostream_mixed_02(test_context<abc::test::log>& context) {
    char expected[] =
        "{"
            "\"a1\":{"
                "\"a11\":[1,true],"
                "\"a12\":[\"abc\",2]"
            "},"
            "\"a2\":["
                "{"
                    "\"a211\":[4,\"def\",false],"
                    "\"a212\":[null]"
                "}"
            "]"
        "}";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::json_ostream<abc::size::_64, abc::test::log> ostream(&sb, context.log);

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    token->item = abc::json::item::begin_object;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x10281) && passed;

        token->item = abc::json::item::property;
        std::strcpy(token->value.property, "a1");
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10282) && passed;

        token->item = abc::json::item::begin_object;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10283) && passed;

            token->item = abc::json::item::property;
            std::strcpy(token->value.property, "a11");
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10284) && passed;

            token->item = abc::json::item::begin_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10285) && passed;

                token->item = abc::json::item::number;
                token->value.number = 1;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10286) && passed;

                token->item = abc::json::item::boolean;
                token->value.boolean = true;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10287) && passed;

            token->item = abc::json::item::end_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10288) && passed;

            token->item = abc::json::item::property;
            std::strcpy(token->value.property, "a12");
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10289) && passed;

            token->item = abc::json::item::begin_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1028a) && passed;

                token->item = abc::json::item::string;
                std::strcpy(token->value.string, "abc");
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x1028b) && passed;

                token->item = abc::json::item::number;
                token->value.number = 2;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x1028c) && passed;

            token->item = abc::json::item::end_array;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1028d) && passed;

        token->item = abc::json::item::end_object;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1028e) && passed;

        token->item = abc::json::item::property;
        std::strcpy(token->value.property, "a2");
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1028f) && passed;

        token->item = abc::json::item::begin_array;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x10290) && passed;

            token->item = abc::json::item::begin_object;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x10291) && passed;

                token->item = abc::json::item::property;
                std::strcpy(token->value.property, "a211");
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10292) && passed;

                token->item = abc::json::item::begin_array;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10293) && passed;

                    token->item = abc::json::item::number;
                    token->value.number = 4;
                    ostream.put_token(token);
                    passed = verify_stream(context, ostream, 0x10294) && passed;

                    token->item = abc::json::item::string;
                    std::strcpy(token->value.string, "def");
                    ostream.put_token(token);
                    passed = verify_stream(context, ostream, 0x10295) && passed;

                    token->item = abc::json::item::boolean;
                    token->value.boolean = false;
                    ostream.put_token(token);
                    passed = verify_stream(context, ostream, 0x10296) && passed;

                token->item = abc::json::item::end_array;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10297) && passed;

                token->item = abc::json::item::property;
                std::strcpy(token->value.property, "a212");
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10298) && passed;

                token->item = abc::json::item::begin_array;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x10299) && passed;

                    token->item = abc::json::item::null;
                    ostream.put_token(token);
                    passed = verify_stream(context, ostream, 0x1029a) && passed;

                token->item = abc::json::item::end_array;
                ostream.put_token(token);
                passed = verify_stream(context, ostream, 0x1029b) && passed;

            token->item = abc::json::item::end_object;
            ostream.put_token(token);
            passed = verify_stream(context, ostream, 0x1029c) && passed;

        token->item = abc::json::item::end_array;
        ostream.put_token(token);
        passed = verify_stream(context, ostream, 0x1029d) && passed;

    token->item = abc::json::item::end_object;
    ostream.put_token(token);
    passed = verify_stream(context, ostream, 0x1029e) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x1029f) && passed;

    return passed;
}


// --------------------------------------------------------------


bool test_json_istream_move(test_context<abc::test::log>& context) {
    char content[] =
        "false 42 ";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::json_istream<abc::size::_64, abc::test::log> istream1(&sb, context.log);

    char buffer[101];
    abc::json::token_t* token = (abc::json::token_t*)buffer;
    bool passed = true;

    std::size_t size = sizeof(abc::json::item_t) + sizeof(bool);

    istream1.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::boolean, istream1, 0x10721, "%x", size) && passed;
    passed = verify_value(context, token->value.boolean, false, istream1, 0x10722, "%u", size) && passed;

    abc::json_istream<abc::size::_64, abc::test::log> istream2(std::move(istream1));

    size = sizeof(abc::json::item_t) + sizeof(double);

    istream2.get_token(token, sizeof(buffer));
    passed = verify_value(context, token->item, abc::json::item::number, istream2, 0x10723, "%x", size) && passed;
    passed = verify_value(context, token->value.number, 42.0, istream2, 0x10724, "%f", size) && passed;

    return passed;
}


bool test_json_ostream_move(test_context<abc::test::log>& context) {
    char expected[] =
        "true 42";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    char token_buffer [sizeof(abc::json::item_t) + 1024 + 1];
    abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(token_buffer);

    bool passed = true;

    abc::json_ostream<abc::size::_64, abc::test::log> ostream1(&sb, context.log);

    token->item = abc::json::item::boolean;
    token->value.boolean = true;
    ostream1.put_token(token);
    passed = verify_stream(context, ostream1, 0x10725) && passed;

    abc::json_ostream<abc::size::_64, abc::test::log> ostream2(std::move(ostream1));
    ostream2.put_space();

    token->item = abc::json::item::number;
    token->value.number = 42.0;
    ostream2.put_token(token);
    passed = verify_stream(context, ostream2, 0x10726) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10727) && passed;

    return passed;
}


// --------------------------------------------------------------


template <typename JsonStream>
static bool verify_string(test_context<abc::test::log>& context, const char* actual, const char* expected, const JsonStream& stream, tag_t tag) {
    bool passed = true;

    passed = context.are_equal(actual, expected, tag) && passed;
    passed = verify_stream(context, stream, sizeof(abc::json::item_t) + std::strlen(expected), tag) && passed;

    return passed;
}


template <typename JsonStream, typename Value>
static bool verify_value(test_context<abc::test::log>& context, const Value& actual, const Value& expected, const JsonStream& stream, tag_t tag, const char* format, std::size_t expected_gcount) {
    bool passed = true;

    passed = context.are_equal(actual, expected, tag, format) && passed;
    passed = verify_stream(context, stream, expected_gcount, tag) && passed;

    return passed;
}
#endif
