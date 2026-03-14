/*
MIT License

Copyright (c) 2018-2026 Zlatko Michailov 

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

#include <future>
#include <atomic>
#include <string>
#include <streambuf>

#include "../../diag/i/diag_ready.i.h"
#include "json.i.h"
#include "http.i.h"
#include "daemon.i.h"
#include "endpoint.i.h"


namespace abc { namespace net { namespace msg {

    /**
     * @brief Abstract message processor.
     */
    class message_processor {
    public:
        /**
         * @brief         Processes a message.
         * @param message Message to process.
         */
        virtual void process_message(const json::value& message) = 0;
    };


    // --------------------------------------------------------------


    /**
     * @brief Abstract transport.
     */
    class transport {
    public:
        /**
         * @brief         Sends a message.
         * @param message Message to send.
         */
        virtual void send_message(const json::value& message) = 0;

        /**
         * @brief           Sets the message processor to process incoming messages.
         * @param processor Message processor.
         * @param key       Optional key to identify the message processor for multiplexing.
         */
        virtual void set_message_processor(message_processor* processor, const char* key = nullptr) = 0;
    };


    // --------------------------------------------------------------


    /**
     * @brief Transport over streambuf's.
     */
    class streambuf_transport
        : public transport
        , public daemon {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief         Constructor.
         * @param sb_in   Input stream buffer.
         * @param sb_out  Output stream buffer.
         * @param log    `diag::log_ostream` pointer. May be `nullptr`.
         */
        streambuf_transport(std::streambuf* sb_in, std::streambuf* sb_out, diag::log_ostream* log = nullptr);

        /**
         * @brief Deleted.
         */
        streambuf_transport(streambuf_transport&& other) noexcept = delete;

        /**
         * @brief Deleted.
         */
        streambuf_transport(const streambuf_transport& other) = delete;

    protected:
        /**
         * @brief         Constructor.
         * @param origin  Origin.
         * @param sb_in   Input stream buffer.
         * @param sb_out  Output stream buffer.
         * @param log    `diag::log_ostream` pointer. May be `nullptr`.
         */
        streambuf_transport(const char* origin, std::streambuf* sb_in, std::streambuf* sb_out, diag::log_ostream* log = nullptr);

    // `transport` overrides.
    public:
        /**
         * @brief         Sends a message.
         * @param message Message to send.
         */
        virtual void send_message(const json::value& message) override;

        /**
         * @brief           Sets the message processor to process incoming messages.
         * @param processor Message processor.
         * @param key       Optional key to identify the message processor for multiplexing.
         *                  If provided, must be `nullptr`. Multiplexing is not supported for streambuf transport.
         */ 
        virtual void set_message_processor(message_processor* processor, const char* key = nullptr) override;

    // `daemon` overrides.
    protected:
        /**
         * @brief   Blocks the current thread until a whole line of input is read.
         * @details When a whole line is read, it is parsed as a JSON message and processed by the message processor.
         */
        virtual void on_idle() override;

    protected:
        /**
         * @brief Input stream buffer.
         */
        std::streambuf* _sb_in;

        /**
         * @brief Output stream buffer.
         */
        std::streambuf* _sb_out;

        /**
         * @brief `diag::log_ostream` pointer passed in to the constructor. May be `nullptr`.
         */
        diag::log_ostream* _log;

        /**
         * @brief Message processor.
         */
        message_processor* _processor = nullptr;

        /**
         * @brief Input stream around the passed input stream buffer.
         */
        std::istream _strm_in;

    };


    // --------------------------------------------------------------


    /**
     * @brief Transport over the console - `std::cin` and `std::cout`.
     */
    class console_transport
        : public streambuf_transport{

    public:
        /**
         * @brief         Constructor.
         * @param log    `diag::log_ostream` pointer. May be `nullptr`.
         */
        console_transport(diag::log_ostream* log = nullptr);
    };


    // --------------------------------------------------------------


    /**
     * @brief   Endpoint for http server transport.
     * @details This class overrides the necessary methods to provide transport-specific functionality.
     *          This class is not to be used directly.
     */
    class http_transport_endpoint
        : public http::endpoint {

        using base = http::endpoint;

    public:
        /**
         * @brief        Constructor.
         * @param config `endpoint_config` instance.
         * @param log    `diag::log_ostream` pointer. May be `nullptr`.
         */
        http_transport_endpoint(http::endpoint_config&& config, diag::log_ostream* log = nullptr);

    protected:
        /**
         * @brief Creates and returns an instance of ServerSocket.
         */
        virtual std::unique_ptr<tcp_server_socket> create_server_socket() override;

        /**
         * @brief         Processes a REST request.
         * @param http    A reference to `http::server`.
         * @param request A reference to `http::request`.
         */
        virtual void process_rest_request(http::server& http, const http::request& request) override;

    private:
        /**
         * @brief         Starts a server-sent event (SSE) stream.
         * @param http    A reference to `http::server`.
         * @param request A reference to `http::request`.
         */
        void process_event_stream_request(http::server& http, const http::request& request);

        /**
         * @brief         Returns the event stream ID from the request.
         * @param request A reference to `http::request`.
         */
        std::uint32_t get_event_stream_id(const http::request& request);
    };


    // --------------------------------------------------------------


    /**
     * @brief http server transport.
     */
    class http_server_transport
        : protected diag::diag_ready<const char*>
        , public transport {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief         Constructor.
         * @param config `endpoint_config` instance.
         * @param log    `diag::log_ostream` pointer. May be `nullptr`.
         */
        http_server_transport(http::endpoint_config&& config, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        http_server_transport(http_server_transport&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        http_server_transport(const http_server_transport& other) = delete;

    public:
        /**
         * @brief         Sends a message.
         * @param message Message to send.
         */
        virtual void send_message(const json::value& message) override;

        /**
         * @brief           Sets the message processor to process incoming messages.
         * @param processor Message processor.
         * @param key       Optional key to identify the message processor for multiplexing.
         *                  The http endpoint supports multiplexing by REST path.
         *                  If provided, must be the REST path for the given processor.
         */
        virtual void set_message_processor(message_processor* processor, const char* key = nullptr) override;
    };


    // --------------------------------------------------------------


    /**
     * @brief http client transport.
     */
    class http_client_transport
        : protected diag::diag_ready<const char*>
        , public transport {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief         Constructor.
         * @param config `endpoint_config` instance.
         * @param log    `diag::log_ostream` pointer. May be `nullptr`.
         */
        http_client_transport(http::endpoint_config&& config, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        http_client_transport(http_client_transport&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        http_client_transport(const http_client_transport& other) = delete;

    public:
        /**
         * @brief         Sends a message.
         * @param message Message to send.
         */
        virtual void send_message(const json::value& message) override;

        /**
         * @brief           Sets the message processor to process incoming messages.
         * @param processor Message processor.
         * @param key       Optional key to identify the message processor for multiplexing.
         *                  If provided, must be `nullptr`. Multiplexing is not supported for http client transport.
         */
        virtual void set_message_processor(message_processor* processor, const char* key = nullptr) override;
    };


    // --------------------------------------------------------------

} } }
