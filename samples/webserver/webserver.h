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


	namespace protocol {
		constexpr const char* http_11		= "HTTP/1.1";
	}


	namespace content_type {
		constexpr const char* text			= "text/plain; charset=utf-8";
		constexpr const char* html			= "text/html; charset=utf-8";
		constexpr const char* css			= "text/css; charset=utf-8";
		constexpr const char* javascript	= "text/javascript; charset=utf-8";
		constexpr const char* xml			= "text/xml; charset=utf-8";

		constexpr const char* png			= "image/png";
		constexpr const char* jpeg			= "image/jpeg";
		constexpr const char* gif			= "image/gif";
		constexpr const char* bmp			= "image/bmp";
		constexpr const char* svg			= "image/svg+xml";
	}


	// --------------------------------------------------------------


	template <typename Log>
	class webserver {
		static constexpr std::size_t method_size		= abc::size::_16;
		static constexpr std::size_t resource_size		= abc::size::_512;
		static constexpr std::size_t protocol_size		= abc::size::_16;
		static constexpr std::size_t file_chunk_size	= abc::size::k1;
		static constexpr std::size_t fsize_size			= abc::size::_16;

	public:
		webserver(webserver_config* config, Log* log);

	public:
		std::future<void>	start_async();
		void				start();

	protected:
		virtual void		process_file_request(abc::http_server_stream<Log>& http, const char* method, const char* resource, const char* path);
		virtual void		process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource);
		virtual bool		is_file_request(const char* method, const char* resource);
		virtual void		send_simple_response(abc::http_server_stream<Log>& http, const char* status_code, const char* content_type, const char* body);
		virtual const char*	get_content_type_from_path(const char* path);
		virtual const char*	get_reason_phrase_from_status_code(const char* status_code);

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


	template <typename Log>
	inline webserver<Log>::webserver(webserver_config* config, Log* log)
		: _config(config)
		, _log(log)
		, _requests_in_progress(0)
		, _is_shutdown_requested(false) {
		if (log == nullptr) {
			throw abc::exception<std::logic_error>("Running a web server without logging is a bad idea.", __TAG__);
		}
	}


	template <typename Log>
	inline std::future<void> webserver<Log>::start_async() {
		// We can't use std::async() here because we want to detach and return our own std::future.
		std::thread(&webserver<Log>::start, this).detach();

		// Therefore, we return our own future.
		return _promise.get_future();
	}


	template <typename Log>
	inline void webserver<Log>::start() {
		_log->put_blank_line();
		_log->put_blank_line();
		_log->put_line("Running...\n");
		_log->put_blank_line();

		// Create a listener, bind to a port, and start listening.
		abc::tcp_server_socket listener(_log);
		listener.bind(_config->port);
		listener.listen(_config->listen_queue_size);

		while (true) {
			// Accept the next request and process it asynchronously.
			abc::tcp_client_socket client = listener.accept();
			std::thread(&webserver<Log>::process_request, this, std::move(client)).detach();
		}
	}


	template <typename Log>
	inline void webserver<Log>::process_request(tcp_client_socket<Log>&& socket) {
		_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, ">>> Request");

		// Create a socket_streambuf over the tcp_client_socket.
		abc::socket_streambuf sb(&socket);

		// Create an hhtp_server_stream, which combines http_request_istream and http_response_ostream.
		abc::http_server_stream<Log> http(&sb);

		// Read the request line.
		char method[method_size + 1];
		http.get_method(method, sizeof(method));
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Received Method   = '%s'", method);

		char path[resource_size + 1];
		std::strcpy(path, _config->root_dir);
		char* resource = path + _config->root_dir_len;
		http.get_resource(resource, sizeof(path) - _config->root_dir_len);
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Received Resource = '%s'", resource);

		char protocol[protocol_size + 1];
		http.get_protocol(protocol, sizeof(protocol));
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Received Protocol = '%s'", protocol);

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
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Response sent");
		_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "<<< Request");
		_log->put_blank_line();

		if (--_requests_in_progress == 0 && _is_shutdown_requested.load()) {
			_log->put_blank_line();
			_log->put_line("Down.\n");
			_log->put_blank_line();
			_log->put_blank_line();

			_promise.set_value();
		}
	}


	template <typename Log>
	inline void webserver<Log>::process_file_request(abc::http_server_stream<Log>& http, const char* method, const char* /*resource*/, const char* path) {
		_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Received File Path = '%s'", path);

		// If the method is not GET, return 405.
		if (std::strcmp(method, "GET") != 0) {
			_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Error: Invalid method '%s' in a static file request. Must be 'GET'.", method);

			send_simple_response(http, "405", content_type::text, "GET is the only supported method for static files.");
			return;
		}

		// Check if the file exists.
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "CWD = %s", std::filesystem::current_path().c_str());
		std::error_code ec;
		std::uintmax_t fsize = std::filesystem::file_size(path, ec);

		// If the file was not opened, return 404.
		if (ec) {
			_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Error: File not found");

			send_simple_response(http, "404", content_type::text, "The requested resource was not found.");
			return;
		}

		// The file was opened, return 200.
		char fsize_buffer[fsize_size + 1];
		std::snprintf(fsize_buffer, fsize_size, "%llu", fsize);
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "File size = %s", fsize_buffer);
		
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Sending response 200");
		http.put_protocol("HTTP/1.1");
		http.put_status_code("200");
		http.put_reason_phrase("OK");

		const char* content_type = get_content_type_from_path(path);
		if (content_type != nullptr) {
			http.put_header_name("Content-Type");
			http.put_header_value(content_type);
		}

		http.put_header_name("Content-Length");
		http.put_header_value(fsize_buffer);
		http.end_headers();

		std::ifstream file(path);
		char file_chunk[file_chunk_size];
		for (std::uintmax_t sent_size = 0; sent_size < fsize; sent_size += file_chunk_size) {
			file.read(file_chunk, sizeof(file_chunk));
			http.put_body(file_chunk, file.gcount());
		}
	}


	template <typename Log>
	inline void webserver<Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Received REST");

		if (std::strcmp(method, "POST") == 0 && std::strcmp(resource, "/shutdown") == 0) {
			set_shutdown_requested();
		}

		send_simple_response(http, "200", content_type::text, "TODO: Override process_rest_request().");
	}


	template <typename Log>
	inline void webserver<Log>::send_simple_response(abc::http_server_stream<Log>& http, const char* status_code, const char* content_type, const char* body) {
		_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Sending simple response");

		char content_length[fsize_size + 1];
		std::snprintf(content_length, fsize_size, "%llu", std::strlen(body));

		http.put_protocol(protocol::http_11);
		http.put_status_code(status_code);
		http.put_reason_phrase(get_reason_phrase_from_status_code(status_code));
		http.put_header_name("Content-Type");
		http.put_header_value(content_type);
		http.put_header_name("Content-Length");
		http.put_header_value(content_length);
		http.end_headers();
		http.put_body(body);

		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Sent Status Code    = %s", status_code);
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Sent Content-Type   = %s", content_type);
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Sent Content-Length = %s", content_length);
	}


	template <typename Log>
	inline const char* webserver<Log>::get_content_type_from_path(const char* path) {
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


	template <typename Log>
	inline const char* webserver<Log>::get_reason_phrase_from_status_code(const char* status_code) {
		if (std::strcmp(status_code, "200") == 0) {
			return "OK";
		}
		else if (std::strcmp(status_code, "201") == 0) {
			return "Created";
		}
		else if (std::strcmp(status_code, "202") == 0) {
			return "Accepted";
		}

		else if (std::strcmp(status_code, "301") == 0) {
			return "Moved Permanently";
		}
		else if (std::strcmp(status_code, "302") == 0) {
			return "Found";
		}

		else if (std::strcmp(status_code, "400") == 0) {
			return "Bad Request";
		}
		else if (std::strcmp(status_code, "401") == 0) {
			return "Unauthorized";
		}
		else if (std::strcmp(status_code, "403") == 0) {
			return "Forbidden";
		}
		else if (std::strcmp(status_code, "404") == 0) {
			return "Not Found";
		}
		else if (std::strcmp(status_code, "405") == 0) {
			return "Method Not Allowed";
		}
		else if (std::strcmp(status_code, "413") == 0) {
			return "Payload Too Large";
		}
		else if (std::strcmp(status_code, "414") == 0) {
			return "URI Too Long";
		}
		else if (std::strcmp(status_code, "429") == 0) {
			return "Too Many Requests";
		}

		else if (std::strcmp(status_code, "500") == 0) {
			return "Internal Server Error";
		}
		else if (std::strcmp(status_code, "501") == 0) {
			return "Not Implemented";
		}
		else if (std::strcmp(status_code, "503") == 0) {
			return "Service Unavailable";
		}
		
		return nullptr;
	}


	template <typename Log>
	inline bool webserver<Log>::is_file_request(const char* method, const char* resource) {
		return std::strncmp(resource, _config->files_prefix, _config->files_prefix_len) == 0
			|| (std::strcmp(method, "GET") == 0 && std::strcmp(resource, "/favicon.ico") == 0);
	}


	template <typename Log>
	inline void webserver<Log>::set_shutdown_requested() {
		_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "--- Shutdown requested ---");
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


