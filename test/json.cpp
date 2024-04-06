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
#include <sstream>
#include <functional>

#include "inc/json.h"


using json_literal_verifier = std::function<bool(test_context&, const abc::net::json::value<test_log*>&)>;
bool json_value_copy_move(test_context& context, abc::net::json::value<test_log*>&& value, abc::net::json::value_type type, json_literal_verifier&& verify_literal);


bool test_json_value_empty(test_context& context) {
    abc::net::json::value<test_log*> value(context.log());

    json_literal_verifier verifier = 
        [] (test_context& /*context*/, const abc::net::json::value<test_log*>& /*value*/) -> bool { 
            return true;
        };

    return json_value_copy_move(context, std::move(value), abc::net::json::value_type::empty, std::move(verifier));
}


bool test_json_value_null(test_context& context) {
    abc::net::json::value<test_log*> value(nullptr, context.log());

    json_literal_verifier verifier = 
        [] (test_context& /*context*/, const abc::net::json::value<test_log*>& /*value*/) -> bool { 
            return true;
        };

    return json_value_copy_move(context, std::move(value), abc::net::json::value_type::null, std::move(verifier));
}


bool test_json_value_boolean(test_context& context) {
    abc::net::json::value<test_log*> value(true, context.log());

    json_literal_verifier verifier = 
        [] (test_context& context, const abc::net::json::value<test_log*>& value) -> bool { 
            return context.are_equal(value.boolean(), true, __TAG__, "%u");
        };

    return json_value_copy_move(context, std::move(value), abc::net::json::value_type::boolean, std::move(verifier));
}


bool test_json_value_number(test_context& context) {
    abc::net::json::value<test_log*> value(42.5, context.log());

    json_literal_verifier verifier = 
        [] (test_context& context, const abc::net::json::value<test_log*>& value) -> bool { 
            return context.are_equal(value.number(), 42.5, __TAG__, "%f");
        };

    return json_value_copy_move(context, std::move(value), abc::net::json::value_type::number, std::move(verifier));
}


bool test_json_value_string(test_context& context) {
    constexpr const char* str = "jabberwocky";

    abc::net::json::value<test_log*> value(str, context.log());

    json_literal_verifier verifier = 
        [] (test_context& context, const abc::net::json::value<test_log*>& value) -> bool { 
            return context.are_equal(value.string().c_str(), str, __TAG__);
        };

    return json_value_copy_move(context, std::move(value), abc::net::json::value_type::string, std::move(verifier));
}


bool test_json_value_array_simple(test_context& context) {
    const abc::net::json::literal::array<test_log*> arr {
        abc::net::json::value<test_log*>(         context.log()),
        abc::net::json::value<test_log*>(nullptr, context.log()),
        abc::net::json::value<test_log*>(true,    context.log()),
        abc::net::json::value<test_log*>(99.9,    context.log()),
        abc::net::json::value<test_log*>("xyz",   context.log()),
    };

    abc::net::json::value<test_log*> value(abc::copy(arr), context.log());

    json_literal_verifier verifier = 
        [] (test_context& context, const abc::net::json::value<test_log*>& value) -> bool { 
            bool passed = true;

            passed = context.are_equal(value.array()[0].type(),            abc::net::json::value_type::empty, __TAG__, "%u") & passed;
            passed = context.are_equal(value.array()[1].type(),            abc::net::json::value_type::null,  __TAG__, "%u") & passed;
            passed = context.are_equal(value.array()[2].boolean(),         true, __TAG__, "%u") & passed;
            passed = context.are_equal(value.array()[3].number(),          99.9, __TAG__, "%4.1f") & passed;
            passed = context.are_equal(value.array()[4].string().c_str(),  "xyz", __TAG__) & passed;

            return passed;
        };

    return json_value_copy_move(context, std::move(value), abc::net::json::value_type::array, std::move(verifier));
}


bool test_json_value_object_simple(test_context& context) {
    const abc::net::json::literal::object<test_log*> obj {
        { "empty",   abc::net::json::value<test_log*>(         context.log()) },
        { "null",    abc::net::json::value<test_log*>(nullptr, context.log()) },
        { "boolean", abc::net::json::value<test_log*>(true,    context.log()) },
        { "number",  abc::net::json::value<test_log*>(99.9,    context.log()) },
        { "string",  abc::net::json::value<test_log*>("xyz",   context.log()) },
    };

    abc::net::json::value<test_log*> value(abc::copy(obj), context.log());

    json_literal_verifier verifier = 
        [] (test_context& context, const abc::net::json::value<test_log*>& value) -> bool { 
            bool passed = true;

            passed = context.are_equal(value.object().at("empty").type(),            abc::net::json::value_type::empty, __TAG__, "%u") & passed;
            passed = context.are_equal(value.object().at("null").type(),             abc::net::json::value_type::null,  __TAG__, "%u") & passed;
            passed = context.are_equal(value.object().at("boolean").boolean(),       true, __TAG__, "%u") & passed;
            passed = context.are_equal(value.object().at("number").number(),         99.9, __TAG__, "%4.1f") & passed;
            passed = context.are_equal(value.object().at("string").string().c_str(), "xyz", __TAG__) & passed;

            return passed;
        };

    return json_value_copy_move(context, std::move(value), abc::net::json::value_type::object, std::move(verifier));
}


bool test_json_value_array_complex(test_context& context) {
    const abc::net::json::literal::array<test_log*> arr {
        abc::net::json::value<test_log*>(         context.log()),
        abc::net::json::value<test_log*>(nullptr, context.log()),
        abc::net::json::value<test_log*>(true,    context.log()),
        abc::net::json::value<test_log*>(99.9,    context.log()),
        abc::net::json::value<test_log*>("xyz",   context.log()),
    };

    const abc::net::json::literal::array<test_log*> complex_arr {
        abc::net::json::value<test_log*>(abc::copy(arr), context.log()),
        abc::net::json::value<test_log*>(abc::copy(arr), context.log()),
        abc::net::json::value<test_log*>(abc::copy(arr), context.log()),
    };

    abc::net::json::value<test_log*> value(abc::copy(complex_arr), context.log());

    json_literal_verifier verifier = 
        [] (test_context& context, const abc::net::json::value<test_log*>& value) -> bool { 
            bool passed = true;

            for (std::size_t i = 0; i < value.array().size(); i++) {
                passed = context.are_equal(value.array()[i].array()[0].type(),            abc::net::json::value_type::empty, __TAG__, "%u") & passed;
                passed = context.are_equal(value.array()[i].array()[1].type(),            abc::net::json::value_type::null,  __TAG__, "%u") & passed;
                passed = context.are_equal(value.array()[i].array()[2].boolean(),         true, __TAG__, "%u") & passed;
                passed = context.are_equal(value.array()[i].array()[3].number(),          99.9, __TAG__, "%4.1f") & passed;
                passed = context.are_equal(value.array()[i].array()[4].string().c_str(),  "xyz", __TAG__) & passed;
            }

            return passed;
        };

    return json_value_copy_move(context, std::move(value), abc::net::json::value_type::array, std::move(verifier));

}


bool test_json_value_object_complex(test_context& context) {
    const abc::net::json::literal::object<test_log*> obj {
        { "empty",   abc::net::json::value<test_log*>(         context.log()) },
        { "null",    abc::net::json::value<test_log*>(nullptr, context.log()) },
        { "boolean", abc::net::json::value<test_log*>(true,    context.log()) },
        { "number",  abc::net::json::value<test_log*>(99.9,    context.log()) },
        { "string",  abc::net::json::value<test_log*>("xyz",   context.log()) },
    };

    const abc::net::json::literal::object<test_log*> complex_obj {
        { "one",   abc::net::json::value<test_log*>(abc::copy(obj), context.log()) },
        { "two",   abc::net::json::value<test_log*>(abc::copy(obj), context.log()) },
        { "three", abc::net::json::value<test_log*>(abc::copy(obj), context.log()) },
    };

    abc::net::json::value<test_log*> value(abc::copy(complex_obj), context.log());

    json_literal_verifier verifier = 
        [] (test_context& context, const abc::net::json::value<test_log*>& value) -> bool { 
            std::string keys[] = { "one", "two", "three" };
            bool passed = true;

            for (const std::string& key : keys) {
                passed = context.are_equal(value.object().at(key).object().at("empty").type(),            abc::net::json::value_type::empty, __TAG__, "%u") & passed;
                passed = context.are_equal(value.object().at(key).object().at("null").type(),             abc::net::json::value_type::null,  __TAG__, "%u") & passed;
                passed = context.are_equal(value.object().at(key).object().at("boolean").boolean(),       true, __TAG__, "%u") & passed;
                passed = context.are_equal(value.object().at(key).object().at("number").number(),         99.9, __TAG__, "%4.1f") & passed;
                passed = context.are_equal(value.object().at(key).object().at("string").string().c_str(), "xyz", __TAG__) & passed;
            }

            return passed;
        };

    return json_value_copy_move(context, std::move(value), abc::net::json::value_type::object, std::move(verifier));
}


