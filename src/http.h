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


#pragma once

#include <cstring>

#include "ascii.h"
#include "exception.h"
#include "http.i.h"


namespace abc {

	template <typename StdStream, typename LogPtr>
	inline _http_stream<StdStream, LogPtr>::_http_stream(std::streambuf* sb, http::item_t next, const LogPtr& log_ptr)
		: StdStream(sb)
		, _next(next)
		, _gcount(0)
		, _log_ptr(log_ptr) {
		if (_log_ptr != nullptr) {
			_log_ptr->push_back(category::abc::http, severity::abc, __TAG__, "_http_stream::_http_stream()");
		}
	}


	template <typename StdStream, typename LogPtr>
	inline void _http_stream<StdStream, LogPtr>::reset(http::item_t next) {
		if (_log_ptr != nullptr) {
			_log_ptr->push_back(category::abc::http, severity::abc, __TAG__, "_http_stream::reset() next=%lu", (std::uint32_t)next);
		}

		StdStream::clear(StdStream::goodbit);
		_next = next;
		_gcount = 0;
	}


	template <typename StdStream, typename LogPtr>
	inline http::item_t _http_stream<StdStream, LogPtr>::next() const noexcept {
		return _next;
	}


	template <typename StdStream, typename LogPtr>
	inline std::size_t _http_stream<StdStream, LogPtr>::gcount() const {
		return _gcount;
	}


	template <typename StdStream, typename LogPtr>
	inline bool _http_stream<StdStream, LogPtr>::eof() const {
		return StdStream::eof();
	}


	template <typename StdStream, typename LogPtr>
	inline bool _http_stream<StdStream, LogPtr>::good() const {
		return StdStream::good();
	}


	template <typename StdStream, typename LogPtr>
	inline bool _http_stream<StdStream, LogPtr>::bad() const {
		return StdStream::bad();
	}


	template <typename StdStream, typename LogPtr>
	inline bool _http_stream<StdStream, LogPtr>::fail() const {
		return StdStream::fail();
	}


	template <typename StdStream, typename LogPtr>
	inline bool _http_stream<StdStream, LogPtr>::operator!() const {
		return StdStream::operator!();
	}


	template <typename StdStream, typename LogPtr>
	inline _http_stream<StdStream, LogPtr>::operator bool() const {
		return StdStream::operator bool();
	}


	template <typename StdStream, typename LogPtr>
	inline void _http_stream<StdStream, LogPtr>::assert_next(http::item_t item) {
		if (_next != item) {
			throw exception<std::logic_error, LogPtr>("_next", __TAG__, log_ptr());
		}
	}


	template <typename StdStream, typename LogPtr>
	inline void _http_stream<StdStream, LogPtr>::set_next(http::item_t item) noexcept {
		_next = item;
	}


	template <typename StdStream, typename LogPtr>
	inline void _http_stream<StdStream, LogPtr>::set_gcount(std::size_t gcount) noexcept {
		_gcount = gcount;
	}


	template <typename StdStream, typename LogPtr>
	inline bool _http_stream<StdStream, LogPtr>::is_good() const {
		return StdStream::good() && !StdStream::eof();
	}


	template <typename StdStream, typename LogPtr>
	inline void _http_stream<StdStream, LogPtr>::set_bad() {
		StdStream::clear(StdStream::badbit | StdStream::failbit);
	}


	template <typename StdStream, typename LogPtr>
	inline void _http_stream<StdStream, LogPtr>::set_fail() {
		StdStream::setstate(StdStream::failbit);
	}


	template <typename StdStream, typename LogPtr>
	inline const LogPtr& _http_stream<StdStream, LogPtr>::log_ptr() const noexcept {
		return _log_ptr;
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline _http_istream<LogPtr>::_http_istream(std::streambuf* sb, http::item_t next, const LogPtr& log_ptr)
		: _http_stream<std::istream, LogPtr>(sb, next, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_istream::_http_istream()");
		}
	}


	template <typename LogPtr>
	inline void _http_istream<LogPtr>::get_protocol(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_istream::get_protocol() >>>");
		}

		this->assert_next(http::item::protocol);

		std::size_t gcount = this->get_alphas(buffer, size);
		if (gcount != 4 || std::strncmp("HTTP", buffer, 4) != 0) {
			this->set_bad();
		}

		if (gcount == size - 1) {
			this->set_fail();
		}

		if (this->is_good()) {
			char ch = this->get_char();
			if (ch == '/') {
				buffer[gcount++] = '/';
			}
			else {
				this->set_bad();
			}
		}

		if (this->is_good()) {
			gcount += this->get_digits(buffer + gcount, size - gcount);
		}

		if (gcount == size - 1) {
			this->set_fail();
		}

