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


#include <iostream>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <future>
#include <thread>
#include <atomic>
#include <exception>
#include <cstring>

#include "../../src/exception.h"
#include "../../src/log.h"
#include "../../src/socket.h"
#include "../../src/http.h"


namespace abc { namespace samples {

	struct webserver_config {
		webserver_config(const char* port, std::size_t listen_queue_size, const char* root_dir, const char* files_prefix);

		const char* const	port;

		const std::size_t	listen_queue_size;

		const char* const	root_dir; 
		const std::size_t	root_dir_len; // Computed

		const char* const	files_prefix;
		const std::size_t	files_prefix_len; // Computed
	};


	// --------------------------------------------------------------


	struct webserver_limits {
		static constexpr std::size_t method_size		= abc::size::_32;
		static constexpr std::size_t resource_size		= abc::size::k2;
		static constexpr std::size_t protocol_size		= abc::size::_16;
		static constexpr std::size_t file_chunk_size	= abc::size::k1;
		static constexpr std::size_t fsize_size			= abc::size::_32;
	};


	// --------------------------------------------------------------


	namespace protocol {
		constexpr const char* HTTP_11					= "HTTP/1.1";
	}


	namespace method {
		constexpr const char* GET						= "GET";
		constexpr const char* POST						= "POST";
		constexpr const char* PUT						= "PUT";
		constexpr const char* DELETE					= "DELETE";
		constexpr const char* HEAD						= "HEAD";
	}


	namespace status_code {
		constexpr const char* OK						= "200";
		constexpr const char* Created					= "201";
		constexpr const char* Accepted					= "202";

		constexpr const char* Moved_Permanently			= "301";
		constexpr const char* Found						= "302";

		constexpr const char* Bad_Request				= "400";
		constexpr const char* Unauthorized				= "401";
		constexpr const char* Forbidden					= "403";
		constexpr const char* Not_Found					= "404";
		constexpr const char* Method_Not_Allowed		= "405";
		constexpr const char* Payload_Too_Large			= "413";
		constexpr const char* URI_Too_Long				= "414";
		constexpr const char* Too_Many_Requests			= "429";

		constexpr const char* Internal_Server_Error		= "500";
		constexpr const char* Not_Implemented			= "501";
		constexpr const char* Service_Unavailable		= "503";
	}


	namespace reason_phrase {
		constexpr const char* OK						= "OK";
		constexpr const char* Created					= "Created";
		constexpr const char* Accepted					= "Accepted";

		constexpr const char* Moved_Permanently			= "Moved Permanently";
		constexpr const char* Found						= "Found";

		constexpr const char* Bad_Request				= "Bad Request";
		constexpr const char* Unauthorized				= "Unauthorized";
		constexpr const char* Forbidden					= "Forbidden";
		constexpr const char* Not_Found					= "Not Found";
		constexpr const char* Method_Not_Allowed		= "Method Not Allowed";
		constexpr const char* Payload_Too_Large			= "Payload Too Large";
		constexpr const char* URI_Too_Long				= "URI Too Long";
		constexpr const char* Too_Many_Requests			= "Too Many Requests";

		constexpr const char* Internal_Server_Error		= "Internal Server Error";
		constexpr const char* Not_Implemented			= "Not Implemented";
		constexpr const char* Service_Unavailable		= "Service Unavailable";
	}


	namespace header {
		constexpr const char* Content_Type				= "Content-Type";
		constexpr const char* Content_Length			= "Content-Length";
	}


	namespace content_type {
		constexpr const char* text						= "text/plain; charset=utf-8";
		constexpr const char* html						= "text/html; charset=utf-8";
		constexpr const char* css						= "text/css; charset=utf-8";
		constexpr const char* javascript				= "text/javascript; charset=utf-8";
		constexpr const char* xml						= "text/xml; charset=utf-8";

		constexpr const char* json						= "application/json";

		constexpr const char* png						= "image/png";
		constexpr const char* jpeg						= "image/jpeg";
		constexpr const char* gif						= "image/gif";
		constexpr const char* bmp						= "image/bmp";
		constexpr const char* svg						= "image/svg+xml";
	}


	// --------------------------------------------------------------


	template <typename Limits, typename Log>
	class webserver {
	public:
		webserver(webserver_config* config, Log* log);

	public:
		std::future<void>	start_async();
		void				start();

	protected:
		virtual void		process_file_request(abc::http_server_stream<Log>& http, const char* method, const char* resource, const char* path);
		virtual void		process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource);
		virtual bool		is_file_request(const char* method, const char* resource);
		virtual void		send_simple_response(abc::http_server_stream<Log>& http, const char* status_code, const char* reason_phrase, const char* content_type, const char* body, abc::tag_t tag);
		virtual const char*	get_content_type_from_path(const char* path);

	protected:
		void				process_request(tcp_client_socket<Log>&& socket);
		void				set_shutdown_requested();

