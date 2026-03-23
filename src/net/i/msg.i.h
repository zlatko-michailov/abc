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
#include <map>

#include "../../diag/i/diag_ready.i.h"
#include "json.i.h"
#include "http.i.h"
#include "daemon.i.h"
#include "endpoint.i.h"


namespace abc { namespace net { namespace msg {

    /**
     * @brief Abstract outbound transport.
     */
    class otransport {
    public:
        /**
         * @brief         Sends a message.
         * @param message Message to send.
         */
        virtual void send_message(const json::value& message) = 0;
    };


    // --------------------------------------------------------------


    /**
     * @brief Abstract message processor.
     */
    class processor {
    public:
        /**
         * @brief           Processes a message.
         * @param message   Message to process.
         * @param transport Outbound transport for an eventual result.
         */
        virtual void process_message(const json::value& message, otransport* transport) = 0;
    };


    // --------------------------------------------------------------


    /**
     * @brief Abstract inbound transport.
     */
    class itransport {
    public:
        /**
         * @brief           Sets the message processor to process incoming messages.
         * @param processor Message processor.
         * @param key       Optional key to identify the connection if this inbound transport supports multiplexing.
         */
        virtual void set_processor(processor* processor, const char* key = nullptr) = 0;
    };


    // --------------------------------------------------------------


    /**
     * @brief Outbound transport over streambuf.
     */
    class streambuf_otransport
        : public otransport
        , public diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief     Constructor.
         * @param sb  Output stream buffer.
         * @param log `diag::log_ostream` pointer. May be `nullptr`.
         */
        streambuf_otransport(std::streambuf* sb, diag::log_ostream* log = nullptr);

    protected:
        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param sb     Output stream buffer.
         * @param log    `diag::log_ostream` pointer. May be `nullptr`.
         */
        streambuf_otransport(const char* origin, std::streambuf* sb, diag::log_ostream* log = nullptr);

    // `otransport` overrides.
    public:
        /**
         * @brief         Sends a message.
         * @param message Message to send.
         */
        virtual void send_message(const json::value& message) override;

    protected:
        /**
         * @brief Output stream buffer.
         */
        std::streambuf* _sb;

        /**
         * @brief `diag::log_ostream` pointer passed in to the constructor. May be `nullptr`.
         */
        diag::log_ostream* _log;
    };


    // --------------------------------------------------------------


    /**
     * @brief Inbound transport over streambuf.
     */
    class streambuf_itransport
        : public itransport
        , public daemon {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief        Constructor.
         * @param sb     Input stream buffer.
         * @param sb_out Output stream buffer for message responses.
         * @param log    `diag::log_ostream` pointer. May be `nullptr`.
         */
        streambuf_itransport(std::streambuf* sb, std::streambuf* sb_out, diag::log_ostream* log = nullptr);

    protected:
        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param sb     Input stream buffer.
         * @param sb_out Output stream buffer for message responses.
         * @param log    `diag::log_ostream` pointer. May be `nullptr`.
         */
        streambuf_itransport(const char* origin, std::streambuf* sb, std::streambuf* sb_out, diag::log_ostream* log = nullptr);

    // `itransport` overrides.
    public:
        /**
         * @brief           Sets the message processor to process incoming messages.
         * @param processor Message processor.
         * @param key       Optional key to identify the connection if the transport supports multiplexing.
         *                  If provided, must be `nullptr`. Multiplexing is not supported for streambuf transport.
         */ 
        virtual void set_processor(processor* processor, const char* key = nullptr) override;

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
        std::streambuf* _sb;

        /**
         * @brief `diag::log_ostream` pointer passed in to the constructor. May be `nullptr`.
         */
        diag::log_ostream* _log;

        /**
         * @brief Message processor.
         */
        processor* _processor = nullptr;

        /**
         * @brief `streambuf_otransport` to be used for message responses.
         */
        streambuf_otransport _otransport;
    };


    // --------------------------------------------------------------


    /**
     * @brief Transport over the console - `std::cin` and `std::cout`.
     */
    class console_transport
        : public streambuf_otransport
        , public streambuf_itransport {

    public:
        /**
         * @brief     Constructor.
         * @param log `diag::log_ostream` pointer. May be `nullptr`.
         */
        console_transport(diag::log_ostream* log = nullptr);
    };


