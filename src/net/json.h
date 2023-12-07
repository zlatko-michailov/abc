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

#include <cstdlib>
#include <cstdio>

#include "../size.h"
#include "../ascii.h"
#include "../stream.h"
#include "../util.h"
#include "i/json.i.h"


namespace abc { namespace net { namespace json {

    template <typename LogPtr>
    inline value<LogPtr>::value(const LogPtr& log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::empty) {
    }


    template <typename LogPtr>
    inline value<LogPtr>::value(literal::null, const LogPtr& log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::null) {
    }


    template <typename LogPtr>
    inline value<LogPtr>::value(literal::boolean b, const LogPtr& log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::boolean)
        , _boolean(b) {
    }


    template <typename LogPtr>
    inline value<LogPtr>::value(literal::number n, const LogPtr& log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::number)
        , _number(n) {
    }


    template <typename LogPtr>
    inline value<LogPtr>::value(const char* str, const LogPtr& log)
        : value(std::string(str), log) {
    }


    template <typename LogPtr>
    inline value<LogPtr>::value(literal::string&& str, const LogPtr& log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::string)
        , _string(std::move(str)) {
    }


    template <typename LogPtr>
    inline value<LogPtr>::value(literal::array<LogPtr>&& arr, const LogPtr& log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::array)
        , _array(std::move(arr)) {
    }


    template <typename LogPtr>
    inline value<LogPtr>::value(literal::object<LogPtr>&& obj, const LogPtr& log) noexcept
        : diag_base(copy(_origin), log)
        , _type(value_type::object)
        , _object(std::move(obj)) {
    }


    template <typename LogPtr>
    inline value<LogPtr>::value(const value& other)
        : diag_base(copy(_origin), other.log()) {

        copy_from(other);
    }


    template <typename LogPtr>
    inline value<LogPtr>::value(value&& other) noexcept 
        : diag_base(copy(_origin), other.log()) {

        move_from(std::move(other));
    }


    template <typename LogPtr>
    inline value<LogPtr>::~value() noexcept {
        clear();
    }


    template <typename LogPtr>
    inline void value<LogPtr>::clear() noexcept {
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
                using literal_array = literal::array<LogPtr>;
                _array.~literal_array();
                break;

            case value_type::object:
                using literal_object = literal::object<LogPtr>;
                _object.~literal_object();
                break;
        }

        _type = value_type::empty;
    }


    template <typename LogPtr>
    inline value<LogPtr>& value<LogPtr>::operator = (const value& other) {
        clear();
        copy_from(other);

        return *this;
    }


    template <typename LogPtr>
    inline value<LogPtr>& value<LogPtr>::operator = (value&& other) noexcept {
        clear();
        move_from(std::move(other));

        return *this;
    }


    template <typename LogPtr>
    inline value_type value<LogPtr>::type() const noexcept {
        return _type;
    }


    template <typename LogPtr>
    inline literal::boolean value<LogPtr>::boolean() const {
        diag_base::assert("boolean()", _type == value_type::boolean, __TAG__, "_type=%u", _type);

        return _boolean;
    }

    
    template <typename LogPtr>
    inline literal::number value<LogPtr>::number() const {
        diag_base::assert("number()", _type == value_type::number, __TAG__, "_type=%u", _type);

        return _number;
    }


    template <typename LogPtr>
    inline const literal::string& value<LogPtr>::string() const {
        diag_base::assert("string()", _type == value_type::string, __TAG__, "_type=%u", _type);

        return _string;
    }


    template <typename LogPtr>
    inline literal::string& value<LogPtr>::string() {
        diag_base::assert("string()", _type == value_type::string, __TAG__, "_type=%u", _type);

        return _string;
    }


    template <typename LogPtr>
    inline const literal::array<LogPtr>& value<LogPtr>::array() const {
        diag_base::assert("array()", _type == value_type::array, __TAG__, "_type=%u", _type);

        return _array;
    }


    template <typename LogPtr>
    inline literal::array<LogPtr>& value<LogPtr>::array() {
        diag_base::assert("array()", _type == value_type::array, __TAG__, "_type=%u", _type);

        return _array;
    }


    template <typename LogPtr>
    inline const literal::object<LogPtr>& value<LogPtr>::object() const {
        diag_base::assert("object()", _type == value_type::object, __TAG__, "_type=%u", _type);

        return _object;
    }