	protected:
		webserver_config*	_config;
		Log*				_log;

	private:
		std::promise<void>	_promise;
		std::atomic_int32_t	_requests_in_progress;
		std::atomic_bool	_is_shutdown_requested;
	};


	// --------------------------------------------------------------


	template <typename Limits, typename Log>
	inline webserver<Limits, Log>::webserver(webserver_config* config, Log* log)
		: _config(config)
		, _log(log)
		, _requests_in_progress(0)
		, _is_shutdown_requested(false) {
		if (log == nullptr) {
			throw abc::exception<std::logic_error>("Running a web server without logging is a bad idea.", 0x102dd);
		}
	}


	template <typename Limits, typename Log>
	inline std::future<void> webserver<Limits, Log>::start_async() {
		// We can't use std::async() here because we want to detach and return our own std::future.
		std::thread(&webserver<Limits, Log>::start, this).detach();

		// Therefore, we return our own future.
		return _promise.get_future();
	}


	template <typename Limits, typename Log>
	inline void webserver<Limits, Log>::start() {
		_log->put_blank_line();
		_log->put_blank_line();
		_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Started endpoint (%s)", _config->port);

		// Create a listener, bind to a port, and start listening.
		abc::tcp_server_socket listener(_log);
		listener.bind(_config->port);
		listener.listen(_config->listen_queue_size);

		_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Listening (%s)", _config->port);
		_log->put_blank_line();

		while (true) {
			// Accept the next request and process it asynchronously.
			abc::tcp_client_socket client = listener.accept();
			std::thread(&webserver<Limits, Log>::process_request, this, std::move(client)).detach();
		}
	}


	template <typename Limits, typename Log>
	inline void webserver<Limits, Log>::process_request(tcp_client_socket<Log>&& socket) {
		_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x102de, "Begin handling request (%s)", _config->port);

		// Create a socket_streambuf over the tcp_client_socket.
		abc::socket_streambuf sb(&socket);

		// Create an hhtp_server_stream, which combines http_request_istream and http_response_ostream.
		abc::http_server_stream<Log> http(&sb);

