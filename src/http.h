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
	inline _http_stream<LogPtr>::_http_stream(std::streambuf* sb, const LogPtr& log_ptr)
		: std::istream(sb)
		, _http_state(http::stream_state::not_started)
		, _gcount(0)
		, _log_ptr(log_ptr) {
	}


	template <typename LogPtr>
	inline http::stream_state_t _http_stream<LogPtr>::http_state() const noexcept {
		return _http_state;
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
	inline void _http_stream<LogPtr>::assert_http_state(http::stream_state_t http_state) {
		if (_http_state != http_state) {
			throw exception<std::logic_error, LogPtr>("_http_state", __TAG__, log_ptr());
		}
	}


	template <typename LogPtr>
	inline void _http_stream<LogPtr>::set_http_state(http::stream_state_t http_state) noexcept {
		_http_state = http_state;
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
		clear(failbit);
	}


	template <typename LogPtr>
	inline const LogPtr& _http_stream<LogPtr>::log_ptr() const noexcept {
		return _log_ptr;
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline _http_istream<LogPtr>::_http_istream(std::streambuf* sb, const LogPtr& log_ptr)
		: _http_stream<LogPtr>(sb, log_ptr) {
	}


	// void	get_headername(char* buffer, std::size_t size);
	// void	get_headervalue(char* buffer, std::size_t size);
	// void	get_body(char* buffer, std::size_t size);


	template <typename LogPtr>
	inline std::size_t _http_istream<LogPtr>::get_token(char* buffer, std::size_t size) {
		return get_chars(ascii::http::is_token, buffer, size);
	}


	template <typename LogPtr>
	inline std::size_t _http_istream<LogPtr>::get_prints(char* buffer, std::size_t size) {
		return get_chars(ascii::is_abcprint, buffer, size);
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
		: _http_istream<LogPtr>(sb, log_ptr) {
	}


	template <typename LogPtr>
	inline void http_request_istream<LogPtr>::get_method(char* buffer, std::size_t size) {
		this->assert_http_state(http::stream_state::not_started);

		std::size_t gcount = this->get_token(buffer, size);
		this->set_gcount(gcount);

		this->set_http_state(http::stream_state::after_method);
		this->skip_spaces();
	}


	template <typename LogPtr>
	inline void http_request_istream<LogPtr>::get_resource(char* buffer, std::size_t size) {
		this->assert_http_state(http::stream_state::after_method);

		std::size_t gcount = this->get_prints(buffer, size);
		this->set_gcount(gcount);

		this->set_http_state(http::stream_state::after_resource);
		this->skip_spaces();
	}


	template <typename LogPtr>
	inline void http_request_istream<LogPtr>::get_protocol(char* buffer, std::size_t size) {
		this->assert_http_state(http::stream_state::after_resource);

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
		this->set_http_state(http::stream_state::after_protocol);
		this->skip_spaces();
		this->skip_crlf();
	}


	// --------------------------------------------------------------

}

