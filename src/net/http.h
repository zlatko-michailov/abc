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

#include <cstring>

#include "../ascii.h"
#include "../util.h"
#include "../stream.h"
#include "../diag/diag_ready.h"
#include "i/http.i.h"


namespace abc { namespace net { namespace http {

    inline state::state(const char* origin, item next, diag::log_ostream* log) noexcept
        : diag_base(copy(origin), log)
        , _next(next) {

        constexpr const char* suborigin = "state()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: origin='%s' next=%u", origin, (unsigned)next);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void state::reset(item next) {
        constexpr const char* suborigin = "reset()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: next=%u", (unsigned)next);

        _next = next;

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline item state::next() const noexcept {
        return _next;
    }


    inline void state::assert_next(item item) {
        constexpr const char* suborigin = "assert_next()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: item=%u", (unsigned)item);

        diag_base::assert(suborigin, _next == item, __TAG__, "_next=%u, item=%u:", (unsigned)_next, (unsigned)item);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    inline istream::istream(const char* origin, std::streambuf* sb, item next, diag::log_ostream* log)
        : base(sb)
        , state_base(origin, next, log) {

        constexpr const char* suborigin = "istream()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: origin='%s', next=%u", origin, (unsigned)next);

        diag_base::expect(suborigin, sb != nullptr, __TAG__, "sb");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline istream::istream(istream&& other) noexcept
        : base(std::move(other))
        , state_base(std::move(other)) {
    }


    inline std::string istream::get_protocol() {
        constexpr const char* suborigin = "get_protocol()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        state_base::assert_next(item::protocol);

        // Format: HTTP/1.1

        // Read 'HTTP'
        constexpr std::size_t estimated_len = size::_8;
        std::string protocol = get_alphas(estimated_len);
        if (protocol.length() != 4 || !ascii::are_equal_i_n("HTTP", protocol.c_str(), 4)) {
            base::set_bad();
        }

        // Read '/'
        if (base::is_good()) {
            char ch = get_char();
            if (ch == '/') {
                protocol += '/';
            }
            else {
                base::set_bad();
            }
        }

        // Read '1'
        if (base::is_good()) {
            constexpr std::size_t estimated_len = size::_8;
            std::string digits = get_digits(estimated_len);
            if (!digits.empty()) {
                protocol += digits;
            }
            else {
                base::set_bad();
            }
        }

        // Read '.'
        if (base::is_good()) {
            char ch = get_char();
            if (ch == '.') {
                protocol += '.';
            }
            else {
                base::set_bad();
            }
        }

        // Read '1'
        if (base::is_good()) {
            constexpr std::size_t estimated_len = size::_8;
            std::string digits = get_digits(estimated_len);
            if (!digits.empty()) {
                protocol += digits;
            }
            else {
                base::set_bad();
            }
        }

        skip_spaces();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: protocol='%s'", protocol.c_str());

        return protocol;
    }


    inline headers istream::get_headers() {
        constexpr const char* suborigin = "get_headers()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        state_base::assert_next(item::header_name);

        std::size_t gcount = 0;
        headers headers;

        while (state_base::next() == item::header_name) {
            // Read header name.
            std::string header_name = get_header_name();
            gcount += header_name.length();

            if (state_base::next() == item::header_value) {
                // Read header value.
                std::string header_value = get_header_value();
                gcount += 1 + header_value.length() + 2; // : <value> CR LF

                // Insert the pair into the map.
                std::pair<std::string, std::string> pair(std::move(header_name), std::move(header_value));
                headers.insert(std::move(pair));
            }
        }

        set_gstate(gcount, item::body);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: headers.size()=%zu", headers.size());

        return headers;
    }


    inline std::string istream::get_header_name() {
        constexpr const char* suborigin = "get_header_name()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        state_base::assert_next(item::header_name);

        // Format: 'Name :'

        // Read Name
        constexpr std::size_t estimated_len = size::_256;
        std::string header_name = get_token(estimated_len);
        skip_spaces();

        // Headers end when there is a blank line, i.e. the name is empty.
        if (header_name.empty()) {
            skip_crlf();

            set_gstate(0, item::body);
            return header_name;
        }

        // Read ':'
        if (base::is_good()) {
            char ch = get_char();
            if (ch != ':') {
                base::set_bad();
            }
        }
        skip_spaces();

        set_gstate(header_name.length(), item::header_value);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: header_name='%s'", header_name.c_str());

        return header_name;
    }


