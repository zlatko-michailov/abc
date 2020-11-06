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


#include <future>
#include <atomic>

#include "log.h"
#include "socket.h"
#include "http.h"


namespace abc {

	struct endpoint_config {
		endpoint_config(const char* port, std::size_t listen_queue_size, const char* root_dir, const char* files_prefix);

		const char* const	port;

		const std::size_t	listen_queue_size;

		const char* const	root_dir; 
		const std::size_t	root_dir_len; // Computed

		const char* const	files_prefix;
		const std::size_t	files_prefix_len; // Computed
	};


	// --------------------------------------------------------------


	struct endpoint_limits {
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
		constexpr const char* Connection				= "Connection";
		constexpr const char* Content_Type				= "Content-Type";
		constexpr const char* Content_Length			= "Content-Length";
	}


	namespace connection {
		constexpr const char* close						= "close";
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
	class endpoint {
	public:
		endpoint(endpoint_config* config, Log* log);

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
		endpoint_config*	_config;
		Log*				_log;

	private:
		std::promise<void>	_promise;
		std::atomic_int32_t	_requests_in_progress;
		std::atomic_bool	_is_shutdown_requested;
	};


	// --------------------------------------------------------------

}


