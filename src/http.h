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

#include "ascii.h"
#include "exception.h"
#include "http.i.h"


namespace abc {

	template <typename LogPtr>
	inline _http_stream<LogPtr>::_http_stream(std::streambuf* sb, http::item_t next, const LogPtr& log_ptr)
		: std::istream(sb)
		, _next(next)
		, _gcount(0)
		, _log_ptr(log_ptr) {
		if (_log_ptr != nullptr) {
			_log_ptr->push_back(category::abc::http, severity::abc, __TAG__, "_http_stream::_http_stream()");
		}
	}


	template <typename LogPtr>
	inline void _http_stream<LogPtr>::reset(http::item_t next) {
		if (_log_ptr != nullptr) {
			_log_ptr->push_back(category::abc::http, severity::abc, __TAG__, "_http_stream::reset() next=%lu", (std::uint32_t)next);
		}

		this->clear(goodbit);
		_next = next;
		_gcount = 0;
	}


	template <typename LogPtr>
	inline http::item_t _http_stream<LogPtr>::next() const noexcept {
		return _next;
	}


	template <typename LogPtr>
	inline std::size_t _http_stream<LogPtr>::gcount() const {
		return _gcount;
	}


	template <typename LogPtr>
	inline bool _http_stream<LogPtr>::eof() const {
		return std::istream::eof();
	}


	template <typename LogPtr>
	inline bool _http_stream<LogPtr>::good() const {
		return std::istream::good();
	}


	template <typename LogPtr>
	inline bool _http_stream<LogPtr>::bad() const {
		return std::istream::bad();
	}


	template <typename LogPtr>
	inline bool _http_stream<LogPtr>::fail() const {
		return std::istream::fail();
	}


	template <typename LogPtr>
	inline bool _http_stream<LogPtr>::operator!() const {
		return std::istream::operator!();
	}


	template <typename LogPtr>
	inline _http_stream<LogPtr>::operator bool() const {
		return std::istream::operator bool();
	}


	template <typename LogPtr>
	inline void _http_stream<LogPtr>::assert_next(http::item_t item) {
		if (_next != item) {
			throw exception<std::logic_error, LogPtr>("_next", __TAG__, log_ptr());
		}
	}


	template <typename LogPtr>
	inline void _http_stream<LogPtr>::set_next(http::item_t item) noexcept {
		_next = item;
	}


	template <typename LogPtr>
	inline void _http_stream<LogPtr>::set_gcount(std::size_t gcount) noexcept {
		_gcount = gcount;
	}


	template <typename LogPtr>
	inline bool _http_stream<LogPtr>::is_good() const {
		return good() && !eof();
	}


	template <typename LogPtr>
	inline void _http_stream<LogPtr>::set_bad() {
		clear(badbit | failbit);
	}


	template <typename LogPtr>
	inline void _http_stream<LogPtr>::set_fail() {
		setstate(failbit);
	}


	template <typename LogPtr>
	inline const LogPtr& _http_stream<LogPtr>::log_ptr() const noexcept {
		return _log_ptr;
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline _http_istream<LogPtr>::_http_istream(std::streambuf* sb, http::item_t next, const LogPtr& log_ptr)
		: _http_stream<LogPtr>(sb, next, log_ptr) {
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


	// _http_ostream


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline http_request_istream<LogPtr>::http_request_istream(std::streambuf* sb, const LogPtr& log_ptr)
		: _http_istream<LogPtr>(sb, http::item::method, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::http, severity::abc, __TAG__, "http_request_istream::htp_request_istream()");
		}
	}


	template <typename LogPtr>
	inline void http_request_istream<LogPtr>::reset() {
		_http_stream<LogPtr>::reset(http::item::method);
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

	// http_request_ostream

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
		_http_stream<LogPtr>::reset(http::item::protocol);
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

	// http_response_ostream

	// --------------------------------------------------------------

}