    inline std::string istream::get_header_value() {
        constexpr const char* suborigin = "get_header_value()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        state_base::assert_next(item::header_value);

        // Format: prints CRLF[ prints CRLF[...]]

        std::string header_value;
        skip_spaces();

        bool has_more = true;

        // Multi-line
        while (has_more) {
            // One line
            while (has_more) {
                constexpr std::size_t estimated_len = size::k2;
                header_value += get_prints(estimated_len);

                has_more = base::is_good() && !header_value.empty() && skip_spaces() > 0 && ascii::is_abcprint(peek_char());
                if (has_more) {
                    header_value += ' ';
                }
            }

            skip_crlf();

            has_more = base::is_good() && !header_value.empty() && skip_spaces() > 0 && ascii::is_abcprint(peek_char());
            if (has_more) {
                header_value += ' ';
            }
        }

        skip_spaces();

        set_gstate(header_value.length(), item::header_name);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: header_value='%s'", header_value.c_str());

        return header_value;
    }


    inline std::string istream::get_body(std::size_t max_len) {
        constexpr const char* suborigin = "get_body()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: max_len=%zu", max_len);

        state_base::assert_next(item::body);

        constexpr std::size_t estimated_len = size::k4;
        std::string body = get_any_chars(estimated_len, max_len);

        set_gstate(body.length(), base::eof() ? item::eof : item::body);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: body='%s'", body.c_str());

        return body;
    }


    inline std::string istream::get_token(std::size_t estimated_len) {
        return get_chars(ascii::http::is_token, estimated_len);
    }


    inline std::string istream::get_prints(std::size_t estimated_len) {
        return get_chars(ascii::is_abcprint, estimated_len);
    }


    inline std::string istream::get_prints_and_spaces(std::size_t estimated_len) {
        return get_chars(ascii::is_abcprint_or_space, estimated_len);
    }


    inline std::string istream::get_alphas(std::size_t estimated_len) {
        return get_chars(ascii::is_alpha, estimated_len);
    }


    inline std::string istream::get_digits(std::size_t estimated_len) {
        return get_chars(ascii::is_digit, estimated_len);
    }


    inline std::string istream::get_any_chars(std::size_t estimated_len, std::size_t max_len) {
        return get_chars(ascii::is_any, estimated_len, max_len);
    }


    inline std::string istream::get_chars(ascii::predicate_t&& predicate, std::size_t estimated_len, std::size_t max_len) {
        std::string chars;
        chars.reserve(estimated_len);

        std::size_t len = 0;

        while (predicate(peek_char()) && base::is_good() && len++ < max_len) {
            chars.push_back(get_char());
        }

        return chars;
    }


    inline char istream::get_char() {
        char ch = peek_char();

        if (base::is_good()) {
            base::get();
        }

        return ch;
    }


    inline char istream::peek_char() {
        if (!base::is_good()) {
            return (char)std::char_traits<char>::eof(); 
        }

        base::int_type ch = base::peek();

        if (ch == std::char_traits<char>::eof()) {
            base::set_eof();
        }

        return (char)ch;
    }


    inline std::size_t istream::skip_spaces() {
        return skip_chars(ascii::is_space);
    }


    inline std::size_t istream::skip_crlf() {
        char ch = get_char();
        if (ch != '\r') {
            base::set_fail();
            return 0;
        }

        ch = get_char();
        if (ch != '\n') {
            base::set_fail();
            return 1;
        }

        return 2;
    }


    inline std::size_t istream::skip_chars(ascii::predicate_t&& predicate) {
        std::size_t gcount = 0;
        
        while (base::is_good() && predicate(peek_char())) {
            base::get();
            gcount++;
        }

        return gcount;
    }
    
    
    inline void istream::set_gstate(std::size_t gcount, item next) {
        constexpr const char* suborigin = "set_gstate()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: gcount=%zu, next=%u", gcount, (unsigned)next);