bool json_value_copy_move(test_context& context, abc::net::json::value<test_log*>&& value, abc::net::json::value_type type, json_literal_verifier&& verify_literal) {
    abc::net::json::value<test_log*> empty(context.log());

    bool passed = true;

    passed = context.are_equal(value.type(), type, __TAG__, "%u") & passed;
    
    abc::net::json::value<test_log*> value_copy_ctr(value);
    {
        passed = context.are_equal(value_copy_ctr.type(), type, __TAG__, "%u") & passed;
        passed = context.are_equal(value_copy_ctr == value, true, __TAG__, "%u *") & passed;
        passed = verify_literal(context, value_copy_ctr) & passed;

        passed = context.are_equal(value.type(), type, __TAG__, "%u") & passed;
        passed = context.are_equal(value == value, true, __TAG__, "%u *") & passed;
        passed = verify_literal(context, value) & passed;
    }

    abc::net::json::value<test_log*> value_copy_assign(context.log());
    value_copy_assign = value;
    {
        passed = context.are_equal(value_copy_assign.type(), type, __TAG__, "%u") & passed;
        passed = context.are_equal(value_copy_assign == value, true, __TAG__, "%u *") & passed;
        passed = verify_literal(context, value_copy_assign) & passed;
        
        passed = context.are_equal(value.type(), type, __TAG__, "%u") & passed;
        passed = context.are_equal(value == value, true, __TAG__, "%u *") & passed;
        passed = verify_literal(context, value) & passed;
    }

    abc::net::json::value<test_log*> value_move_ctr(std::move(value_copy_ctr));
    {
        passed = context.are_equal(value_move_ctr.type(), type, __TAG__, "%u") & passed;
        passed = context.are_equal(value_move_ctr == value, true, __TAG__, "%u *") & passed;
        passed = verify_literal(context, value_move_ctr) & passed;

        passed = context.are_equal(value_copy_ctr.type(), abc::net::json::value_type::empty, __TAG__, "%u") & passed;
        passed = context.are_equal(value_copy_ctr == empty, true, __TAG__, "%u *") & passed;
    }

    abc::net::json::value<test_log*> value_move_assign(context.log());
    value_move_assign = std::move(value_copy_assign);
    {
        passed = context.are_equal(value_move_assign.type(), type, __TAG__, "%u") & passed;
        passed = context.are_equal(value_move_assign == value, true, __TAG__, "%u *") & passed;
        passed = verify_literal(context, value_move_assign) & passed;

        passed = context.are_equal(value_copy_assign.type(), abc::net::json::value_type::empty, __TAG__, "%u") & passed;
        passed = context.are_equal(value_copy_assign == empty, true, __TAG__, "%u *") & passed;
    }

    return passed;
}


// --------------------------------------------------------------


template <typename JsonStream>
static bool verify_string(test_context& context, const char* actual, const char* expected, const JsonStream& stream, abc::diag::tag_t tag);

template <typename JsonStream, typename Value>
static bool verify_integral(test_context& context, const Value& actual, const Value& expected, const JsonStream& stream, abc::diag::tag_t tag, const char* format, std::size_t expected_gcount);


bool test_json_istream_null(test_context& context) {
    std::string content =
        " \r \t \n  null \t \r \n";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::null, istream, 0x10123, "%x", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_boolean_01(test_context& context) {
    std::string content =
        "false";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::boolean, istream, 0x10124, "%x", token.string.length()) && passed;
    passed = verify_integral(context, token.boolean, false, istream, 0x10125, "%u", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_boolean_02(test_context& context) {
    std::string content =
        "true";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::boolean, istream, 0x10126, "%x", token.string.length()) && passed;
    passed = verify_integral(context, token.boolean, true, istream, 0x10127, "%u", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_number_01(test_context& context) {
    std::string content =
        "\t\t\t\t 42 \r\n";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x10128, "%x", token.string.length()) && passed;
    passed = verify_integral(context, token.number, 42.0, istream, 0x10129, "%f", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_number_02(test_context& context) {
    std::string content =
        " +1234.567 ";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x1012a, "%x", token.string.length()) && passed;
    passed = verify_integral(context, token.number, 1234.567, istream, 0x1012b, "%f", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_number_03(test_context& context) {
    std::string content =
        "\t -56.0000 \t";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x1012c, "%x", token.string.length()) && passed;
    passed = verify_integral(context, token.number, -56.0, istream, 0x1012d, "%f", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_number_04(test_context& context) {
    std::string content =
        "\n\r -67.899e23 \r\n";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x1012e, "%x", token.string.length()) && passed;
    passed = verify_integral(context, token.number, -67.899e23, istream, 0x1012f, "%f", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_number_05(test_context& context) {
    std::string content =
        "\n\r -88776655443322.999E-5 \r\n";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x10130, "%x", token.string.length()) && passed;
    passed = verify_integral(context, token.number, -88776655443322.999E-5, istream, 0x10131, "%f", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_string_01(test_context& context) {
    std::string content =
        "\"\"";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::string, istream, 0x10132, "%x", token.string.length()) && passed;
    passed = verify_string(context, token.string.c_str(), "", istream, 0x10133) && passed;

    return passed;
}


bool test_json_istream_string_02(test_context& context) {
    std::string content =
        " \r  \"abc xyz\" \n  ";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::string, istream, 0x10134, "%x", token.string.length()) && passed;
    passed = verify_string(context, token.string.c_str(), "abc xyz", istream, 0x10135) && passed;

    return passed;
}


bool test_json_istream_string_03(test_context& context) {
    std::string content =
        "\n\"a\\nb\\rc\\txyz\"";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::string, istream, 0x10136, "%x", token.string.length()) && passed;
    passed = verify_string(context, token.string.c_str(), "a\nb\rc\txyz", istream, 0x10137) && passed;

    return passed;
}


bool test_json_istream_string_04(test_context& context) {
    std::string content =
        "\n   \"абв\\u0020юя\"  ";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::string, istream, 0x10138, "%x", token.string.length()) && passed;
    passed = verify_string(context, token.string.c_str(), "абв юя", istream, 0x10139) && passed;

    return passed;
}


bool test_json_istream_array_01(test_context& context) {
    std::string content =
        "[]";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x1013a, "%x", token.string.length()) && passed;
    {
    }
    token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x1013b, "%x", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_array_02(test_context& context) {
    std::string content =
        "\n[\n\t12.34,\r\n\tnull,\n    true,\r\n    \"abc\"]";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x1013c, "%x", token.string.length()) && passed;
    {
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x1013d, "%x", token.string.length()) && passed;
        passed = verify_integral(context, token.number, 12.34, istream, 0x1013e, "%f", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::null, istream, 0x1013f, "%x", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::boolean, istream, 0x10140, "%x", token.string.length()) && passed;
        passed = verify_integral(context, token.boolean, true, istream, 0x10141, "%u", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::string, istream, 0x10142, "%x", token.string.length()) && passed;
        passed = verify_string(context, token.string.c_str(), "abc", istream, 0x10143) && passed;
    }
    token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x10144, "%x", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_array_03(test_context& context) {
    std::string content =
        "[ 1, 2, [[3], [4]], [[[5]]] ]";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x10145, "%x", token.string.length()) && passed;
    {
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x10146, "%x", token.string.length()) && passed;
        passed = verify_integral(context, token.number, 1.0, istream, 0x10147, "%f", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x10148, "%x", token.string.length()) && passed;
        passed = verify_integral(context, token.number, 2.0, istream, 0x10149, "%f", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x1014a, "%x", token.string.length()) && passed;
        {
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x1014b, "%x", token.string.length()) && passed;
            {
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x1014c, "%x", token.string.length()) && passed;
                passed = verify_integral(context, token.number, 3.0, istream, 0x1014d, "%f", token.string.length()) && passed;
            }
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x1014e, "%x", token.string.length()) && passed;

            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x1014f, "%x", token.string.length()) && passed;
            {
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x10150, "%x", token.string.length()) && passed;
                passed = verify_integral(context, token.number, 4.0, istream, 0x10151, "%f", token.string.length()) && passed;
            }
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x10152, "%x", token.string.length()) && passed;
        }
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x10153, "%x", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x10154, "%x", token.string.length()) && passed;
        {
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x10155, "%x", token.string.length()) && passed;
            {
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x10156, "%x", token.string.length()) && passed;
                {
                    token = istream.get_token();
                    passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x10157, "%x", token.string.length()) && passed;
                    passed = verify_integral(context, token.number, 5.0, istream, 0x10158, "%f", token.string.length()) && passed;
                }
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x10159, "%x", token.string.length()) && passed;
            }
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x1015a, "%x", token.string.length()) && passed;
        }
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x1015b, "%x", token.string.length()) && passed;
    }
    token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x1015c, "%x", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_object_01(test_context& context) {
    std::string content =
        "{}";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x1015d, "%x", token.string.length()) && passed;
    {
    }
    token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x1015e, "%x", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_object_02(test_context& context) {
    std::string content = R"####(

{ 
            "a":12.34, 
        "bb" : null, 
    
        "ccc": true, 
    
    "dddd"           
    : "abc"}
)####";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x1015f, "%x", token.string.length()) && passed;
    {
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x10160, "%x", token.string.length()) && passed;
        passed = verify_string(context, token.string.c_str(), "a", istream, 0x10161) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x10162, "%x", token.string.length()) && passed;
        passed = verify_integral(context, token.number, 12.34, istream, 0x10163, "%f", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x10164, "%x", token.string.length()) && passed;
        passed = verify_string(context, token.string.c_str(), "bb", istream, 0x10165) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::null, istream, 0x10166, "%x", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x10167, "%x", token.string.length()) && passed;
        passed = verify_string(context, token.string.c_str(), "ccc", istream, 0x10168) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::boolean, istream, 0x10169, "%x", token.string.length()) && passed;
        passed = verify_integral(context, token.boolean, true, istream, 0x1016a, "%f", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x1016b, "%x", token.string.length()) && passed;
        passed = verify_string(context, token.string.c_str(), "dddd", istream, 0x1016c) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::string, istream, 0x1016d, "%x", token.string.length()) && passed;
        passed = verify_string(context, token.string.c_str(), "abc", istream, 0x1016e) && passed;
    }
    token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x1016f, "%x", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_object_03(test_context& context) {
    std::string content = R"####(
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

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x10170, "%x", token.string.length()) && passed;
    {
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x10171, "%x", token.string.length()) && passed;
        passed = verify_string(context, token.string.c_str(), "a1", istream, 0x10172) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x10173, "%x", token.string.length()) && passed;
        passed = verify_integral(context, token.number, 1.0, istream, 0x10174, "%f", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x10175, "%x", token.string.length()) && passed;
        passed = verify_string(context, token.string.c_str(), "a2", istream, 0x10176) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x10177, "%x", token.string.length()) && passed;
        passed = verify_integral(context, token.number, 2.0, istream, 0x10178, "%f", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x10179, "%x", token.string.length()) && passed;
        passed = verify_string(context, token.string.c_str(), "a3", istream, 0x1017a) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x1017b, "%x", token.string.length()) && passed;
        {
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x1017c, "%x", token.string.length()) && passed;
            passed = verify_string(context, token.string.c_str(), "a31", istream, 0x1017d) && passed;

            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x1017e, "%x", token.string.length()) && passed;
            {
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x1017f, "%x", token.string.length()) && passed;
                passed = verify_string(context, token.string.c_str(), "a313", istream, 0x10180) && passed;

                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x10181, "%x", token.string.length()) && passed;
                passed = verify_integral(context, token.number, 3.0, istream, 0x10182, "%f", token.string.length()) && passed;
            }
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x10183, "%x", token.string.length()) && passed;

            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x10184, "%x", token.string.length()) && passed;
            passed = verify_string(context, token.string.c_str(), "a32", istream, 0x10185) && passed;

            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x10186, "%x", token.string.length()) && passed;
            {
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x10187, "%x", token.string.length()) && passed;
                passed = verify_string(context, token.string.c_str(), "a324", istream, 0x10188) && passed;

                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x10189, "%x", token.string.length()) && passed;
                passed = verify_integral(context, token.number, 4.0, istream, 0x1018a, "%f", token.string.length()) && passed;
            }
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x1018b, "%x", token.string.length()) && passed;
        }
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x1018c, "%x", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x1018d, "%x", token.string.length()) && passed;
        passed = verify_string(context, token.string.c_str(), "a5", istream, 0x1018e) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x1018f, "%x", token.string.length()) && passed;
        {
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x10190, "%x", token.string.length()) && passed;
            passed = verify_string(context, token.string.c_str(), "a51", istream, 0x10191) && passed;

            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x10192, "%x", token.string.length()) && passed;
            {
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x10193, "%x", token.string.length()) && passed;
                passed = verify_string(context, token.string.c_str(), "a512", istream, 0x10194) && passed;

                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x10195, "%x", token.string.length()) && passed;
                {
                    token = istream.get_token();
                    passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x10196, "%x", token.string.length()) && passed;
                    passed = verify_string(context, token.string.c_str(), "a5123", istream, 0x10197) && passed;

                    token = istream.get_token();
                    passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x10198, "%x", token.string.length()) && passed;
                    passed = verify_integral(context, token.number, 5.0, istream, 0x10199, "%f", token.string.length()) && passed;
                }
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x1019a, "%x", token.string.length()) && passed;
            }
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x1019b, "%x", token.string.length()) && passed;
        }
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x1019c, "%x", token.string.length()) && passed;
    }
    token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x1019d, "%x", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_mixed_01(test_context& context) {
    std::string content = R"####(
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

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x1019e, "%x", token.string.length()) && passed;
    {
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x1019f, "%x", token.string.length()) && passed;
        {
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x101a0, "%x", token.string.length()) && passed;
            passed = verify_string(context, token.string.c_str(), "a11", istream, 0x101a1) && passed;

            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x101a2, "%x", token.string.length()) && passed;
            {
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x101a3, "%x", token.string.length()) && passed;
                passed = verify_integral(context, token.number, 1.0, istream, 0x101a4, "%f", token.string.length()) && passed;

                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::boolean, istream, 0x101a5, "%x", token.string.length()) && passed;
                passed = verify_integral(context, token.boolean, true, istream, 0x101a6, "%u", token.string.length()) && passed;
            }
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x101a7, "%x", token.string.length()) && passed;

            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x101a8, "%x", token.string.length()) && passed;
            passed = verify_string(context, token.string.c_str(), "a12", istream, 0x101a9) && passed;

            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x101aa, "%x", token.string.length()) && passed;
            {
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::string, istream, 0x101ab, "%x", token.string.length()) && passed;
                passed = verify_string(context, token.string.c_str(), "abc", istream, 0x101ac) && passed;

                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x101ad, "%x", token.string.length()) && passed;
                passed = verify_integral(context, token.number, 2.0, istream, 0x101ae, "%u", token.string.length()) && passed;
            }
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x101af, "%x", token.string.length()) && passed;
        }
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x101b0, "%x", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x101b1, "%x", token.string.length()) && passed;
        {
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x101b2, "%x", token.string.length()) && passed;
            {
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x101b3, "%x", token.string.length()) && passed;
                passed = verify_string(context, token.string.c_str(), "a211", istream, 0x101b4) && passed;

                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x101b5, "%x", token.string.length()) && passed;
                {
                    token = istream.get_token();
                    passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x101b6, "%x", token.string.length()) && passed;
                    passed = verify_integral(context, token.number, 4.0, istream, 0x101b7, "%f", token.string.length()) && passed;

                    token = istream.get_token();
                    passed = verify_integral(context, token.type, abc::net::json::token_type::string, istream, 0x101b8, "%x", token.string.length()) && passed;
                    passed = verify_string(context, token.string.c_str(), "def", istream, 0x101b9) && passed;

                    token = istream.get_token();
                    passed = verify_integral(context, token.type, abc::net::json::token_type::boolean, istream, 0x101ba, "%x", token.string.length()) && passed;
                    passed = verify_integral(context, token.boolean, false, istream, 0x101bb, "%u", token.string.length()) && passed;
                }
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x101bc, "%x", token.string.length()) && passed;

                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x101bd, "%x", token.string.length()) && passed;
                passed = verify_string(context, token.string.c_str(), "a212", istream, 0x101be) && passed;

                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x101bf, "%x", token.string.length()) && passed;
                {
                    token = istream.get_token();
                    passed = verify_integral(context, token.type, abc::net::json::token_type::null, istream, 0x101c0, "%x", token.string.length()) && passed;
                }
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x101c1, "%x", token.string.length()) && passed;
            }
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x101c2, "%x", token.string.length()) && passed;
        }
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x101c3, "%x", token.string.length()) && passed;
    }
    token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x101c4, "%x", token.string.length()) && passed;

    return passed;
}


