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


#include <future>
#include <atomic>

#include "log.i.h"
#include "socket.i.h"
#include "http.i.h"


namespace abc {

	/**
	 * @brief	`endpoint` settings.
	 */
	struct endpoint_config {
		/**
		 * @brief					Constructor. Properties can only be set at construction.
		 * 
		 * @param port				Port number to listen at.
		 * @param listen_queue_size	Maximum number of new connections pending to be processed.
		 * @param root_dir			Local directory that is the root for static files.
		 * @param files_prefix		Virtual path that maps to the root directory.
		 */
		endpoint_config(const char* port, std::size_t listen_queue_size, const char* root_dir, const char* files_prefix);

		/**
		 * @brief			Port number to listen at.
		 */
		const char* const	port;

		/**
		 * @brief			Maximum number of new connections pending to be processed.
		 */
		const std::size_t	listen_queue_size;

		/**
		 * @brief			Local directory that is the root for static files.
		 */
		const char* const	root_dir;

		/**
		 * @brief			Length of `root_dir`.
		 */
		const std::size_t	root_dir_len;

		/**
		 * @brief			Virtual path that maps to the root directory.
		 */
		const char* const	files_prefix;

		/**
		 * @brief			Length of `files_prefix`.
		 */
		const std::size_t	files_prefix_len;
	};


	// --------------------------------------------------------------

	/**
	 * @brief	Default limits for `endpoint` parse buffers.
	 */
	struct endpoint_limits {
		/**
		 * @brief						Maximum http method size - for GET, POST, DELETE, ...
		 */
		static constexpr std::size_t	method_size		= abc::size::_32;

		/**
		 * @brief						Maximum http resource size - for URL.
		 */
		static constexpr std::size_t	resource_size		= abc::size::k2;

		/**
		 * @brief						Maximum http protocol size - for http, https, ...
		 */
		static constexpr std::size_t	protocol_size		= abc::size::_16;

		/**
		 * @brief						Maximum http resource chunk size - for sending resources.
		 */
		static constexpr std::size_t	file_chunk_size	= abc::size::k1;

		/**
		 * @brief						Maximum http fsize size - for 64-bit numbers.
		 */
		static constexpr std::size_t	fsize_size			= abc::size::_32;
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

	/**
	 * @brief					Base http endpoint.
	 * @details					This class supports the most common functionality - static files and `POST /shutdown`.
	 * 							To handle other REST requests or special files, a derived class must be used.
	 * 
	 * @tparam Limits			Endpoint limits.
	 * @tparam Log				Logging facility.
	 * 
	 * @see endpoint_limits
	 * @see log_ostream
	 */
	template <typename Limits, typename Log>
	class endpoint {
	public:
		/**
		 * @brief				Constructor.
		 * 
		 * @param config		Pointer to an `endpoint_config` instance. Required.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		endpoint(endpoint_config* config, Log* log);

	public:
		/**
		 * @brief				Starts the endpoint on a separate thread.
		 * 
		 * @return				`std::future<void>` that will get set after a `POST /shutdown` is received from a client.
		 */
		std::future<void>		start_async();

		/**
		 * @brief				Starts the endpoint on the current thread.
		 * @details				This thread will block until a `POST /shutdown` is received from a client.
		 */
		void					start();

	protected:
		/**
		 * @brief				Processes a GET request for a static file.
		 * 
		 * @param http			Reference to a `http_server_stream`. 
		 * @param method		Http request method. Must be "GET".
		 * @param resource		Virtual path to the file.
		 * @param path			Local path to the file.
		 */
		virtual void			process_file_request(http_server_stream<Log>& http, const char* method, const char* resource, const char* path);

		/**
		 * @brief				Processes a REST request.
		 * 
		 * @param http			Reference to a `http_server_stream`.
		 * @param method		Http request method.
		 * @param resource		Virtual resource path.
		 */
		virtual void			process_rest_request(http_server_stream<Log>& http, const char* method, const char* resource);

		/**
		 * @brief				Checks if the resource is a static file.
		 * 
		 * @param method		Http request method.
		 * @param resource		Virtual resource path.
		 * 
		 * @return				Whether the resource is a static file.
		 */
		virtual bool			is_file_request(const char* method, const char* resource);

		/**
		 * @brief				Sends a response with the given content.
		 * 
		 * @param http			Reference to a `http_server_stream`.
		 * @param status_code	Http status code.
		 * @param reason_phrase	Http status reason phrase.
		 * @param content_type	Http Content-Type response header value.
		 * @param body			Http response body.
		 * @param tag			Logging tag provided by the caller, so that this response could be tracked as part of the call flow.
		 */
		virtual void			send_simple_response(http_server_stream<Log>& http, const char* status_code, const char* reason_phrase, const char* content_type, const char* body, tag_t tag);

		/**
		 * @brief				Determines the http response Content-Type based on the file extension.
		 * 
		 * @param path			Path (virtual or local) with an extension.
		 * 
		 * @return				The value for the Content-Type response header.
		 */
		virtual const char*		get_content_type_from_path(const char* path);

	protected:
		/**
		 * @brief				Processes (any kind of) a request.
		 * @details				This is the top-level method that reads the http request line, and calls either `process_file_request()` or `process_rest_request()`.
		 * 
		 * @param socket		Connection/client socket to read the request from and to send the response to.
		 */
		void					process_request(tcp_client_socket<Log>&& socket);

		/**
		 * @brief				Sets the "shutdown requested" flag.
		 */
		void					set_shutdown_requested();

		/**
		 * @brief				Gets the "shutdown requested" flag.
		 * 
		 * @return				The state of the "shutdown flag".
		 */
		bool					is_shutdown_requested() const;

	protected:
		/**
		 * @brief				The config settings pass in to the constructor.
		 */
		endpoint_config*		_config;

		/**
		 * @brief				The log passed in to the constructor.
		 */
		Log*					_log;

	private:
		/**
		 * @brief				The `std::promise` that is returned by `start_async()`, which gets signaled when shutdown is requested.
		 */
		std::promise<void>		_promise;

		/**
		 * @brief				Number of requests currently in progress.
		 */
		std::atomic_int32_t		_requests_in_progress;

		/**
		 * @brief				Flag that gets set when `POST /shutdown` is received.
		 */
		std::atomic_bool		_is_shutdown_requested;
	};


	// --------------------------------------------------------------

}