        base::set_gcount(gcount);
        state_base::reset(next);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    inline ostream::ostream(const char* origin, std::streambuf* sb, item next, diag::log_ostream* log)
        : base(sb)
        , state_base(origin, next, log) {

        constexpr const char* suborigin = "ostream()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: origin='%s', next=%u", origin, (unsigned)next);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline ostream::ostream(ostream&& other) noexcept
        : base(std::move(other))
        , state_base(std::move(other)) {
    }


    inline void ostream::put_headers(const headers& headers) {
        constexpr const char* suborigin = "put_headers()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        for (const headers::value_type& header : headers) {
            put_header_name(header.first.c_str());
            put_header_value(header.second.c_str());
        }

        end_headers();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void ostream::put_header_name(const char* header_name, std::size_t header_name_len) {
        constexpr const char* suborigin = "put_header_name()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: header_name='%s'", header_name);

        diag_base::expect(suborigin, header_name != nullptr, __TAG__, "header_name != nullptr");

        state_base::assert_next(item::header_name);

        if (header_name_len == size::strlen) {
            header_name_len = std::strlen(header_name);
        }

        std::size_t pcount = put_token(header_name, header_name_len);

        if (base::is_good()) {
            if (pcount < header_name_len) {
                base::set_bad();
            }
        }

        if (base::is_good()) {
            base::put(':');
            put_space();
        }

        set_pstate(item::header_value);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void ostream::put_header_value(const char* header_value, std::size_t header_value_len) {
        constexpr const char* suborigin = "put_header_value()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: header_value='%s'", header_value);

        diag_base::expect(suborigin, header_value != nullptr, __TAG__, "header_value != nullptr");

        state_base::assert_next(item::header_value);

        if (header_value_len == size::strlen) {
            header_value_len = std::strlen(header_value);
        }

        std::size_t pcount = 0;
        do {
            std::size_t sp = count_leading_spaces_in_header_value(header_value + pcount, header_value_len - pcount);

            if (pcount > 0 && sp > 0 && pcount + sp < header_value_len) {
                put_space();
            }

            pcount += sp;

            if (pcount < header_value_len) {
                if (ascii::is_abcprint(header_value[pcount])) {
                    pcount += put_prints(header_value + pcount, header_value_len - pcount);
                }
                else {
                    base::set_bad();
                }
            }
        }
        while (base::is_good() && pcount < header_value_len);

        put_crlf();

        set_pstate(item::header_name);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pcount=%zu", pcount);
    }