bool test_json_istream_mixed_02(test_context& context) {
    std::string content = R"####(
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

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x101c5, "%x", token.string.length()) && passed;
    {
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x101c6, "%x", token.string.length()) && passed;
        passed = verify_string(context, token.string.c_str(), "a1", istream, 0x101c7) && passed;

        abc::net::json::token token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x101c8, "%x", token.string.length()) && passed;
        {
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x101c9, "%x", token.string.length()) && passed;
            passed = verify_string(context, token.string.c_str(), "a11", istream, 0x101ca) && passed;

            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x101cb, "%x", token.string.length()) && passed;
            {
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x101cc, "%x", token.string.length()) && passed;
                passed = verify_integral(context, token.number, 1.0, istream, 0x101cd, "%f", token.string.length()) && passed;

                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::boolean, istream, 0x101ce, "%x", token.string.length()) && passed;
                passed = verify_integral(context, token.boolean, true, istream, 0x101cf, "%u", token.string.length()) && passed;
            }
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x101d0, "%x", token.string.length()) && passed;

            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x101d1, "%x", token.string.length()) && passed;
            passed = verify_string(context, token.string.c_str(), "a12", istream, 0x101d2) && passed;

            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x101d3, "%x", token.string.length()) && passed;
            {
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::string, istream, 0x101d4, "%x", token.string.length()) && passed;
                passed = verify_string(context, token.string.c_str(), "abc", istream, 0x101d5) && passed;

                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x101d6, "%x", token.string.length()) && passed;
                passed = verify_integral(context, token.number, 2.0, istream, 0x101d7, "%u", token.string.length()) && passed;
            }
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x101d8, "%x", token.string.length()) && passed;
        }
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x101d9, "%x", token.string.length()) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x101da, "%x", token.string.length()) && passed;
        passed = verify_string(context, token.string.c_str(), "a2", istream, 0x101db) && passed;

        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x101dc, "%x", token.string.length()) && passed;
        {
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::begin_object, istream, 0x101dd, "%x", token.string.length()) && passed;
            {
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x101de, "%x", token.string.length()) && passed;
                passed = verify_string(context, token.string.c_str(), "a211", istream, 0x101df) && passed;

                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x101e0, "%x", token.string.length()) && passed;
                {
                    token = istream.get_token();
                    passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream, 0x101e1, "%x", token.string.length()) && passed;
                    passed = verify_integral(context, token.number, 4.0, istream, 0x101e2, "%f", token.string.length()) && passed;

                    token = istream.get_token();
                    passed = verify_integral(context, token.type, abc::net::json::token_type::string, istream, 0x101e3, "%x", token.string.length()) && passed;
                    passed = verify_string(context, token.string.c_str(), "def", istream, 0x101e4) && passed;

                    token = istream.get_token();
                    passed = verify_integral(context, token.type, abc::net::json::token_type::boolean, istream, 0x101e5, "%x", token.string.length()) && passed;
                    passed = verify_integral(context, token.boolean, false, istream, 0x101e6, "%u", token.string.length()) && passed;
                }
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x101e7, "%x", token.string.length()) && passed;

                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::property, istream, 0x101e8, "%x", token.string.length()) && passed;
                passed = verify_string(context, token.string.c_str(), "a212", istream, 0x101e9) && passed;

                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::begin_array, istream, 0x101ea, "%x", token.string.length()) && passed;
                {
                    token = istream.get_token();
                    passed = verify_integral(context, token.type, abc::net::json::token_type::null, istream, 0x101eb, "%x", token.string.length()) && passed;
                }
                token = istream.get_token();
                passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x101ec, "%x", token.string.length()) && passed;
            }
            token = istream.get_token();
            passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x101ed, "%x", token.string.length()) && passed;
        }
        token = istream.get_token();
        passed = verify_integral(context, token.type, abc::net::json::token_type::end_array, istream, 0x101ee, "%x", token.string.length()) && passed;
    }
    token = istream.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::end_object, istream, 0x101ef, "%x", token.string.length()) && passed;

    return passed;
}


