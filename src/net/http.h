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
#include "../stream.h"
#include "i/http.i.h"


namespace abc { namespace net {

    template <typename LogPtr>
    inline http_state<LogPtr>::http_state(const char* origin, http::item_t next, const LogPtr& log) noexcept
        : diag_base(copy(origin), log)
        , _next(next) {

        constexpr const char* suborigin = "http_state()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: origin='%s' next=%u", origin, (unsigned)next);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void http_state<LogPtr>::reset(http::item_t next) {
        constexpr const char* suborigin = "reset()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: next=%u", (unsigned)next);

        _next = next;

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline http::item_t http_state<LogPtr>::next() const noexcept {
        return _next;
    }


    template <typename LogPtr>
    inline void http_state<LogPtr>::assert_next(http::item_t item) {
        constexpr const char* suborigin = "assert_next()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: item=%u", (unsigned)item);

        diag_base::assert(suborigin, _next == item, __TAG__, "_next=%u, item=%u:", (unsigned)_next, (unsigned)item);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline http_istream<LogPtr>::http_istream(const char* origin, std::streambuf* sb, http::item_t next, const LogPtr& log)
        : base(sb)
        , state(origin, next, log) {

        constexpr const char* suborigin = "http_istream()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: origin='%s', next=%u", origin, (unsigned)next);

        diag_base::expect(suborigin, sb != nullptr, __TAG__, "sb");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline http_istream<LogPtr>::http_istream(http_istream&& other)
        : base(std::move(other))
        , state(std::move(other)) {
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_protocol() {
        constexpr const char* suborigin = "get_protocol()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        state::assert_next(http::item::protocol);

        // Format: HTTP/1.1

        // Read 'HTTP'
        std::string protocol = get_alphas();
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
            std::string digits = get_digits();
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
            std::string digits = get_digits();
            if (!digits.empty()) {
                protocol += digits;
            }
            else {
                base::set_bad();
            }
        }

        skip_spaces();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: protocol='%s'", protocol.c_str());

        return std::move(protocol);
    }


