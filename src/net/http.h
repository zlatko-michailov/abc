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

#include "ascii.h"
#include "exception.h"
#include "stream.h"
#include "i/http.i.h"


namespace abc { namespace net {

    template <typename LogPtr>
    inline http_state<LogPtr>::http_state(http::item_t next, const LogPtr& log) noexcept
        : diag_base("abc::net::http_state", log)
        , _next(next) {

        constexpr const char* suborigin = "http_state()";
        diag_base::put_any(suborigin, severity::callstack, __TAG__, "Begin: next=%u", (unsigned)next);

        diag_base::put_any(suborigin, severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline void http_state<LogPtr>::reset(http::item_t next) {
        constexpr const char* suborigin = "reset()";
        diag_base::put_any(suborigin, severity::callstack, __TAG__, "Begin: next=%u", (unsigned)next);

        _next = next;

        diag_base::put_any(suborigin, severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline http::item_t http_state<LogPtr>::next() const noexcept {
        return _next;
    }


    template <typename LogPtr>
    inline void http_state<LogPtr>::assert_next(http::item_t item) {
        constexpr const char* suborigin = "assert_next()";
        diag_base::put_any(suborigin, severity::callstack, __TAG__, "Begin: item=%u", (unsigned)item);

        diag_base::assert(suborigin, _next == item, __TAG__, "_next=%u, item=%u:", (unsigned)_next, (unsigned)item);

        diag_base::put_any(suborigin, severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline http_istream<LogPtr>::http_istream(std::streambuf* sb, http::item_t next, const LogPtr& log)
        : base(sb)
        , state(next, log) {

        constexpr const char* suborigin = "http_istream()";
        diag_base::put_any(suborigin, severity::callstack, __TAG__, "Begin: next=%u", (unsigned)next);

        diag_base::expect(suborigin, sb != nullptr, __TAG__, "sb");

        diag_base::put_any(suborigin, severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline http_istream<LogPtr>::http_istream(http_istream&& other)
        : base(std::move(other))
        , state(std::move(other)) {
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_protocol() {
        constexpr const char* suborigin = "get_protocol()";
        diag_base::put_any(suborigin, severity::callstack, __TAG__, "Begin:");

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

        diag_base::put_any(suborigin, severity::callstack, __TAG__, "End: protocol='%s'", protocol.c_str());

        return std::move(protocol);
    }


    template <typename LogPtr>
    inline http_headers http_istream<LogPtr>::get_headers() {
        constexpr const char* suborigin = "get_headers()";
        diag_base::put_any(suborigin, severity::callstack, __TAG__, "Begin:");

        state::assert_next(http::item::header_name);

        http_headers headers;

        while (state::next() == http::item::header_name) {
            // Read header name.
            std::string header_name = get_header_name();

            if (state::next() == http::item::header_value) {
                // Read header value.
                std::string header_value = get_header_value();

                // Insert the pair into the map.
                std::pair<std::string, std::string> pair(std::move(header_name), std::move(header_value));
                headers.insert(std::move(pair));
            }
        }

        diag_base::put_any(suborigin, severity::callstack, __TAG__, "End: headers.size()=%zu", headers.size());

        return std::move(headers);
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_header_name() {
        constexpr const char* suborigin = "get_header_name()";
        diag_base::put_any(suborigin, severity::callstack, __TAG__, "Begin:");

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

        diag_base::put_any(suborigin, severity::callstack, __TAG__, "End: header_name='%s'", header_name.c_str());

        return std::move(header_name);
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_header_value() {
        constexpr const char* suborigin = "get_header_value()";
        diag_base::put_any(suborigin, severity::callstack, __TAG__, "Begin:");

        state::assert_next(http::item::header_value);

        // Format: prints CRLF[ prints CRLF[...]]

        std::string header_value;
        skip_spaces();

        bool has_more = true;
        while (has_more) {
            header_value += get_prints();
            skip_spaces();
            skip_crlf();

            has_more = !header_value.empty() && skip_spaces() > 0 && ascii::is_abcprint(peek_char());
            if (has_more) {
                header_value += ' ';
            }
        }

        skip_spaces();

        set_gstate(header_value.length(), http::item::header_name);

        diag_base::put_any(suborigin, severity::callstack, __TAG__, "End: header_value='%s'", header_value.c_str());

        return std::move(header_value);
    }


    template <typename LogPtr>
    inline std::string http_istream<LogPtr>::get_body(std::size_t max_len) {
        constexpr const char* suborigin = "get_body()";
        diag_base::put_any(suborigin, severity::callstack, __TAG__, "Begin: max_len=%zu", max_len);

        state::assert_next(http::item::body);

        std::string body = get_any_chars(max_len);

        set_gstate(body.length(), http::item::body);

        diag_base::put_any(suborigin, severity::callstack, __TAG__, "End: body='%s'", body.c_str());

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
            chars += base::get();
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
        diag_base::put_any(suborigin, severity::callstack, __TAG__, "Begin: gcount=%zu, next=%u", gcount, (unsigned)next);

        base::set_gcount(gcount);
        state::reset(next);

        diag_base::put_any(suborigin, severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    template <typename Log>
    inline http_ostream<Log>::http_ostream(std::streambuf* sb, http::item_t next, Log* log)
        : base(sb)
        , state(next, log) {
        Log* log_local = state::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x10047, "http_ostream::http_ostream()");
        }
    }


    template <typename Log>
    inline http_ostream<Log>::http_ostream(http_ostream&& other)
        : base(std::move(other))
        , state(std::move(other)) {
    }


    template <typename Log>
    inline void http_ostream<Log>::set_pstate(http::item_t next) {
        base::flush();
        state::reset(next);
    }


    template <typename Log>
    inline std::size_t http_ostream<Log>::put_protocol(const char* buffer, std::size_t size) {
        Log* log_local = state::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x10048, "http_ostream::put_protocol() >>>");
        }

        state::assert_next(http::item::protocol);

        if (size == size::strlen) {
            size = std::strlen(buffer);
        }

        std::size_t gcount = 0;    

        if (size < 5 || !ascii::are_equal_i_n(buffer, "HTTP/", 5)) {
            base::set_bad();
        }
        else {
            gcount = put_bytes("HTTP/", 5); //// put_any_chars()
        }

        if (base::is_good() && gcount < size) {
            std::size_t gcount_local = put_digits(buffer + gcount, size - gcount);
            if (gcount_local == 0) {
                base::set_bad();
            }
            else {
                gcount += gcount_local;
            }
        }

        if (base::is_good() && gcount < size) {
            if (buffer[gcount] != '.') {
                base::set_bad();
            }
            else {
                base::put('.');
                gcount++;
            }
        }

        if (base::is_good() && gcount < size) {
            std::size_t gcount_local = put_digits(buffer + gcount, size - gcount);
            if (gcount_local == 0) {
                base::set_bad();
            }
            else {
                gcount += gcount_local;
            }
        }

        if (base::is_good() && gcount < size) {
            base::set_bad();
        }

        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::optional, 0x10049, "http_ostream::put_protocol() <<< buffer='%s', size=%zu, gcount=%zu", buffer, size, gcount);
        }

        return gcount;
    }


    template <typename Log>
    inline void http_ostream<Log>::put_header_name(const char* buffer, std::size_t size) {
        Log* log_local = state::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x1004a, "http_ostream::put_header_name() >>>");
        }

        state::assert_next(http::item::header_name);

        if (size == size::strlen) {
            size = std::strlen(buffer);
        }

        std::size_t pcount = put_token(buffer, size);

        if (base::is_good()) {
            if (pcount < size) {
                base::set_bad();
            }
        }

        if (base::is_good()) {
            base::put(':');
            put_space();
        }

        set_pstate(http::item::header_value);

        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::optional, 0x1004b, "http_ostream::put_header_name() <<< buffer='%s', size=%zu, pcount=%zu", buffer, size, pcount);
        }
    }


    template <typename Log>
    inline void http_ostream<Log>::put_header_value(const char* buffer, std::size_t size) {
        Log* log_local = state::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x1004c, "http_ostream::put_header_value() >>>");
        }

        state::assert_next(http::item::header_value);

        if (size == size::strlen) {
            size = std::strlen(buffer);
        }

        std::size_t pcount = 0;
        do {
            std::size_t sp = skip_spaces_in_header_value(buffer + pcount, size - pcount);

            if (pcount > 0 && sp > 0 && pcount + sp < size) {
                put_space();
            }

            pcount += sp;

            if (pcount < size) {
                if (ascii::is_abcprint(buffer[pcount])) {
                    pcount += put_prints(buffer + pcount, size - pcount);
                }
                else {
                    base::set_bad();
                }
            }
        }
        while (base::is_good() && pcount < size);

        put_crlf();

        set_pstate(http::item::header_name);

        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::optional, 0x1004d, "http_ostream::put_header_value() <<< buffer='%s', size=%zu, pcount=%zu", buffer, size, pcount);
        }
    }


