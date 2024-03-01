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


#include <future>
#include <atomic>
#include <string>

#include "../../size.h"
#include "../../diag/i/diag_ready.i.h"
#include "socket.i.h"
#include "http.i.h"


namespace abc { namespace net { namespace http {

    /**
     * @brief `endpoint` settings.
     */
    struct endpoint_config {
        /**
         * @brief                    Constructor. Properties can only be set at construction.
         * @param port               Port number to listen at.
         * @param listen_queue_size  Maximum number of new connections pending to be processed.
         * @param root_dir           Local directory that is the root for static files.
         * @param files_prefix       Virtual path that maps to the root directory.
         * @param cert_file_path     Full path to the TLS certificate file. May be `""`.
         * @param pkey_file_path     Full path to the TLS private key file. May be `""`.
         * @param pkey_file_password Password for the TLS private key file. May be `""`.
         */
        endpoint_config(const char* port, std::size_t listen_queue_size, const char* root_dir, const char* files_prefix,
                        const char* cert_file_path = "", const char* pkey_file_path = "", const char* pkey_file_password = "");

        /**
         * @brief Port number to listen at.
         */
        const std::string port;

        /**
         * @brief Maximum number of new connections pending to be processed.
         */
        const std::size_t listen_queue_size;

        /**
         * @brief Local directory that is the root for static files.
         */
        const std::string root_dir;

        /**
         * @brief Virtual path that maps to the root directory.
         */
        const std::string files_prefix;

        /**
         * @brief Full path to the TLS certificate file.
         */
        const std::string cert_file_path;

        /**
         * @brief Full path to the TLS private key file.
         */
        const std::string pkey_file_path;

        /**
         * @brief Password for the TLS private key file.
         */
        const std::string pkey_file_password;
    };


    // --------------------------------------------------------------

    namespace protocol {
        constexpr const char* HTTP_11                 = "HTTP/1.1";
    }


    namespace method {
        constexpr const char* GET                     = "GET";
        constexpr const char* POST                    = "POST";
        constexpr const char* PUT                     = "PUT";
        constexpr const char* DELETE                  = "DELETE";
        constexpr const char* HEAD                    = "HEAD";
    }


    namespace status_code {
        constexpr status_code_t OK                    = 200;
        constexpr status_code_t Created               = 201;
        constexpr status_code_t Accepted              = 202;

        constexpr status_code_t Moved_Permanently     = 301;
        constexpr status_code_t Found                 = 302;

        constexpr status_code_t Bad_Request           = 400;
        constexpr status_code_t Unauthorized          = 401;
        constexpr status_code_t Forbidden             = 403;
        constexpr status_code_t Not_Found             = 404;
        constexpr status_code_t Method_Not_Allowed    = 405;
        constexpr status_code_t Payload_Too_Large     = 413;
        constexpr status_code_t URI_Too_Long          = 414;
        constexpr status_code_t Too_Many_Requests     = 429;

        constexpr status_code_t Internal_Server_Error = 500;
        constexpr status_code_t Not_Implemented       = 501;
        constexpr status_code_t Service_Unavailable   = 503;
    }


    namespace reason_phrase {
        constexpr const char* OK                      = "OK";
        constexpr const char* Created                 = "Created";
        constexpr const char* Accepted                = "Accepted";

        constexpr const char* Moved_Permanently       = "Moved Permanently";
        constexpr const char* Found                   = "Found";

        constexpr const char* Bad_Request             = "Bad Request";
        constexpr const char* Unauthorized            = "Unauthorized";
        constexpr const char* Forbidden               = "Forbidden";
        constexpr const char* Not_Found               = "Not Found";
        constexpr const char* Method_Not_Allowed      = "Method Not Allowed";
        constexpr const char* Payload_Too_Large       = "Payload Too Large";
        constexpr const char* URI_Too_Long            = "URI Too Long";
        constexpr const char* Too_Many_Requests       = "Too Many Requests";

        constexpr const char* Internal_Server_Error   = "Internal Server Error";
        constexpr const char* Not_Implemented         = "Not Implemented";
        constexpr const char* Service_Unavailable     = "Service Unavailable";
    }


    namespace header {
        constexpr const char* Connection              = "Connection";
        constexpr const char* Content_Type            = "Content-Type";
        constexpr const char* Content_Length          = "Content-Length";
    }


    namespace connection {
        constexpr const char* close                   = "close";
    }


    namespace content_type {
        constexpr const char* text                    = "text/plain; charset=utf-8";
        constexpr const char* html                    = "text/html; charset=utf-8";
        constexpr const char* css                     = "text/css; charset=utf-8";
        constexpr const char* javascript              = "text/javascript; charset=utf-8";
        constexpr const char* xml                     = "text/xml; charset=utf-8";

        constexpr const char* json                    = "application/json";

