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
		using stream_state_t = std::uint8_t;

		namespace stream_state {
			constexpr stream_state_t not_started			= 0;
			constexpr stream_state_t after_method			= 1;
			constexpr stream_state_t after_resource			= 2;
			constexpr stream_state_t after_protocol			= 3;
			constexpr stream_state_t after_headername		= 4;
			constexpr stream_state_t after_headervalue		= 5;
			constexpr stream_state_t after_all_headers		= 6;
			constexpr stream_state_t complete				= 7;
		}

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
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	class _http_stream : protected std::istream {
	public:
		_http_stream(std::streambuf* sb, const LogPtr& log_ptr);
		_http_stream(_http_stream&& other) = default;

	public:
		http::stream_state_t	http_state() const noexcept;

		std::size_t				gcount() const;
		bool					eof() const;
		bool					good() const;
		bool					bad() const;
		bool					fail() const;
		bool					operator!() const;
								operator bool() const;

	protected:
		void					assert_http_state(http::stream_state_t http_state);
		void					set_http_state(http::stream_state_t http_state) noexcept;
		void					set_gcount(std::size_t gcount) noexcept;
		bool					is_good() const;
		void					set_bad();
		void					set_fail();
		const LogPtr&			log_ptr() const noexcept;

	private:
		http::stream_state_t	_http_state;
		std::size_t				_gcount;
		LogPtr					_log_ptr;

	};


	template <typename LogPtr>
	class _http_istream : public _http_stream<LogPtr> {
	public:
		_http_istream(std::streambuf* sb, const LogPtr& log_ptr);
		_http_istream(_http_istream&& other) = default;

	public:
		void		get_headername(char* buffer, std::size_t size);
		void		get_headervalue(char* buffer, std::size_t size);
		void		get_body(char* buffer, std::size_t size);

	protected:
		std::size_t	get_token(char* buffer, std::size_t size);
		std::size_t	get_prints(char* buffer, std::size_t size);
		std::size_t	get_alphas(char* buffer, std::size_t size);
		std::size_t	get_digits(char* buffer, std::size_t size);
		std::size_t	skip_spaces();
		std::size_t	skip_crlf();

		template <typename Predicate>
		std::size_t	get_chars(Predicate&& predicate, char* buffer, std::size_t size);
		template <typename Predicate>
		std::size_t	skip_chars(Predicate&& predicate);
		char		get_char();
		char		peek_char();
	};


	template <typename LogPtr>
	class _http_ostream : public _http_stream<LogPtr> {
	public:
		_http_ostream(std::streambuf* sb, const LogPtr& log_ptr);
		_http_ostream(_http_ostream&& other) = default;

	public:
		void	put_headername(const char* headername);
		void	put_headervalue(const char* headervalue);
		void	put_body(const char* body);
	};


	template <typename LogPtr = null_log_ptr>
	class http_request_istream : public _http_istream<LogPtr> {
	public:
		http_request_istream(std::streambuf* sb, const LogPtr& log_ptr);
		http_request_istream(http_request_istream&& other) = default;

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

	public:
		void	put_method(const char* method);
		void	put_resource(const char* resource);
		void	put_protocol(const char* protocol);
	};


	// --------------------------------------------------------------

}