		if (this->is_good()) {
			char ch = this->get_char();
			if (ch == '.') {
				buffer[gcount++] = '.';
			}
			else {
				this->set_bad();
			}
		}

		if (this->is_good()) {
			gcount += this->get_digits(buffer + gcount, size - gcount);
		}

		this->set_gcount(gcount);
		this->skip_spaces();

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_istream::get_protocol() <<< protocol='%s', gcount=%lu", buffer, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline void _http_istream<LogPtr>::get_header_name(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_istream::get_header_name() >>>");
		}

		this->assert_next(http::item::header_name);

		std::size_t gcount = this->get_token(buffer, size);
		this->skip_spaces();

		if (gcount == 0) {
			this->skip_crlf();
			this->set_gcount(gcount);
			this->set_next(http::item::body);
			return;
		}

		if (this->is_good()) {
			char ch = this->get_char();
			if (ch != ':') {
				this->set_bad();
			}
		}

		this->set_gcount(gcount);
		this->set_next(http::item::header_value);
		this->skip_spaces();

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_istream::get_header_name() <<< header_name='%s', gcount=%lu", buffer, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline void _http_istream<LogPtr>::get_header_value(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_istream::get_header_value() >>>");
		}

		this->assert_next(http::item::header_value);

		std::size_t gcount = 0;
		std::size_t gc;

		do {
			do {
				std::size_t sp = this->skip_spaces();
				if (gcount > 0 && sp > 0 && ascii::is_abcprint(this->peek_char())) {
					if (gcount == size - 1) {
						this->set_fail();
						this->set_gcount(gcount);
						return;
					}

					buffer[gcount++] = ' ';
				}

				gc = this->get_prints(buffer + gcount, size - gcount);
				gcount += gc;
			}
			while (this->is_good() && gc > 0);
			this->skip_crlf();
		}
		while (this->is_good() && ascii::is_space(this->peek_char()));

		this->set_gcount(gcount);
		this->set_next(http::item::header_name);
		this->skip_spaces();

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_istream::get_header_value() <<< header_value='%s', gcount=%lu", buffer, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline void _http_istream<LogPtr>::get_body(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_istream::get_body() >>>");
		}

		this->assert_next(http::item::body);

		std::size_t gcount = this->get_bytes(buffer, size);

		this->set_gcount(gcount);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_istream::get_body() <<< body='%s', gcount=%lu", buffer, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline std::size_t _http_istream<LogPtr>::get_token(char* buffer, std::size_t size) {
		return get_chars(ascii::http::is_token, buffer, size);
	}


	template <typename LogPtr>
	inline std::size_t _http_istream<LogPtr>::get_prints(char* buffer, std::size_t size) {
		return get_chars(ascii::is_abcprint, buffer, size);
	}


	template <typename LogPtr>
	inline std::size_t _http_istream<LogPtr>::get_prints_and_spaces(char* buffer, std::size_t size) {
		return get_chars(ascii::is_abcprint_or_space, buffer, size);
	}


	template <typename LogPtr>
	inline std::size_t _http_istream<LogPtr>::get_alphas(char* buffer, std::size_t size) {
		return get_chars(ascii::is_alpha, buffer, size);
	}


	template <typename LogPtr>
	inline std::size_t _http_istream<LogPtr>::get_digits(char* buffer, std::size_t size) {
		return get_chars(ascii::is_digit, buffer, size);
	}


	template <typename LogPtr>
	inline std::size_t _http_istream<LogPtr>::skip_spaces() {
		return skip_chars(ascii::is_space);
	}


	template <typename LogPtr>
	inline std::size_t _http_istream<LogPtr>::skip_crlf() {
		char ch = get_char();
		if (ch != '\r') {
			this->set_fail();
			return 0;
		}

		ch = get_char();
		if (ch != '\n') {
			this->set_fail();
			return 1;
		}

		return 2;
	}


	template <typename LogPtr>
	inline std::size_t _http_istream<LogPtr>::get_bytes(char* buffer, std::size_t size) {
		std::size_t gcount = 0;

		while (this->is_good() && gcount < size) {
			buffer[gcount++] = this->get();
		}

		return gcount;
	}


	template <typename LogPtr>
	template <typename Predicate>
	inline std::size_t _http_istream<LogPtr>::get_chars(Predicate&& predicate, char* buffer, std::size_t size) {
		std::size_t gcount = 0;

		while (this->is_good() && predicate(peek_char())) {
			if (gcount == size - 1) {
				this->set_fail();
				break;
			}

			buffer[gcount++] = this->get();
		}
		buffer[gcount] = '\0';

		return gcount;
	}


	template <typename LogPtr>
	template <typename Predicate>
	inline std::size_t _http_istream<LogPtr>::skip_chars(Predicate&& predicate) {
		std::size_t gcount = 0;
		
		while (this->is_good() && predicate(peek_char())) {
			this->get();
			gcount++;
		}

		return gcount;
	}
	
	
	template <typename LogPtr>
	inline char _http_istream<LogPtr>::get_char() {
		char ch = peek_char();

		if (this->is_good()) {
			this->get();
		}

		return ch;
	}


	template <typename LogPtr>
	inline char _http_istream<LogPtr>::peek_char() {
		char ch = this->peek();

		if (!ascii::is_ascii(ch)) {
			this->set_bad();
			ch = '\0';
		}

		return ch;
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline _http_ostream<LogPtr>::_http_ostream(std::streambuf* sb, http::item_t next, const LogPtr& log_ptr)
		: _http_stream<std::ostream, LogPtr>(sb, next, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::_http_ostream()");
		}
	}


	template <typename LogPtr>
	inline std::size_t _http_ostream<LogPtr>::put_protocol(const char* protocol) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::put_protocol() >>>");
		}

		this->assert_next(http::item::protocol);

		std::size_t gcount = std::strlen(protocol);
		this->write(protocol, gcount);

		this->set_gcount(gcount);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::put_protocol() <<< protocol='%s', gcount=%lu", protocol, (std::uint32_t)gcount);
		}

		return gcount;
	}


	template <typename LogPtr>
	inline void _http_ostream<LogPtr>::put_header_name(const char* header_name) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::put_header_name() >>>");
		}

		this->assert_next(http::item::header_name);

		std::size_t gcount = std::strlen(header_name);
		this->write(header_name, gcount);

		gcount += this->put(':');
		gcount += this->put_space();

		this->set_gcount(gcount);
		this->set_next(http::item::header_value);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::put_header_name() <<< header_name='%s', gcount=%lu", header_name, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline void _http_ostream<LogPtr>::put_header_value(const char* header_value) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::put_header_value() >>>");
		}

		this->assert_next(http::item::header_value);

		std::size_t gcount = std::strlen(header_value);
		this->write(header_value, gcount);

		gcount += this->put_crlf();

		this->set_gcount(gcount);
		this->set_next(http::item::header_name);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::put_header_value() <<< header_value='%s', gcount=%lu", header_value, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline void _http_ostream<LogPtr>::end_headers() {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::end_headers() >>>");
		}

		this->assert_next(http::item::header_name);

		std::size_t gcount = this->put_crlf();

		this->set_gcount(gcount);
		this->set_next(http::item::body);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::end_headers() <<< gcount=%lu", (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline void _http_ostream<LogPtr>::put_body(const char* body) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::put_body() >>>");
		}

		this->assert_next(http::item::body);

		std::size_t gcount = std::strlen(body);
		this->write(body, gcount);

		this->set_gcount(gcount);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::put_body() <<< body='%s', gcount=%lu", body, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline std::size_t _http_ostream<LogPtr>::put_crlf() {
		this->put('\r');
		this->put('\n');

		return 2;
	}


	template <typename LogPtr>
	inline std::size_t _http_ostream<LogPtr>::put_space() {
		this->put(' ');

		return 1;
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline http_request_istream<LogPtr>::http_request_istream(std::streambuf* sb, const LogPtr& log_ptr)
		: _http_istream<LogPtr>(sb, http::item::method, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_request_istream::http_request_istream()");
		}
	}


	template <typename LogPtr>
	inline void http_request_istream<LogPtr>::reset() {
		_http_stream<std::istream, LogPtr>::reset(http::item::method);
	}


	template <typename LogPtr>
	inline void http_request_istream<LogPtr>::get_method(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_request_istream::get_method() >>>");
		}

		this->assert_next(http::item::method);

		std::size_t gcount = this->get_token(buffer, size);
		this->set_gcount(gcount);

		this->set_next(http::item::resource);
		this->skip_spaces();

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_request_istream::get_method() <<< method='%s', gcount=%lu", buffer, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline void http_request_istream<LogPtr>::get_resource(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_request_istream::get_resource() >>>");
		}

		this->assert_next(http::item::resource);

		std::size_t gcount = this->get_prints(buffer, size);
		this->set_gcount(gcount);

		this->set_next(http::item::protocol);
		this->skip_spaces();

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_request_istream::get_resource() <<< resource='%s', gcount=%lu", buffer, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline void http_request_istream<LogPtr>::get_protocol(char* buffer, std::size_t size) {
		_http_istream<LogPtr>::get_protocol(buffer, size);

		this->set_next(http::item::header_name);
		this->skip_crlf();
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline http_request_ostream<LogPtr>::http_request_ostream(std::streambuf* sb, const LogPtr& log_ptr)
		: _http_ostream<LogPtr>(sb, http::item::method, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_request_ostream::http_request_ostream()");
		}
	}


	template <typename LogPtr>
	inline void http_request_ostream<LogPtr>::reset() {
		_http_stream<std::ostream, LogPtr>::reset(http::item::method);
	}


	template <typename LogPtr>
	inline void http_request_ostream<LogPtr>::put_method(const char* method) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::put_method() >>>");
		}

		this->assert_next(http::item::method);

		std::size_t gcount = std::strlen(method);
		this->write(method, gcount);

		gcount += this->put_space();

		this->set_gcount(gcount);
		this->set_next(http::item::resource);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::put_method() <<< method='%s', gcount=%lu", method, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline void http_request_ostream<LogPtr>::put_resource(const char* resource) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::put_resource() >>>");
		}

		this->assert_next(http::item::resource);

		std::size_t gcount = std::strlen(resource);
		this->write(resource, gcount);

		gcount += this->put_space();

		this->set_gcount(gcount);
		this->set_next(http::item::protocol);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "_http_ostream::put_resource() <<< resource='%s', gcount=%lu", resource, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline void http_request_ostream<LogPtr>::put_protocol(const char* protocol) {
		std::size_t gcount = _http_ostream<LogPtr>::put_protocol(protocol);

		gcount += this->put_crlf();

		this->set_gcount(gcount);
		this->set_next(http::item::header_name);
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline http_response_istream<LogPtr>::http_response_istream(std::streambuf* sb, const LogPtr& log_ptr)
		: _http_istream<LogPtr>(sb, http::item::protocol, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_response_istream::http_response_istream()");
		}
	}


	template <typename LogPtr>
	inline void http_response_istream<LogPtr>::reset() {
		_http_stream<std::istream, LogPtr>::reset(http::item::protocol);
	}


	template <typename LogPtr>
	inline void http_response_istream<LogPtr>::get_protocol(char* buffer, std::size_t size) {
		_http_istream<LogPtr>::get_protocol(buffer, size);

		this->set_next(http::item::status_code);
	}

	template <typename LogPtr>
	inline void http_response_istream<LogPtr>::get_status_code(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_response_istream::get_status_code() >>>");
		}

		this->assert_next(http::item::status_code);

		std::size_t gcount = this->get_digits(buffer, size);
		this->set_gcount(gcount);

		this->set_next(http::item::reason_phrase);
		this->skip_spaces();

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_response_istream::get_status_code() <<< status_code='%s', gcount=%lu", buffer, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline void http_response_istream<LogPtr>::get_reason_phrase(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_response_istream::get_reason_phrase() >>>");
		}

		this->assert_next(http::item::reason_phrase);

		std::size_t gcount = this->get_prints_and_spaces(buffer, size);
		this->set_gcount(gcount);

		this->set_next(http::item::header_name);
		this->skip_spaces();
		this->skip_crlf();

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_response_istream::get_reson_phrase() <<< reason_phrase='%s', gcount=%lu", buffer, (std::uint32_t)gcount);
		}
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline http_response_ostream<LogPtr>::http_response_ostream(std::streambuf* sb, const LogPtr& log_ptr)
		: _http_ostream<LogPtr>(sb, http::item::protocol, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_response_ostream::http_response_ostream()");
		}
	}


	template <typename LogPtr>
	inline void http_response_ostream<LogPtr>::reset() {
		_http_stream<std::ostream, LogPtr>::reset(http::item::protocol);
	}


	template <typename LogPtr>
	inline void http_response_ostream<LogPtr>::put_protocol(const char* protocol) {
		std::size_t gcount = _http_ostream<LogPtr>::put_protocol(protocol);

		gcount += this->put_space();

		this->set_gcount(gcount);
		this->set_next(http::item::status_code);
	}


	template <typename LogPtr>
	inline void http_response_ostream<LogPtr>::put_status_code(const char* status_code) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_response_ostream::put_status_code() >>>");
		}

		this->assert_next(http::item::status_code);

		std::size_t gcount = std::strlen(status_code);
		this->write(status_code, gcount);

		gcount += this->put_space();

		this->set_gcount(gcount);
		this->set_next(http::item::reason_phrase);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_response_ostream::put_status_code() <<< status_code='%s', gcount=%lu", status_code, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr>
	inline void http_response_ostream<LogPtr>::put_reason_phrase(const char* reason_phrase) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_response_ostream::put_reason_phrase() >>>");
		}

		this->assert_next(http::item::reason_phrase);

		std::size_t gcount = 0;
		if (reason_phrase != nullptr) {
			gcount = std::strlen(reason_phrase);
			this->write(reason_phrase, gcount);
		}

		gcount += this->put_crlf();

		this->set_gcount(gcount);
		this->set_next(http::item::header_name);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_response_ostream::put_reason_phrase() <<< reason_phrase='%s', gcount=%lu", reason_phrase != nullptr ? reason_phrase : "<nullptr>", (std::uint32_t)gcount);
		}
	}


	// --------------------------------------------------------------

}

