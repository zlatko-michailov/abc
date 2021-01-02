/*
MIT License

Copyright (c) 2018-2021 Zlatko Michailov 

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

#include <streambuf>
#include <istream>
#include <ostream>

#include "stream.i.h"
#include "log.i.h"


namespace abc {

	namespace http {
		using item_t = std::uint8_t;

		namespace item {
			constexpr item_t method			= 0;
			constexpr item_t resource		= 1;
			constexpr item_t protocol		= 2;
			constexpr item_t status_code	= 3;
			constexpr item_t reason_phrase	= 4;
			constexpr item_t header_name	= 5;
			constexpr item_t header_value	= 6;
			constexpr item_t body			= 7;
		}
	}


	// --------------------------------------------------------------


	template <typename Log>
	class _http_state {
	protected:
		_http_state(http::item_t next, Log* log);
		_http_state(_http_state&& other) = default;

	public:
		http::item_t	next() const noexcept;

	protected:
		void			reset(http::item_t next);
		void			assert_next(http::item_t item);
		void			set_next(http::item_t item) noexcept;

		Log*			log() const noexcept;

	private:
		http::item_t	_next;
		Log*			_log;
	};


	// --------------------------------------------------------------


	template <typename Log>
	class _http_istream : public _istream, public _http_state<Log> {
		using base  = _istream;
		using state = _http_state<Log>;

	protected:
		_http_istream(std::streambuf* sb, http::item_t next, Log* log);
		_http_istream(_http_istream&& other) = default;

	public:
		void		get_header_name(char* buffer, std::size_t size);
		void		get_header_value(char* buffer, std::size_t size);
		void		get_body(char* buffer, std::size_t size);

	protected:
		void		set_gstate(std::size_t gcount, http::item_t next);

		std::size_t	get_protocol(char* buffer, std::size_t size);

		std::size_t	get_token(char* buffer, std::size_t size);
		std::size_t	get_prints(char* buffer, std::size_t size);
		std::size_t	get_prints_and_spaces(char* buffer, std::size_t size);
		std::size_t	get_alphas(char* buffer, std::size_t size);
		std::size_t	get_digits(char* buffer, std::size_t size);

		std::size_t	skip_spaces();
		std::size_t	skip_crlf();

		std::size_t	get_bytes(char* buffer, std::size_t size);
		template <typename Predicate>
		std::size_t	get_chars(Predicate&& predicate, char* buffer, std::size_t size);
		template <typename Predicate>
		std::size_t	skip_chars(Predicate&& predicate);
		char		get_char();
		char		peek_char();
	};


	// --------------------------------------------------------------


	template <typename Log>
	class _http_ostream : public _ostream, public _http_state<Log> {
		using base  = _ostream;
		using state = _http_state<Log>;

	protected:
		_http_ostream(std::streambuf* sb, http::item_t next, Log* log);
		_http_ostream(_http_ostream&& other) = default;

	public:
		void		put_header_name(const char*buffer, std::size_t size = size::strlen);
		void		put_header_value(const char* buffer, std::size_t size = size::strlen);
		void		end_headers();
		void		put_body(const char* buffer, std::size_t size = size::strlen);

	protected:
		void		set_pstate(http::item_t next);

		std::size_t	put_protocol(const char* buffer, std::size_t size);

		std::size_t	put_token(const char* buffer, std::size_t size);
		std::size_t	put_prints(const char* buffer, std::size_t size);
		std::size_t	put_prints_and_spaces(const char* buffer, std::size_t size);
		std::size_t	put_alphas(const char* buffer, std::size_t size);
		std::size_t	put_digits(const char* buffer, std::size_t size);
		std::size_t	put_crlf();
		std::size_t	put_space();

		std::size_t	put_bytes(const char* buffer, std::size_t size);
		template <typename Predicate>
		std::size_t	put_chars(Predicate&& predicate, const char* buffer, std::size_t size);
		std::size_t put_char(char ch);

		std::size_t skip_spaces_in_header_value(const char* buffer, std::size_t size);
		std::size_t skip_spaces(const char* buffer, std::size_t size);
	};


	// --------------------------------------------------------------


	template <typename Log = null_log>
	class http_request_istream : public _http_istream<Log> {
		using base  = _http_istream<Log>;

	public:
		http_request_istream(std::streambuf* sb, Log* log = nullptr);
		http_request_istream(http_request_istream&& other) = default;

		void	reset();

	public:
		void	get_method(char* buffer, std::size_t size);
		void	get_resource(char* buffer, std::size_t size);
		void	get_protocol(char* buffer, std::size_t size);
	};


	// --------------------------------------------------------------


	template <typename Log = null_log>
	class http_request_ostream : public _http_ostream<Log> {
		using base  = _http_ostream<Log>;

	public:
		http_request_ostream(std::streambuf* sb, Log* log = nullptr);
		http_request_ostream(http_request_ostream&& other) = default;

		void	reset();

	public:
		void	put_method(const char* method, std::size_t size = size::strlen);
		void	put_resource(const char* resource, std::size_t size = size::strlen);
		void	put_protocol(const char* protocol, std::size_t size = size::strlen);
	};


	// --------------------------------------------------------------


	template <typename Log = null_log>
	class http_response_istream : public _http_istream<Log> {
		using base  = _http_istream<Log>;

	public:
		http_response_istream(std::streambuf* sb, Log* log = nullptr);
		http_response_istream(http_response_istream&& other) = default;

		void	reset();

	public:
		void	get_protocol(char* buffer, std::size_t size);
		void	get_status_code(char* buffer, std::size_t size);
		void	get_reason_phrase(char* buffer, std::size_t size);
	};


	// --------------------------------------------------------------


	template <typename Log = null_log>
	class http_response_ostream : public _http_ostream<Log> {
		using base  = _http_ostream<Log>;

	public:
		http_response_ostream(std::streambuf* sb, Log* log = nullptr);
		http_response_ostream(http_response_ostream&& other) = default;

		void	reset();

	public:
		void	put_protocol(const char* protocol, std::size_t size = size::strlen);
		void	put_status_code(const char* status_code, std::size_t size = size::strlen);
		void	put_reason_phrase(const char* reason_phrase, std::size_t size = size::strlen);
	};


	// --------------------------------------------------------------


	template <typename Log = null_log>
	class http_client_stream : public http_request_ostream<Log>, public http_response_istream<Log> {
	public:
		http_client_stream(std::streambuf* sb, Log* log = nullptr);
		http_client_stream(http_client_stream&& other) = default;
	};


	// --------------------------------------------------------------


	template <typename Log = null_log>
	class http_server_stream : public http_request_istream<Log>, public http_response_ostream<Log> {
	public:
		http_server_stream(std::streambuf* sb, Log* log = nullptr);
		http_server_stream(http_server_stream&& other) = default;
	};

}