    template <typename LogPtr>
    inline literal::object<LogPtr>& value<LogPtr>::object() {
        diag_base::assert("object()", _type == value_type::object, __TAG__, "_type=%u", _type);

        return _object;
    }


    template <typename LogPtr>
    inline bool value<LogPtr>::operator ==(const value& other) const noexcept {
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
        diag_base::ensure("operator ==", false, __TAG__, "_type=%u", _type);
        return false;
    }


    template <typename LogPtr>
    inline bool value<LogPtr>::operator !=(const value& other) const noexcept {
        return !(*this == other);
    }


    template <typename LogPtr>
    inline void value<LogPtr>::copy_from(const value& other) {
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
                new (&_array) literal::array<LogPtr>(other._array);
                break;

            case value_type::object:
                new (&_object) literal::object<LogPtr>(other._object);
                break;
        }
    }


    template <typename LogPtr>
    inline void value<LogPtr>::move_from(value&& other) noexcept {
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
                new (&_array) literal::array<LogPtr>(std::move(other._array));
                break;

            case value_type::object:
                new (&_object) literal::object<LogPtr>(std::move(other._object));
                break;
        }

        other.clear();
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline state<LogPtr>::state(const char* origin, const LogPtr& log)
        : diag_base(copy(origin), log)
        , _expect_property(false) {

        constexpr const char* suborigin = "state()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: origin='%s'", origin);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void state<LogPtr>::reset() noexcept {
        constexpr const char* suborigin = "reset()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        _expect_property = false;
        _nest_stack.c.clear();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline const std::stack<nest_type>& state<LogPtr>::nest_stack() const noexcept {
        return _nest_stack;
    }


    template <typename LogPtr>
    inline bool state<LogPtr>::expect_property() const noexcept {
        return _expect_property;
    }


    template <typename LogPtr>
    inline void state<LogPtr>::set_expect_property(bool expect) {
        constexpr const char* suborigin = "set_expect_property()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: expect=%u", expect);

        diag_base::expect(suborigin, !expect || (!_nest_stack.empty() && _nest_stack.top() == nest_type::object), __TAG__, "expect");

        _expect_property = expect;

        diag_base::ensure(suborigin, !_expect_property || (!_nest_stack.empty() && _nest_stack.top() == nest_type::object), __TAG__, "_expect_property");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void state<LogPtr>::nest(nest_type type) {
        constexpr const char* suborigin = "nest()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: type=%u", type);

        _nest_stack.push(type);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void state<LogPtr>::unnest(nest_type type) {
        constexpr const char* suborigin = "unnest()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: type=%u", type);

        diag_base::expect(!_nest_stack.empty() && _nest_stack.top() == type, __TAG__, "type");

        _nest_stack.pop();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline istream<LogPtr>::istream(std::streambuf* sb, const LogPtr& log)
        : base(sb)
        , state_base("abc::net::json::istream", log) {

        constexpr const char* suborigin = "istream()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline istream<LogPtr>::istream(istream&& other)
        : base(std::move(other))
        , state_base(std::move(other)) {
    }


    template <typename LogPtr>
    inline void istream<LogPtr>::skip_value() {
        constexpr const char* suborigin = "skip_value()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        std::size_t nest_stack_size = state_base::nest_stack.size();
        do {
            get_token(false);
        }
        while (state_base::nest_stack.size() > nest_stack_size);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline token istream<LogPtr>::get_token() {
        constexpr const char* suborigin = "get_token()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        token tok;
        bool trail_comma = true;

        skip_spaces();
        char ch = peek_char();

        if (state_base::expect_property()) {
            if (ch == '"') {
                tok.string = get_string();
                tok.type = token_type::property;

                skip_spaces();
                ch = peek_char();
                expect_char(ch, ':', true, suborigin, __TAG__);

                state_base::set_expect_property(false);
                trail_comma = false;
            }
            else {
                expect_char(ch, '}', true, suborigin, __TAG__);
                unnest(nest_type::object, suborigin, __TAG__);

                tok.type = token_type::end_object;

                state_base::set_expect_property(true);
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
                unnest(nest_type::array, suborigin, __TAG__);

                tok.type = token_type::end_array;
            }
            else if (ch == '{') {
                base::get();
                state_base::nest(nest_type::object);

                tok.type = token_type::begin_object;

                trail_comma = false;
            }
            else {
                diag_base::throw_exception(suborigin, __TAG__, "Unexpected ch=%c (\\u%4.4x)", ch, ch);
                base::set_bad();
            }

            state_base::set_expect_property(true);
        }

        if (trail_comma && state_base::nest_stack::size() > 0) {
            skip_spaces();

            ch = peek_char();
            if (ch == ',') {
                base::get();
            }
            else {
                if (state_base::expect_property()) {
                    expect_char(ch, '}', false, suborigin, __TAG__);
                }
                else {
                    expect_char(ch, ']', false, suborigin, __TAG__);
                }
            }
        }

        base::set_gcount(tok.string.length());

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: tok.type=%u, tok.string='%s'", tok.type, tok.string);

        return tok;
    }


    template <typename LogPtr>
    inline void istream<LogPtr>::unnest(nest_type type, const char* suborigin, diag::tag_t tag) {
        nest_type actual = nest_type::none;
        if (!state_base::nest_stack.empty()) {
            actual = state_base::nest_stack.top();
        }

        if (actual == type) {
            state_base::unnest(type);
        }
        else {
            base::set_bad();
            diag_base::throw_exception(suborigin, tag, "actual_nest_type=%u, expected_nest_type=%u", actual, type);
        }
    }


    template <typename LogPtr>
    inline literal::string istream<LogPtr>::get_string() {
        constexpr const char* suborigin = "get_string()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

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

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: str='%s'", str);

        return str;
    }


    template <typename LogPtr>
    inline literal::string istream<LogPtr>::get_number() {
        constexpr const char* suborigin = "get_number()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

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

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: str='%s'", str);

        return str;
    }


    template <typename LogPtr>
    inline literal::string istream<LogPtr>::get_literal(const char* literal) {
        constexpr const char* suborigin = "get_number()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: literal='%s'", literal);

        literal::string str;

        for (const char* itr = literal; *itr != '\0'; itr++) {
            char ch = peek_char();
            expect_char(ch, *itr, true, suborigin, __TAG__);

            str += ch;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: str='%s'", str);

        return str;
    }


    template <typename LogPtr>
    inline char istream<LogPtr>::get_escaped_char() {
        constexpr const char* suborigin = "get_escaped_char()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        char ch = peek_char();
        expect_char(ch, '\\', true, suborigin, __TAG__);

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
                diag_base::throw_exception(suborigin, __TAG__, "str='%s'", str);
            }
            else if (str[0] == '0' && str[1] == '0') {
                ch = (ascii::hex(str[2]) << 4) | ascii::hex(str[3]);
            }
            else {
                base::set_bad();
                diag_base::throw_exception(suborigin, __TAG__, "Wide chars not supported.");
            }
        }

        return ch;
    }


    template <typename LogPtr>
    inline void istream<LogPtr>::expect_char(char actual, char expected, bool should_get, const char* suborigin, diag::tag_t tag) {
        if (actual != expected) {
            base::set_bad();
            diag_base::throw_exception(suborigin, tag, "actual_char=%c (\\u%4.4x), expected_char=%c (\\u%4.4x)", actual, actual, expected, expected);
        }
        else if (should_get) {
            base::get();
        }
    }


    template <typename LogPtr>
    inline literal::string istream<LogPtr>::get_hex() {
        return get_chars(ascii::is_hex);
    }


    template <typename LogPtr>
    inline literal::string istream<LogPtr>::get_digits() {
        return get_chars(ascii::is_digit);
    }


    template <typename LogPtr>
    inline literal::string istream<LogPtr>::get_chars(ascii::predicate_t&& predicate) {
        literal::string str;

        while (base::is_good() && predicate(peek_char())) {
            str += base::get();
        }

        return str;
    }


    template <typename LogPtr>
    inline std::size_t istream<LogPtr>::skip_spaces() {
        return skip_chars(ascii::json::is_space);
    }


    template <typename LogPtr>
    inline std::size_t istream<LogPtr>::skip_chars(ascii::predicate_t&& predicate) {
        std::size_t gcount = 0;
        
        while (base::is_good() && predicate(peek_char())) {
            base::get();
            gcount++;
        }

        return gcount;
    }
    
    
    template <typename LogPtr>
    inline char istream<LogPtr>::get_char() {
        char ch = peek_char();

        if (base::is_good()) {
            base::get();
        }

        return ch;
    }


    template <typename LogPtr>
    inline char istream<LogPtr>::peek_char() {
        char ch = base::peek();

        if (!ascii::json::is_valid(ch)) {
            base::set_bad();
            ch = '\0';
        }

        return ch;
    }


    // --------------------------------------------------------------


#if 0 //// TODO:
    template <typename LogPtr>
    inline json_ostream<LogPtr>::json_ostream(std::streambuf* sb, Log* log)
        : base(sb)
        , state(log)
        , _skip_comma(false) {
        Log* log_local = state::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::json, severity::abc::debug, 0x10114, "json_ostream::json_ostream()");
        }
    }


    template <typename LogPtr>
    inline json_ostream<LogPtr>::json_ostream(json_ostream&& other)
        : base(std::move(other))
        , state(std::move(other))
        , _skip_comma(other._skip_comma) {
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_token(const json::token_t* buffer, std::size_t size) {
        Log* log_local = state::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::json, severity::abc::optional, 0x10115, "json_ostream::put_token() item='%4.4x' >>>", buffer->item);
        }

        switch (buffer->item)
        {
        case json::item::null:
            put_null();
            break;

        case json::item::boolean:
            put_boolean(buffer->value.boolean);
            break;

        case json::item::number:
            put_number(buffer->value.number);
            break;

        case json::item::string:
            put_string(buffer->value.string, size);
            break;

        case json::item::property:
            put_property(buffer->value.property, size);
            break;

        case json::item::begin_array:
            put_begin_array();
            break;

        case json::item::end_array:
            put_end_array();
            break;

        case json::item::begin_object:
            put_begin_object();
            break;

        case json::item::end_object:
            put_end_object();
            break;

        default:
            if (log_local != nullptr) {
                log_local->put_any(category::abc::json, severity::important, 0x10116, "json_ostream::put_token() Unexpected item=%4.4x <<<", buffer->item);
            }

            base::set_bad();
            break;
        }

        if (log_local != nullptr) {
            log_local->put_any(category::abc::json, severity::abc::debug, 0x10117, "json_ostream::put_token() <<<");
        }
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_space() {
        put_chars(" ", 1);
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_tab() {
        put_chars("\t", 1);
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_cr() {
        put_chars("\r", 1);
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_lf() {
        put_chars("\n", 1);
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_null() {
        if (state::expect_property()) {
            Log* log_local = state::log();
            if (log_local != nullptr) {
                log_local->put_any(category::abc::json, severity::important, 0x10118, "json_ostream::put_null() Expected a property.");
            }

            base::set_bad();
            return;
        }

        if (state::levels() > 0 && state::top_level() == json::level::array && !_skip_comma) {
            put_chars(",", 1);
        }

        put_chars("null", 4);

        _skip_comma = false;
        state::set_expect_property(true);
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_boolean(bool value) {
        if (state::expect_property()) {
            Log* log_local = state::log();
            if (log_local != nullptr) {
                log_local->put_any(category::abc::json, severity::important, 0x10119, "json_ostream::put_boolean() Expected a property.");
            }

            base::set_bad();
            return;
        }

        if (state::levels() > 0 && state::top_level() == json::level::array && !_skip_comma) {
            put_chars(",", 1);
        }

        if (value) {
            put_chars("true", 4);
        }
        else {
            put_chars("false", 5);
        }

        _skip_comma = false;
        state::set_expect_property(true);
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_number(double value) {
        if (state::expect_property()) {
            Log* log_local = state::log();
            if (log_local != nullptr) {
                log_local->put_any(category::abc::json, severity::important, 0x1011a, "json_ostream::put_number() Expected a property.");
            }

            base::set_bad();
            return;
        }

        if (state::levels() > 0 && state::top_level() == json::level::array && !_skip_comma) {
            put_chars(",", 1);
        }

        char literal[19 + 6 + 1];
        std::size_t size = std::snprintf(literal, sizeof(literal), "%.16lg", value);

        put_chars(literal, size);

        _skip_comma = false;
        state::set_expect_property(true);
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_string(const char* buffer, std::size_t size) {
        if (state::expect_property()) {
            Log* log_local = state::log();
            if (log_local != nullptr) {
                log_local->put_any(category::abc::json, severity::important, 0x1011b, "json_ostream::put_string() Expected a property.");
            }

            base::set_bad();
            return;
        }

        if (size == size::strlen) {
            size = std::strlen(buffer);
        }

        if (state::levels() > 0 && state::top_level() == json::level::array && !_skip_comma) {
            put_chars(",", 1);
        }

        put_chars("\"", 1);
        put_chars(buffer, size);
        put_chars("\"", 1);

        _skip_comma = false;
        state::set_expect_property(true);
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_property(const char* buffer, std::size_t size) {
        if (!state::expect_property()) {
            Log* log_local = state::log();
            if (log_local != nullptr) {
                log_local->put_any(category::abc::json, severity::important, 0x1011c, "json_ostream::put_property() Expected a value.");
            }

            base::set_bad();
            return;
        }

        if (size == size::strlen) {
            size = std::strlen(buffer);
        }

        if (state::levels() > 0 && state::top_level() == json::level::object && !_skip_comma) {
            put_chars(",", 1);
        }

        put_chars("\"", 1);
        put_chars(buffer, size);
        put_chars("\":", 2);

        _skip_comma = true;
        state::set_expect_property(false);
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_begin_array() {
        if (state::expect_property()) {
            Log* log_local = state::log();
            if (log_local != nullptr) {
                log_local->put_any(category::abc::json, severity::important, 0x1011d, "json_ostream::put_begin_array() Expected a property.");
            }

            base::set_bad();
            return;
        }

        if (state::levels() > 0 && state::top_level() == json::level::array && !_skip_comma) {
            put_chars(",", 1);
        }

        put_chars("[", 1);

        base::set_bad_if(!state::push_level(json::level::array));

        _skip_comma = true;
        state::set_expect_property(false);
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_end_array() {
        if (state::expect_property()) {
            Log* log_local = state::log();
            if (log_local != nullptr) {
                log_local->put_any(category::abc::json, severity::important, 0x1011e, "json_ostream::put_end_array() Expected a property.");
            }

            base::set_bad();
            return;
        }

        put_chars("]", 1);

        base::set_bad_if(!state::pop_level(json::level::array));

        _skip_comma = false;
        state::set_expect_property(true);
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_begin_object() {
        if (state::expect_property()) {
            Log* log_local = state::log();
            if (log_local != nullptr) {
                log_local->put_any(category::abc::json, severity::important, 0x1011f, "json_ostream::put_begin_object() Expected a property.");
            }

            base::set_bad();
            return;
        }

        if (state::levels() > 0 && state::top_level() == json::level::array && !_skip_comma) {
            put_chars(",", 1);
        }

        put_chars("{", 1);

        base::set_bad_if(!state::push_level(json::level::object));

        _skip_comma = true;
        state::set_expect_property(true);
    }


    template <typename LogPtr>
    inline void json_ostream<LogPtr>::put_end_object() {
        if (!state::expect_property()) {
            Log* log_local = state::log();
            if (log_local != nullptr) {
                log_local->put_any(category::abc::json, severity::important, 0x10120, "json_ostream::put_null() Expected a value.");
            }

            base::set_bad();
            return;
        }

        put_chars("}", 1);

        base::set_bad_if(!state::pop_level(json::level::object));

        _skip_comma = false;
        state::set_expect_property(true);
    }


    template <typename LogPtr>
    inline std::size_t json_ostream<LogPtr>::put_chars(const char* buffer, std::size_t size) {
        Log* log_local = state::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::json, severity::abc::optional, 0x10121, "json_ostream::put_chars() buffer='%s' >>>", buffer);
        }

        std::size_t pcount = 0;

        while (base::is_good() && pcount < size) {
            base::put(buffer[pcount++]);
        }

        if (pcount < size) {
            base::set_fail();
        }

        base::flush();

        if (log_local != nullptr) {
            log_local->put_any(category::abc::json, severity::abc::optional, 0x10122, "json_ostream::put_chars() pcount=%zu <<<", pcount);
        }

        return pcount;
    }


    template <typename LogPtr>
    inline std::size_t json_ostream<LogPtr>::put_char(char ch) {
        if (base::is_good()) {
            base::put(ch);
        }

        return base::is_good() ? 1 : 0;
    }


    // --------------------------------------------------------------
#endif

} } }