		// Read the request line.
		char method[Limits::method_size + 1];
		http.get_method(method, sizeof(method));
		_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x102df, "Received Method   = '%s'", method);

		char path[Limits::resource_size + 1];
		std::strcpy(path, _config->root_dir);
		char* resource = path + _config->root_dir_len;
		http.get_resource(resource, sizeof(path) - _config->root_dir_len);
		_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x102e0, "Received Resource = '%s'", resource);

		char protocol[Limits::protocol_size + 1];
		http.get_protocol(protocol, sizeof(protocol));
		_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x102e1, "Received Protocol = '%s'", protocol);

		// It's OK to read a request as long as we don't return a broken response.
		if (_is_shutdown_requested.load()) {
			return;
		}

		++_requests_in_progress;

		// This sample web server supports two kinds of requests:
		//    a) requests for static files
		//    b) REST requests
		if (is_file_request(method, resource)) {
			process_file_request(http, method, resource, path);
		}
		else {
			process_rest_request(http, method, resource);
		}

		// Don't forget to flush!
		http.flush();
		_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x102e2, "Response sent");
		_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x102e3, "End handling request (%s)", _config->port);
		_log->put_blank_line();

		if (--_requests_in_progress == 0 && _is_shutdown_requested.load()) {
			_log->put_blank_line();
			_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Stopped endpoint (%s)", _config->port);
			_log->put_blank_line();
			_log->put_blank_line();

			_promise.set_value();
		}
	}


	template <typename Limits, typename Log>
	inline void webserver<Limits, Log>::process_file_request(abc::http_server_stream<Log>& http, const char* method, const char* /*resource*/, const char* path) {
		_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x102e4, "Received File Path = '%s'", path);

		// If the method is not GET, return 405.
		if (std::strcmp(method, method::GET) != 0) {
			send_simple_response(http, status_code::Method_Not_Allowed, reason_phrase::Method_Not_Allowed, content_type::text, "GET is the only supported method for static files.", 0x102e5);
			return;
		}

		// Check if the file exists.
		_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x102e6, "CWD = %s", std::filesystem::current_path().c_str());
		std::error_code ec;
		std::uintmax_t fsize = std::filesystem::file_size(path, ec);

		// If the file was not opened, return 404.
		if (ec) {
			send_simple_response(http, status_code::Not_Found, reason_phrase::Not_Found, content_type::text, "Error: The requested resource was not found.", 0x102e7);
			return;
		}

		// The file was opened, return 200.
		char fsize_buffer[Limits::fsize_size + 1];
		std::snprintf(fsize_buffer, Limits::fsize_size, "%lu", fsize);
		_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x102e8, "File size = %s", fsize_buffer);
		
		_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x102e9, "Sending response 200");
		http.put_protocol(protocol::HTTP_11);
		http.put_status_code(status_code::OK);
		http.put_reason_phrase(reason_phrase::OK);

		const char* content_type = get_content_type_from_path(path);
		if (content_type != nullptr) {
			http.put_header_name(header::Content_Type);
			http.put_header_value(content_type);
		}

		http.put_header_name(header::Content_Length);
		http.put_header_value(fsize_buffer);
		http.end_headers();

		std::ifstream file(path);
		char file_chunk[Limits::file_chunk_size];
		for (std::uintmax_t sent_size = 0; sent_size < fsize; sent_size += Limits::file_chunk_size) {
			file.read(file_chunk, sizeof(file_chunk));
			http.put_body(file_chunk, file.gcount());
		}
	}


	template <typename Limits, typename Log>
	inline void webserver<Limits, Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x102ea, "Received REST");

		if (std::strcmp(method, method::POST) == 0 && std::strcmp(resource, "/shutdown") == 0) {
			set_shutdown_requested();
		}

		send_simple_response(http, status_code::OK, status_code::OK, content_type::text, "TODO: Override process_rest_request().", 0x102eb);
	}


	template <typename Limits, typename Log>
	inline void webserver<Limits, Log>::send_simple_response(abc::http_server_stream<Log>& http, const char* status_code, const char* reason_phrase, const char* content_type, const char* body, abc::tag_t tag) {
		_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x102ec, "Sending simple response");

		char content_length[Limits::fsize_size + 1];
		std::snprintf(content_length, Limits::fsize_size, "%lu", std::strlen(body));

		http.put_protocol(protocol::HTTP_11);
		http.put_status_code(status_code);
		http.put_reason_phrase(reason_phrase);
		http.put_header_name(header::Content_Type);
		http.put_header_value(content_type);
		http.put_header_name(header::Content_Length);
		http.put_header_value(content_length);
		http.end_headers();
		http.put_body(body);

		_log->put_any(abc::category::abc::samples, abc::severity::debug, tag, "Sent Status Code    = %s", status_code);
		_log->put_any(abc::category::abc::samples, abc::severity::debug, tag, "Sent Content-Type   = %s", content_type);
		_log->put_any(abc::category::abc::samples, abc::severity::debug, tag, "Sent Content-Length = %s", content_length);
		_log->put_any(abc::category::abc::samples, abc::severity::debug, tag, "Sent Body           = %s", body);
	}


	template <typename Limits, typename Log>
	inline const char* webserver<Limits, Log>::get_content_type_from_path(const char* path) {
		const char* ext = std::strrchr(path, '.');
		if (ext == nullptr) {
			return nullptr;
		}

		if (std::strcmp(ext, ".html") == 0) {
			return content_type::html;
		}
		else if (std::strcmp(ext, ".css") == 0) {
			return content_type::css;
		}
		else if (std::strcmp(ext, ".js") == 0) {
			return content_type::javascript;
		}
		else if (std::strcmp(ext, ".txt") == 0) {
			return content_type::text;
		}
		else if (std::strcmp(ext, ".xml") == 0) {
			return content_type::xml;
		}
		else if (std::strcmp(ext, ".png") == 0) {
			return content_type::png;
		}
		else if (std::strcmp(ext, ".jpeg") == 0) {
			return content_type::jpeg;
		}
		else if (std::strcmp(ext, ".jpg") == 0) {
			return content_type::jpeg;
		}
		else if (std::strcmp(ext, ".gif") == 0) {
			return content_type::gif;
		}
		else if (std::strcmp(ext, ".bmp") == 0) {
			return content_type::bmp;
		}
		else if (std::strcmp(ext, ".svg") == 0) {
			return content_type::svg;
		}
		
		return nullptr;
	}


	template <typename Limits, typename Log>
	inline bool webserver<Limits, Log>::is_file_request(const char* method, const char* resource) {
		return std::strncmp(resource, _config->files_prefix, _config->files_prefix_len) == 0
			|| (std::strcmp(method, method::GET) == 0 && std::strcmp(resource, "/favicon.ico") == 0);
	}


	template <typename Limits, typename Log>
	inline void webserver<Limits, Log>::set_shutdown_requested() {
		_log->put_any(abc::category::abc::samples, abc::severity::important, 0x102ed, "--- Shutdown requested ---");
		_is_shutdown_requested.store(true);
	}


	// --------------------------------------------------------------


	inline webserver_config::webserver_config(const char* port, std::size_t listen_queue_size, const char* root_dir, const char* files_prefix)
		: port(port)

		, listen_queue_size(listen_queue_size)

		, root_dir(root_dir)
		, root_dir_len(root_dir != nullptr ? std::strlen(root_dir) : 0)

		, files_prefix(files_prefix)
		, files_prefix_len(files_prefix != nullptr ? std::strlen(files_prefix) : 0) {
	}


	// --------------------------------------------------------------

}}


