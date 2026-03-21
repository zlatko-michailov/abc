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

#include "../diag/diag_ready.h"
#include "json.h"
#include "http.h"
#include "daemon.h"
#include "endpoint.h"
#include "i/msg.i.h"


namespace abc { namespace net { namespace msg {

    inline streambuf_otransport::streambuf_otransport(const char* origin, std::streambuf* sb, diag::log_ostream* log)
        : diag_base(copy(origin), log)
        , _sb(sb)
        , _log(log) {
    }


    inline streambuf_otransport::streambuf_otransport(std::streambuf* sb, diag::log_ostream* log)
        : streambuf_otransport("abc::net::msg::streambuf_otransport", sb, log) {
    }


    inline void streambuf_otransport::send_message(const json::value& message) {
        constexpr const char* suborigin = "send_message()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        json::writer writer(_sb, _log);
        writer.put_value(message);

        json::ostream ostream(_sb, _log);
        ostream.put_lf();
        ostream.flush();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    inline streambuf_itransport::streambuf_itransport(const char* origin, std::streambuf* sb, std::streambuf* sb_out, diag::log_ostream* log)
        : daemon(copy(origin), log)
        , _sb(sb)
        , _log(log)
        , _otransport(sb_out, log) {
    }


    inline streambuf_itransport::streambuf_itransport(std::streambuf* sb, std::streambuf* sb_out, diag::log_ostream* log)
        : streambuf_itransport("abc::net::msg::streambuf_itransport", sb, sb_out, log) {
    }


    inline void streambuf_itransport::set_processor(processor* processor, const char* key) {
        constexpr const char* suborigin = "set_processor()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");
    
        diag_base::expect(suborigin, processor != nullptr, __TAG__, "processor != nullptr");
        diag_base::expect(suborigin, key == nullptr, __TAG__, "key == nullptr"); // Multiplexing is not supported for streambuf itransport.

        _processor = processor;

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void streambuf_itransport::on_idle() {
        constexpr const char* suborigin = "on_idle()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::expect(suborigin, _processor != nullptr, __TAG__, "_processor != nullptr");

        // Do not crash on bad input.
        try {
            std::istream strm(_sb);
            std::string line;
            std::getline(strm, line);

            // Parse the line as a JSON value.
            std::stringbuf sb_line(line, std::ios::in);
            json::reader json_reader(&sb_line, _log);
            json::value message = json_reader.get_value();

            // Process the message.
            _processor->process_message(message, &_otransport);
        }
        catch (const diag::input_error& ex) {
            diag_base::put_any(suborigin, diag::severity::important, __TAG__, "Input error: %s", ex.what());
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    inline console_transport::console_transport(diag::log_ostream* log)
        : streambuf_otransport("abc::net::msg::console_otransport", std::cout.rdbuf(), log)
        , streambuf_itransport("abc::net::msg::console_itransport", std::cin.rdbuf(), std::cout.rdbuf(), log) {
    }


    // --------------------------------------------------------------


#if 0
    inline http_server_transport::http_server_transport(http::endpoint_config&& config, diag::log_ostream* log)
        : base("abc::net::msg::http_server_transport", std::move(config), log) {

        constexpr const char* suborigin = "http_server_transport()";
        base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::expect(suborigin, config.files_prefix.empty(), __TAG__, "config.files_prefix.empty()"); // File requests should be disabled.

        base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void http_server_transport::send_message(const json::value& message, const char* key) {
        constexpr const char* suborigin = "send_message()";
        base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        //// TODO: NOW

        base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void http_server_transport::set_message_processor(message_processor* processor, const char* key) {
        constexpr const char* suborigin = "set_message_processor()";
        base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::expect(suborigin, processor != nullptr, __TAG__, "processor != nullptr");
        base::expect(suborigin, key != nullptr, __TAG__, "key != nullptr"); // Multiplexing is required for the http transport.

        _processors[key] = processor;

        base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void http_server_transport::process_rest_request(http::server& http, const http::request& request) {
        constexpr const char* suborigin = "process_rest_request()";
        base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        // https://modelcontextprotocol.io/specification/2025-11-25/basic/transports#sending-messages-to-the-server

        // Accept header.
        http::headers::const_iterator accept_header = request.headers.find(http::header::Accept);
        base::require(suborigin, __TAG__, accept_header != request.headers.end(), http::status_code::Bad_Request, http::reason_phrase::Bad_Request, http::content_type::text, "Missing 'Accept' header.");

        bool acceptsJson = false;
        bool acceptsEventStream = false;
        std::stringstream acceptHeaderValueStream(accept_header->second);
        std::string acceptType;
        while (std::getline(acceptHeaderValueStream, acceptType, ',') && !acceptsJson && !acceptsEventStream) {
            if (ascii::are_equal_i(acceptType.c_str(), http::content_type::json)) {
                acceptsJson = true;
            }
            else if (ascii::are_equal_i(acceptType.c_str(), http::content_type::event_stream)) {
                acceptsEventStream = true;
            }
        }

        json::value message(nullptr);

        if (request.method == http::method::POST) {
            base::require(suborigin, __TAG__, acceptsJson, http::status_code::Bad_Request, http::reason_phrase::Bad_Request, http::content_type::text, "Must accept JSON.");
            base::require(suborigin, __TAG__, acceptsEventStream, http::status_code::Bad_Request, http::reason_phrase::Bad_Request, http::content_type::text, "Must accept event stream.");

            // Content-Type header is not required.

            // JSON-RPC body.
            json::reader json_reader(static_cast<http::request_reader&>(http).rdbuf(), base::log());
            message = json_reader.get_value();

            json::json_rpc_validator json_rpc_validator(base::log());
            bool isJsonRpc = json_rpc_validator.is_simple_request(message) || json_rpc_validator.is_simple_notification(message) || json_rpc_validator.is_simple_response(message);
            base::require(suborigin, __TAG__, isJsonRpc, http::status_code::Bad_Request, http::reason_phrase::Bad_Request, http::content_type::text, "Must be a valid JSON-RPC message.");
        }
        else if (request.method == http::method::GET) {
            base::require(suborigin, __TAG__, acceptsEventStream, http::status_code::Bad_Request, http::reason_phrase::Bad_Request, http::content_type::text, "Must accept event stream.");
        }
        else {
            base::require(suborigin, __TAG__, false, http::status_code::Bad_Request, http::reason_phrase::Bad_Request, http::content_type::text, "The method must be POST or GET.");
        }

        // Get the processor for the requested path.
        std::map<std::string, message_processor*>::iterator processor = _processors.find(request.resource.path);
        base::require(suborigin, __TAG__, processor != _processors.end(), http::status_code::Bad_Request, http::reason_phrase::Bad_Request, http::content_type::text, "There is no processor for the requested path.");

        // Process the message.
        processor->second->process_message(message);

        base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }
#endif

#if 0
    /**
     * @brief http client transport.
     */
    class http_client_transport
        : public transport
        , protected diag::diag_ready<const char*>  {

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
#endif


    // --------------------------------------------------------------

} } }
