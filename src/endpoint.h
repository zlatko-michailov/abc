/*
MIT License

Copyright (c) 2018-2022 Zlatko Michailov 

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

#include <iostream>
#include <fstream>
#include <system_error>
#include <future>
#include <thread>
#include <atomic>
#include <exception>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "exception.h"
#include "log.h"
#include "socket.h"
#include "http.h"
#include "i/endpoint.i.h"


namespace abc {

	template <typename Limits, typename Log>
	inline endpoint<Limits, Log>::endpoint(endpoint_config* config, Log* log)
		: _config(config)
		, _log(log)
		, _requests_in_progress(0)
		, _is_shutdown_requested(false) {
	}


	template <typename Limits, typename Log>
	inline std::future<void> endpoint<Limits, Log>::start_async() {
		// We can't use std::async() here because we want to detach and return our own std::future.
		std::thread(&endpoint<Limits, Log>::start, this).detach();

		// Therefore, we return our own future.
		return _promise.get_future();
	}


	template <typename Limits, typename Log>
	inline void endpoint<Limits, Log>::start() {
		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::important, 0x102f1, "Started endpoint (%s)", _config->port);
		}

		// Create a listener, bind to a port, and start listening.
		abc::tcp_server_socket<Log> listener(_log);
		listener.bind(_config->port);
		listener.listen(_config->listen_queue_size);

		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::important, 0x102f2, "Listening (%s)", _config->port);
			_log->put_blank_line();
		}

		while (true) {
			// Accept the next request and process it asynchronously.
			abc::tcp_client_socket<Log> client = listener.accept();
			std::thread(&endpoint<Limits, Log>::process_request, this, std::move(client)).detach();
		}
	}


	template <typename Limits, typename Log>
	inline void endpoint<Limits, Log>::process_request(tcp_client_socket<Log>&& socket) {
		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::important, 0x102de, "Begin handling request (%s)", _config->port);
		}

		// Create a socket_streambuf over the tcp_client_socket.
		abc::socket_streambuf<tcp_client_socket<Log>, Log> sb(&socket);

		// Create an hhtp_server_stream, which combines http_request_istream and http_response_ostream.
		abc::http_server_stream<Log> http(&sb);

		// Read the request line.
		char method[Limits::method_size + 1];
		http.get_method(method, sizeof(method));
		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::important, 0x102df, "Received Method   = '%s'", method);
		}

		char path[Limits::resource_size + 1];
		std::strcpy(path, _config->root_dir);
		char* resource = path + _config->root_dir_len;
		http.get_resource(resource, sizeof(path) - _config->root_dir_len);
		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::important, 0x102e0, "Received Resource = '%s'", resource);
		}

		char protocol[Limits::protocol_size + 1];
		http.get_protocol(protocol, sizeof(protocol));
		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::important, 0x102e1, "Received Protocol = '%s'", protocol);
		}

		// It's OK to read a request as long as we don't return a broken response.
		if (_is_shutdown_requested.load()) {
			return;
		}

		++_requests_in_progress;

		// This endpoint supports two kinds of requests:
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
		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::optional, 0x102e2, "Response sent");
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::important, 0x102e3, "End handling request (%s)", _config->port);
			_log->put_blank_line();
		}

		if (--_requests_in_progress == 0 && _is_shutdown_requested.load()) {
			if (_log != nullptr) {
				_log->put_blank_line();
				_log->put_any(abc::category::abc::endpoint, abc::severity::abc::important, 0x102f3, "Stopped endpoint (%s)", _config->port);
			}

			_promise.set_value();
		}
	}


	template <typename Limits, typename Log>
	inline void endpoint<Limits, Log>::process_file_request(abc::http_server_stream<Log>& http, const char* method, const char* /*resource*/, const char* path) {
		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::important, 0x102e4, "Received File Path = '%s'", path);
		}

		// If the method is not GET, return 405.
		if (!ascii::are_equal_i(method, method::GET)) {
			send_simple_response(http, status_code::Method_Not_Allowed, reason_phrase::Method_Not_Allowed, content_type::text, "GET is the only supported method for static files.", 0x102e5);
			return;
		}

		// Check if the file exists.
		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::optional, 0x102e6, "Root dir = '%s'", _config->root_dir);
		}

		struct stat st;
		int err = ::stat(path, &st);

		// If the file was not opened, return 404.
		if (err != 0) {
			send_simple_response(http, status_code::Not_Found, reason_phrase::Not_Found, content_type::text, "Error: The requested resource was not found.", 0x102e7);
			return;
		}

		// The file was opened, return 200.
		std::size_t fsize = static_cast<std::size_t>(st.st_size);
		char fsize_buffer[Limits::fsize_size + 1];
		std::snprintf(fsize_buffer, Limits::fsize_size, "%zu", fsize);

		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::optional, 0x102e8, "File size = %s", fsize_buffer);
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::optional, 0x102e9, "Sending response 200");
		}

		http.put_protocol(protocol::HTTP_11);
		http.put_status_code(status_code::OK);
		http.put_reason_phrase(reason_phrase::OK);

		http.put_header_name(header::Connection);
		http.put_header_value(connection::close);

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
		for (std::size_t sent_size = 0; sent_size < fsize; sent_size += Limits::file_chunk_size) {
			file.read(file_chunk, sizeof(file_chunk));
			http.put_body(file_chunk, file.gcount());
		}
	}


	template <typename Limits, typename Log>
	inline void endpoint<Limits, Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::important, 0x102ea, "Received REST");
		}

		if (ascii::are_equal_i(method, method::POST) && ascii::are_equal_i(resource, "/shutdown")) {
			set_shutdown_requested();
		}

		send_simple_response(http, status_code::OK, status_code::OK, content_type::text, "TODO: Override process_rest_request().", 0x102eb);
	}


	template <typename Limits, typename Log>
	inline void endpoint<Limits, Log>::send_simple_response(abc::http_server_stream<Log>& http, const char* status_code, const char* reason_phrase, const char* content_type, const char* body, abc::tag_t tag) {
		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::optional, 0x102ec, "Sending simple response");
		}

		char content_length[Limits::fsize_size + 1];
		std::snprintf(content_length, Limits::fsize_size, "%zu", std::strlen(body));

		http.put_protocol(protocol::HTTP_11);
		http.put_status_code(status_code);
		http.put_reason_phrase(reason_phrase);

		http.put_header_name(header::Connection);
		http.put_header_value(connection::close);
		http.put_header_name(header::Content_Type);
		http.put_header_value(content_type);
		http.put_header_name(header::Content_Length);
		http.put_header_value(content_length);
		http.end_headers();

		http.put_body(body);

		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::optional, tag, "Sent Status Code    = %s", status_code);
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::optional, tag, "Sent Content-Type   = %s", content_type);
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::optional, tag, "Sent Content-Length = %s", content_length);
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::optional, tag, "Sent Body           = %s", body);
		}
	}


	template <typename Limits, typename Log>
	inline const char* endpoint<Limits, Log>::get_content_type_from_path(const char* path) {
		const char* ext = std::strrchr(path, '.');
		if (ext == nullptr) {
			return nullptr;
		}

		if (ascii::are_equal_i(ext, ".html")) {
			return content_type::html;
		}
		else if (ascii::are_equal_i(ext, ".css")) {
			return content_type::css;
		}
		else if (ascii::are_equal_i(ext, ".js")) {
			return content_type::javascript;
		}
		else if (ascii::are_equal_i(ext, ".txt")) {
			return content_type::text;
		}
		else if (ascii::are_equal_i(ext, ".xml")) {
			return content_type::xml;
		}
		else if (ascii::are_equal_i(ext, ".png")) {
			return content_type::png;
		}
		else if (ascii::are_equal_i(ext, ".jpeg")) {
			return content_type::jpeg;
		}
		else if (ascii::are_equal_i(ext, ".jpg")) {
			return content_type::jpeg;
		}
		else if (ascii::are_equal_i(ext, ".gif")) {
			return content_type::gif;
		}
		else if (ascii::are_equal_i(ext, ".bmp")) {
			return content_type::bmp;
		}
		else if (ascii::are_equal_i(ext, ".svg")) {
			return content_type::svg;
		}
		
		return nullptr;
	}


	template <typename Limits, typename Log>
	inline bool endpoint<Limits, Log>::is_file_request(const char* method, const char* resource) {
		return ascii::are_equal_i_n(resource, _config->files_prefix, _config->files_prefix_len)
			|| (ascii::are_equal_i(method, method::GET) && ascii::are_equal_i(resource, "/favicon.ico"));
	}


	template <typename Limits, typename Log>
	inline void endpoint<Limits, Log>::set_shutdown_requested() {
		if (_log != nullptr) {
			_log->put_any(abc::category::abc::endpoint, abc::severity::abc::important, 0x102ed, "--- Shutdown requested ---");
		}

		_is_shutdown_requested.store(true);
	}


	// --------------------------------------------------------------


	inline endpoint_config::endpoint_config(const char* port, std::size_t listen_queue_size, const char* root_dir, const char* files_prefix)
		: port(port)

		, listen_queue_size(listen_queue_size)

		, root_dir(root_dir)
		, root_dir_len(root_dir != nullptr ? std::strlen(root_dir) : 0)

		, files_prefix(files_prefix)
		, files_prefix_len(files_prefix != nullptr ? std::strlen(files_prefix) : 0) {
	}


	// --------------------------------------------------------------

}