    template <typename LogPtr>
    inline http_headers http_istream<LogPtr>::get_headers() {
        constexpr const char* suborigin = "get_headers()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        state::assert_next(http::item::header_name);

        std::size_t gcount = 0;
        http_headers headers;

        while (state::next() == http::item::header_name) {
            // Read header name.
            std::string header_name = get_header_name();
            gcount += header_name.length();

            if (state::next() == http::item::header_value) {
                // Read header value.
                std::string header_value = get_header_value();
                gcount += 1 + header_value.length() + 2; // : <value> CR LF

                // Insert the pair into the map.
                std::pair<std::string, std::string> pair(std::move(header_name), std::move(header_value));
                headers.insert(std::move(pair));
            }
        }

        set_gstate(gcount, http::item::body);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: headers.size()=%zu", headers.size());

        return std::move(headers);
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_header_name() {
        constexpr const char* suborigin = "get_header_name()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        state::assert_next(http::item::header_name);

        // Format: 'Name :'

        // Read Name
        std::string header_name = get_token();
        skip_spaces();

        // Headers end when there is a blank line, i.e. the name is empty.
        if (header_name.empty()) {
            skip_crlf();

            set_gstate(0, http::item::body);
            return std::move(header_name);
        }

        // Read ':'
        if (base::is_good()) {
            char ch = get_char();
            if (ch != ':') {
                base::set_bad();
            }
        }
        skip_spaces();

        set_gstate(header_name.length(), http::item::header_value);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: header_name='%s'", header_name.c_str());

        return std::move(header_name);
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_header_value() {
        constexpr const char* suborigin = "get_header_value()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        state::assert_next(http::item::header_value);

        // Format: prints CRLF[ prints CRLF[...]]

        std::string header_value;
        skip_spaces();

        bool has_more = true;

        // Multi-line
        while (has_more) {
            // One line
            while (has_more) {
                header_value += get_prints();

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

        set_gstate(header_value.length(), http::item::header_name);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: header_value='%s'", header_value.c_str());

        return std::move(header_value);
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_body(std::size_t max_len) {
        constexpr const char* suborigin = "get_body()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: max_len=%zu", max_len);

        state::assert_next(http::item::body);

        std::string body = get_any_chars(max_len);

        set_gstate(body.length(), base::eof() ? http::item::eof : http::item::body);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: body='%s'", body.c_str());

        return std::move(body);
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_token() {
        return get_chars(ascii::http::is_token);
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_prints() {
        return get_chars(ascii::is_abcprint);
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_prints_and_spaces() {
        return get_chars(ascii::is_abcprint_or_space);
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_alphas() {
        return get_chars(ascii::is_alpha);
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_digits() {
        return get_chars(ascii::is_digit);
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_any_chars(std::size_t max_len) {
        return get_chars(ascii::is_any, max_len);
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_chars(ascii::predicate_t&& predicate, std::size_t max_len) {
        std::string chars;
        std::size_t len = 0;

        while (base::is_good() && predicate(peek_char()) && len++ < max_len) {
            chars += get_char();
        }

        return std::move(chars);
    }


    template <typename LogPtr>
    inline char http_istream<LogPtr>::get_char() {
        char ch = peek_char();

        if (base::is_good()) {
            base::get();
        }

        return ch;
    }


    template <typename LogPtr>
    inline char http_istream<LogPtr>::peek_char() {
        char ch = base::peek();

        if (!ascii::is_ascii(ch)) {
            base::set_bad();
            base::set_eof();
            ch = '\0';
        }

        return ch;
    }


    template <typename LogPtr>
    inline std::size_t http_istream<LogPtr>::skip_spaces() {
        return skip_chars(ascii::is_space);
    }


    template <typename LogPtr>
    inline std::size_t http_istream<LogPtr>::skip_crlf() {
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


    template <typename LogPtr>
    inline std::size_t http_istream<LogPtr>::skip_chars(ascii::predicate_t&& predicate) {
        std::size_t gcount = 0;
        
        while (base::is_good() && predicate(peek_char())) {
            base::get();
            gcount++;
        }

        return gcount;
    }
    
    
    template <typename LogPtr>
    inline void http_istream<LogPtr>::set_gstate(std::size_t gcount, http::item_t next) {
        constexpr const char* suborigin = "set_gstate()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: gcount=%zu, next=%u", gcount, (unsigned)next);

        base::set_gcount(gcount);
        state::reset(next);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline http_ostream<LogPtr>::http_ostream(const char* origin, std::streambuf* sb, http::item_t next, const LogPtr& log)
        : base(sb)
        , state(origin, next, log) {

        constexpr const char* suborigin = "http_ostream()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: origin='%s', next=%u", origin, (unsigned)next);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline http_ostream<LogPtr>::http_ostream(http_ostream&& other)
        : base(std::move(other))
        , state(std::move(other)) {
    }


    template <typename LogPtr>
    inline void http_ostream<LogPtr>::put_headers(const http_headers& headers) {
        constexpr const char* suborigin = "put_headers()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        for (const http_headers::value_type& header : headers) {
            put_header_name(header.first);
            put_header_value(header.second);
        }

        end_headers();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void http_ostream<LogPtr>::put_header_name(const char* header_name, std::size_t header_name_len) {
        constexpr const char* suborigin = "put_header_name()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: header_name='%s'", header_name);

        diag_base::expect(suborigin, header_name != nullptr, __TAG__, "header_name != nullptr");

        state::assert_next(http::item::header_name);

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

        set_pstate(http::item::header_value);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void http_ostream<LogPtr>::put_header_value(const char* header_value, std::size_t header_value_len) {
        constexpr const char* suborigin = "put_header_value()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: header_value='%s'", header_value);

        diag_base::expect(suborigin, header_value != nullptr, __TAG__, "header_value != nullptr");

        state::assert_next(http::item::header_value);

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

        set_pstate(http::item::header_name);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pcount=%zu", pcount);
    }


    template <typename LogPtr>
    inline void http_ostream<LogPtr>::end_headers() {
        constexpr const char* suborigin = "put_header_value()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        state::assert_next(http::item::header_name);

        put_crlf();

        set_pstate(http::item::body);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void http_ostream<LogPtr>::put_body(const char* body, std::size_t body_len) {
        constexpr const char* suborigin = "put_body()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::expect(suborigin, body != nullptr, __TAG__, "body != nullptr");

        state::assert_next(http::item::body);

        if (body_len == size::strlen) {
            body_len = std::strlen(body);
        }

        std::size_t pcount = put_any_chars(body, body_len);

        set_pstate(http::item::body);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: pcount=%zu", pcount);
    }


    template <typename LogPtr>
    inline std::size_t http_ostream<LogPtr>::put_protocol(const char* protocol, std::size_t protocol_len) {
        constexpr const char* suborigin = "put_protocol()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: protocol='%s'", protocol);

        diag_base::expect(suborigin, protocol != nullptr, __TAG__, "protocol != nullptr");

        state::assert_next(http::item::protocol);

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


    template <typename LogPtr>
    inline std::size_t http_ostream<LogPtr>::put_token(const char* token, std::size_t token_len) {
        return put_chars(ascii::http::is_token, token, token_len);
    }


    template <typename LogPtr>
    inline std::size_t http_ostream<LogPtr>::put_prints(const char* prints, std::size_t prints_len) {
        return put_chars(ascii::is_abcprint, prints, prints_len);
    }


    template <typename LogPtr>
    inline std::size_t http_ostream<LogPtr>::put_prints_and_spaces(const char* prints_and_spaces, std::size_t prints_and_spaces_len) {
        return put_chars(ascii::is_abcprint_or_space, prints_and_spaces, prints_and_spaces_len);
    }


    template <typename LogPtr>
    inline std::size_t http_ostream<LogPtr>::put_alphas(const char* alphas, std::size_t alphas_len) {
        return put_chars(ascii::is_alpha, alphas, alphas_len);
    }


    template <typename LogPtr>
    inline std::size_t http_ostream<LogPtr>::put_digits(const char* digits, std::size_t digits_len) {
        return put_chars(ascii::is_digit, digits, digits_len);
    }


    template <typename LogPtr>
    inline std::size_t http_ostream<LogPtr>::put_crlf() {
        return put_char('\r') + put_char('\n');
    }


    template <typename LogPtr>
    inline std::size_t http_ostream<LogPtr>::put_space() {
        return put_char(' ');
    }


    template <typename LogPtr>
    inline std::size_t http_ostream<LogPtr>::put_any_chars(const char* any_chars, std::size_t any_chars_len) {
        std::size_t pcount = 0;

        while (base::is_good() && pcount < any_chars_len) {
            base::put(any_chars[pcount++]);
        }

        return pcount;
    }


    template <typename LogPtr>
    inline std::size_t http_ostream<LogPtr>::put_chars(ascii::predicate_t&& predicate, const char* chars, std::size_t chars_len) {
        std::size_t pcount = 0;

        while (base::is_good() && pcount < chars_len && predicate(chars[pcount])) {
            base::put(chars[pcount++]);
        }

        return pcount;
    }


    template <typename LogPtr>
    inline std::size_t http_ostream<LogPtr>::put_char(char ch) {
        if (base::is_good()) {
            base::put(ch);
        }

        return base::is_good() ? 1 : 0;;
    }


    template <typename LogPtr>
    inline std::size_t http_ostream<LogPtr>::count_leading_spaces_in_header_value(const char* header_value, std::size_t header_value_len) {
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


    //// TODO:
    /*template <typename LogPtr>
    inline std::size_t http_ostream<LogPtr>::count_leading_spaces(const char* content, std::size_t content_len) {
        std::size_t sp = 0;

        while (sp < content_len && ascii::is_space(content[sp])) {
            sp++;
        }

        return sp;
    }*/


    template <typename LogPtr>
    inline void http_ostream<LogPtr>::set_pstate(http::item_t next) {
        constexpr const char* suborigin = "set_pstate()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: next=%u", (unsigned)next);

        base::flush();
        state::reset(next);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


   // --------------------------------------------------------------


    template <typename LogPtr>
    inline http_request_istream<LogPtr>::http_request_istream(std::streambuf* sb, const LogPtr& log)
        : base("abc:net::http_request_istream", sb, http::item::method, log) {

        constexpr const char* suborigin = "http_request_istream()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline http_request_istream<LogPtr>::http_request_istream(http_request_istream&& other)
        : base(std::move(other)) {
    }


    template <typename LogPtr>
    inline void http_request_istream<LogPtr>::reset() {
        constexpr const char* suborigin = "reset()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::reset(http::item::method);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline std::string http_request_istream<LogPtr>::get_method() {
        constexpr const char* suborigin = "get_method()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::assert_next(http::item::method);

        std::string method = base::get_token();
        base::skip_spaces();

        base::set_gstate(method.length(), http::item::resource);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: gcount=%zu, method='%s'", method.length(), method.c_str());

        return std::move(method);
    }


    template <typename LogPtr>
    inline http_resource http_request_istream<LogPtr>::get_resource() {
        constexpr const char* suborigin = "get_resource()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::assert_next(http::item::resource);

        std::string raw_resource = base::get_prints();
        base::skip_spaces();

        base::set_gstate(raw_resource.length(), http::item::protocol);

        http_resource resource = split_resource(raw_resource);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: gcount=%zu, raw_resource='%s'", raw_resource.length(), raw_resource.c_str());

        return std::move(resource);
    }


    template <typename LogPtr>
    inline std::string http_request_istream<LogPtr>::get_protocol() {
        constexpr const char* suborigin = "get_protocol()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        std::string protocol = base::get_protocol();
        base::skip_crlf();

        base::set_gstate(protocol.length(), http::item::header_name);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: gcount=%zu, protocol='%s'", protocol.length(), protocol.c_str());

        return std::move(protocol);
    }


    template <typename LogPtr>
    inline http_resource http_request_istream<LogPtr>::split_resource(const std::string& raw_resource) {
        constexpr const char* suborigin = "split_resource()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: raw_resource='%s'", raw_resource.c_str());

        // Format: path?param1=...&param2=...

        http_resource resource;

        // Extract the path.
        std::string::size_type end_pos = raw_resource.find('?');
        resource.path = raw_resource.substr(0, end_pos);

        while (end_pos != std::string::npos) {
            std::string::size_type begin_pos = ++end_pos;
            end_pos = raw_resource.find_first_of("=&", begin_pos);

            if (end_pos != std::string::npos) {
                std::string param_name = raw_resource.substr(begin_pos, end_pos - begin_pos);

                std::string param_value;
                if (raw_resource[end_pos] == '=') {
                    begin_pos = ++end_pos;
                    end_pos = raw_resource.find('&', begin_pos);
                    param_value = raw_resource.substr(begin_pos, end_pos - begin_pos);
                }

                resource.parameters[std::move(param_name)] = std::move(param_value);
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        return std::move(resource);
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline http_request_ostream<LogPtr>::http_request_ostream(std::streambuf* sb, const LogPtr& log)
        : base("abc:net::http_request_ostream", sb, http::item::method, log) {

        constexpr const char* suborigin = "http_request_ostream()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline http_request_ostream<LogPtr>::http_request_ostream(http_request_ostream&& other)
        : base(std::move(other)) {
    }


    template <typename LogPtr>
    inline void http_request_ostream<LogPtr>::reset() {
        constexpr const char* suborigin = "reset()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::reset(http::item::method);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void http_request_ostream<LogPtr>::put_method(const char* method, std::size_t method_len) {
        constexpr const char* suborigin = "put_method()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: method='%s'", method);

        diag_base::expect(suborigin, method != nullptr, __TAG__, "method != nullptr");

        base::assert_next(http::item::method);

        if (method_len == size::strlen) {
            method_len = std::strlen(method);
        }

        std::size_t pcount = base::put_token(method, method_len);
        base::put_space();

        base::set_pstate(http::item::resource);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void http_request_ostream<LogPtr>::put_resource(const char* resource, std::size_t resource_len) {
        constexpr const char* suborigin = "put_method()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: resource='%s'", resource);

        diag_base::expect(suborigin, resource != nullptr, __TAG__, "resource != nullptr");

        base::assert_next(http::item::resource);

        if (resource_len == size::strlen) {
            resource_len = std::strlen(resource);
        }

        std::size_t pcount = base::put_prints(resource, resource_len);
        base::put_space();

        base::set_pstate(http::item::protocol);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void http_request_ostream<LogPtr>::put_protocol(const char* protocol, std::size_t protocol_len) {
        constexpr const char* suborigin = "put_protocol()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: protocol='%s'", protocol);

        diag_base::expect(suborigin, protocol != nullptr, __TAG__, "protocol != nullptr");

        base::put_protocol(protocol, protocol_len);
        base::put_crlf();

        base::set_pstate(http::item::header_name);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline http_response_istream<LogPtr>::http_response_istream(std::streambuf* sb, const LogPtr& log)
        : base("abc:net::http_response_istream", sb, http::item::protocol, log) {

        constexpr const char* suborigin = "http_response_istream()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline http_response_istream<LogPtr>::http_response_istream(http_response_istream&& other)
        : base(std::move(other)) {
    }


    template <typename LogPtr>
    inline void http_response_istream<LogPtr>::reset() {
        constexpr const char* suborigin = "reset()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::reset(http::item::protocol);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline std::string http_response_istream<LogPtr>::get_protocol() {
        constexpr const char* suborigin = "get_protocol()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        std::string protocol = base::get_protocol();

        base::set_gstate(protocol.length(), http::item::status_code);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: gcount=%zu, protocol='%s'", protocol.length(), protocol.c_str());

        return std::move(protocol);
    }

    template <typename LogPtr>
    inline http_status_code http_response_istream<LogPtr>::get_status_code() {
        constexpr const char* suborigin = "get_status_code()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::assert_next(http::item::status_code);

        std::string digits = base::get_digits();
        base::skip_spaces();

        http_status_code status_code = static_cast<http_status_code>(std::stoul(digits));

        base::set_gstate(digits.length, http::item::reason_phrase);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: gcount=%zu, status_code='%u'", digits.length(), status_code);

        return status_code;
    }


    template <typename LogPtr>
    inline std::string http_response_istream<LogPtr>::get_reason_phrase() {
        constexpr const char* suborigin = "get_reason_phrase()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::assert_next(http::item::reason_phrase);

        std::string reason_phrase = base::get_prints_and_spaces();
        base::skip_spaces();
        base::skip_crlf();

        base::set_gstate(reason_phrase.length(), http::item::header_name);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: gcount=%zu, reason_phrase='%s'", reason_phrase.length(), reason_phrase.c_str());

        return std::move(reason_phrase);
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline http_response_ostream<LogPtr>::http_response_ostream(std::streambuf* sb, const LogPtr& log)
        : base("abc:net::http_response_ostream", sb, http::item::protocol, log) {

        constexpr const char* suborigin = "http_response_ostream()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline http_response_ostream<LogPtr>::http_response_ostream(http_response_ostream&& other)
        : base(std::move(other)) {
    }


    template <typename LogPtr>
    inline void http_response_ostream<LogPtr>::reset() {
        constexpr const char* suborigin = "reset()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::reset(http::item::protocol);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void http_response_ostream<LogPtr>::put_protocol(const char* protocol, std::size_t protocol_len) {
        constexpr const char* suborigin = "put_protocol()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: protocol='%s'", protocol);

        diag_base::expect(suborigin, protocol != nullptr, __TAG__, "protocol != nullptr");

        base::put_protocol(protocol, protocol_len);
        base::put_space();

        base::set_pstate(http::item::status_code);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void http_response_ostream<LogPtr>::put_status_code(http_status_code status_code) {
        constexpr const char* suborigin = "put_status_code()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: status_code='%u'", (unsigned)status_code);

        base::assert_next(http::item::status_code);

        char digits[12];
        std::snprintf(digits, sizeof(digits) * sizeof(char), "%u", (unsigned)status_code);

        std::size_t pcount = base::put_digits(digits, std::strlen(digits));
        base::put_space();

        base::set_pstate(http::item::reason_phrase);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void http_response_ostream<LogPtr>::put_reason_phrase(const char* reason_phrase, std::size_t reason_phrase_len) {
        constexpr const char* suborigin = "put_reason_phrase()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: reason_phrase='%s'", reason_phrase);

        base::assert_next(http::item::reason_phrase);

        std::size_t pcount = 0;
        if (reason_phrase != nullptr) {
            if (reason_phrase_len == size::strlen) {
                reason_phrase_len = std::strlen(reason_phrase);
            }

            pcount = base::put_prints_and_spaces(reason_phrase, reason_phrase_len);
        }

        base::put_crlf();

        base::set_pstate(http::item::header_name);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline http_client_stream<LogPtr>::http_client_stream(std::streambuf* sb, const LogPtr& log)
        : http_request_ostream<LogPtr>(sb, log)
        , http_response_istream<LogPtr>(sb, log) {
    }


    template <typename LogPtr>
    inline http_client_stream<LogPtr>::http_client_stream(http_client_stream&& other)
        : http_request_ostream<LogPtr>(std::move(other))
        , http_response_istream<LogPtr>(std::move(other)) {
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline http_server_stream<LogPtr>::http_server_stream(std::streambuf* sb, const LogPtr& log)
        : http_request_istream<LogPtr>(sb, log)
        , http_response_ostream<LogPtr>(sb, log) {
    }


    template <typename LogPtr>
    inline http_server_stream<LogPtr>::http_server_stream(http_server_stream&& other)
        : http_request_istream<LogPtr>(std::move(other))
        , http_response_ostream<LogPtr>(std::move(other)) {
    }

} }