// --------------------------------------------------------------


bool test_json_reader_null(test_context& context) {
    std::string content =
        " \r \t \n  null \t \r \n";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::null, __TAG__, "%u") & passed;

    return passed;
}


bool test_json_reader_boolean_01(test_context& context) {
    std::string content =
        "false";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::boolean, __TAG__, "%u") & passed;
    passed = context.are_equal(value.boolean(), false, __TAG__, "%u") & passed;

    return passed;
}


bool test_json_reader_boolean_02(test_context& context) {
    std::string content =
        "true";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::boolean, __TAG__, "%u") & passed;
    passed = context.are_equal(value.boolean(), true, __TAG__, "%u") & passed;

    return passed;
}


bool test_json_reader_number_01(test_context& context) {
    std::string content =
        "\t\t\t\t 42 \r\n";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
    passed = context.are_equal(value.number(), 42.0, __TAG__, "%f") & passed;

    return passed;
}


bool test_json_reader_number_02(test_context& context) {
    std::string content =
        " +1234.567 ";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
    passed = context.are_equal(value.number(), 1234.567, __TAG__, "%f") & passed;

    return passed;
}


bool test_json_reader_number_03(test_context& context) {
    std::string content =
        "\t -56.0000 \t";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
    passed = context.are_equal(value.number(), -56.0, __TAG__, "%f") & passed;

    return passed;
}


bool test_json_reader_number_04(test_context& context) {
    std::string content =
        "\n\r -67.899e23 \r\n";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
    passed = context.are_equal(value.number(), -67.899e23, __TAG__, "%f") & passed;

    return passed;
}


bool test_json_reader_number_05(test_context& context) {
    std::string content =
        "\n\r -88776655443322.999E-5 \r\n";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
    passed = context.are_equal(value.number(), -88776655443322.999E-5, __TAG__, "%f") & passed;

    return passed;
}


bool test_json_reader_string_01(test_context& context) {
    std::string content =
        "\"\"";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::string, __TAG__, "%u") & passed;
    passed = context.are_equal(value.string().c_str(), "", std::strlen(""), __TAG__) & passed;

    return passed;
}


bool test_json_reader_string_02(test_context& context) {
    std::string content =
        " \r  \"abc xyz\" \n  ";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::string, __TAG__, "%u") & passed;
    passed = context.are_equal(value.string().c_str(), "abc xyz", std::strlen("abc xyz"), __TAG__) & passed;

    return passed;
}


bool test_json_reader_string_03(test_context& context) {
    std::string content =
        "\n\"a\\nb\\rc\\txyz\"";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::string, __TAG__, "%u") & passed;
    passed = context.are_equal(value.string().c_str(), "a\nb\rc\txyz", std::strlen("a\nb\rc\txyz"), __TAG__) & passed;

    return passed;
}


bool test_json_reader_string_04(test_context& context) {
    std::string content =
        "\n   \"абв\\u0020юя\"  ";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::string, __TAG__, "%u") & passed;
    passed = context.are_equal(value.string().c_str(), "абв юя", std::strlen("абв юя"), __TAG__) & passed;

    return passed;
}


bool test_json_reader_array_01(test_context& context) {
    std::string content =
        "[]";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
    passed = context.are_equal(value.array().size(), (std::size_t)0, __TAG__, "%zu") & passed;
    {
    }

    return passed;
}


