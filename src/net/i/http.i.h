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


	/**
	 * @brief				Internal. http semantic state. 
	 * @tparam Log			Logging facility.
	 */
	template <typename Log>
	class http_state {
	protected:
		/**
		 * @brief			Constructor.
		 * @param next		Next item.
		 * @param log		Pointer to a `Log` instance. May be `nullptr`.
		 */
		http_state(http::item_t next, Log* log) noexcept;

		/**
		 * @brief			Move constructor.
		 */
		http_state(http_state&& other) = default;

		/**
		 * @brief			Copy constructor.
		 */
		http_state(const http_state& other) = default;

	public:
		/**
		 * @brief			Returns the next expected item.
		 */
		http::item_t next() const noexcept;

	protected:
		/**
		 * @brief			Resets the next item.
		 * @param next		Next item.
		 */
		void reset(http::item_t next);

		/**
		 * @brief			Throws if `item` doesn't match `next()`.
		 * @param item		Item to verify.
		 */
		void assert_next(http::item_t item);

		/**
		 * @brief			Returns the log passed in to the constructor.
		 */
		Log* log() const noexcept;

	private:
		/**
		 * @brief			The next expected item.
		 */
		http::item_t _next;

		/**
		 * @brief			The log passed in to the constructor.
		 */
		Log* _log;
	};


	// --------------------------------------------------------------


	/**
	 * @brief				Internal. Common http input stream. Used to read a request on the server or to read a response on the client.
	 * @tparam Log			Logging facility.
	 */
	template <typename Log>
	class http_istream : public istream, public http_state<Log> {
		using base  = istream;
		using state = http_state<Log>;

	protected:
		/**
		 * @brief			Constructor.
		 * @param sb		`std::streambuf` to read from.
		 * @param next		Next expected item.
		 * @param log		Pointer to a `Log` instance. May be `nullptr`.
		 */
		http_istream(std::streambuf* sb, http::item_t next, Log* log);

		/**
		 * @brief			Move constructor.
		 */
		http_istream(http_istream&& other);

		/**
		 * @brief			Deleted.
		 */
		http_istream(const http_istream& other) = delete;

	public:
		/**
		 * @brief			Reads a header name from the http stream, and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 */
		void get_header_name(char* buffer, std::size_t size);

		/**
		 * @brief			Reads a header value from the http stream, and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 */
		void get_header_value(char* buffer, std::size_t size);

		/**
		 * @brief			Reads a body from the http stream, and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 */
		void get_body(char* buffer, std::size_t size);

	protected:
		/**
		 * @brief			Set the gcount and the next expected item for this input stream.
		 * @param gcount	gcount.
		 * @param next		Next expected item.
		 */
		void set_gstate(std::size_t gcount, http::item_t next);

		/**
		 * @brief			Reads the protocol and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 * @return			The count of chars read. 
		 */
		std::size_t get_protocol(char* buffer, std::size_t size);

		/**
		 * @brief			Reads an http token and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 * @return			The count of chars read. 
		 */
		std::size_t get_token(char* buffer, std::size_t size);

		/**
		 * @brief			Reads a sequence of printable chars and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 * @return			The count of chars read. 
		 */
		std::size_t get_prints(char* buffer, std::size_t size);

		/**
		 * @brief			Reads a sequence of printable chars and spaces and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 * @return			The count of chars read. 
		 */
		std::size_t get_prints_and_spaces(char* buffer, std::size_t size);

		/**
		 * @brief			Reads a sequence of letters and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 * @return			The count of chars read. 
		 */
		std::size_t get_alphas(char* buffer, std::size_t size);

		/**
		 * @brief			Reads a sequence of digits and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 * @return			The count of chars read. 
		 */
		std::size_t get_digits(char* buffer, std::size_t size);

		/**
		 * @brief			Skips a sequence of spaces.
		 * @return			The count of chars read. 
		 */
		std::size_t skip_spaces();

		/**
		 * @brief			Skips a CR LF sequence.
		 * @return			The count of chars read. 
		 */
		std::size_t skip_crlf();

		/**
		 * @brief			Reads a sequence of chars with no restrictions and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 * @return			The count of chars read. 
		 */
		std::size_t get_bytes(char* buffer, std::size_t size);

		/**
		 * @brief			Reads a sequence of chars that match a predicate and copies it into the given buffer.
		 * @param predicate	Predicate.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 * @return			The count of chars read. 
		 */
		std::size_t get_chars(ascii::predicate_t&& predicate, char* buffer, std::size_t size);

		/**
		 * @brief			Skips a sequence of chars that match a predicate.
		 * @param predicate	Predicate.
		 * @return			The count of chars read. 
		 */
		std::size_t skip_chars(ascii::predicate_t&& predicate);

		/**
		 * @brief			Gets the next char from the stream and moves forward.
		 * @return			The next char.
		 */
		char get_char();

		/**
		 * @brief			Returns the next char from the stream without moving forward.
		 */
		char peek_char();
	};


	// --------------------------------------------------------------


	/**
	 * @brief				Internal. Common http output stream. Used to write a request on the client or to write a response on the server.
	 * @tparam Log			Logging facility.
	 */
	template <typename Log>
	class http_ostream : public ostream, public http_state<Log> {
		using base  = ostream;
		using state = http_state<Log>;

	protected:
		/**
		 * @brief			Constructor.
		 * @param sb		`std::streambuf` to write to.
		 * @param next		Next expected item.
		 * @param log		Pointer to a `Log` instance. May be `nullptr`.
		 */
		http_ostream(std::streambuf* sb, http::item_t next, Log* log);

		/**
		 * @brief			Move constructor.
		 */
		http_ostream(http_ostream&& other);

		/**
		 * @brief			Deleted.
		 */
		http_ostream(const http_ostream& other) = delete;

	public:
		/**
		 * @brief			Writes a header name to the http stream from the given buffer.
		 * @param buffer	Buffer to copy from.
		 * @param size		Content size.
		 */
		void put_header_name(const char*buffer, std::size_t size = size::strlen);

		/**
		 * @brief			Writes a header value to the http stream from the given buffer.
		 * @param buffer	Buffer to copy from.
		 * @param size		Content size.
		 */
		void put_header_value(const char* buffer, std::size_t size = size::strlen);

		/**
		 * @brief			Writes the end of headers to the http stream.
		 */
		void end_headers();

		/**
		 * @brief			Writes a body to the http stream from the given buffer.
		 * @param buffer	Buffer to copy from.
		 * @param size		Content size.
		 */
		void put_body(const char* buffer, std::size_t size = size::strlen);

	protected:
		/**
		 * @brief			Sets the next expected item for the stream.
		 * @param next		The next expected item.
		 */
		void set_pstate(http::item_t next);

		/**
		 * @brief			Writes a protocol to the http stream from the given buffer.
		 * @param buffer	Buffer to copy from.
		 * @param size		Content size.
		 * @return			The count of chars written. 
		 */
		std::size_t put_protocol(const char* buffer, std::size_t size);

		/**
		 * @brief			Writes a token to the http stream from the given buffer.
		 * @param buffer	Buffer to copy from.
		 * @param size		Content size.
		 * @return			The count of chars written. 
		 */
		std::size_t put_token(const char* buffer, std::size_t size);

		/**
		 * @brief			Writes a sequence of printable chars to the http stream from the given buffer.
		 * @param buffer	Buffer to copy from.
		 * @param size		Content size.
		 * @return			The count of chars written. 
		 */
		std::size_t put_prints(const char* buffer, std::size_t size);

		/**
		 * @brief			Writes a sequence of printable chars and spaces to the http stream from the given buffer.
		 * @param buffer	Buffer to copy from.
		 * @param size		Content size.
		 * @return			The count of chars written. 
		 */
		std::size_t put_prints_and_spaces(const char* buffer, std::size_t size);

		/**
		 * @brief			Writes a sequence of letters to the http stream from the given buffer.
		 * @param buffer	Buffer to copy from.
		 * @param size		Content size.
		 * @return			The count of chars written. 
		 */
		std::size_t put_alphas(const char* buffer, std::size_t size);

		/**
		 * @brief			Writes a sequence of digits to the http stream from the given buffer.
		 * @param buffer	Buffer to copy from.
		 * @param size		Content size.
		 * @return			The count of chars written. 
		 */
		std::size_t put_digits(const char* buffer, std::size_t size);

		/**
		 * @brief			Writes a CR LF sequence to the http stream.
		 * @return			The count of chars written. 
		 */
		std::size_t put_crlf();

		/**
		 * @brief			Writes a space to the http stream.
		 * @return			The count of chars written. 
		 */
		std::size_t put_space();

		/**
		 * @brief			Writes a sequence of bytes to the http stream from the given buffer.
		 * @param buffer	Buffer to copy from.
		 * @param size		Content size.
		 * @return			The count of chars written. 
		 */
		std::size_t put_bytes(const char* buffer, std::size_t size);

		/**
		 * @brief			Writes a sequence of chars that match a predicate to the http stream from the given buffer.
		 * @param predicate	Predicate.
		 * @param buffer	Buffer to copy from.
		 * @param size		Content size.
		 * @return			The count of chars written. 
		 */
		std::size_t put_chars(ascii::predicate_t&& predicate, const char* buffer, std::size_t size);

		/**
		 * @brief			Writes a char to the http stream.
		 * @param ch		Char to write.
		 * @return			1 = success. 0 = error.
		 */
		std::size_t put_char(char ch);

		/**
		 * @brief			Skips/counts the leading spaces from a header value residing in the given buffer. Does not write to the http stream.
		 * @param buffer	Buffer.
		 * @param size		Content size.
		 * @return			The count of chars. 
		 */
		std::size_t skip_spaces_in_header_value(const char* buffer, std::size_t size);

		/**
		 * @brief			Skips/counts the leading spaces from the given buffer. Does not write to the http stream.
		 * @param buffer	Buffer.
		 * @param size		Content size.
		 * @return			The count of chars. 
		 */
		std::size_t skip_spaces(const char* buffer, std::size_t size);
	};


	// --------------------------------------------------------------


	/**
	 * @brief				http request input stream. Used on the server side to read requests.
	 * @tparam Log			Logging facility.
	 */
	template <typename Log = null_log>
	class http_request_istream : public http_istream<Log> {
		using base  = http_istream<Log>;

	public:
		/**
		 * @brief			Constructor.
		 * @param sb		`std::streambuf` to read from.
		 * @param log		Pointer to a `Log` instance. May be `nullptr`.
		 */
		http_request_istream(std::streambuf* sb, Log* log = nullptr);

		/**
		 * @brief			Move constructor.
		 */
		http_request_istream(http_request_istream&& other);

		/**
		 * @brief			Deleted.
		 */
		http_request_istream(const http_request_istream& other) = delete;

		/**
		 * @brief			Resets the read state.
		 */
		void reset();

	public:
		/**
		 * @brief			Reads an http method from the http stream, and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 */
		void get_method(char* buffer, std::size_t size);

		/**
		 * @brief			Reads an http resource from the http stream, and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 */
		void get_resource(char* buffer, std::size_t size);

		/**
		 * @brief			Reads an http protocol from the http stream, and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 */
		void get_protocol(char* buffer, std::size_t size);

	public:
		/**
		 * @brief			Splits an http resource by replacing the '?' and each '&' with '\0' in the given buffer.
		 * @param buffer	Buffer where the resource is modified in place.
		 * @param size		Buffer size.
		 */
		static void split_resource(char* buffer, std::size_t size);

		/**
		 * @brief Get the resource parameter object
		 * @param buffer	Buffer where the resource is already split.
		 * @param size		Buffer size.
		 * @param parameter_name Parameter name
		 * @return			Pointer to the beginning of the parameter or `nullptr` if there is no such parameter.
		 */
		static const char* get_resource_parameter(const char* buffer, std::size_t size, const char* parameter_name);
	};


	// --------------------------------------------------------------


	/**
	 * @brief				http request output stream. Used on the client side to write requests.
	 * @tparam Log			Logging facility.
	 */
	template <typename Log = null_log>
	class http_request_ostream : public http_ostream<Log> {
		using base  = http_ostream<Log>;

	public:
		/**
		 * @brief			Constructor.
		 * @param sb		`std::streambuf` to write to.
		 * @param log		Pointer to a `Log` instance. May be `nullptr`.
		 */
		http_request_ostream(std::streambuf* sb, Log* log = nullptr);

		/**
		 * @brief			Move constructor.
		 */
		http_request_ostream(http_request_ostream&& other);

		/**
		 * @brief			Deleted.
		 */
		http_request_ostream(const http_request_ostream& other) = delete;

		/**
		 * @brief			Resets the write state.
		 */
		void reset();

	public:
		/**
		 * @brief			Writes an http method to the http stream.
		 * @param method	Buffer to copy from.
		 * @param size		Content size.
		 */
		void put_method(const char* method, std::size_t size = size::strlen);

		/**
		 * @brief			Writes an http resource to the http stream.
		 * @param resource	Buffer to copy from.
		 * @param size		Content size.
		 */
		void put_resource(const char* resource, std::size_t size = size::strlen);

		/**
		 * @brief			Writes an http protocol to the http stream.
		 * @param protocol	Buffer to copy from.
		 * @param size		Content size.
		 */
		void put_protocol(const char* protocol, std::size_t size = size::strlen);
	};


	// --------------------------------------------------------------


	/**
	 * @brief				http response input stream. Used on the client side to read responses.
	 * @tparam Log			Logging facility.
	 */
	template <typename Log = null_log>
	class http_response_istream : public http_istream<Log> {
		using base  = http_istream<Log>;

	public:
		/**
		 * @brief			Constructor.
		 * @param sb		`std::streambuf` to read from.
		 * @param log		Pointer to a `Log` instance. May be `nullptr`.
		 */
		http_response_istream(std::streambuf* sb, Log* log = nullptr);

		/**
		 * @brief			Move constructor.
		 */
		http_response_istream(http_response_istream&& other);

		/**
		 * @brief			Deleted.
		 */
		http_response_istream(const http_response_istream& other) = delete;

		/**
		 * @brief			Resets the read state.
		 */
		void reset();

	public:
		/**
		 * @brief			Reads an http protocol from the http stream, and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 */
		void get_protocol(char* buffer, std::size_t size);

		/**
		 * @brief			Reads an http status code from the http stream, and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 */
		void get_status_code(char* buffer, std::size_t size);

		/**
		 * @brief			Reads an http reason phrase from the http stream, and copies it into the given buffer.
		 * @param buffer	Buffer to copy to.
		 * @param size		Buffer size.
		 */
		void get_reason_phrase(char* buffer, std::size_t size);
	};


	// --------------------------------------------------------------


	/**
	 * @brief					http response output stream. Used on the server side to write responses.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log = null_log>
	class http_response_ostream : public http_ostream<Log> {
		using base  = http_ostream<Log>;

	public:
		/**
		 * @brief				Constructor.
		 * @param sb			`std::streambuf` to write to.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		http_response_ostream(std::streambuf* sb, Log* log = nullptr);

		/**
		 * @brief				Move constructor.
		 */
		http_response_ostream(http_response_ostream&& other);

		/**
		 * @brief				Deleted.
		 */
		http_response_ostream(const http_response_ostream& other) = delete;

		/**
		 * @brief				Resets the write state.
		 */
		void reset();

	public:
		/**
		 * @brief				Writes an http protocol to the http stream.
		 * @param protocol		Buffer to copy from.
		 * @param size			Content size.
		 */
		void put_protocol(const char* protocol, std::size_t size = size::strlen);

		/**
		 * @brief				Writes an http status code to the http stream.
		 * @param status_code	Buffer to copy from.
		 * @param size			Content size.
		 */
		void put_status_code(const char* status_code, std::size_t size = size::strlen);

		/**
		 * @brief				Writes an http reason phrase to the http stream.
		 * @param reason_phrase	Buffer to copy from.
		 * @param size			Content size.
		 */
		void put_reason_phrase(const char* reason_phrase, std::size_t size = size::strlen);
	};


	// --------------------------------------------------------------


	/**
	 * @brief			Combination of `http_request_ostream` and `http_response_istream`. Used on the client side.
	 * @tparam Log		Logging facility.
	 */
	template <typename Log = null_log>
	class http_client_stream : public http_request_ostream<Log>, public http_response_istream<Log> {
	public:
		/**
		 * @brief		Constructor.
		 * 
		 * @param sb	`std::streambuf` to read from and to write to.
		 * @param log	Pointer to a `Log` instance. May be `nullptr`.
		 */
		http_client_stream(std::streambuf* sb, Log* log = nullptr);

		/**
		 * @brief		Move constructor.
		 */
		http_client_stream(http_client_stream&& other);

		/**
		 * @brief		Deleted.
		 */
		http_client_stream(const http_client_stream& other) = delete;
	};


	// --------------------------------------------------------------


	/**
	 * @brief			Combination of `http_request_istream` and `http_response_ostream`. Used on the server side.
	 * @tparam Log		Logging facility.
	 */
	template <typename Log = null_log>
	class http_server_stream : public http_request_istream<Log>, public http_response_ostream<Log> {
	public:
		/**
		 * @brief		Constructor.
		 * @param sb	`std::streambuf` to read from and to write to.
		 * @param log	Pointer to a `Log` instance. May be `nullptr`.
		 */
		http_server_stream(std::streambuf* sb, Log* log = nullptr);

		/**
		 * @brief		Move constructor.
		 */
		http_server_stream(http_server_stream&& other);

		/**
		 * @brief		Deleted.
		 */
		http_server_stream(const http_server_stream& other) = delete;
	};

}