    inline void ostream::end_headers() {
        constexpr const char* suborigin = "put_header_value()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        state_base::assert_next(item::header_name);

        put_crlf();

        set_pstate(item::body);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void ostream::put_body(const char* body, std::size_t body_len) {
        constexpr const char* suborigin = "put_body()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::expect(suborigin, body != nullptr, __TAG__, "body != nullptr");

        state_base::assert_next(item::body);

        if (body_len == size::strlen) {
            body_len = std::strlen(body);
        }

        std::size_t pcount = put_any_chars(body, body_len);

        set_pstate(item::body);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pcount=%zu", pcount);
    }


    inline std::size_t ostream::put_protocol(const char* protocol, std::size_t protocol_len) {
        constexpr const char* suborigin = "put_protocol()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: protocol='%s'", protocol);

        diag_base::expect(suborigin, protocol != nullptr, __TAG__, "protocol != nullptr");

        state_base::assert_next(item::protocol);

        if (protocol_len == size::strlen) {
            protocol_len = std::strlen(protocol);
        }

        std::size_t pcount = 0;    

        if (protocol_len < 5 || !ascii::are_equal_i_n(protocol, "HTTP/", 5)) {
            base::set_bad();
        }
        else {
            pcount = put_any_chars("HTTP/", 5);
        }

        if (base::is_good() && pcount < protocol_len) {
            std::size_t pcount_local = put_digits(protocol + pcount, protocol_len - pcount);
            if (pcount_local == 0) {
                base::set_bad();
            }
            else {
                pcount += pcount_local;
            }
        }

        if (base::is_good() && pcount < protocol_len) {
            if (protocol[pcount] != '.') {
                base::set_bad();
            }
            else {
                base::put('.');
                pcount++;
            }
        }

        if (base::is_good() && pcount < protocol_len) {
            std::size_t pcount_local = put_digits(protocol + pcount, protocol_len - pcount);
            if (pcount_local == 0) {
                base::set_bad();
            }
            else {
                pcount += pcount_local;
            }
        }

        if (base::is_good() && pcount < protocol_len) {
            base::set_bad();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pcount=%zu", pcount);

        return pcount;
    }


    inline std::size_t ostream::put_token(const char* token, std::size_t token_len) {
        return put_chars(ascii::http::is_token, token, token_len);
    }


    inline std::size_t ostream::put_prints(const char* prints, std::size_t prints_len) {
        return put_chars(ascii::is_abcprint, prints, prints_len);
    }


    inline std::size_t ostream::put_prints_and_spaces(const char* prints_and_spaces, std::size_t prints_and_spaces_len) {
        return put_chars(ascii::is_abcprint_or_space, prints_and_spaces, prints_and_spaces_len);
    }


    inline std::size_t ostream::put_alphas(const char* alphas, std::size_t alphas_len) {
        return put_chars(ascii::is_alpha, alphas, alphas_len);
    }


    inline std::size_t ostream::put_digits(const char* digits, std::size_t digits_len) {
        return put_chars(ascii::is_digit, digits, digits_len);
    }


    inline std::size_t ostream::put_crlf() {
        return put_char('\r') + put_char('\n');
    }


    inline std::size_t ostream::put_space() {
        return put_char(' ');
    }


    inline std::size_t ostream::put_any_chars(const char* any_chars, std::size_t any_chars_len) {
        std::size_t pcount = 0;

        while (base::is_good() && pcount < any_chars_len) {
            base::put(any_chars[pcount++]);
        }

        return pcount;
    }


    inline std::size_t ostream::put_chars(ascii::predicate_t&& predicate, const char* chars, std::size_t chars_len) {
        constexpr const char* suborigin = "put_chars()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: chars='%s'", chars);

        diag_base::expect(suborigin, chars != nullptr, __TAG__, "chars != nullptr");

        std::size_t pcount = 0;

        if (chars_len == size::strlen) {
            chars_len = std::strlen(chars);
        }

        while (base::is_good() && pcount < chars_len && predicate(chars[pcount])) {
            base::put(chars[pcount++]);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pcount=%zu", pcount);

        return pcount;
    }


    inline std::size_t ostream::put_char(char ch) {
        if (base::is_good()) {
            base::put(ch);
        }

        return base::is_good() ? 1 : 0;;
    }


    inline std::size_t ostream::count_leading_spaces_in_header_value(const char* header_value, std::size_t header_value_len) {
        std::size_t sp = 0;

        while (sp < header_value_len) {
            if (ascii::is_space(header_value[sp])) {
                sp++;
            }
            else if (sp + 3 < header_value_len && header_value[sp] == '\r' && header_value[sp + 1] == '\n' && ascii::is_space(header_value[sp + 2])) {
                sp += 3;
            }
            else {
                break;
            }
        }

        return sp;
    }


    inline void ostream::set_pstate(item next) {
        constexpr const char* suborigin = "set_pstate()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: next=%u", (unsigned)next);

        base::flush();
        state_base::reset(next);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


   // --------------------------------------------------------------


    inline request_istream::request_istream(const char* origin, std::streambuf* sb, diag::log_ostream* log)
        : base(origin, sb, item::method, log) {

        constexpr const char* suborigin = "request_istream()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline request_istream::request_istream(std::streambuf* sb, diag::log_ostream* log)
        : request_istream("abc::net::http::request_istream", sb, log) {
    }


    inline request_istream::request_istream(request_istream&& other) noexcept
        : base(std::move(other)) {
    }


    inline void request_istream::reset() {
        constexpr const char* suborigin = "reset()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::set_gstate(0, item::method);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline std::string request_istream::get_method() {
        constexpr const char* suborigin = "get_method()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::assert_next(item::method);

        constexpr std::size_t estimated_len = size::_16;
        std::string method = base::get_token(estimated_len);
        base::skip_spaces();

        base::set_gstate(method.length(), item::resource);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: gcount=%zu, method='%s'", method.length(), method.c_str());

        return method;
    }


    inline resource request_istream::get_resource() {
        constexpr const char* suborigin = "get_resource()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::assert_next(item::resource);

        constexpr std::size_t estimated_len = size::k2;
        std::string raw_resource = base::get_prints(estimated_len);
        base::skip_spaces();

        base::set_gstate(raw_resource.length(), item::protocol);

        resource resource = split_resource(raw_resource);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: gcount=%zu, raw_resource='%s'", raw_resource.length(), raw_resource.c_str());

        return resource;
    }


    inline std::string request_istream::get_protocol() {
        constexpr const char* suborigin = "get_protocol()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        std::string protocol = base::get_protocol();
        base::skip_crlf();

        base::set_gstate(protocol.length(), item::header_name);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: gcount=%zu, protocol='%s'", protocol.length(), protocol.c_str());

        return protocol;
    }


    inline resource request_istream::split_resource(const std::string& raw_resource) {
        constexpr const char* suborigin = "split_resource()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: raw_resource='%s'", raw_resource.c_str());

        // Format: path?param1=...&param2=...#...

        resource resource;

        // Extract the path.
        std::string::size_type begin_pos = 0;

        std::string::size_type end_pos = raw_resource.find_first_of("?#");
        if (end_pos == std::string::npos) {
            end_pos = raw_resource.length();
        }

        resource.path = util::url_decode(raw_resource.c_str() + begin_pos, end_pos - begin_pos);

        while (end_pos < raw_resource.length()) {
            begin_pos = end_pos + 1;
            if (begin_pos >= raw_resource.length()) {
                break;
            }

            // Fragment
            if (raw_resource[end_pos] == '#') {
                end_pos = raw_resource.length();
                resource.fragment = util::url_decode(raw_resource.c_str() + begin_pos, end_pos - begin_pos);
                break;
            }

            // Query/parameters.
            end_pos = raw_resource.find_first_of("=&#", begin_pos);
            if (end_pos == std::string::npos) {
                end_pos = raw_resource.length();
            }

            // Parameter name.
            std::string param_name = util::url_decode(raw_resource.c_str() + begin_pos, end_pos - begin_pos);
            std::string param_value;

            // Parameter value.
            if (end_pos < raw_resource.length() && raw_resource[end_pos] == '=') {
                begin_pos = end_pos + 1;

                end_pos = raw_resource.find_first_of("&#", begin_pos);
                if (end_pos == std::string::npos) {
                    end_pos = raw_resource.length();
                }

                param_value = util::url_decode(raw_resource.c_str() + begin_pos, end_pos - begin_pos);
            }

            if (!param_name.empty()) {
                resource.query[std::move(param_name)] = std::move(param_value);
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        return resource;
    }


   // --------------------------------------------------------------


    inline request_reader::request_reader(const char* origin, std::streambuf* sb, diag::log_ostream* log)
        : base(origin, sb, log) {

        constexpr const char* suborigin = "request_reader()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline request_reader::request_reader(std::streambuf* sb, diag::log_ostream* log)
        : request_reader("abc::net::http::request_reader", sb, log) {
    }


    inline request_reader::request_reader(request_reader&& other) noexcept
        : base(std::move(other)) {
    }


    inline request request_reader::get_request() {
        constexpr const char* suborigin = "get_request()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        request request;

        request.method   = base::get_method();
        request.resource = base::get_resource();
        request.protocol = base::get_protocol();
        request.headers  = base::get_headers();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        return request;
    }


    inline std::string request_reader::get_body(std::size_t max_len) {
        return base::get_body(max_len);
    }


    inline std::streambuf* request_reader::rdbuf() const {
        return base::rdbuf();
    }


    // --------------------------------------------------------------


    inline request_ostream::request_ostream(const char* origin, std::streambuf* sb, diag::log_ostream* log)
        : base(origin, sb, item::method, log) {

        constexpr const char* suborigin = "request_ostream()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline request_ostream::request_ostream(std::streambuf* sb, diag::log_ostream* log)
        : request_ostream("abc::net::http::request_ostream", sb, log) {
    }


    inline request_ostream::request_ostream(request_ostream&& other) noexcept
        : base(std::move(other)) {
    }


    inline void request_ostream::reset() {
        constexpr const char* suborigin = "reset()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::set_pstate(item::method);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void request_ostream::put_method(const char* method, std::size_t method_len) {
        constexpr const char* suborigin = "put_method()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: method='%s'", method);

        diag_base::expect(suborigin, method != nullptr, __TAG__, "method != nullptr");

        base::assert_next(item::method);

        if (method_len == size::strlen) {
            method_len = std::strlen(method);
        }

        base::put_token(method, method_len);
        base::put_space();

        base::set_pstate(item::resource);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void request_ostream::put_resource(const resource& resource) {
        constexpr const char* suborigin = "put_resource()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::assert_next(item::resource);

        std::string url_encoded_path = util::url_encode(resource.path.c_str(), resource.path.length());
        base::put_any_chars(url_encoded_path.c_str(), url_encoded_path.length());

        if (!resource.query.empty()) {
            base::put_char('?');

            bool is_first = true;
            for (auto query_itr = resource.query.cbegin(); query_itr != resource.query.cend(); query_itr++ ) {
                if (!is_first) {
                    base::put_char('&');
                }
                is_first = false;

                std::string url_encoded_parameter_name = util::url_encode(query_itr->first.c_str(), query_itr->first.length());
                base::put_any_chars(url_encoded_parameter_name.c_str(), url_encoded_parameter_name.length());

                if (!query_itr->second.empty()) {
                    base::put_char('=');

                    std::string url_encoded_parameter_value = util::url_encode(query_itr->second.c_str(), query_itr->second.length());
                    base::put_any_chars(url_encoded_parameter_value.c_str(), url_encoded_parameter_value.length());
                }
            }
        }

        if (!resource.fragment.empty()) {
            base::put_char('#');
            std::string url_encoded_fragment = util::url_encode(resource.fragment.c_str(), resource.fragment.length());
            base::put_any_chars(url_encoded_fragment.c_str(), url_encoded_fragment.length());
        }

        base::put_space();

        base::set_pstate(item::protocol);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void request_ostream::put_resource(const char* resource, std::size_t resource_len) {
        constexpr const char* suborigin = "put_method()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: resource='%s'", resource);

        diag_base::expect(suborigin, resource != nullptr, __TAG__, "resource != nullptr");

        base::assert_next(item::resource);

        if (resource_len == size::strlen) {
            resource_len = std::strlen(resource);
        }

        base::put_prints(resource, resource_len);
        base::put_space();

        base::set_pstate(item::protocol);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void request_ostream::put_protocol(const char* protocol, std::size_t protocol_len) {
        constexpr const char* suborigin = "put_protocol()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: protocol='%s'", protocol);

        diag_base::expect(suborigin, protocol != nullptr, __TAG__, "protocol != nullptr");

        base::put_protocol(protocol, protocol_len);
        base::put_crlf();

        base::set_pstate(item::header_name);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


   // --------------------------------------------------------------


    inline request_writer::request_writer(const char* origin, std::streambuf* sb, diag::log_ostream* log)
        : base(origin, sb, log) {

        constexpr const char* suborigin = "request_writer()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline request_writer::request_writer(std::streambuf* sb, diag::log_ostream* log)
        : request_writer("abc::net::http::request_writer", sb, log) {
    }


    inline request_writer::request_writer(request_writer&& other) noexcept
        : base(std::move(other)) {
    }


    inline void request_writer::put_request(const request& request) {
        constexpr const char* suborigin = "put_request()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::put_method(request.method.c_str(), request.method.length());
        base::put_resource(request.resource);
        base::put_protocol(request.protocol.c_str(), request.protocol.length());
        base::put_headers(request.headers);

        base::flush();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void request_writer::put_body(const char* body, std::size_t body_len) {
        base::put_body(body, body_len);

        base::flush();
    }


    inline std::streambuf* request_writer::rdbuf() const {
        return base::rdbuf();
    }


    // --------------------------------------------------------------


    inline response_istream::response_istream(const char* origin, std::streambuf* sb, diag::log_ostream* log)
        : base(origin, sb, item::protocol, log) {

        constexpr const char* suborigin = "response_istream()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline response_istream::response_istream(std::streambuf* sb, diag::log_ostream* log)
        : response_istream("abc::net::http::response_istream", sb, log) {
    }


    inline response_istream::response_istream(response_istream&& other) noexcept
        : base(std::move(other)) {
    }


    inline void response_istream::reset() {
        constexpr const char* suborigin = "reset()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::set_gstate(0, item::protocol);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline std::string response_istream::get_protocol() {
        constexpr const char* suborigin = "get_protocol()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        std::string protocol = base::get_protocol();

        base::set_gstate(protocol.length(), item::status_code);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: gcount=%zu, protocol='%s'", protocol.length(), protocol.c_str());

        return protocol;
    }


    inline status_code_t response_istream::get_status_code() {
        constexpr const char* suborigin = "get_status_code()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::assert_next(item::status_code);

        constexpr std::size_t estimated_len = size::_8;
        std::string digits = base::get_digits(estimated_len);
        base::skip_spaces();

        status_code_t status_code = static_cast<status_code_t>(std::stoul(digits));

        base::set_gstate(digits.length(), item::reason_phrase);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: gcount=%zu, status_code='%u'", digits.length(), (unsigned)status_code);

        return status_code;
    }


    inline std::string response_istream::get_reason_phrase() {
        constexpr const char* suborigin = "get_reason_phrase()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::assert_next(item::reason_phrase);

        constexpr std::size_t estimated_len = size::_256;
        std::string reason_phrase = base::get_prints_and_spaces(estimated_len);
        base::skip_spaces();
        base::skip_crlf();

        base::set_gstate(reason_phrase.length(), item::header_name);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: gcount=%zu, reason_phrase='%s'", reason_phrase.length(), reason_phrase.c_str());

        return reason_phrase;
    }


    // --------------------------------------------------------------


    inline response_reader::response_reader(const char* origin, std::streambuf* sb, diag::log_ostream* log)
        : base(origin, sb, log) {

        constexpr const char* suborigin = "response_reader()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline response_reader::response_reader(std::streambuf* sb, diag::log_ostream* log)
        : response_reader("abc::net::http::response_reader", sb, log) {
    }


    inline response_reader::response_reader(response_reader&& other) noexcept
        : base(std::move(other)) {
    }


    inline response response_reader::get_response() {
        constexpr const char* suborigin = "get_response()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        response response;

        response.protocol      = base::get_protocol();
        response.status_code   = base::get_status_code();
        response.reason_phrase = base::get_reason_phrase();
        response.headers       = base::get_headers();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        return response;
    }


    inline std::string response_reader::get_body(std::size_t max_len) {
        return base::get_body(max_len);
    }


    inline std::streambuf* response_reader::rdbuf() const {
        return base::rdbuf();
    }


    // --------------------------------------------------------------


    inline response_ostream::response_ostream(const char* origin, std::streambuf* sb, diag::log_ostream* log)
        : base(origin, sb, item::protocol, log) {

        constexpr const char* suborigin = "response_ostream()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline response_ostream::response_ostream(std::streambuf* sb, diag::log_ostream* log)
        : response_ostream("abc::net::http::response_ostream", sb, log) {
    }


    inline response_ostream::response_ostream(response_ostream&& other) noexcept
        : base(std::move(other)) {
    }


    inline void response_ostream::reset() {
        constexpr const char* suborigin = "reset()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::set_pstate(item::protocol);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void response_ostream::put_protocol(const char* protocol, std::size_t protocol_len) {
        constexpr const char* suborigin = "put_protocol()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: protocol='%s'", protocol);

        diag_base::expect(suborigin, protocol != nullptr, __TAG__, "protocol != nullptr");

        base::put_protocol(protocol, protocol_len);
        base::put_space();

        base::set_pstate(item::status_code);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void response_ostream::put_status_code(status_code_t status_code) {
        constexpr const char* suborigin = "put_status_code()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: status_code='%u'", (unsigned)status_code);

        base::assert_next(item::status_code);

        char digits[12];
        std::snprintf(digits, sizeof(digits) * sizeof(char), "%u", (unsigned)status_code);

        base::put_digits(digits, std::strlen(digits));
        base::put_space();

        base::set_pstate(item::reason_phrase);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void response_ostream::put_reason_phrase(const char* reason_phrase, std::size_t reason_phrase_len) {
        constexpr const char* suborigin = "put_reason_phrase()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: reason_phrase='%s'", reason_phrase);

        base::assert_next(item::reason_phrase);

        if (reason_phrase != nullptr) {
            if (reason_phrase_len == size::strlen) {
                reason_phrase_len = std::strlen(reason_phrase);
            }

            base::put_prints_and_spaces(reason_phrase, reason_phrase_len);
        }

        base::put_crlf();

        base::set_pstate(item::header_name);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    inline response_writer::response_writer(const char* origin, std::streambuf* sb, diag::log_ostream* log)
        : base(origin, sb, log) {

        constexpr const char* suborigin = "response_writer()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline response_writer::response_writer(std::streambuf* sb, diag::log_ostream* log)
        : response_writer("abc::net::http::response_writer", sb, log) {
    }


    inline response_writer::response_writer(response_writer&& other) noexcept
        : base(std::move(other)) {
    }


    inline void response_writer::put_response(const response& response) {
        constexpr const char* suborigin = "put_response()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::put_protocol(response.protocol.c_str(), response.protocol.length());
        base::put_status_code(response.status_code);
        base::put_reason_phrase(response.reason_phrase.c_str(), response.reason_phrase.length());
        base::put_headers(response.headers);

        base::flush();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void response_writer::put_body(const char* body, std::size_t body_len) {
        base::put_body(body, body_len);

        base::flush();
    }


    inline std::streambuf* response_writer::rdbuf() const {
        return base::rdbuf();
    }


    // --------------------------------------------------------------


    inline client::client(std::streambuf* sb, diag::log_ostream* log)
        : request_writer(sb, log)
        , response_reader(sb, log) {
    }


    inline client::client(client&& other) noexcept
        : request_writer(std::move(other))
        , response_reader(std::move(other)) {
    }


    // --------------------------------------------------------------


    inline server::server(std::streambuf* sb, diag::log_ostream* log)
        : request_reader(sb, log)
        , response_writer(sb, log) {
    }


    inline server::server(server&& other) noexcept
        : request_reader(std::move(other))
        , response_writer(std::move(other)) {
    }


    // --------------------------------------------------------------


    inline std::string util::url_encode(const char* chars, std::size_t chars_len) {
        if (chars == nullptr) {
            return std::string();
        }

        if (chars_len == size::strlen) {
            chars_len = std::strlen(chars);
        }

        std::string result;

        for (std::size_t i = 0; i < chars_len; i++) {
            char ch = chars[i];

            if (!ascii::http::is_url_safe(ch)) {
                result += '%';
                result += ascii::to_digit16((ch >> 4) & 0xF);
                result += ascii::to_digit16(ch & 0xF);
            }
            else {
                result += ch;
            }
        }

        return result;
    }


    inline std::string util::url_decode(const char* chars, std::size_t chars_len) {
        if (chars == nullptr) {
            return std::string();
        }

        if (chars_len == size::strlen) {
            chars_len = std::strlen(chars);
        }

        std::string result;

        for (std::size_t i = 0; i < chars_len; i++) {
            char ch = chars[i];

            if (ch == '%' && (i + 2 < chars_len) && ascii::is_hex(chars[i + 1]) && ascii::is_hex(chars[i + 2])) {
                ch = static_cast<char>((ascii::hex(chars[i + 1]) << 4) | ascii::hex(chars[i + 2]));
                i += 2;
            }

            result += ch;
        }

        return result;
    }


} } }