    // --------------------------------------------------------------


    class http_server_itransport;


    /**
     * @brief   Outbound transport over HTTP as response.
     * @details This class is not be instantiated directly.
     */
    class http_server_response_otransport
        : public otransport
        , public diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

        friend class http_server_itransport;

    private:
        /**
         * @brief     Constructor.
         * @param sb  Output stream buffer.
         * @param log `diag::log_ostream` pointer. May be `nullptr`.
         */
        http_server_response_otransport(std::streambuf* sb, diag::log_ostream* log = nullptr);

    // `otransport` overrides.
    public:
        /**
         * @brief         Sends a message.
         * @param message Message to send.
         */
        virtual void send_message(const json::value& message) override;

    protected:
        /**
         * @brief Output stream buffer.
         */
        std::streambuf* _sb;

        /**
         * @brief `diag::log_ostream` pointer passed in to the constructor. May be `nullptr`.
         */
        diag::log_ostream* _log;
    };


    // --------------------------------------------------------------


    class http_server_itransport;


    /**
     * @brief   Outbound transport over HTTP as Server-Sent Event (SSE).
     * @details This class is not be instantiated directly.
     */
    class http_server_event_otransport
        : public otransport
        , public diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

        friend class http_server_itransport;

    private:
        /**
         * @brief     Constructor.
         * @param sb  Output stream buffer.
         * @param log `diag::log_ostream` pointer. May be `nullptr`.
         */
        http_server_event_otransport(std::streambuf* sb, diag::log_ostream* log = nullptr);

    // `otransport` overrides.
    public:
        /**
         * @brief         Sends a message.
         * @param message Message to send.
         */
        virtual void send_message(const json::value& message) override;

    protected:
        /**
         * @brief Output stream buffer.
         */
        std::streambuf* _sb;

        /**
         * @brief `diag::log_ostream` pointer passed in to the constructor. May be `nullptr`.
         */
        diag::log_ostream* _log;
    };


    // --------------------------------------------------------------


    /**
     * @brief   Inbound transport over an HTTP endpoint.
     * @details This class multiplexes message processors by REST path.
     */
    class http_server_itransport
        : public itransport
        , public http::endpoint {

        using base = http::endpoint;

    public:
        /**
         * @brief        Constructor.
         * @param config `endpoint_config` instance.
         * @param log    `diag::log_ostream` pointer. May be `nullptr`.
         */
        http_server_itransport(http::endpoint_config&& config, diag::log_ostream* log = nullptr);

    // `itransport` overrides.
    public:
        /**
         * @brief           Sets the message processor to process incoming messages.
         * @param processor Message processor.
         * @param key       Optional key to identify the connection.
         *                  The HTTP endpoint multiplexes by REST path.
         *                  Must be the REST path for the given processor.
         */
        virtual void set_processor(processor* processor, const char* key = nullptr) override;

    // `endpoint` overrides.
    protected:
        /**
         * @brief         Processes a REST request.
         * @param http    A reference to `http::server`.
         * @param request A reference to `http::request`.
         */
        virtual void process_rest_request(http::server& http, const http::request& request) override;

    private:
        /**
         * @brief Map of REST path to message processor, for multiplexing.
         */
        //// TODO: Define all the metadata that should accompany a processor.
        std::map<std::string, processor*> _processors;
    };


    // --------------------------------------------------------------


    /**
     * @brief Outbound and inbound transport over an HTTP client.
     */
    class http_client_transport
        : public otransport
        , public itransport
        , public daemon {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief     Constructor.
         * @param url URL of the HTTP endpoint.
         * @param log `diag::log_ostream` pointer. May be `nullptr`.
         */
        http_client_transport(const char* url, diag::log_ostream* log = nullptr);

    // `otransport` overrides.
    public:
        /**
         * @brief         Sends a message.
         * @param message Message to send.
         */
        virtual void send_message(const json::value& message) override;

    // `itransport` overrides.
    public:
        /**
         * @brief           Sets the message processor to process incoming messages.
         * @param processor Message processor.
         * @param key       Optional key to identify the connection.
         *                  If provided, must be `nullptr`. Multiplexing is not supported for http client transport.
         */
        virtual void set_processor(processor* processor, const char* key = nullptr) override;

    // `daemon` overrides.
    public:
    };


    // --------------------------------------------------------------

} } }