        constexpr const char* png                     = "image/png";
        constexpr const char* jpeg                    = "image/jpeg";
        constexpr const char* gif                     = "image/gif";
        constexpr const char* bmp                     = "image/bmp";
        constexpr const char* svg                     = "image/svg+xml";
    }


    // --------------------------------------------------------------

    /**
     * @brief               Base http endpoint.
     * @details             This class supports the most common functionality - reads requests and dispatches them for REST- or file-processing.
     *                      This class must be subclassed to implement the processing of requests.
     * @tparam ServerSocket Server socket.
     * @tparam ClientSocket Client/connection socket.
     * @tparam LogPtr       Pointer type to `log_ostream`.
     */
    template <typename ServerSocket, typename ClientSocket, typename LogPtr>
    class endpoint
        : protected diag::diag_ready<const char*, LogPtr>  {

        using diag_base = diag::diag_ready<const char*, LogPtr>;

    public:
        /**
         * @brief        Constructor.
         * @param config `endpoint_config` instance
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        endpoint(endpoint_config&& config, const LogPtr& log);

        /**
         * @brief Move Constructor.
         */
        endpoint(endpoint<ServerSocket, ClientSocket, LogPtr>&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        endpoint(const endpoint<ServerSocket, ClientSocket, LogPtr>& other) = delete;

    protected:
        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param config `endpoint_config` instance
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        endpoint(const char* origin, endpoint_config&& config, const LogPtr& log);

    public:
        /**
         * @brief  Starts the endpoint on a separate thread.
         * @return `std::future<void>` that will get set after a `POST /shutdown` is received from a client.
         */
        std::future<void> start_async();

        /**
         * @brief   Starts the endpoint on the current thread.
         * @details This thread will block until a `POST /shutdown` is received from a client.
         */
        void start();

    protected:
        /**
         * @brief Creates and returns an instance of ServerSocket.
         */
        virtual ServerSocket create_server_socket() = 0;

        /**
         * @brief         Processes a GET request for a static file.
         * @param http    A reference to `http::server`.
         * @param request A reference to `http::request`.
         */
        virtual void process_file_request(server<LogPtr>& http, const request& request);

        /**
         * @brief         Processes a REST request.
         * @param http    A reference to `http::server`.
         * @param request A reference to `http::request`.
         */
        virtual void process_rest_request(server<LogPtr>& http, const request& request);

        /**
         * @brief         Checks if the resource is a static file.
         * @param request A reference to `http::request`.
         * @return        Whether the resource is a static file.
         */
        virtual bool is_file_request(const request& request);

        /**
         * @brief               Sends a response with the given content.
         * @param http          A reference to `http::server`.
         * @param status_code   Http status code.
         * @param reason_phrase Http status reason phrase.
         * @param content_type  Http Content-Type response header value.
         * @param body          Http response body.
         * @param tag           Logging tag provided by the caller, so that this response could be tracked as part of the call flow.
         */
        virtual void send_simple_response(server<LogPtr>& http, status_code_t status_code, const char* reason_phrase, const char* content_type, const char* body, diag::tag_t tag);

        /**
         * @brief      Determines the http response Content-Type based on the file extension.
         * @param path Path (virtual or local) with an extension.
         * @return     The value for the Content-Type response header.
         */
        virtual const char* get_content_type_from_path(const char* path);

    protected:
        /**
         * @brief            Processes (any kind of) a request.
         * @details          This is the top-level method that reads the http request line, and calls either `process_file_request()` or `process_rest_request()`.
         * @param connection Connection/client socket to read the request from and to send the response to.
         */
        void process_request(ClientSocket&& connection);

        /**
         * @brief Sets the "shutdown requested" flag.
         */
        void set_shutdown_requested();

        /**
         * @brief Returns the state of the "shutdown requested" flag.
         */
        bool is_shutdown_requested() const;

        /**
         * @brief Makes a physical path under `root_dir` from the virtual path of the request.
         */
        std::string make_root_dir_path(const request& request) const;

    private:
        /**
         * @brief Thread function for the 'start' thread.
         */
        static void start_thread_func(endpoint<ServerSocket, ClientSocket, LogPtr>* this_ptr);

        /**
         * @brief Thread function for the 'process_request' thread.
         */
        static void process_request_thread_func(endpoint<ServerSocket, ClientSocket, LogPtr>* this_ptr, ClientSocket&& connection);

    protected:
        /**
         * @brief Returns the config settings passed in to the constructor.
         */
        const endpoint_config& config() const;

    private:
        /**
         * @brief The config settings passed in to the constructor.
         */
        endpoint_config _config;

        /**
         * @brief The `std::promise` that is returned by `start_async()`, which gets signaled when shutdown is requested.
         */
        std::promise<void> _promise;

        /**
         * @brief Number of requests currently in progress.
         */
        std::atomic_int32_t _requests_in_progress;

        /**
         * @brief Flag that gets set when `POST /shutdown` is received.
         */
        std::atomic_bool _is_shutdown_requested;
    };


    // --------------------------------------------------------------

} } }
