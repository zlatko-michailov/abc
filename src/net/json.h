/*
MIT License

Copyright (c) 2018-2026 Zlatko Michailov 

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

#include <cstdlib>
#include <cstdio>
#include <regex>

#include "../root/size.h"
#include "../root/ascii.h"
#include "../root/util.h"
#include "../stream/stream.h"
#include "../diag/diag_ready.h"
#include "i/json.i.h"


namespace abc { namespace net { namespace json {

    inline value::value(diag::log_ostream* log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::empty) {
    }


    inline value::value(literal::null, diag::log_ostream* log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::null) {
    }


    inline value::value(literal::boolean b, diag::log_ostream* log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::boolean)
        , _boolean(b) {
    }


    inline value::value(literal::number n, diag::log_ostream* log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::number)
        , _number(n) {
    }


    inline value::value(int n, diag::log_ostream* log) noexcept
        : value(static_cast<literal::number>(n), log) {
    }


    inline value::value(unsigned n, diag::log_ostream* log) noexcept
        : value(static_cast<literal::number>(n), log) {
    }


    inline value::value(long n, diag::log_ostream* log) noexcept
        : value(static_cast<literal::number>(n), log) {
    }


    inline value::value(unsigned long n, diag::log_ostream* log) noexcept
        : value(static_cast<literal::number>(n), log) {
    }


    inline value::value(long long n, diag::log_ostream* log) noexcept
        : value(static_cast<literal::number>(n), log) {
    }


    inline value::value(unsigned long long n, diag::log_ostream* log) noexcept
        : value(static_cast<literal::number>(n), log) {
    }


    inline value::value(const char* str, diag::log_ostream* log)
        : value(std::string(str), log) {
    }


    inline value::value(literal::string&& str, diag::log_ostream* log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::string)
        , _string(std::move(str)) {
    }


    inline value::value(literal::array&& arr, diag::log_ostream* log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::array)
        , _array(std::move(arr)) {
    }


    inline value::value(literal::object&& obj, diag::log_ostream* log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::object)
        , _object(std::move(obj)) {
    }


    inline value::value(const value& other)
        : diag_base(copy(_origin), other.log()) {

        copy_from(other);
    }


    inline value::value(value&& other) noexcept 
        : diag_base(copy(_origin), other.log()) {

        move_from(std::move(other));
    }


    inline value::~value() noexcept {
        clear();
    }


    inline void value::clear() noexcept {
        switch (_type) {
            case value_type::empty:
            case value_type::null:
            case value_type::boolean:
            case value_type::number:
                break;

            case value_type::string:
                using literal_string = literal::string;
                _string.~literal_string();
                break;

            case value_type::array:
                using literal_array = literal::array;
                _array.~literal_array();
                break;

            case value_type::object:
                using literal_object = literal::object;
                _object.~literal_object();
                break;
        }

        _type = value_type::empty;
    }


    inline value& value::operator = (const value& other) {
        clear();
        copy_from(other);

        return *this;
    }


    inline value& value::operator = (value&& other) noexcept {
        clear();
        move_from(std::move(other));

        return *this;
    }


    inline value_type value::type() const noexcept {
        return _type;
    }


    inline literal::boolean value::boolean() const {
        diag_base::assert("boolean()", _type == value_type::boolean, 0x10934, "_type=%u", _type);

        return _boolean;
    }

    
    inline literal::number value::number() const {
        diag_base::assert("number()", _type == value_type::number, 0x10935, "_type=%u", _type);

        return _number;
    }


    inline const literal::string& value::string() const {
        diag_base::assert("string()", _type == value_type::string, 0x10936, "_type=%u", _type);

        return _string;
    }


    inline literal::string& value::string() {
        diag_base::assert("string()", _type == value_type::string, 0x10937, "_type=%u", _type);

        return _string;
    }


    inline const literal::array& value::array() const {
        diag_base::assert("array()", _type == value_type::array, 0x10938, "_type=%u", _type);

        return _array;
    }


    inline literal::array& value::array() {
        diag_base::assert("array()", _type == value_type::array, 0x10939, "_type=%u", _type);

        return _array;
    }


    inline const literal::object& value::object() const {
        diag_base::assert("object()", _type == value_type::object, 0x1093a, "_type=%u", _type);

        return _object;
    }


    inline literal::object& value::object() {
        diag_base::assert("object()", _type == value_type::object, 0x1093b, "_type=%u", _type);

        return _object;
    }


    inline bool value::operator ==(const value& other) const noexcept {
        if (_type != other._type) {
            return false;
        }

        switch (_type) {
            case value_type::empty:
            case value_type::null:
                return true;

            case value_type::boolean:
                return _boolean == other._boolean;

            case value_type::number:
                return _number == other._number;

            case value_type::string:
                return _string == other._string;

            case value_type::array:
                return _array == other._array;

            case value_type::object:
                return _object == other._object;
        }

        // Must remain unreached.
        diag_base::ensure("operator ==", false, 0x1093c, "_type=%u", _type);
        return false;
    }


    inline bool value::operator !=(const value& other) const noexcept {
        return !(*this == other);
    }


    inline void value::copy_from(const value& other) {
        _type = other._type;

        switch (_type) {
            case value_type::empty:
            case value_type::null:
                break;

            case value_type::boolean:
                _boolean = other._boolean;
                break;

            case value_type::number:
                _number = other._number;
                break;

            case value_type::string:
                new (&_string) literal::string(other._string);
                break;

            case value_type::array:
                new (&_array) literal::array(other._array);
                break;

            case value_type::object:
                new (&_object) literal::object(other._object);
                break;
        }
    }


    inline void value::move_from(value&& other) noexcept {
        _type = other._type;

        switch (_type) {
            case value_type::empty:
            case value_type::null:
                break;

            case value_type::boolean:
                _boolean = other._boolean;
                break;

            case value_type::number:
                _number = other._number;
                break;

            case value_type::string:
                new (&_string) literal::string(std::move(other._string));
                break;

            case value_type::array:
                new (&_array) literal::array(std::move(other._array));
                break;

            case value_type::object:
                new (&_object) literal::object(std::move(other._object));
                break;
        }

        other.clear();
    }


    // --------------------------------------------------------------


    inline state::state(const char* origin, diag::log_ostream* log)
        : diag_base(copy(origin), log)
        , _expect_property(false) {

        constexpr const char* suborigin = "state()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1093d, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1093e, "End:");
    }


    inline void state::reset() noexcept {
        constexpr const char* suborigin = "reset()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1093f, "Begin:");

        _expect_property = false;
        while (!_nest_stack.empty()) {
            _nest_stack.pop();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10940, "End:");
    }


    inline const std::stack<nest_type>& state::nest_stack() const noexcept {
        return _nest_stack;
    }


    inline std::stack<nest_type>& state::nest_stack() noexcept {
        return _nest_stack;
    }


    inline bool state::expect_property() const noexcept {
        return _expect_property;
    }


    inline void state::set_expect_property(bool expect) {
        constexpr const char* suborigin = "set_expect_property()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10941, "Begin: expect=%u", expect);

        diag_base::expect(suborigin, !expect || (!_nest_stack.empty() && _nest_stack.top() == nest_type::object), 0x10942, "expect");

        _expect_property = expect;

        diag_base::ensure(suborigin, !_expect_property || (!_nest_stack.empty() && _nest_stack.top() == nest_type::object), 0x10943, "_expect_property");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10944, "End:");
    }


    inline void state::nest(nest_type type) {
        constexpr const char* suborigin = "nest()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10945, "Begin: type=%u", type);

        _nest_stack.push(type);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10946, "End:");
    }


    inline void state::unnest(nest_type type) {
        constexpr const char* suborigin = "unnest()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10947, "Begin: type=%u", type);

        diag_base::expect(suborigin, !_nest_stack.empty() && _nest_stack.top() == type, 0x10948, "type");

        _nest_stack.pop();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10949, "End:");
    }


    // --------------------------------------------------------------


    inline istream::istream(const char* origin, std::streambuf* sb, diag::log_ostream* log)
        : base(sb)
        , state_base(origin, log) {

        constexpr const char* suborigin = "istream()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1094a, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1094b, "End:");
    }


    inline istream::istream(std::streambuf* sb, diag::log_ostream* log)
        : istream("abc::net::json::istream", sb, log) {
    }


    inline istream::istream(istream&& other) noexcept
        : base(std::move(other))
        , state_base(std::move(other)) {
    }


    inline void istream::skip_value() {
        constexpr const char* suborigin = "skip_value()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1094c, "Begin:");

        std::size_t nest_stack_size = state_base::nest_stack().size();
        do {
            get_token();
        }
        while (state_base::nest_stack().size() > nest_stack_size);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1094d, "End:");
    }


    inline token istream::get_token() {
        constexpr const char* suborigin = "get_token()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1094e, "Begin:");

        token tok;
        bool trail_comma = true;

        skip_spaces();
        char ch = peek_char();

        if (!state_base::nest_stack().empty() && state_base::nest_stack().top() == nest_type::object && state_base::expect_property()) {
            if (ch == '"') {
                tok.string = get_string();
                tok.type = token_type::property;

                skip_spaces();
                ch = peek_char();
                expect_char(ch, ':', true, suborigin, 0x1094f);

                state_base::set_expect_property(false);
                trail_comma = false;
            }
            else {
                expect_char(ch, '}', true, suborigin, 0x10950);
                unnest(nest_type::object, suborigin, 0x10951);

                tok.type = token_type::end_object;

                if (!state_base::nest_stack().empty() && state_base::nest_stack().top() == nest_type::object) {
                    state_base::set_expect_property(true);
                }
            }
        }
        else {
            if (ch == 'n') {
                tok.string = get_literal("null");
                tok.type = token_type::null;
            }
            else if (ch == 'f') {
                tok.string = get_literal("false");
                tok.boolean = false;
                tok.type = token_type::boolean;
            }
            else if (ch == 't') {
                tok.string = get_literal("true");
                tok.boolean = true;
                tok.type = token_type::boolean;
            }
            else if (ascii::is_digit(ch) || ch == '+' || ch == '-') {
                tok.string = get_number();
                tok.number = std::atof(tok.string.c_str());
                tok.type = token_type::number;
            }
            else if (ch == '"') {
                tok.string = get_string();
                tok.type = token_type::string;
            }
            else if (ch == '[') {
                base::get();
                state_base::nest(nest_type::array);

                tok.type = token_type::begin_array;

                trail_comma = false;
            }
            else if (ch == ']') {
                base::get();
                unnest(nest_type::array, suborigin, 0x10952);

                tok.type = token_type::end_array;
            }
            else if (ch == '{') {
                base::get();
                state_base::nest(nest_type::object);

                tok.type = token_type::begin_object;

                trail_comma = false;
            }
            else {
                base::set_bad();
                diag_base::template throw_exception<diag::input_error>(suborigin, 0x10953, "Unexpected ch=%c (\\u%4.4x)", ch, ch);
            }

            if (!state_base::nest_stack().empty() && state_base::nest_stack().top() == nest_type::object) {
                state_base::set_expect_property(true);
            }
        }

        if (trail_comma && !state_base::nest_stack().empty()) {
            skip_spaces();

            ch = peek_char();
            if (ch == ',') {
                base::get();
            }
            else {
                if (state_base::nest_stack().top() == nest_type::object && state_base::expect_property()) {
                    expect_char(ch, '}', false, suborigin, 0x10954);
                }
                else {
                    expect_char(ch, ']', false, suborigin, 0x10955);
                }
            }
        }

        base::set_gcount(tok.string.length());

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10956, "End: tok.type=%u, tok.string='%s'", tok.type, tok.string.c_str());

        return tok;
    }


    inline void istream::unnest(nest_type type, const char* suborigin, diag::tag_t tag) {
        nest_type actual = nest_type::none;
        if (!state_base::nest_stack().empty()) {
            actual = state_base::nest_stack().top();
        }

        if (actual == type) {
            state_base::unnest(type);
        }
        else {
            base::set_bad();
            diag_base::template throw_exception<diag::input_error>(suborigin, tag, "actual_nest_type=%u, expected_nest_type=%u", actual, type);
        }
    }


    inline literal::string istream::get_string() {
        constexpr const char* suborigin = "get_string()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10957, "Begin:");

        literal::string str;

        char ch = peek_char();
        if (ch == '"') {
            base::get();

            for (;;) {
                str += get_chars(ascii::json::is_string_content);

                ch = peek_char();
                if (ch == '"') {
                    base::get();
                    break;
                }
                else if (ch == '\\') {
                    str += get_escaped_char();
                }
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10958, "End: str='%s'", str.c_str());

        return str;
    }


    inline literal::string istream::get_number() {
        constexpr const char* suborigin = "get_number()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10959, "Begin:");

        literal::string str;

        char ch = peek_char();
        if (ch == '+' || ch == '-') {
            str += base::get();
        }

        str += get_digits();

        ch = peek_char();
        if (ch == '.') {
            str += base::get();

            str += get_digits();
        }

        ch = peek_char();
        if (ch == 'e' || ch == 'E') {
           str += base::get();

            ch = peek_char();
            if (ch == '+' || ch == '-') {
                str += base::get();
            }

            str += get_digits();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1095a, "End: str='%s'", str.c_str());

        return str;
    }


    inline literal::string istream::get_literal(const char* literal) {
        constexpr const char* suborigin = "get_literal()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1095b, "Begin: literal='%s'", literal);

        literal::string str;

        for (const char* itr = literal; *itr != '\0'; itr++) {
            char ch = peek_char();
            expect_char(ch, *itr, true, suborigin, 0x1095c);

            str += ch;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1095d, "End: str='%s'", str.c_str());

        return str;
    }


    inline char istream::get_escaped_char() {
        constexpr const char* suborigin = "get_escaped_char()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1095e, "Begin:");

        char ch = peek_char();
        expect_char(ch, '\\', true, suborigin, 0x1095f);

        ch = peek_char();
        if (ch == '"' || ch == '\\' || ch == '/') {
            base::get();
        }
        else if (ch == 'b') {
            base::get();
            ch = '\b';
        }
        else if (ch == 'f') {
            base::get();
            ch = '\f';
        }
        else if (ch == 'n') {
            base::get();
            ch = '\n';
        }
        else if (ch == 'r') {
            base::get();
            ch = '\r';
        }
        else if (ch == 't') {
            base::get();
            ch = '\t';
        }
        else if (ch == 'u') {
            base::get();

            literal::string str = get_hex();

            if (str.length() != 4) {
                base::set_bad();
                diag_base::template throw_exception<diag::input_error>(suborigin, 0x10960, "str='%s'", str.c_str());
            }
            else if (str[0] == '0' && str[1] == '0') {
                ch = (ascii::hex(str[2]) << 4) | ascii::hex(str[3]);
            }
            else {
                base::set_bad();
                diag_base::template throw_exception<diag::input_error>(suborigin, 0x10961, "Wide chars not supported.");
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10962, "End: ch='%c' (0x%2.2x)", ch, ch);

        return ch;
    }


    inline void istream::expect_char(char actual, char expected, bool should_get, const char* suborigin, diag::tag_t tag) {
        if (actual != expected) {
            base::set_bad();
            diag_base::template throw_exception<diag::input_error>(suborigin, tag, "actual_char=%c (\\u%4.4x), expected_char=%c (\\u%4.4x)", actual, actual, expected, expected);
        }
        else if (should_get) {
            base::get();
        }
    }


    inline literal::string istream::get_hex() {
        return get_chars(ascii::is_hex);
    }


    inline literal::string istream::get_digits() {
        return get_chars(ascii::is_digit);
    }


    inline literal::string istream::get_chars(ascii::predicate_t&& predicate) {
        literal::string str;

        while (base::is_good() && predicate(peek_char())) {
            str += base::get();
        }

        return str;
    }


    inline std::size_t istream::skip_spaces() {
        return skip_chars(ascii::json::is_space);
    }


    inline std::size_t istream::skip_chars(ascii::predicate_t&& predicate) {
        std::size_t gcount = 0;
        
        while (base::is_good() && predicate(peek_char())) {
            base::get();
            gcount++;
        }

        return gcount;
    }
    
    
    inline char istream::get_char() {
        char ch = peek_char();

        if (base::is_good()) {
            base::get();
        }

        return ch;
    }


    inline char istream::peek_char() {
        char ch = base::peek();

        if (!ascii::json::is_valid(ch)) {
            base::set_bad();
            ch = '\0';
        }

        return ch;
    }


   // --------------------------------------------------------------


    inline reader::reader(const char* origin, std::streambuf* sb, diag::log_ostream* log)
        : base(origin, sb, log) {

        constexpr const char* suborigin = "reader()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10963, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10964, "End:");
    }


    inline reader::reader(std::streambuf* sb, diag::log_ostream* log)
        : reader("abc::net::json::reader", sb, log) {
    }


    inline reader::reader(reader&& other) noexcept
        : base(std::move(other)) {
    }


    inline value reader::get_value() {
        constexpr const char* suborigin = "get_value()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10965, "Begin:");

        token token = base::get_token();

        value value = get_value_from_token(std::move(token));

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10966, "End:");

        return value;
    }


    inline value reader::get_value_from_token(token&& token) {
        constexpr const char* suborigin = "get_value_from_token()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10967, "Begin:");

        switch (token.type) {
            case token_type::null:
                return value(nullptr, base::log());

            case token_type::boolean:
                return value(token.boolean, base::log());

            case token_type::number:
                return value(token.number, base::log());

            case token_type::string:
                return value(std::move(token.string), base::log());

            case token_type::begin_array:
                return value(get_array(), base::log());

            case token_type::begin_object:
                return value(get_object(), base::log());

            default:
                base::set_bad();
                diag_base::template throw_exception<diag::input_error>(suborigin, 0x10968, "Unexpected token_type=%u", token.type);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10969, "End:");

        return value(base::log());
    }


    inline literal::array reader::get_array() {
        constexpr const char* suborigin = "get_array()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1096a, "Begin:");

        literal::array array;

        token token = base::get_token();
        while (token.type != token_type::end_array) {
            value value = get_value_from_token(std::move(token));
            if (value.type() == value_type::empty) {
                base::set_bad();
                diag_base::template throw_exception<diag::input_error>(suborigin, 0x1096b, "Unexpected value_type=%u", value.type());
            }

            array.push_back(std::move(value));

            token = base::get_token();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1096c, "End: size=%zu", array.size());

        return array;
    }


    inline literal::object reader::get_object() {
        constexpr const char* suborigin = "get_object()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1096d, "Begin:");

        literal::object object;

        token token = base::get_token();
        while (token.type != token_type::end_object) {
            if (token.type != token_type::property) {
                base::set_bad();
                diag_base::template throw_exception<diag::input_error>(suborigin, 0x1096e, "Unexpected token_type=%u", token.type);
            }

            value value = get_value();
            if (value.type() == value_type::empty) {
                base::set_bad();
                diag_base::template throw_exception<diag::input_error>(suborigin, 0x1096f, "Unexpected value_type=%u", value.type());
            }

            object[token.string] = std::move(value);

            token = base::get_token();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10970, "End: size=%zu", object.size());

        return object;
    }


    // --------------------------------------------------------------


    inline ostream::ostream(const char* origin, std::streambuf* sb, diag::log_ostream* log)
        : base(sb)
        , state_base(origin, log)
        , _skip_comma(false) {

        constexpr const char* suborigin = "ostream()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10971, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10972, "End:");
    }


    inline ostream::ostream(std::streambuf* sb, diag::log_ostream* log)
        : ostream("abc::net::json::ostream", sb, log) {
    }


    inline ostream::ostream(ostream&& other) noexcept
        : base(std::move(other))
        , state_base(std::move(other))
        , _skip_comma(other._skip_comma) {
    }


    inline void ostream::put_token(const token& token) {
        constexpr const char* suborigin = "put_token()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10115, "Begin: token.type=0x%2.2x", token.type);

        switch (token.type)
        {
        case token_type::null:
            put_null();
            break;

        case token_type::boolean:
            put_boolean(token.boolean);
            break;

        case token_type::number:
            put_number(token.number);
            break;

        case token_type::string:
            put_string(token.string);
            break;

        case token_type::property:
            put_property(token.string);
            break;

        case token_type::begin_array:
            put_begin_array();
            break;

        case token_type::end_array:
            put_end_array();
            break;

        case token_type::begin_object:
            put_begin_object();
            break;

        case token_type::end_object:
            put_end_object();
            break;

        default:
            base::set_bad();
            diag_base::expect(suborigin, false, 0x10116, "Invalid token.type=0x%2.2x", token.type);
            break;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10117, "End:");
    }


    inline void ostream::put_null() {
        constexpr const char* suborigin = "put_null()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10973, "Begin:");

        std::size_t pcount = put_literal("null", 4);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10974, "End: pcount=%zu", pcount);
    }


    inline void ostream::put_boolean(literal::boolean b) {
        constexpr const char* suborigin = "put_boolean()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10975, "Begin: b=%d", b);

        std::size_t pcount = 0;
        if (b) {
            pcount = put_literal("true", 4);
        }
        else {
            pcount = put_literal("false", 5);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10976, "End: pcount=%zu", pcount);
    }


    inline void ostream::put_number(literal::number n) {
        constexpr const char* suborigin = "put_number()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10977, "Begin: n=%.16lg", n);

        char literal[19 + 6 + 1];
        std::size_t size = std::snprintf(literal, sizeof(literal), "%.16lg", n);

        std::size_t pcount = put_literal(literal, size);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10978, "End: pcount=%zu", pcount);
    }


    inline void ostream::put_string(const literal::string& s) {
        constexpr const char* suborigin = "put_string()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10979, "Begin: s='%s'", s.c_str());

        put_literal_precond();

        std::size_t pcount = 0;
        pcount += put_chars("\"", 1);
        pcount += put_chars(s.c_str(), s.length());
        pcount += put_chars("\"", 1);

        put_literal_postcond();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1097a, "End: pcount=%zu", pcount);
    }


    inline void ostream::put_property(const literal::string& name) {
        constexpr const char* suborigin = "put_property()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1097b, "Begin: name='%s'", name.c_str());

        if (state_base::nest_stack().empty() || state_base::nest_stack().top() != nest_type::object || !state_base::expect_property()) {
            base::set_bad();
            diag_base::expect(suborigin, false, 0x1097c, "Did not expect a property.");
            return;
        }

        if (!_skip_comma) {
            put_chars(",", 1);
        }

        std::size_t pcount = 0;
        pcount += put_chars("\"", 1);
        pcount += put_chars(name.c_str(), name.length());
        pcount += put_chars("\":", 2);

        _skip_comma = true;
        state_base::set_expect_property(false);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1097d, "End: pcount=%zu", pcount);
    }


    inline void ostream::put_begin_array() {
        constexpr const char* suborigin = "put_begin_array()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1097e, "Begin:");

        put_literal_precond();

        std::size_t pcount = put_chars("[", 1);

        state_base::nest_stack().push(nest_type::array);
        _skip_comma = true;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1097f, "End: pcount=%zu", pcount);
    }


    inline void ostream::put_end_array() {
        constexpr const char* suborigin = "put_end_array()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10980, "Begin:");

        if (state_base::nest_stack().empty() || state_base::nest_stack().top() != nest_type::array) {
            base::set_bad();
            diag_base::expect(suborigin, false, 0x10981, "Not in an array.");
            return;
        }

        std::size_t pcount = put_chars("]", 1);

        state_base::nest_stack().pop();
        put_literal_postcond();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10982, "End: pcount=%zu", pcount);
    }


    inline void ostream::put_begin_object() {
        constexpr const char* suborigin = "put_begin_object()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10983, "Begin:");

        put_literal_precond();

        std::size_t pcount = put_chars("{", 1);

        state_base::nest_stack().push(nest_type::object);
        state_base::set_expect_property(true);
        _skip_comma = true;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10984, "End: pcount=%zu", pcount);
    }


    inline void ostream::put_end_object() {
        constexpr const char* suborigin = "put_end_object()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10985, "Begin:");

        if (state_base::nest_stack().empty() || state_base::nest_stack().top() != nest_type::object) {
            base::set_bad();
            diag_base::expect(suborigin, false, 0x10986, "Not in an object.");
            return;
        }

        std::size_t pcount = put_chars("}", 1);

        state_base::nest_stack().pop();
        put_literal_postcond();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10987, "End: pcount=%zu", pcount);
    }


    inline void ostream::put_space() {
        put_chars(" ", 1);
    }


    inline void ostream::put_tab() {
        put_chars("\t", 1);
    }


    inline void ostream::put_cr() {
        put_chars("\r", 1);
    }


    inline void ostream::put_lf() {
        put_chars("\n", 1);
    }


    inline std::size_t ostream::put_literal(const char* chars, std::size_t chars_len) {
        constexpr const char* suborigin = "put_literal()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10988, "Begin: chars='%s'", chars);

        put_literal_precond();

        std::size_t pcount = put_chars(chars, chars_len);

        put_literal_postcond();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10989, "End: pcount=%zu", pcount);

        return pcount;
    }


    inline void ostream::put_literal_precond() {
        constexpr const char* suborigin = "put_literal_precond()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1098a, "Begin:");

        if (!state_base::nest_stack().empty() && state_base::nest_stack().top() == nest_type::object && state_base::expect_property()) {
            base::set_bad();
            diag_base::expect(suborigin, false, 0x1098b, "Expected a property.");
            return;
        }

        if (!state_base::nest_stack().empty() && state_base::nest_stack().top() == nest_type::array && !_skip_comma) {
            put_chars(",", 1);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1098c, "End:");
    }


    inline void ostream::put_literal_postcond() {
        constexpr const char* suborigin = "put_literal_postcond()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1098d, "Begin:");

        _skip_comma = false;

        if (!state_base::nest_stack().empty() && state_base::nest_stack().top() == nest_type::object) {
            state_base::set_expect_property(true);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1098e, "End:");
    }


    inline std::size_t ostream::put_chars(const char* chars, std::size_t chars_len) {
        constexpr const char* suborigin = "put_chars()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10121, "Begin: chars='%s'", chars);

        std::size_t pcount = 0;

        if (chars_len == size::strlen) {
            chars_len = std::strlen(chars);
        }

        while (base::is_good() && pcount < chars_len) {
            base::put(chars[pcount++]);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10122, "End: pcount=%zu", pcount);

        return pcount;
    }


    inline std::size_t ostream::put_char(char ch) {
        if (base::is_good()) {
            base::put(ch);
            return 1;
        }

        return 0;
    }


// --------------------------------------------------------------


    inline writer::writer(const char* origin, std::streambuf* sb, diag::log_ostream* log)
        : base(origin, sb, log) {

        constexpr const char* suborigin = "writer()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1098f, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10990, "End:");
    }


    inline writer::writer(std::streambuf* sb, diag::log_ostream* log)
        : writer("abc::net::json::writer", sb, log) {
    }


    inline writer::writer(writer&& other) noexcept
        : base(std::move(other)) {
    }


    inline void writer::put_value(const value& value) {
        constexpr const char* suborigin = "put_value()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10991, "Begin: type=%u", value.type());

        switch (value.type()) {
            case value_type::null:
                base::put_null();
                break;

            case value_type::boolean:
                base::put_boolean(value.boolean());
                break;

            case value_type::number:
                base::put_number(value.number());
                break;

            case value_type::string:
                base::put_string(value.string());
                break;

            case value_type::array:
                put_array(value.array());
                break;

            case value_type::object:
                put_object(value.object());
                break;

            default:
                diag_base::expect(suborigin, false, 0x10992, "Unexpected value_type=%u", value.type());
        }

        base::flush();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10993, "End:");
    }


    inline void writer::put_array(const literal::array& array) {
        constexpr const char* suborigin = "put_array()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10994, "Begin: size=%zu", array.size());

        base::put_begin_array();

        for (typename literal::array::const_reference item : array) {
            put_value(item);
        }

        base::put_end_array();

        base::flush();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10995, "End:");
    }


    inline void writer::put_object(const literal::object& object) {
        constexpr const char* suborigin = "put_object()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10996, "Begin: size=%zu", object.size());

        base::put_begin_object();

        for (typename literal::object::const_reference item : object) {
            base::put_property(item.first);
            put_value(item.second);
        }

        base::put_end_object();

        base::flush();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10997, "End:");
    }


    // --------------------------------------------------------------


    inline json_rpc_validator::json_rpc_validator(diag::log_ostream* log)
        : diag_base("abc::net::json::json_rpc_validator", log) {

        constexpr const char* suborigin = "json_rpc_validator()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline bool json_rpc_validator::is_batch_request(const value& value) const {
        constexpr const char* suborigin = "is_batch_request()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = value.type() == value_type::array && !value.array().empty();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_rpc_validator::is_simple_request(const value& value) const {
        constexpr const char* suborigin = "is_simple_request()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = value.type() == value_type::object;

        if (ok) {
            literal::object::const_iterator jsonrpc_itr = value.object().find("jsonrpc");
            ok = jsonrpc_itr != value.object().end() && is_jsonrpc(jsonrpc_itr->second);
        }

        if (ok) {
            literal::object::const_iterator id_itr = value.object().find("id");
            ok = id_itr != value.object().end() && is_id(id_itr->second);
        }

        if (ok) {
            literal::object::const_iterator method_itr = value.object().find("method");
            ok = method_itr != value.object().end() && is_method(method_itr->second);
        }

        if (ok) {
            literal::object::const_iterator params_itr = value.object().find("params");
            if (params_itr != value.object().end()) {
                ok = is_params(params_itr->second);
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_rpc_validator::is_simple_notification(const value& value) const {
        constexpr const char* suborigin = "is_simple_notification()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = value.type() == value_type::object;

        if (ok) {
            literal::object::const_iterator jsonrpc_itr = value.object().find("jsonrpc");
            ok = jsonrpc_itr != value.object().end() && is_jsonrpc(jsonrpc_itr->second);
        }

        if (ok) {
            literal::object::const_iterator id_itr = value.object().find("id");
            ok = id_itr == value.object().end();
        }

        if (ok) {
            literal::object::const_iterator method_itr = value.object().find("method");
            ok = method_itr != value.object().end() && is_method(method_itr->second);
        }

        if (ok) {
            literal::object::const_iterator params_itr = value.object().find("params");
            if (params_itr != value.object().end()) {
                ok = is_params(params_itr->second);
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_rpc_validator::is_batch_response(const value& value) const {
        constexpr const char* suborigin = "is_batch_response()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = value.type() == value_type::array && !value.array().empty();

        if (ok) {
            for (const json::value& item : value.array()) {
                ok = is_simple_response(item);
                if (!ok) {
                    break;
                }
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_rpc_validator::is_simple_response(const value& value) const {
        constexpr const char* suborigin = "is_simple_response()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = is_result_response(value) || is_error_response(value);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_rpc_validator::is_result_response(const value& value) const {
        constexpr const char* suborigin = "is_result_response()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = value.type() == value_type::object;

        if (ok) {
            literal::object::const_iterator jsonrpc_itr = value.object().find("jsonrpc");
            ok = jsonrpc_itr != value.object().end() && is_jsonrpc(jsonrpc_itr->second);
        }

        if (ok) {
            literal::object::const_iterator id_itr = value.object().find("id");
            ok = id_itr != value.object().end() && is_id(id_itr->second);
        }

        if (ok) {
            literal::object::const_iterator result_itr = value.object().find("result");
            ok = result_itr != value.object().end();
        }

        if (ok) {
            literal::object::const_iterator error_itr = value.object().find("error");
            ok = error_itr == value.object().end();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_rpc_validator::is_error_response(const value& value) const {
        constexpr const char* suborigin = "is_error_response()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = value.type() == value_type::object;

        if (ok) {
            literal::object::const_iterator jsonrpc_itr = value.object().find("jsonrpc");
            ok = jsonrpc_itr != value.object().end() && is_jsonrpc(jsonrpc_itr->second);
        }

        if (ok) {
            literal::object::const_iterator id_itr = value.object().find("id");
            ok = id_itr != value.object().end() && is_id(id_itr->second);
        }

        if (ok) {
            literal::object::const_iterator error_itr = value.object().find("error");
            ok = error_itr != value.object().end() && is_error(error_itr->second);
        }

        if (ok) {
            literal::object::const_iterator result_itr = value.object().find("result");
            ok = result_itr == value.object().end();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_rpc_validator::is_error(const value& value) const {
        constexpr const char* suborigin = "is_error()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = value.type() == value_type::object;

        if (ok) {
            literal::object::const_iterator code_itr = value.object().find("code");
            ok = code_itr != value.object().end() && is_code(code_itr->second);
        }

        if (ok) {
            literal::object::const_iterator message_itr = value.object().find("message");
            ok = message_itr != value.object().end() && is_message(message_itr->second);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_rpc_validator::is_jsonrpc(const value& value) const {
        return value.type() == value_type::string && value.string() == "2.0";
    }


    inline bool json_rpc_validator::is_id(const value& value) const {
        return value.type() == value_type::string || (value.type() == value_type::number && static_cast<literal::number>(static_cast<std::int64_t>(value.number())) == value.number());
    }


    inline bool json_rpc_validator::is_method(const value& value) const {
        return value.type() == value_type::string;
    }


    inline bool json_rpc_validator::is_params(const value& value) const {
        return value.type() == value_type::array || value.type() == value_type::object;
    }


    inline bool json_rpc_validator::is_code(const value& value) const {
        return value.type() == value_type::number && static_cast<literal::number>(static_cast<std::int64_t>(value.number())) == value.number();
    }


    inline bool json_rpc_validator::is_message(const value& value) const {
        return value.type() == value_type::string;
    }


    // --------------------------------------------------------------


    inline json_schema_validator::json_schema_validator(diag::log_ostream* log)
        : diag_base("abc::net::json::json_schema_validator", log) {

        constexpr const char* suborigin = "json_schema_validator()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline bool json_schema_validator::is_valid(const value& document, const value& schema) const {
        constexpr const char* suborigin = "is_valid()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = is_valid(document, schema, schema);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_schema_validator::is_valid(const value& fragment, const value& fragment_schema, const value& document_schema) const {
        constexpr const char* suborigin = "is_valid(fragment)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = true;

        // boolean
        if (fragment_schema.type() == value_type::boolean) {
            ok = fragment_schema.boolean();
        }
        // object
        else if (fragment_schema.type() == value_type::object) {
            const value_type fragment_type = fragment.type();

            if (fragment_type == value_type::boolean) {
                ok = is_valid_boolean(fragment.boolean(), fragment_schema, document_schema);
            }
            else if (fragment_type == value_type::number) {
                ok = is_valid_number(fragment.number(), fragment_schema, document_schema);
            }
            else if (fragment_type == value_type::string) {
                ok = is_valid_string(fragment.string(), fragment_schema, document_schema);
            }
            else if (fragment_type == value_type::array) {
                ok = is_valid_array(fragment.array(), fragment_schema, document_schema);
            }
            else if (fragment_type == value_type::object) {
                ok = is_valid_object(fragment.object(), fragment_schema, document_schema);
            }

            // $ref
            literal::object::const_iterator ref_itr = fragment_schema.object().find("$ref");
            if (ok && ref_itr != fragment_schema.object().end()) {
                if (ref_itr->second.type() == value_type::string) {
                    const literal::string& ref = ref_itr->second.string();

                    diag_base::require(suborigin, ascii::are_equal_n(ref.c_str(), "#/$defs/", 8), __TAG__, "ref.c_str(), == '#/$defs/'");

                    const value& refed_fragment_schema = resolve_ref(ref.substr(8).c_str(), document_schema);
                    ok = is_valid(fragment, refed_fragment_schema, document_schema);
                }
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_schema_validator::is_valid_boolean(const literal::boolean& b, const value& fragment_schema, const value& /*document_schema*/) const {
        constexpr const char* suborigin = "is_valid_boolean()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = true;

        // type
        literal::object::const_iterator type_itr = fragment_schema.object().find("type");
        if (ok && type_itr != fragment_schema.object().end()) {
            ok = type_itr->second.type() == value_type::string && type_itr->second.string() == "boolean";
        }

        // const
        literal::object::const_iterator const_itr = fragment_schema.object().find("const");
        if (ok && const_itr != fragment_schema.object().end()) {
            ok = const_itr->second.type() == value_type::boolean && const_itr->second.boolean() == b;
        }

        // enum
        literal::object::const_iterator enum_itr = fragment_schema.object().find("enum");
        if (ok && enum_itr != fragment_schema.object().end()) {
            diag_base::require(suborigin, enum_itr->second.type() == value_type::array, __TAG__, "enum_itr->second.type() == value_type::array");

            const literal::array& enum_arr = enum_itr->second.array();
            for (const value& item : enum_arr) {
                ok = item.type() == value_type::boolean && item.boolean() == b;
                if (ok) {
                    break;
                }
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_schema_validator::is_valid_number(const literal::number& num, const value& fragment_schema, const value& /*document_schema*/) const {
        constexpr const char* suborigin = "is_valid_number()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = true;

        // type
        literal::object::const_iterator type_itr = fragment_schema.object().find("type");
        if (ok && type_itr != fragment_schema.object().end()) {
            ok = type_itr->second.type() == value_type::string && type_itr->second.string() == "number";
        }

        // const
        literal::object::const_iterator const_itr = fragment_schema.object().find("const");
        if (ok && const_itr != fragment_schema.object().end()) {
            ok = const_itr->second.type() == value_type::number && const_itr->second.number() == num;
        }

        // enum
        literal::object::const_iterator enum_itr = fragment_schema.object().find("enum");
        if (ok && enum_itr != fragment_schema.object().end()) {
            diag_base::require(suborigin, enum_itr->second.type() == value_type::array, __TAG__, "enum_itr->second.type() == value_type::array");

            const literal::array& enum_arr = enum_itr->second.array();
            for (const value& item : enum_arr) {
                ok = item.type() == value_type::number && item.number() == num;
                if (ok) {
                    break;
                }
            }
        }

        // minimum
        literal::object::const_iterator minimum_itr = fragment_schema.object().find("minimum");
        if (ok && minimum_itr != fragment_schema.object().end()) {
            ok = minimum_itr->second.type() == value_type::number && minimum_itr->second.number() <= num;
        }

        // exclusiveMinimum
        literal::object::const_iterator exclusive_minimum_itr = fragment_schema.object().find("exclusiveMinimum");
        if (ok && exclusive_minimum_itr != fragment_schema.object().end()) {
            ok = exclusive_minimum_itr->second.type() == value_type::number && exclusive_minimum_itr->second.number() < num;
        }

        // maximum
        literal::object::const_iterator maximum_itr = fragment_schema.object().find("maximum");
        if (ok && maximum_itr != fragment_schema.object().end()) {
            ok = maximum_itr->second.type() == value_type::number && maximum_itr->second.number() >= num;
        }

        // exclusiveMaximum
        literal::object::const_iterator exclusive_maximum_itr = fragment_schema.object().find("exclusiveMaximum");
        if (ok && exclusive_maximum_itr != fragment_schema.object().end()) {
            ok = exclusive_maximum_itr->second.type() == value_type::number && exclusive_maximum_itr->second.number() > num;
        }

        // multipleOf
        literal::object::const_iterator multiple_of_itr = fragment_schema.object().find("multipleOf");
        if (ok && multiple_of_itr != fragment_schema.object().end()) {
            diag_base::require(suborigin, multiple_of_itr->second.type() == value_type::number, __TAG__, "multiple_of_itr->second.type() == value_type::number");
            diag_base::require(suborigin, multiple_of_itr->second.number() != 0, __TAG__, "multiple_of_itr->second.number() != 0");

            ok = num / multiple_of_itr->second.number() == static_cast<int>(num / multiple_of_itr->second.number());
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_schema_validator::is_valid_string(const literal::string& str, const value& fragment_schema, const value& /*document_schema*/) const {
        constexpr const char* suborigin = "is_valid_string()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = true;

        // type
        literal::object::const_iterator type_itr = fragment_schema.object().find("type");
        if (ok && type_itr != fragment_schema.object().end()) {
            ok = type_itr->second.type() == value_type::string && type_itr->second.string() == "string";
        }

        // const
        literal::object::const_iterator const_itr = fragment_schema.object().find("const");
        if (ok && const_itr != fragment_schema.object().end()) {
            ok = const_itr->second.type() == value_type::string && const_itr->second.string() == str;
        }

        // enum
        literal::object::const_iterator enum_itr = fragment_schema.object().find("enum");
        if (ok && enum_itr != fragment_schema.object().end()) {
            diag_base::require(suborigin, enum_itr->second.type() == value_type::array, __TAG__, "enum_itr->second.type() == value_type::array");

            const literal::array& enum_arr = enum_itr->second.array();
            for (const value& item : enum_arr) {
                ok = item.type() == value_type::string && item.string() == str;
                if (ok) {
                    break;
                }
            }
        }

        // pattern
        literal::object::const_iterator pattern_itr = fragment_schema.object().find("pattern");
        if (ok && pattern_itr != fragment_schema.object().end()) {
            diag_base::require(suborigin, pattern_itr->second.type() == value_type::string, __TAG__, "pattern_itr->second.type() == value_type::string");

            ok = std::regex_match(str, std::regex(pattern_itr->second.string()));
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_schema_validator::is_valid_array(const literal::array& arr, const value& fragment_schema, const value& document_schema) const {
        constexpr const char* suborigin = "is_valid_array()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = true;

        // type
        literal::object::const_iterator type_itr = fragment_schema.object().find("type");
        if (ok && type_itr != fragment_schema.object().end()) {
            ok = type_itr->second.type() == value_type::string && type_itr->second.string() == "array";
        }

        // items
        literal::object::const_iterator items_itr = fragment_schema.object().find("items");
        if (ok && items_itr != fragment_schema.object().end()) {
            const value& item_schema = items_itr->second;
            for (const value& item : arr) {
                ok = is_valid(item, item_schema, document_schema);
                if (!ok) {
                    break;
                }
            }
        }

        // prefixItems
        literal::object::const_iterator prefix_items_itr = fragment_schema.object().find("prefixItems");
        if (ok && prefix_items_itr != fragment_schema.object().end()) {
            const value& prefix_items_schema = prefix_items_itr->second;

            diag_base::require(suborigin, prefix_items_schema.type() == value_type::array, __TAG__, "prefix_items_schema.type() == value_type::array");

            ok = arr.size() == prefix_items_schema.array().size();
            
            for (std::size_t i = 0; ok && i < prefix_items_schema.array().size() && i < arr.size(); i++) {
                ok = is_valid(arr[i], prefix_items_schema.array()[i], document_schema);
            }
        }

        // contains (minContains, maxContains)
        literal::object::const_iterator contains_itr = fragment_schema.object().find("contains");
        if (ok && contains_itr != fragment_schema.object().end()) {
            literal::object::const_iterator min_contains_itr = fragment_schema.object().find("minContains");
            std::size_t min_contains = min_contains_itr != fragment_schema.object().end() ? static_cast<std::size_t>(static_cast<std::int64_t>(min_contains_itr->second.number())) : 1;
            
            literal::object::const_iterator max_contains_itr = fragment_schema.object().find("maxContains");
            std::size_t max_contains = max_contains_itr != fragment_schema.object().end() ? static_cast<std::size_t>(static_cast<std::int64_t>(max_contains_itr->second.number())) : arr.size();

            const value& contains_schema = contains_itr->second;
            std::size_t contains_count = 0;
            for (const value& item : arr) {
                if (is_valid(item, contains_schema, document_schema)) {
                    contains_count++;
                }
            }

            ok = contains_count >= min_contains && contains_count <= max_contains;
        }

        // minItems
        literal::object::const_iterator min_items_itr = fragment_schema.object().find("minItems");
        if (ok && min_items_itr != fragment_schema.object().end()) {
            diag_base::require(suborigin, min_items_itr->second.type() == value_type::number, __TAG__, "minItems_itr->second.type() == value_type::number");
            diag_base::require(suborigin, min_items_itr->second.number() >= 0, __TAG__, "minItems_itr->second.number() >= 0");
            diag_base::require(suborigin, min_items_itr->second.number() == static_cast<double>(static_cast<std::int64_t>(min_items_itr->second.number())), __TAG__, "minItems_itr->second.number() is integer");

            std::size_t min_items = static_cast<std::size_t>(static_cast<std::int64_t>(min_items_itr->second.number()));
            ok = arr.size() >= min_items;
        }

        // maxItems
        literal::object::const_iterator max_items_itr = fragment_schema.object().find("maxItems");
        if (ok && max_items_itr != fragment_schema.object().end()) {
            diag_base::require(suborigin, max_items_itr->second.type() == value_type::number, __TAG__, "maxItems_itr->second.type() == value_type::number");
            diag_base::require(suborigin, max_items_itr->second.number() >= 0, __TAG__, "maxItems_itr->second.number() >= 0");
            diag_base::require(suborigin, max_items_itr->second.number() == static_cast<double>(static_cast<std::int64_t>(max_items_itr->second.number())), __TAG__, "maxItems_itr->second.number() is integer");

            std::size_t max_items = static_cast<std::size_t>(static_cast<std::int64_t>(max_items_itr->second.number()));
            ok = arr.size() <= max_items;
        }

        // uniqueItems
        literal::object::const_iterator unique_items_itr = fragment_schema.object().find("uniqueItems");
        if (ok && unique_items_itr != fragment_schema.object().end()) {
            const value& unique_items_schema = unique_items_itr->second;

            diag_base::require(suborigin, unique_items_schema.type() == value_type::boolean, __TAG__, "unique_items_schema.type() == value_type::boolean");

            if (unique_items_schema.boolean()) {
                for (std::size_t i = 0; i < arr.size(); i++) {
                    for (std::size_t j = i + 1; j < arr.size(); j++) {
                        if (arr[i] == arr[j]) {
                            ok = false;
                            break;
                        }
                    }
                    if (!ok) {
                        break;
                    }
                }
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline bool json_schema_validator::is_valid_object(const literal::object& obj, const value& fragment_schema, const value& document_schema) const {
        constexpr const char* suborigin = "is_valid_object()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        bool ok = true;

        // type
        literal::object::const_iterator type_itr = fragment_schema.object().find("type");
        if (ok && type_itr != fragment_schema.object().end()) {
            ok = type_itr->second.type() == value_type::string && type_itr->second.string() == "object";
        }

        // properties
        literal::object::const_iterator properties_itr = fragment_schema.object().find("properties");
        if (ok && properties_itr != fragment_schema.object().end()) {
            const value& properties_schema = properties_itr->second;
            for (const literal::object::value_type& prop : obj) {
                literal::object::const_iterator property_schema_itr = properties_schema.object().find(prop.first);
                if (property_schema_itr != properties_schema.object().end()) {
                    ok = is_valid(prop.second, property_schema_itr->second, document_schema);
                    if (!ok) {
                        break;
                    }
                }
            }
        }

        // required
        literal::object::const_iterator required_itr = fragment_schema.object().find("required");
        if (ok && required_itr != fragment_schema.object().end()) {
            diag_base::require(suborigin, required_itr->second.type() == value_type::array, __TAG__, "required_itr->second.type() == value_type::array");

            for (const value& required_prop : required_itr->second.array()) {
                diag_base::require(suborigin, required_prop.type() == value_type::string, __TAG__, "required_prop.type() == value_type::string");

                ok = obj.find(required_prop.string()) != obj.end();
                if (!ok) {
                    break;
                }
            }
        }

        // minProperties
        literal::object::const_iterator min_properties_itr = fragment_schema.object().find("minProperties");
        if (ok && min_properties_itr != fragment_schema.object().end()) {
            diag_base::require(suborigin, min_properties_itr->second.type() == value_type::number, __TAG__, "minProperties_itr->second.type() == value_type::number");
            diag_base::require(suborigin, min_properties_itr->second.number() >= 0, __TAG__, "minProperties_itr->second.number() >= 0");
            diag_base::require(suborigin, min_properties_itr->second.number() == static_cast<double>(static_cast<std::int64_t>(min_properties_itr->second.number())), __TAG__, "minProperties_itr->second.number() is integer");

            std::size_t min_properties = static_cast<std::size_t>(static_cast<std::int64_t>(min_properties_itr->second.number()));
            ok = obj.size() >= min_properties;
        }

        // maxProperties
        literal::object::const_iterator max_properties_itr = fragment_schema.object().find("maxProperties");
        if (ok && max_properties_itr != fragment_schema.object().end()) {
            diag_base::require(suborigin, max_properties_itr->second.type() == value_type::number, __TAG__, "maxProperties_itr->second.type() == value_type::number");
            diag_base::require(suborigin, max_properties_itr->second.number() >= 0, __TAG__, "maxProperties_itr->second.number() >= 0");
            diag_base::require(suborigin, max_properties_itr->second.number() == static_cast<double>(static_cast<std::int64_t>(max_properties_itr->second.number())), __TAG__, "maxProperties_itr->second.number() is integer");

            std::size_t max_properties = static_cast<std::size_t>(static_cast<std::int64_t>(max_properties_itr->second.number()));
            ok = obj.size() <= max_properties;
        }

        // additionalProperties
        literal::object::const_iterator additional_properties_itr = fragment_schema.object().find("additionalProperties");
        if (ok && additional_properties_itr != fragment_schema.object().end()) {
            const value& additional_properties_schema = additional_properties_itr->second;
            diag_base::require(suborigin, additional_properties_schema.type() == value_type::boolean, __TAG__, "additional_properties_schema.type() == value_type::boolean");

            if (!additional_properties_schema.boolean()) {
                literal::object::const_iterator properties_itr = fragment_schema.object().find("properties");
                for (const literal::object::value_type& prop : obj) {
                    const literal::object& properties = fragment_schema.object().find("properties")->second.object();
                    ok = properties_itr != fragment_schema.object().end() && properties.find(prop.first) != properties.end();
                    if (!ok) {
                        break;
                    }
                }
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: ok=%d", ok);

        return ok;
    }


    inline const value& json_schema_validator::resolve_ref(const char* ref, const value& document_schema) const {
        constexpr const char* suborigin = "resolve_ref()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        literal::object::const_iterator defs_itr = document_schema.object().find("$defs");
        diag_base::require(suborigin, defs_itr != document_schema.object().end(), __TAG__, "defs_itr != document_schema.object().end()");

        const value& defs_schema = defs_itr->second;
        diag_base::require(suborigin, defs_schema.type() == value_type::object, __TAG__, "defs_schema.type() == value_type::object");

        literal::object::const_iterator def_itr = defs_schema.object().find(ref);
        diag_base::require(suborigin, def_itr != defs_schema.object().end(), __TAG__, "def_itr != defs_schema.object().end()");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        return def_itr->second;
    }


    // --------------------------------------------------------------

} } }