    template <typename Log>
    inline void http_ostream<Log>::end_headers() {
        Log* log_local = state::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x1004e, "http_ostream::end_headers() >>>");
        }

        state::assert_next(http::item::header_name);

        put_crlf();

        set_pstate(http::item::body);

        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x1004f, "http_ostream::end_headers() <<<");
        }
    }


    template <typename Log>
    inline void http_ostream<Log>::put_body(const char* buffer, std::size_t size) {
        Log* log_local = state::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x10050, "http_ostream::put_body() >>>");
        }

        state::assert_next(http::item::body);

        if (size == size::strlen) {
            size = std::strlen(buffer);
        }

        std::size_t pcount = put_bytes(buffer, size); //// put_any_chars()

        set_pstate(http::item::body);

        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::optional, 0x10051, "http_ostream::put_body() <<< buffer='%s', size=%zu, pcount=%zu", buffer, size, pcount);
        }
    }


    template <typename Log>
    inline std::size_t http_ostream<Log>::put_crlf() {
        return put_char('\r') + put_char('\n');
    }


    template <typename Log>
    inline std::size_t http_ostream<Log>::put_space() {
        return put_char(' ');
    }


    template <typename Log>
    inline std::size_t http_ostream<Log>::put_token(const char* buffer, std::size_t size) {
        return put_chars(ascii::http::is_token, buffer, size);
    }


    template <typename Log>
    inline std::size_t http_ostream<Log>::put_prints(const char* buffer, std::size_t size) {
        return put_chars(ascii::is_abcprint, buffer, size);
    }


    template <typename Log>
    inline std::size_t http_ostream<Log>::put_prints_and_spaces(const char* buffer, std::size_t size) {
        return put_chars(ascii::is_abcprint_or_space, buffer, size);
    }


    template <typename Log>
    inline std::size_t http_ostream<Log>::put_alphas(const char* buffer, std::size_t size) {
        return put_chars(ascii::is_alpha, buffer, size);
    }


    template <typename Log>
    inline std::size_t http_ostream<Log>::put_digits(const char* buffer, std::size_t size) {
        return put_chars(ascii::is_digit, buffer, size);
    }


    template <typename Log>
    inline std::size_t http_ostream<Log>::put_bytes(const char* buffer, std::size_t size) {
        std::size_t gcount = 0;

        while (base::is_good() && gcount < size) {
            base::put(buffer[gcount++]);
        }

        return gcount;
    }


    template <typename Log>
    inline std::size_t http_ostream<Log>::put_chars(ascii::predicate_t&& predicate, const char* buffer, std::size_t size) {
        std::size_t pcount = 0;

        while (base::is_good() && pcount < size && predicate(buffer[pcount])) {
            base::put(buffer[pcount++]);
        }

        return pcount;
    }


    template <typename Log>
    inline std::size_t http_ostream<Log>::put_char(char ch) {
        if (base::is_good()) {
            base::put(ch);
        }

        return base::is_good() ? 1 : 0;;
    }


    template <typename Log>
    inline std::size_t http_ostream<Log>::skip_spaces_in_header_value(const char* buffer, std::size_t size) {
        std::size_t sp = 0;

        while (sp < size) {
            if (ascii::is_space(buffer[sp])) {
                sp++;
            }
            else if (sp + 3 < size && buffer[sp] == '\r' && buffer[sp + 1] == '\n' && ascii::is_space(buffer[sp + 2])) {
                sp += 3;
            }
            else {
                break;
            }
        }

        return sp;
    }


    template <typename Log>
    inline std::size_t http_ostream<Log>::skip_spaces(const char* buffer, std::size_t size) {
        std::size_t sp = 0;

        while (sp < size && ascii::is_space(buffer[sp])) {
            sp++;
        }

        return sp;
    }


    // --------------------------------------------------------------


    template <typename Log>
    inline http_request_istream<Log>::http_request_istream(std::streambuf* sb, Log* log)
        : base(sb, http::item::method, log) {
        Log* log_local = base::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x10052, "http_request_istream::http_request_istream()");
        }
    }


    template <typename Log>
    inline http_request_istream<Log>::http_request_istream(http_request_istream&& other)
        : base(std::move(other)) {
    }


    template <typename Log>
    inline void http_request_istream<Log>::reset() {
        base::reset(http::item::method);
    }


    template <typename Log>
    inline void http_request_istream<Log>::get_method(char* buffer, std::size_t size) {
        Log* log_local = base::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x10053, "http_request_istream::get_method() >>>");
        }

        base::assert_next(http::item::method);

        std::size_t gcount = base::get_token(buffer, size);

        base::skip_spaces();

        base::set_gstate(gcount, http::item::resource);

        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::optional, 0x10054, "http_request_istream::get_method() <<< method='%s', gcount=%zu", buffer, gcount);
        }
    }


    template <typename Log>
    inline void http_request_istream<Log>::get_resource(char* buffer, std::size_t size) {
        Log* log_local = base::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x10055, "http_request_istream::get_resource() >>>");
        }

        base::assert_next(http::item::resource);

        // The resource is terminated by an extra '\0', so it could be split.
        std::size_t gcount = base::get_prints(buffer, size - 1);
        buffer[gcount + 1] = '\0';

        base::skip_spaces();

        base::set_gstate(gcount, http::item::protocol);

        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::optional, 0x10056, "http_request_istream::get_resource() <<< resource='%s', gcount=%zu", buffer, gcount);
        }
    }


    template <typename Log>
    inline void http_request_istream<Log>::get_protocol(char* buffer, std::size_t size) {
        std::size_t gcount = http_istream<Log>::get_protocol(buffer, size);

        base::skip_crlf();

        base::set_gstate(gcount, http::item::header_name);
    }


    template <typename Log>
    inline void http_request_istream<Log>::split_resource(char* buffer, std::size_t size) {
        const char* const end = buffer + size;
 
        // path?param1=...&param2=...
        char* param = std::strchr(buffer, '?');

        while (param < end && param != nullptr) {
            *param++ = '\0';

            if (param < end && *param != '\0') {
                param = std::strchr(param, '&');
            }
        }
    }


    template <typename Log>
    inline const char* http_request_istream<Log>::get_resource_parameter(const char* buffer, std::size_t size, const char* parameter_name) {
        const char* const end = buffer + size;
        const std::size_t parameter_name_len = std::strlen(parameter_name);
 
        // path\0param1=...\0param2=...\0\0
        const char* param = buffer + std::strlen(buffer) + 1;

        while (param < end && *param != '\0') {
            if (std::strncmp(param, parameter_name, parameter_name_len) == 0 && param[parameter_name_len] == '=') {
                return param + parameter_name_len + 1;
            }

            if (param < end && *param != '\0') {
                param += std::strlen(param) + 1;
            }
        }

        return nullptr;
    }


    // --------------------------------------------------------------


    template <typename Log>
    inline http_request_ostream<Log>::http_request_ostream(std::streambuf* sb, Log* log)
        : base(sb, http::item::method, log) {
        Log* log_local = base::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x10057, "http_request_ostream::http_request_ostream()");
        }
    }


    template <typename Log>
    inline http_request_ostream<Log>::http_request_ostream(http_request_ostream&& other)
        : base(std::move(other)) {
    }


    template <typename Log>
    inline void http_request_ostream<Log>::reset() {
        base::reset(http::item::method);
    }


    template <typename Log>
    inline void http_request_ostream<Log>::put_method(const char* buffer, std::size_t size) {
        Log* log_local = base::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x10058, "http_ostream::put_method() >>>");
        }

        base::assert_next(http::item::method);

        if (size == size::strlen) {
            size = std::strlen(buffer);
        }

        std::size_t pcount = base::put_token(buffer, size);

        base::put_space();

        base::set_pstate(http::item::resource);

        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::optional, 0x10059, "http_ostream::put_method() <<< buffer='%s', size=%zu, pcount=%zu", buffer, size, pcount);
        }
    }


    template <typename Log>
    inline void http_request_ostream<Log>::put_resource(const char* buffer, std::size_t size) {
        Log* log_local = base::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x1005a, "http_ostream::put_resource() >>>");
        }

        base::assert_next(http::item::resource);

        if (size == size::strlen) {
            size = std::strlen(buffer);
        }

        std::size_t pcount = base::put_prints(buffer, size);

        base::put_space();

        base::set_pstate(http::item::protocol);

        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::optional, 0x1005b, "http_ostream::put_resource() <<< buffer='%s', size=%zu, pcount=%zu", buffer, size, pcount);
        }
    }


    template <typename Log>
    inline void http_request_ostream<Log>::put_protocol(const char* buffer, std::size_t size) {
        http_ostream<Log>::put_protocol(buffer, size);

        base::put_crlf();

        base::set_pstate(http::item::header_name);
    }


    // --------------------------------------------------------------


    template <typename Log>
    inline http_response_istream<Log>::http_response_istream(std::streambuf* sb, Log* log)
        : base(sb, http::item::protocol, log) {
        Log* log_local = base::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x1005c, "http_response_istream::http_response_istream()");
        }
    }


    template <typename Log>
    inline http_response_istream<Log>::http_response_istream(http_response_istream&& other)
        : base(std::move(other)) {
    }


    template <typename Log>
    inline void http_response_istream<Log>::reset() {
        base::reset(http::item::protocol);
    }


    template <typename Log>
    inline void http_response_istream<Log>::get_protocol(char* buffer, std::size_t size) {
        std::size_t gcount = http_istream<Log>::get_protocol(buffer, size);

        base::set_gstate(gcount, http::item::status_code);
    }

    template <typename Log>
    inline void http_response_istream<Log>::get_status_code(char* buffer, std::size_t size) {
        Log* log_local = base::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x1005d, "http_response_istream::get_status_code() >>>");
        }

        base::assert_next(http::item::status_code);

        std::size_t gcount = base::get_digits(buffer, size);

        base::skip_spaces();

        base::set_gstate(gcount, http::item::reason_phrase);

        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::optional, 0x1005e, "http_response_istream::get_status_code() <<< status_code='%s', gcount=%zu", buffer, gcount);
        }
    }


    template <typename Log>
    inline void http_response_istream<Log>::get_reason_phrase(char* buffer, std::size_t size) {
        Log* log_local = base::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x1005f, "http_response_istream::get_reason_phrase() >>>");
        }

        base::assert_next(http::item::reason_phrase);

        std::size_t gcount = base::get_prints_and_spaces(buffer, size);

        base::skip_spaces();
        base::skip_crlf();

        base::set_gstate(gcount, http::item::header_name);

        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::optional, 0x10060, "http_response_istream::get_reson_phrase() <<< reason_phrase='%s', gcount=%zu", buffer, gcount);
        }
    }


    // --------------------------------------------------------------


    template <typename Log>
    inline http_response_ostream<Log>::http_response_ostream(std::streambuf* sb, Log* log)
        : base(sb, http::item::protocol, log) {
        Log* log_local = base::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x10061, "http_response_ostream::http_response_ostream()");
        }
    }


    template <typename Log>
    inline http_response_ostream<Log>::http_response_ostream(http_response_ostream&& other)
        : base(std::move(other)) {
    }


    template <typename Log>
    inline void http_response_ostream<Log>::reset() {
        base::reset(http::item::protocol);
    }


    template <typename Log>
    inline void http_response_ostream<Log>::put_protocol(const char* buffer, std::size_t size) {
        http_ostream<Log>::put_protocol(buffer, size);

        base::put_space();

        base::set_pstate(http::item::status_code);
    }


    template <typename Log>
    inline void http_response_ostream<Log>::put_status_code(const char* buffer, std::size_t size) {
        Log* log_local = base::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x10062, "http_response_ostream::put_status_code() >>>");
        }

        base::assert_next(http::item::status_code);

        if (size == size::strlen) {
            size = std::strlen(buffer);
        }

        std::size_t pcount = base::put_digits(buffer, size);

        base::put_space();

        base::set_pstate(http::item::reason_phrase);

        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::optional, 0x10063, "http_response_ostream::put_status_code() <<< buffer='%s', size=%zu, pcount=%zu", buffer, size, pcount);
        }
    }


    template <typename Log>
    inline void http_response_ostream<Log>::put_reason_phrase(const char* buffer, std::size_t size) {
        Log* log_local = base::log();
        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::debug, 0x10064, "http_response_ostream::put_reason_phrase() >>>");
        }

        base::assert_next(http::item::reason_phrase);

        std::size_t pcount = 0;
        if (buffer != nullptr) {
            if (size == size::strlen) {
                size = std::strlen(buffer);
            }

            pcount = base::put_prints_and_spaces(buffer, size);
        }

        base::put_crlf();

        base::set_pstate(http::item::header_name);

        if (log_local != nullptr) {
            log_local->put_any(category::abc::http, severity::abc::optional, 0x10065, "http_response_ostream::put_reason_phrase() <<< buffer='%s', size=%zu, pcount=%zu", buffer != nullptr ? buffer : "<nullptr>", size, pcount);
        }
    }


    // --------------------------------------------------------------


    template <typename Log>
    inline http_client_stream<Log>::http_client_stream(std::streambuf* sb, Log* log)
        : http_request_ostream<Log>(sb, log)
        , http_response_istream<Log>(sb, log) {
    }


    template <typename Log>
    inline http_client_stream<Log>::http_client_stream(http_client_stream&& other)
        : http_request_ostream<Log>(std::move(other))
        , http_response_istream<Log>(std::move(other)) {
    }


    // --------------------------------------------------------------


    template <typename Log>
    inline http_server_stream<Log>::http_server_stream(std::streambuf* sb, Log* log)
        : http_request_istream<Log>(sb, log)
        , http_response_ostream<Log>(sb, log) {
    }


    template <typename Log>
    inline http_server_stream<Log>::http_server_stream(http_server_stream&& other)
        : http_request_istream<Log>(std::move(other))
        , http_response_ostream<Log>(std::move(other)) {
    }

} }

