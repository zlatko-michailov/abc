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
#include <exception>
#include <cstring>

#include "../../src/exception.h"
#include "../../src/log.h"
#include "../../src/socket.h"
#include "../../src/http.h"


namespace abc { namespace samples { namespace webserver {

	template <typename Log>
	class webserver {
	private:
		// Server parameters:
		static constexpr const char* root_dir			= "out/samples/webserver"; // No trailing slash!
		static const     std::size_t root_dir_len		= std::strlen(root_dir);
		static constexpr const char* port				= "30301";
		static constexpr const char* resources			= "/resources/";
		static const     std::size_t resources_len		= std::strlen(resources);
		static constexpr std::size_t queue_size			= 5;
		static constexpr std::size_t method_size		= abc::size::_16;
		static constexpr std::size_t resource_size		= abc::size::_512;
		static constexpr std::size_t protocol_size		= abc::size::_16;
		static constexpr std::size_t file_chunk_size	= abc::size::k1;

	public:
		webserver(Log* log);

	public:
		void take_over();

	protected:
		void process_request(tcp_client_socket<Log>&& socket);
		void process_file_request(abc::http_server_stream<Log>& http, const char* method, const char* path);
		void process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* path, const char* protocol);

	private:
		Log* _log;
	};


	// --------------------------------------------------------------

	template <typename Log>
	inline webserver<Log>::webserver(Log* log)
		: _log(log) {
		if (log == nullptr) {
			throw abc::exception<std::logic_error>("Running a web server without logging is a bad idea.", __TAG__);
		}
	}


	template <typename Log>
	inline void webserver<Log>::take_over() {
		_log->put_blank_line();
		_log->put_line("--------------------\n");
		_log->put_line("Press Ctrl+C to exit\n");
		_log->put_line("--------------------\n");
		_log->put_blank_line();
		_log->put_blank_line();

		// Create a listener, bind to a port, and start listening.
		abc::tcp_server_socket listener(_log);
		listener.bind(port);
		listener.listen(queue_size);

		while (true) {
			// Accept the next request and process it asynchronously.
			abc::tcp_client_socket client = std::move(listener.accept());
			std::future future = std::async([this, socket = std::move(client)]() mutable {
				process_request(std::move(socket));
			});
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
		std::strcpy(path, root_dir);
		char* resource = path + root_dir_len;
		http.get_resource(resource, sizeof(path) - root_dir_len);
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Received Resource = '%s'", resource);

		char protocol[protocol_size + 1];
		http.get_protocol(protocol, sizeof(protocol));
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Received Protocol = '%s'", protocol);

		// This sample web server supports two kinds of requests:
		//    a) requests for static files
		//    b) REST requests
		if (std::strncmp(resource, resources, resources_len) == 0) {
			process_file_request(http, method, path);
		}
		else {
			process_rest_request(http, method, resource, protocol);
		}

		// Don't forget to flush!
		http.flush();
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Response sent");
		_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "<<< Request");
		_log->put_blank_line();
	}


	template <typename Log>
	inline void webserver<Log>::process_file_request(abc::http_server_stream<Log>& http, const char* method, const char* path) {
		_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Received File Path = '%s'", path);

		// If the method is not GET, return 400.
		if (std::strcmp(method, "GET") != 0) {
			_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Error: Invalid method '%s' in a static file request. Must be 'GET'.", method);

			_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Sending response 400");
			http.put_protocol("HTTP/1.1");
			http.put_status_code("400");
			http.put_reason_phrase("Bad Request");
			http.put_header_name("Content-Length");
			http.put_header_value("50");
			http.end_headers();
			http.put_body("GET is the only supported method for static files.");

			return;
		}

		// Check if the file exists.
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "CWD = %s", std::filesystem::current_path().c_str());
		std::error_code ec;
		std::uintmax_t fsize = std::filesystem::file_size(path, ec);

		// If the file was not opened, return 404.
		if (ec) {
			_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Error: File not found");

			_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Sending response 404");
			http.put_protocol("HTTP/1.1");
			http.put_status_code("404");
			http.put_reason_phrase("Not Found");
			http.put_header_name("Content-Length");
			http.put_header_value("37");
			http.end_headers();
			http.put_body("The requested resource was not found.");

			return;
		}

		// The file was opened, return 200.
		char fsize_buffer[30 + 1];
		std::sprintf(fsize_buffer, "%llu", fsize);
		std::ifstream file(path);

		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Sending response 200");
		http.put_protocol("HTTP/1.1");
		http.put_status_code("200");
		http.put_reason_phrase("OK");
		http.put_header_name("Content-Length");
		http.put_header_value(fsize_buffer);
		http.end_headers();

		char file_chunk[file_chunk_size];
		for (std::uintmax_t sent_size = 0; sent_size < fsize; sent_size += file_chunk_size) {
			file.read(file_chunk, sizeof(file_chunk));
			http.put_body(file_chunk, file.gcount());
		}
	}


	template <typename Log>
	inline void webserver<Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* path, const char* protocol) {
		_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Received REST");

		// The request buffer will have to be able to hold a single token - resource, header name, header value.
		//// char request_buffer[request_buffer_size + 1];

		// The response buffer will have to be able to hold the entire response body.
		// That is because we need to know its size, which we should return as the Content-Length header before we return the body itself.
		//// char response_buffer[response_buffer_size + 1];

		// Read the request and write the response body at the same time.

		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Sending response");
		http.put_protocol("HTTP/1.1");
		http.put_status_code("200");
		http.put_reason_phrase("OK");
		http.put_header_name("Content-Length");
		http.put_header_value("28");
		http.end_headers();
		http.put_body("TODO: Echo the REST request.");
	}

}}}