bool test_json_reader_array_02(test_context& context) {
    std::string content =
        "\n[\n\t12.34,\r\n\tnull,\n    true,\r\n    \"abc\"]";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
    passed = context.are_equal(value.array().size(), (std::size_t)4, __TAG__, "%zu") & passed;
    {
        passed = context.are_equal(value.array()[0].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
        passed = context.are_equal(value.array()[0].number(), 12.34, __TAG__, "%f") & passed;

        passed = context.are_equal(value.array()[1].type(), abc::net::json::value_type::null, __TAG__, "%u") & passed;

        passed = context.are_equal(value.array()[2].type(), abc::net::json::value_type::boolean, __TAG__, "%u") & passed;
        passed = context.are_equal(value.array()[2].boolean(), true, __TAG__, "%u") & passed;

        passed = context.are_equal(value.array()[3].type(), abc::net::json::value_type::string, __TAG__, "%u") & passed;
        passed = context.are_equal(value.array()[3].string().c_str(), "abc", std::strlen("abc"), __TAG__) & passed;
    }

    return passed;
}


bool test_json_reader_array_03(test_context& context) {
    std::string content =
        "[ 1, 2, [[3], [4]], [[[5]]] ]";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
    passed = context.are_equal(value.array().size(), (std::size_t)4, __TAG__, "%zu") & passed;
    {
        passed = context.are_equal(value.array()[0].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
        passed = context.are_equal(value.array()[0].number(), 1.0, __TAG__, "%f") & passed;

        passed = context.are_equal(value.array()[1].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
        passed = context.are_equal(value.array()[1].number(), 2.0, __TAG__, "%f") & passed;

        passed = context.are_equal(value.array()[2].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
        passed = context.are_equal(value.array()[2].array().size(), (std::size_t)2, __TAG__, "%zu") & passed;
        {
            passed = context.are_equal(value.array()[2].array()[0].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
            passed = context.are_equal(value.array()[2].array()[0].array().size(), (std::size_t)1, __TAG__, "%zu") & passed;
            {
                passed = context.are_equal(value.array()[2].array()[0].array()[0].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
                passed = context.are_equal(value.array()[2].array()[0].array()[0].number(), 3.0, __TAG__, "%f") & passed;
            }

            passed = context.are_equal(value.array()[2].array()[1].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
            passed = context.are_equal(value.array()[2].array()[1].array().size(), (std::size_t)1, __TAG__, "%zu") & passed;
            {
                passed = context.are_equal(value.array()[2].array()[1].array()[0].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
                passed = context.are_equal(value.array()[2].array()[1].array()[0].number(), 4.0, __TAG__, "%f") & passed;
            }
        }

        passed = context.are_equal(value.array()[3].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
        passed = context.are_equal(value.array()[3].array().size(), (std::size_t)1, __TAG__, "%zu") & passed;
        {
            passed = context.are_equal(value.array()[3].array()[0].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
            passed = context.are_equal(value.array()[3].array()[0].array().size(), (std::size_t)1, __TAG__, "%zu") & passed;
            {
                passed = context.are_equal(value.array()[3].array()[0].array()[0].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
                passed = context.are_equal(value.array()[3].array()[0].array()[0].array().size(), (std::size_t)1, __TAG__, "%zu") & passed;
                {
                    passed = context.are_equal(value.array()[3].array()[0].array()[0].array()[0].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
                    passed = context.are_equal(value.array()[3].array()[0].array()[0].array()[0].number(), 5.0, __TAG__, "%f") & passed;
                }
            }
        }
    }

    return passed;
}


bool test_json_reader_object_01(test_context& context) {
    std::string content =
        "{}";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
    passed = context.are_equal(value.object().size(), (std::size_t)0, __TAG__, "%zu") & passed;
    {
    }

    return passed;
}


bool test_json_reader_object_02(test_context& context) {
    std::string content = R"####(

{ 
            "a":12.34, 
        "bb" : null, 
    
        "ccc": true, 
    
    "dddd"           
    : "abc"}
)####";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
    passed = context.are_equal(value.object().size(), (std::size_t)4, __TAG__, "%zu") & passed;
    {
        passed = context.are_equal(value.object()["a"].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
        passed = context.are_equal(value.object()["a"].number(), 12.34, __TAG__, "%f") & passed;

        passed = context.are_equal(value.object()["bb"].type(), abc::net::json::value_type::null, __TAG__, "%u") & passed;

        passed = context.are_equal(value.object()["ccc"].type(), abc::net::json::value_type::boolean, __TAG__, "%u") & passed;
        passed = context.are_equal(value.object()["ccc"].boolean(), true, __TAG__, "%u") & passed;

        passed = context.are_equal(value.object()["dddd"].type(), abc::net::json::value_type::string, __TAG__, "%u") & passed;
        passed = context.are_equal(value.object()["dddd"].string().c_str(), "abc", std::strlen("abc"), __TAG__) & passed;
    }

    return passed;
}


bool test_json_reader_object_03(test_context& context) {
    std::string content = R"####(
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

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
    passed = context.are_equal(value.object().size(), (std::size_t)4, __TAG__, "%zu") & passed;
    {
        passed = context.are_equal(value.object()["a1"].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
        passed = context.are_equal(value.object()["a1"].number(), 1.0, __TAG__, "%f") & passed;

        passed = context.are_equal(value.object()["a2"].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
        passed = context.are_equal(value.object()["a2"].number(), 2.0, __TAG__, "%f") & passed;

        passed = context.are_equal(value.object()["a3"].type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
        passed = context.are_equal(value.object()["a3"].object().size(), (std::size_t)2, __TAG__, "%zu") & passed;
        {
            passed = context.are_equal(value.object()["a3"].object()["a31"].type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
            passed = context.are_equal(value.object()["a3"].object()["a31"].object().size(), (std::size_t)1, __TAG__, "%zu") & passed;
            {
                passed = context.are_equal(value.object()["a3"].object()["a31"].object()["a313"].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
                passed = context.are_equal(value.object()["a3"].object()["a31"].object()["a313"].number(), 3.0, __TAG__, "%f") & passed;
            }

            passed = context.are_equal(value.object()["a3"].object()["a32"].type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
            passed = context.are_equal(value.object()["a3"].object()["a32"].object().size(), (std::size_t)1, __TAG__, "%zu") & passed;
            {
                passed = context.are_equal(value.object()["a3"].object()["a32"].object()["a324"].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
                passed = context.are_equal(value.object()["a3"].object()["a32"].object()["a324"].number(), 4.0, __TAG__, "%f") & passed;
            }
        }

        passed = context.are_equal(value.object()["a5"].type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
        passed = context.are_equal(value.object()["a5"].object().size(), (std::size_t)1, __TAG__, "%zu") & passed;
        {
            passed = context.are_equal(value.object()["a5"].object()["a51"].type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
            passed = context.are_equal(value.object()["a5"].object()["a51"].object().size(), (std::size_t)1, __TAG__, "%zu") & passed;
            {
                passed = context.are_equal(value.object()["a5"].object()["a51"].object()["a512"].type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
                passed = context.are_equal(value.object()["a5"].object()["a51"].object()["a512"].object().size(), (std::size_t)1, __TAG__, "%zu") & passed;
                {
                    passed = context.are_equal(value.object()["a5"].object()["a51"].object()["a512"].object()["a5123"].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
                    passed = context.are_equal(value.object()["a5"].object()["a51"].object()["a512"].object()["a5123"].number(), 5.0, __TAG__, "%f") & passed;
                }
            }
        }
    }

    return passed;
}


bool test_json_reader_mixed_01(test_context& context) {
    std::string content = R"####(
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

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
    passed = context.are_equal(value.array().size(), (std::size_t)2, __TAG__, "%zu") & passed;
    {
        passed = context.are_equal(value.array()[0].type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
        passed = context.are_equal(value.array()[0].object().size(), (std::size_t)2, __TAG__, "%zu") & passed;
        {
            passed = context.are_equal(value.array()[0].object()["a11"].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
            passed = context.are_equal(value.array()[0].object()["a11"].array().size(), (std::size_t)2, __TAG__, "%zu") & passed;
            {
                passed = context.are_equal(value.array()[0].object()["a11"].array()[0].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
                passed = context.are_equal(value.array()[0].object()["a11"].array()[0].number(), 1.0, __TAG__, "%f") & passed;

                passed = context.are_equal(value.array()[0].object()["a11"].array()[1].type(), abc::net::json::value_type::boolean, __TAG__, "%u") & passed;
                passed = context.are_equal(value.array()[0].object()["a11"].array()[1].boolean(), true, __TAG__, "%u") & passed;
            }

            passed = context.are_equal(value.array()[0].object()["a12"].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
            passed = context.are_equal(value.array()[0].object()["a12"].array().size(), (std::size_t)2, __TAG__, "%zu") & passed;
            {
                passed = context.are_equal(value.array()[0].object()["a12"].array()[0].type(), abc::net::json::value_type::string, __TAG__, "%u") & passed;
                passed = context.are_equal(value.array()[0].object()["a12"].array()[0].string().c_str(), "abc", std::strlen("abc"), __TAG__) & passed;

                passed = context.are_equal(value.array()[0].object()["a12"].array()[1].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
                passed = context.are_equal(value.array()[0].object()["a12"].array()[1].number(), 2.0, __TAG__, "%f") & passed;
            }
        }

        passed = context.are_equal(value.array()[1].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
        passed = context.are_equal(value.array()[1].array().size(), (std::size_t)1, __TAG__, "%zu") & passed;
        {
            passed = context.are_equal(value.array()[1].array()[0].type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
            passed = context.are_equal(value.array()[1].array()[0].object().size(), (std::size_t)2, __TAG__, "%zu") & passed;
            {
                passed = context.are_equal(value.array()[1].array()[0].object()["a211"].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
                passed = context.are_equal(value.array()[1].array()[0].object()["a211"].array().size(), (std::size_t)3, __TAG__, "%zu") & passed;
                {
                    passed = context.are_equal(value.array()[1].array()[0].object()["a211"].array()[0].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
                    passed = context.are_equal(value.array()[1].array()[0].object()["a211"].array()[0].number(), 4.0, __TAG__, "%f") & passed;

                    passed = context.are_equal(value.array()[1].array()[0].object()["a211"].array()[1].type(), abc::net::json::value_type::string, __TAG__, "%u") & passed;
                    passed = context.are_equal(value.array()[1].array()[0].object()["a211"].array()[1].string().c_str(), "def", std::strlen("def"), __TAG__) & passed;

                    passed = context.are_equal(value.array()[1].array()[0].object()["a211"].array()[2].type(), abc::net::json::value_type::boolean, __TAG__, "%u") & passed;
                    passed = context.are_equal(value.array()[1].array()[0].object()["a211"].array()[2].boolean(), false, __TAG__, "%u") & passed;
                }

                passed = context.are_equal(value.array()[1].array()[0].object()["a212"].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
                passed = context.are_equal(value.array()[1].array()[0].object()["a212"].array().size(), (std::size_t)1, __TAG__, "%zu") & passed;
                {
                    passed = context.are_equal(value.array()[1].array()[0].object()["a212"].array()[0].type(), abc::net::json::value_type::null, __TAG__, "%u") & passed;
                }
            }
        }
    }

    return passed;
}


bool test_json_reader_mixed_02(test_context& context) {
    std::string content = R"####(
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

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
    passed = context.are_equal(value.object().size(), (std::size_t)2, __TAG__, "%zu") & passed;
    {
        passed = context.are_equal(value.object()["a1"].type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
        passed = context.are_equal(value.object()["a1"].object().size(), (std::size_t)2, __TAG__, "%zu") & passed;
        {
            passed = context.are_equal(value.object()["a1"].object()["a11"].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
            passed = context.are_equal(value.object()["a1"].object()["a11"].array().size(), (std::size_t)2, __TAG__, "%zu") & passed;
            {
                passed = context.are_equal(value.object()["a1"].object()["a11"].array()[0].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
                passed = context.are_equal(value.object()["a1"].object()["a11"].array()[0].number(), 1.0, __TAG__, "%f") & passed;

                passed = context.are_equal(value.object()["a1"].object()["a11"].array()[1].type(), abc::net::json::value_type::boolean, __TAG__, "%u") & passed;
                passed = context.are_equal(value.object()["a1"].object()["a11"].array()[1].boolean(), true, __TAG__, "%u") & passed;
            }

            passed = context.are_equal(value.object()["a1"].object()["a12"].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
            passed = context.are_equal(value.object()["a1"].object()["a12"].array().size(), (std::size_t)2, __TAG__, "%zu") & passed;
            {
                passed = context.are_equal(value.object()["a1"].object()["a12"].array()[0].type(), abc::net::json::value_type::string, __TAG__, "%u") & passed;
                passed = context.are_equal(value.object()["a1"].object()["a12"].array()[0].string().c_str(), "abc", std::strlen("abc"), __TAG__) & passed;

                passed = context.are_equal(value.object()["a1"].object()["a12"].array()[1].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
                passed = context.are_equal(value.object()["a1"].object()["a12"].array()[1].number(), 2.0, __TAG__, "%f") & passed;
            }
        }

        passed = context.are_equal(value.object()["a2"].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
        passed = context.are_equal(value.object()["a2"].array().size(), (std::size_t)1, __TAG__, "%zu") & passed;
        {
            passed = context.are_equal(value.object()["a2"].array()[0].type(), abc::net::json::value_type::object, __TAG__, "%u") & passed;
            passed = context.are_equal(value.object()["a2"].array()[0].object().size(), (std::size_t)2, __TAG__, "%zu") & passed;
            {
                passed = context.are_equal(value.object()["a2"].array()[0].object()["a211"].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
                passed = context.are_equal(value.object()["a2"].array()[0].object()["a211"].array().size(), (std::size_t)3, __TAG__, "%zu") & passed;
                {
                    passed = context.are_equal(value.object()["a2"].array()[0].object()["a211"].array()[0].type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
                    passed = context.are_equal(value.object()["a2"].array()[0].object()["a211"].array()[0].number(), 4.0, __TAG__, "%f") & passed;

                    passed = context.are_equal(value.object()["a2"].array()[0].object()["a211"].array()[1].type(), abc::net::json::value_type::string, __TAG__, "%u") & passed;
                    passed = context.are_equal(value.object()["a2"].array()[0].object()["a211"].array()[1].string().c_str(), "def", std::strlen("def"), __TAG__) & passed;

                    passed = context.are_equal(value.object()["a2"].array()[0].object()["a211"].array()[2].type(), abc::net::json::value_type::boolean, __TAG__, "%u") & passed;
                    passed = context.are_equal(value.object()["a2"].array()[0].object()["a211"].array()[2].boolean(), false, __TAG__, "%u") & passed;
                }

                passed = context.are_equal(value.object()["a2"].array()[0].object()["a212"].type(), abc::net::json::value_type::array, __TAG__, "%u") & passed;
                passed = context.are_equal(value.object()["a2"].array()[0].object()["a212"].array().size(), (std::size_t)1, __TAG__, "%zu") & passed;
                {
                    passed = context.are_equal(value.object()["a2"].array()[0].object()["a212"].array()[0].type(), abc::net::json::value_type::null, __TAG__, "%u") & passed;
                }
            }
        }
    }

    return passed;
}


// --------------------------------------------------------------


bool test_json_ostream_null(test_context& context) {
    char expected[] =
        " \r\t\n null \t\r\n";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_space();
    passed = verify_stream_good(context, ostream, 0x101fc) && passed;
    ostream.put_cr();
    passed = verify_stream_good(context, ostream, 0x101fd) && passed;
    ostream.put_tab();
    passed = verify_stream_good(context, ostream, 0x101fe) && passed;
    ostream.put_lf();
    passed = verify_stream_good(context, ostream, 0x101ff) && passed;
    ostream.put_space();
    passed = verify_stream_good(context, ostream, 0x10200) && passed;

    ostream.put_null();
    passed = verify_stream_good(context, ostream, 0x10201) && passed;

    ostream.put_space();
    passed = verify_stream_good(context, ostream, 0x10202) && passed;
    ostream.put_tab();
    passed = verify_stream_good(context, ostream, 0x10203) && passed;
    ostream.put_cr();
    passed = verify_stream_good(context, ostream, 0x10204) && passed;
    ostream.put_lf();
    passed = verify_stream_good(context, ostream, 0x10205) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10206) && passed;

    return passed;
}


bool test_json_ostream_boolean_01(test_context& context) {
    char expected[] =
        "\n\nfalse\r\n";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_lf();
    passed = verify_stream_good(context, ostream, 0x10207) && passed;
    ostream.put_lf();
    passed = verify_stream_good(context, ostream, 0x10208) && passed;

    ostream.put_boolean(false);
    passed = verify_stream_good(context, ostream, 0x10209) && passed;

    ostream.put_cr();
    passed = verify_stream_good(context, ostream, 0x1020a) && passed;
    ostream.put_lf();
    passed = verify_stream_good(context, ostream, 0x1020b) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x1020c) && passed;

    return passed;
}


bool test_json_ostream_boolean_02(test_context& context) {
    char expected[] =
        "true";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_boolean(true);
    passed = verify_stream_good(context, ostream, 0x1020d) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x1020e) && passed;

    return passed;
}


bool test_json_ostream_number_01(test_context& context) {
    char expected[] =
        "42";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_number(42);
    passed = verify_stream_good(context, ostream, 0x1020f) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10210) && passed;

    return passed;
}


bool test_json_ostream_number_02(test_context& context) {
    char expected[] =
        "12345.6789012345";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_number(12345.6789012345);
    passed = verify_stream_good(context, ostream, 0x10211) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10212) && passed;

    return passed;
}


bool test_json_ostream_number_03(test_context& context) {
    char expected[] =
        "-8.87766554433221e-10";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_number(-8.87766554433221e-10);
    passed = verify_stream_good(context, ostream, 0x10213) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10214) && passed;

    return passed;
}


bool test_json_ostream_string_01(test_context& context) {
    char expected[] =
        "\"\"";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_string("");
    passed = verify_stream_good(context, ostream, 0x10215) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10216) && passed;

    return passed;
}


bool test_json_ostream_string_02(test_context& context) {
    char expected[] =
        "\"qwerty\"";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_string("qwerty");
    passed = verify_stream_good(context, ostream, 0x10217) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10218) && passed;

    return passed;
}


bool test_json_ostream_array_01(test_context& context) {
    char expected[] =
        "[]";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_begin_array();
    passed = verify_stream_good(context, ostream, 0x10219) && passed;

    ostream.put_end_array();
    passed = verify_stream_good(context, ostream, 0x1021a) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x1021b) && passed;

    return passed;
}


bool test_json_ostream_array_02(test_context& context) {
    char expected[] =
        "[ 12.34,null,true,\"abc\" ]";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_begin_array();
    passed = verify_stream_good(context, ostream, 0x1021c) && passed;
    {
        ostream.put_space();

        ostream.put_number(12.34);
        passed = verify_stream_good(context, ostream, 0x1021d) && passed;

        ostream.put_null();
        passed = verify_stream_good(context, ostream, 0x1021e) && passed;

        ostream.put_boolean(true);
        passed = verify_stream_good(context, ostream, 0x1021f) && passed;

        ostream.put_string("abc");
        passed = verify_stream_good(context, ostream, 0x10220) && passed;

        ostream.put_space();
    }
    ostream.put_end_array();
    passed = verify_stream_good(context, ostream, 0x10221) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10222) && passed;

    return passed;
}


bool test_json_ostream_array_03(test_context& context) {
    char expected[] =
        "[ 1,2,[[3],[4]],[[[5]]] ]";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_begin_array();
    passed = verify_stream_good(context, ostream, 0x10223) && passed;
    {
        ostream.put_space();

        ostream.put_number(1);
        passed = verify_stream_good(context, ostream, 0x10224) && passed;

        ostream.put_number(2);
        passed = verify_stream_good(context, ostream, 0x10225) && passed;

        ostream.put_begin_array();
        passed = verify_stream_good(context, ostream, 0x10226) && passed;
        {
            ostream.put_begin_array();
            passed = verify_stream_good(context, ostream, 0x10227) && passed;
            {
                ostream.put_number(3);
                passed = verify_stream_good(context, ostream, 0x10228) && passed;
            }
            ostream.put_end_array();
            passed = verify_stream_good(context, ostream, 0x10229) && passed;

            ostream.put_begin_array();
            passed = verify_stream_good(context, ostream, 0x1022a) && passed;
            {
                ostream.put_number(4);
                passed = verify_stream_good(context, ostream, 0x1022b) && passed;
            }
            ostream.put_end_array();
            passed = verify_stream_good(context, ostream, 0x1022c) && passed;
        }
        ostream.put_end_array();
        passed = verify_stream_good(context, ostream, 0x1022d) && passed;

        ostream.put_begin_array();
        passed = verify_stream_good(context, ostream, 0x1022e) && passed;
        {
            ostream.put_begin_array();
            passed = verify_stream_good(context, ostream, 0x1022f) && passed;
            {
                ostream.put_begin_array();
                passed = verify_stream_good(context, ostream, 0x10230) && passed;
                {
                    ostream.put_number(5);
                    passed = verify_stream_good(context, ostream, 0x10231) && passed;
                }
                ostream.put_end_array();
                passed = verify_stream_good(context, ostream, 0x10232) && passed;
            }
            ostream.put_end_array();
            passed = verify_stream_good(context, ostream, 0x10233) && passed;
        }
        ostream.put_end_array();
        passed = verify_stream_good(context, ostream, 0x10234) && passed;

        ostream.put_space();
    }
    ostream.put_end_array();
    passed = verify_stream_good(context, ostream, 0x10235) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10236) && passed;

    return passed;
}


bool test_json_ostream_object_01(test_context& context) {
    char expected[] =
        "{}";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_begin_object();
    passed = verify_stream_good(context, ostream, 0x10237) && passed;

    ostream.put_end_object();
    passed = verify_stream_good(context, ostream, 0x10238) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10239) && passed;

    return passed;
}


bool test_json_ostream_object_02(test_context& context) {
    char expected[] =
        "{"
            "\"a\":12.34,"
            "\"bb\":null,"
            "\"ccc\":true,"
            "\"dddd\":\"abc\""
        "}";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_begin_object();
    passed = verify_stream_good(context, ostream, 0x1023a) && passed;
    {
        ostream.put_property("a");
        passed = verify_stream_good(context, ostream, 0x1023b) && passed;

        ostream.put_number(12.34);
        passed = verify_stream_good(context, ostream, 0x1023c) && passed;

        ostream.put_property("bb");
        passed = verify_stream_good(context, ostream, 0x1023d) && passed;

        ostream.put_null();
        passed = verify_stream_good(context, ostream, 0x1023e) && passed;

        ostream.put_property("ccc");
        passed = verify_stream_good(context, ostream, 0x1023f) && passed;

        ostream.put_boolean(true);
        passed = verify_stream_good(context, ostream, 0x10240) && passed;

        ostream.put_property("dddd");
        passed = verify_stream_good(context, ostream, 0x10241) && passed;

        ostream.put_string("abc");
        passed = verify_stream_good(context, ostream, 0x10242) && passed;
    }
    ostream.put_end_object();
    passed = verify_stream_good(context, ostream, 0x10243) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10244) && passed;

    return passed;
}


bool test_json_ostream_object_03(test_context& context) {
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

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_begin_object();
    passed = verify_stream_good(context, ostream, 0x10245) && passed;
    {
        ostream.put_property("a1");
        passed = verify_stream_good(context, ostream, 0x10246) && passed;

        ostream.put_number(1);
        passed = verify_stream_good(context, ostream, 0x10247) && passed;

        ostream.put_property("a2");
        passed = verify_stream_good(context, ostream, 0x10248) && passed;

        ostream.put_number(2);
        passed = verify_stream_good(context, ostream, 0x10249) && passed;

        ostream.put_property("a3");
        passed = verify_stream_good(context, ostream, 0x1024a) && passed;

        ostream.put_begin_object();
        passed = verify_stream_good(context, ostream, 0x1024b) && passed;
        {
            ostream.put_property("a31");
            passed = verify_stream_good(context, ostream, 0x1024c) && passed;

            ostream.put_begin_object();
            passed = verify_stream_good(context, ostream, 0x1024d) && passed;
            {
                ostream.put_property("a313");
                passed = verify_stream_good(context, ostream, 0x1024e) && passed;

                ostream.put_number(3);
                passed = verify_stream_good(context, ostream, 0x1024f) && passed;
            }
            ostream.put_end_object();
            passed = verify_stream_good(context, ostream, 0x10250) && passed;

            ostream.put_property("a32");
            passed = verify_stream_good(context, ostream, 0x10251) && passed;

            ostream.put_begin_object();
            passed = verify_stream_good(context, ostream, 0x10252) && passed;
            {
                ostream.put_property("a324");
                passed = verify_stream_good(context, ostream, 0x10253) && passed;

                ostream.put_number(4);
                passed = verify_stream_good(context, ostream, 0x10254) && passed;
            }
            ostream.put_end_object();
            passed = verify_stream_good(context, ostream, 0x10255) && passed;
        }
        ostream.put_end_object();
        passed = verify_stream_good(context, ostream, 0x10256) && passed;

        ostream.put_property("a5");
        passed = verify_stream_good(context, ostream, 0x10257) && passed;

        ostream.put_begin_object();
        passed = verify_stream_good(context, ostream, 0x10258) && passed;
        {
            ostream.put_property("a51");
            passed = verify_stream_good(context, ostream, 0x10259) && passed;

            ostream.put_begin_object();
            passed = verify_stream_good(context, ostream, 0x1025a) && passed;
            {
                ostream.put_property("a512");
                passed = verify_stream_good(context, ostream, 0x1025b) && passed;

                ostream.put_begin_object();
                passed = verify_stream_good(context, ostream, 0x1025c) && passed;
                {
                    ostream.put_property("a5123");
                    passed = verify_stream_good(context, ostream, 0x1025d) && passed;

                    ostream.put_number(5);
                    passed = verify_stream_good(context, ostream, 0x1025e) && passed;
                }
                ostream.put_end_object();
                passed = verify_stream_good(context, ostream, 0x1025f) && passed;
            }
            ostream.put_end_object();
            passed = verify_stream_good(context, ostream, 0x10260) && passed;
        }
        ostream.put_end_object();
        passed = verify_stream_good(context, ostream, 0x10261) && passed;
    }
    ostream.put_end_object();
    passed = verify_stream_good(context, ostream, 0x10262) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10263) && passed;

    return passed;
}


bool test_json_ostream_mixed_01(test_context& context) {
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

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_begin_array();
    passed = verify_stream_good(context, ostream, 0x10264) && passed;
    {
        ostream.put_begin_object();
        passed = verify_stream_good(context, ostream, 0x10265) && passed;
        {
            ostream.put_property("a11");
            passed = verify_stream_good(context, ostream, 0x10266) && passed;

            ostream.put_begin_array();
            passed = verify_stream_good(context, ostream, 0x10267) && passed;
            {
                ostream.put_number(1);
                passed = verify_stream_good(context, ostream, 0x10268) && passed;

                ostream.put_boolean(true);
                passed = verify_stream_good(context, ostream, 0x10269) && passed;
            }
            ostream.put_end_array();
            passed = verify_stream_good(context, ostream, 0x1026a) && passed;

            ostream.put_property("a12");
            passed = verify_stream_good(context, ostream, 0x1026b) && passed;

            ostream.put_begin_array();
            passed = verify_stream_good(context, ostream, 0x1026c) && passed;
            {
                ostream.put_string("abc");
                passed = verify_stream_good(context, ostream, 0x1026d) && passed;

                ostream.put_number(2);
                passed = verify_stream_good(context, ostream, 0x1026e) && passed;
            }
            ostream.put_end_array();
            passed = verify_stream_good(context, ostream, 0x1026f) && passed;
        }
        ostream.put_end_object();
        passed = verify_stream_good(context, ostream, 0x10270) && passed;

        ostream.put_begin_array();
        passed = verify_stream_good(context, ostream, 0x10271) && passed;
        {
            ostream.put_begin_object();
            passed = verify_stream_good(context, ostream, 0x10272) && passed;
            {
                ostream.put_property("a211");
                passed = verify_stream_good(context, ostream, 0x10273) && passed;

                ostream.put_begin_array();
                passed = verify_stream_good(context, ostream, 0x10274) && passed;
                {
                    ostream.put_number(4);
                    passed = verify_stream_good(context, ostream, 0x10275) && passed;

                    ostream.put_string("def");
                    passed = verify_stream_good(context, ostream, 0x10276) && passed;

                    ostream.put_boolean(false);
                    passed = verify_stream_good(context, ostream, 0x10277) && passed;
                }
                ostream.put_end_array();
                passed = verify_stream_good(context, ostream, 0x10278) && passed;

                ostream.put_property("a212");
                passed = verify_stream_good(context, ostream, 0x10279) && passed;

                ostream.put_begin_array();
                passed = verify_stream_good(context, ostream, 0x1027a) && passed;
                {
                    ostream.put_null();
                    passed = verify_stream_good(context, ostream, 0x1027b) && passed;
                }
                ostream.put_end_array();
                passed = verify_stream_good(context, ostream, 0x1027c) && passed;
            }
            ostream.put_end_object();
            passed = verify_stream_good(context, ostream, 0x1027d) && passed;
        }
        ostream.put_end_array();
        passed = verify_stream_good(context, ostream, 0x1027e) && passed;
    }
    ostream.put_end_array();
    passed = verify_stream_good(context, ostream, 0x1027f) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10280) && passed;

    return passed;
}


bool test_json_ostream_mixed_02(test_context& context) {
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

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream(&sb, context.log());

    bool passed = true;

    ostream.put_begin_object();
    passed = verify_stream_good(context, ostream, 0x10281) && passed;
    {
        ostream.put_property("a1");
        passed = verify_stream_good(context, ostream, 0x10282) && passed;

        ostream.put_begin_object();
        passed = verify_stream_good(context, ostream, 0x10283) && passed;
        {
            ostream.put_property("a11");
            passed = verify_stream_good(context, ostream, 0x10284) && passed;

            ostream.put_begin_array();
            passed = verify_stream_good(context, ostream, 0x10285) && passed;
            {
                ostream.put_number(1);
                passed = verify_stream_good(context, ostream, 0x10286) && passed;

                ostream.put_boolean(true);
                passed = verify_stream_good(context, ostream, 0x10287) && passed;
            }
            ostream.put_end_array();
            passed = verify_stream_good(context, ostream, 0x10288) && passed;

            ostream.put_property("a12");
            passed = verify_stream_good(context, ostream, 0x10289) && passed;

            ostream.put_begin_array();
            passed = verify_stream_good(context, ostream, 0x1028a) && passed;
            {
                ostream.put_string("abc");
                passed = verify_stream_good(context, ostream, 0x1028b) && passed;

                ostream.put_number(2);
                passed = verify_stream_good(context, ostream, 0x1028c) && passed;
            }
            ostream.put_end_array();
            passed = verify_stream_good(context, ostream, 0x1028d) && passed;
        }
        ostream.put_end_object();
        passed = verify_stream_good(context, ostream, 0x1028e) && passed;

        ostream.put_property("a2");
        passed = verify_stream_good(context, ostream, 0x1028f) && passed;

        ostream.put_begin_array();
        passed = verify_stream_good(context, ostream, 0x10290) && passed;
        {
            ostream.put_begin_object();
            passed = verify_stream_good(context, ostream, 0x10291) && passed;
            {
                ostream.put_property("a211");
                passed = verify_stream_good(context, ostream, 0x10292) && passed;

                ostream.put_begin_array();
                passed = verify_stream_good(context, ostream, 0x10293) && passed;
                {
                    ostream.put_number(4);
                    passed = verify_stream_good(context, ostream, 0x10294) && passed;

                    ostream.put_string("def");
                    passed = verify_stream_good(context, ostream, 0x10295) && passed;

                    ostream.put_boolean(false);
                    passed = verify_stream_good(context, ostream, 0x10296) && passed;
                }
                ostream.put_end_array();
                passed = verify_stream_good(context, ostream, 0x10297) && passed;

                ostream.put_property("a212");
                passed = verify_stream_good(context, ostream, 0x10298) && passed;

                ostream.put_begin_array();
                passed = verify_stream_good(context, ostream, 0x10299) && passed;
                {
                    ostream.put_null();
                    passed = verify_stream_good(context, ostream, 0x1029a) && passed;
                }
                ostream.put_end_array();
                passed = verify_stream_good(context, ostream, 0x1029b) && passed;
            }
            ostream.put_end_object();
            passed = verify_stream_good(context, ostream, 0x1029c) && passed;
        }
        ostream.put_end_array();
        passed = verify_stream_good(context, ostream, 0x1029d) && passed;
    }
    ostream.put_end_object();
    passed = verify_stream_good(context, ostream, 0x1029e) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x1029f) && passed;

    return passed;
}


// --------------------------------------------------------------


bool test_json_writer(test_context& context, const abc::net::json::value<test_log*>& value, const char* expected);


bool test_json_writer_null(test_context& context) {
    abc::net::json::value<test_log*> value(nullptr);

    char expected[] =
        "null";

    return test_json_writer(context, value, expected);
}


bool test_json_writer_boolean_01(test_context& context) {
    abc::net::json::value<test_log*> value(false);

    char expected[] =
        "false";

    return test_json_writer(context, value, expected);
}


bool test_json_writer_boolean_02(test_context& context) {
    abc::net::json::value<test_log*> value(true);

    char expected[] =
        "true";

    return test_json_writer(context, value, expected);
}


bool test_json_writer_number_01(test_context& context) {
    abc::net::json::value<test_log*> value(42.0);

    char expected[] =
        "42";

    return test_json_writer(context, value, expected);
}


bool test_json_writer_number_02(test_context& context) {
    abc::net::json::value<test_log*> value(12345.6789012345);

    char expected[] =
        "12345.6789012345";

    return test_json_writer(context, value, expected);
}


bool test_json_writer_number_03(test_context& context) {
    abc::net::json::value<test_log*> value(-8.87766554433221e-10);

    char expected[] =
        "-8.87766554433221e-10";

    return test_json_writer(context, value, expected);
}


bool test_json_writer_string_01(test_context& context) {
    abc::net::json::value<test_log*> value("");

    char expected[] =
        "\"\"";

    return test_json_writer(context, value, expected);
}


bool test_json_writer_string_02(test_context& context) {
    abc::net::json::value<test_log*> value("qwerty");

    char expected[] =
        "\"qwerty\"";

    return test_json_writer(context, value, expected);
}


bool test_json_writer_array_01(test_context& context) {
    abc::net::json::literal::array<test_log*> array;
    abc::net::json::value<test_log*> value(std::move(array));

    char expected[] =
        "[]";

    return test_json_writer(context, value, expected);
}


bool test_json_writer_array_02(test_context& context) {
    abc::net::json::literal::array<test_log*> array
    {
        12.34,
        nullptr,
        true,
        "abc",
    };
    abc::net::json::value<test_log*> value(std::move(array));

    char expected[] =
        "["
            "12.34,"
            "null,"
            "true,"
            "\"abc\""
        "]";

    return test_json_writer(context, value, expected);
}


bool test_json_writer_array_03(test_context& context) {
    abc::net::json::literal::array<test_log*> array
    {
        1.0,
        2.0,
        abc::net::json::literal::array<test_log*>
        {
            abc::net::json::literal::array<test_log*>
            {
                3.0,
            },
            abc::net::json::literal::array<test_log*>
            {
                4.0,
            },
        },
        abc::net::json::value<test_log*>(abc::net::json::literal::array<test_log*> // Clang optimizes these as a single array.
        {
            abc::net::json::value<test_log*>(abc::net::json::literal::array<test_log*>
            {
                abc::net::json::value<test_log*>(abc::net::json::literal::array<test_log*>
                {
                    5.0,
                }),
            }),
        }),
    };
    abc::net::json::value<test_log*> value(std::move(array));

    char expected[] =
        "["
            "1,"
            "2,"
            "["
                "["
                    "3"
                "],"
                "["
                    "4"
                "]"
            "],"
            "["
                "["
                    "["
                        "5"
                    "]"
                "]"
            "]"
        "]";

    return test_json_writer(context, value, expected);
}


bool test_json_writer_object_01(test_context& context) {
    abc::net::json::literal::object<test_log*> object;
    abc::net::json::value<test_log*> value(std::move(object));

    char expected[] =
        "{}";

    return test_json_writer(context, value, expected);
}


bool test_json_writer_object_02(test_context& context) {
    abc::net::json::literal::object<test_log*> object
    {
        { "a", 12.34 },
        { "bb", nullptr },
        { "ccc", true },
        { "dddd", "abc" },
    };
    abc::net::json::value<test_log*> value(std::move(object));

    char expected[] =
        "{"
            "\"a\":12.34,"
            "\"bb\":null,"
            "\"ccc\":true,"
            "\"dddd\":\"abc\""
        "}";

    return test_json_writer(context, value, expected);
}


bool test_json_writer_object_03(test_context& context) {
    abc::net::json::literal::object<test_log*> object
    {
        { "a1", 1.0 },
        { "a2", 2.0 },
        { "a3", abc::net::json::literal::object<test_log*> {
            { "a31", abc::net::json::literal::object<test_log*> {
                { "a313", 3.0 },
            } },
            { "a32", abc::net::json::literal::object<test_log*> {
                { "a324", 4.0 },
            } },
        } },
        { "a5", abc::net::json::literal::object<test_log*> {
            { "a51", abc::net::json::literal::object<test_log*> {
                { "a512", abc::net::json::literal::object<test_log*> {
                    { "a5123", 5.0 },
                } },
            } },
        } },
    };
    abc::net::json::value<test_log*> value(std::move(object));

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

    return test_json_writer(context, value, expected);
}


bool test_json_writer_mixed_01(test_context& context) {
    abc::net::json::literal::array<test_log*> array
    {
        abc::net::json::literal::object<test_log*> {
            { "a11", abc::net::json::literal::array<test_log*> {
                1.0,
                true,
            } },
            { "a12", abc::net::json::literal::array<test_log*> {
                "abc",
                2.0
            } },
        },
        abc::net::json::literal::array<test_log*> {
            abc::net::json::literal::object<test_log*> {
                { "a211", abc::net::json::literal::array<test_log*> {
                    4.0,
                    "def",
                    false,
                } },
                { "a212", abc::net::json::literal::array<test_log*> {
                    nullptr,
                } },
            } },
    };
    abc::net::json::value<test_log*> value(std::move(array));

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

    return test_json_writer(context, value, expected);
}


bool test_json_writer_mixed_02(test_context& context) {
    abc::net::json::literal::object<test_log*> object
    {
        { "a1", abc::net::json::literal::object<test_log*> {
            { "a11", abc::net::json::literal::array<test_log*> {
                1.0,
                true,
            } },
            { "a12", abc::net::json::literal::array<test_log*> {
                "abc",
                2.0,
            } },
        } },
        { "a2", abc::net::json::literal::array<test_log*> {
            abc::net::json::literal::object<test_log*> {
                { "a211", abc::net::json::literal::array<test_log*> {
                    4.0,
                    "def",
                    false,
                } },
                { "a212", abc::net::json::literal::array<test_log*> {
                    nullptr,
                } },
            },
        } },
    };
    abc::net::json::value<test_log*> value(std::move(object));

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

    return test_json_writer(context, value, expected);
}


bool test_json_writer(test_context& context, const abc::net::json::value<test_log*>& value, const char* expected) {
    std::stringbuf sb(std::ios_base::out);

    abc::net::json::writer<test_log*> writer(&sb, context.log());

    bool passed = true;

    writer.put_value(value);

    passed = context.are_equal(sb.str().c_str(), expected, __TAG__) && passed;

    return passed;
}


// --------------------------------------------------------------


bool test_json_istream_move(test_context& context) {
    std::string content =
        "false 42 ";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::istream<test_log*> istream1(&sb, context.log());

    bool passed = true;

    abc::net::json::token token = istream1.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::boolean, istream1, 0x10721, "%x", token.string.length()) && passed;
    passed = verify_integral(context, token.boolean, false, istream1, 0x10722, "%u", token.string.length()) && passed;

    abc::net::json::istream<test_log*> istream2(std::move(istream1));

    token = istream2.get_token();
    passed = verify_integral(context, token.type, abc::net::json::token_type::number, istream2, 0x10723, "%x", token.string.length()) && passed;
    passed = verify_integral(context, token.number, 42.0, istream2, 0x10724, "%f", token.string.length()) && passed;

    return passed;
}


bool test_json_reader_move(test_context& context) {
    std::string content =
        "false 42 ";

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::json::reader<test_log*> reader1(&sb, context.log());

    bool passed = true;

    abc::net::json::value<test_log*> value = reader1.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::boolean, __TAG__, "%u") & passed;
    passed = context.are_equal(value.boolean(), false, __TAG__, "%u") & passed;

    abc::net::json::reader<test_log*> reader2(std::move(reader1));

    value = reader2.get_value();
    passed = context.are_equal(value.type(), abc::net::json::value_type::number, __TAG__, "%u") & passed;
    passed = context.are_equal(value.number(), 42.0, __TAG__, "%f") & passed;

    return passed;
}


bool test_json_ostream_move(test_context& context) {
    char expected[] =
        "true 42";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::ostream<test_log*> ostream1(&sb, context.log());

    bool passed = true;

    ostream1.put_boolean(true);
    passed = verify_stream_good(context, ostream1, 0x10725) && passed;

    abc::net::json::ostream<test_log*> ostream2(std::move(ostream1));
    ostream2.put_space();

    ostream2.put_number(42.0);
    passed = verify_stream_good(context, ostream2, 0x10726) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10727) && passed;

    return passed;
}


bool test_json_writer_move(test_context& context) {
    char expected[] =
        "true 42";

    std::stringbuf sb(std::ios_base::out);

    abc::net::json::writer<test_log*> writer1(&sb, context.log());

    bool passed = true;

    writer1.put_value(true);

    abc::net::json::writer<test_log*> writer2(std::move(writer1));
    
    abc::net::json::ostream<test_log*> ostream(&sb, context.log());
    ostream.put_space();

    writer2.put_value(42.0);

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), __TAG__) && passed;

    return passed;
}


// --------------------------------------------------------------


template <typename JsonStream>
static bool verify_string(test_context& context, const char* actual, const char* expected, const JsonStream& stream, abc::diag::tag_t tag) {
    bool passed = true;

    passed = context.are_equal(actual, expected, tag) && passed;
    passed = verify_stream_good(context, stream, std::strlen(expected), tag) && passed;

    return passed;
}


template <typename JsonStream, typename Value>
static bool verify_integral(test_context& context, const Value& actual, const Value& expected, const JsonStream& stream, abc::diag::tag_t tag, const char* format, std::size_t expected_gcount) {
    bool passed = true;

    passed = context.are_equal(actual, expected, tag, format) && passed;
    passed = verify_stream_good(context, stream, expected_gcount, tag) && passed;

    return passed;
}
