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

#include <streambuf>
#include <istream>
#include <ostream>

#include "log.i.h"


namespace abc {

	template <typename LogPtr>
	class http_request_istream;

	template <typename LogPtr>
	class http_request_ostream;

	template <typename LogPtr>
	class http_response_istream;

	template <typename LogPtr>
	class http_response_ostream;


	template <typename LogPtr>
	class http_client_stream;

	template <typename LogPtr>
	class http_server_stream;


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

#ifdef MAYBE
		static constexpr const char* protocol_11				= "HTTP/1.1";

		namespace request {
			static constexpr const char* method_get				= "GET";
			static constexpr const char* method_post			= "POST";
			static constexpr const char* method_put				= "PUT";
			static constexpr const char* method_delete			= "DELETE";
			static constexpr const char* method_head			= "HEAD";
			static constexpr const char* method_connect			= "CONNECT";
			static constexpr const char* method_options			= "OPTIONS";
		}
#endif
	}

	// --------------------------------------------------------------


	template <typename StdStream, typename LogPtr>
	class _http_stream : protected StdStream {
	protected:
		_http_stream(std::streambuf* sb, http::item_t next, const LogPtr& log_ptr);
		_http_stream(_http_stream&& other) = default;

	public:
		http::item_t	next() const noexcept;

		std::size_t		gcount() const;
		bool			eof() const;
		bool			good() const;
		bool			bad() const;
		bool			fail() const;
		bool			operator!() const;
						operator bool() const;

	protected:
		void			reset(http::item_t next);
		void			assert_next(http::item_t item);
		void			set_next(http::item_t item) noexcept;
		void			set_gcount(std::size_t gcount) noexcept;
		bool			is_good() const;
		void			set_bad();
		void			set_fail();
		const LogPtr&	log_ptr() const noexcept;

	private:
		http::item_t	_next;
		std::size_t		_gcount;
		LogPtr			_log_ptr;

	};


	template <typename LogPtr>
	class _http_istream : public _http_stream<std::istream, LogPtr> {
	protected:
		_http_istream(std::streambuf* sb, http::item_t next, const LogPtr& log_ptr);
		_http_istream(_http_istream&& other) = default;

	public:
		void		get_header_name(char* buffer, std::size_t size);
		void		get_header_value(char* buffer, std::size_t size);
		void		get_body(char* buffer, std::size_t size);

	protected:
		void		get_protocol(char* buffer, std::size_t size);

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


	template <typename LogPtr>
	class _http_ostream : public _http_stream<std::ostream, LogPtr> {
	protected:
		_http_ostream(std::streambuf* sb, http::item_t next, const LogPtr& log_ptr);
		_http_ostream(_http_ostream&& other) = default;

	public:
		void		put_header_name(const char*buffer, std::size_t size = size::strlen);
		void		put_header_value(const char* buffer, std::size_t size = size::strlen);
		void		end_headers();
		void		put_body(const char* buffer, std::size_t size = size::strlen);

	protected:
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


	template <typename LogPtr = null_log_ptr>
	class http_request_istream : public _http_istream<LogPtr> {
	public:
		http_request_istream(std::streambuf* sb, const LogPtr& log_ptr);
		http_request_istream(http_request_istream&& other) = default;

		void	reset();

	public:
		void	get_method(char* buffer, std::size_t size);
		void	get_resource(char* buffer, std::size_t size);
		void	get_protocol(char* buffer, std::size_t size);
	};


	template <typename LogPtr = null_log_ptr>
	class http_request_ostream : public _http_ostream<LogPtr> {
	public:
		http_request_ostream(std::streambuf* sb, const LogPtr& log_ptr);
		http_request_ostream(http_request_ostream&& other) = default;

		void	reset();

	public:
		void	put_method(const char* method, std::size_t size = size::strlen);
		void	put_resource(const char* resource, std::size_t size = size::strlen);
		void	put_protocol(const char* protocol, std::size_t size = size::strlen);
	};


	// --------------------------------------------------------------


	template <typename LogPtr = null_log_ptr>
	class http_response_istream : public _http_istream<LogPtr> {
	public:
		http_response_istream(std::streambuf* sb, const LogPtr& log_ptr);
		http_response_istream(http_response_istream&& other) = default;

		void	reset();

	public:
		void	get_protocol(char* buffer, std::size_t size);
		void	get_status_code(char* buffer, std::size_t size);
		void	get_reason_phrase(char* buffer, std::size_t size);
	};


	template <typename LogPtr = null_log_ptr>
	class http_response_ostream : public _http_ostream<LogPtr> {
	public:
		http_response_ostream(std::streambuf* sb, const LogPtr& log_ptr);
		http_response_ostream(http_response_ostream&& other) = default;

		void	reset();

	public:
		void	put_protocol(const char* protocol, std::size_t size = size::strlen);
		void	put_status_code(const char* status_code, std::size_t size = size::strlen);
		void	put_reason_phrase(const char* reason_phrase, std::size_t size = size::strlen);
	};


	// --------------------------------------------------------------


	template <typename LogPtr = null_log_ptr>
	class http_client_stream : public http_request_ostream<LogPtr>, public http_response_istream<LogPtr> {
	public:
		http_client_stream(std::streambuf* sb, const LogPtr& log_ptr);
		http_client_stream(http_client_stream&& other) = default;
	};


	template <typename LogPtr = null_log_ptr>
	class http_server_stream : public http_request_istream<LogPtr>, public http_response_ostream<LogPtr> {
	public:
		http_server_stream(std::streambuf* sb, const LogPtr& log_ptr);
		http_server_stream(http_server_stream&& other) = default;
	};

}

