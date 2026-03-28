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
#include <thread>

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


    inline http_server_response_otransport::http_server_response_otransport(std::streambuf* sb, diag::log_ostream* log)
        : diag_base("abc::net::msg::http_server_response_otransport", log)
        , _sb(sb)
        , _log(log) {
    }


    inline void http_server_response_otransport::send_message(const json::value& message) {
        constexpr const char* suborigin = "send_message()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        std::stringstream body;
        json::writer json_writer(body.rdbuf(), _log);
        json_writer.put_value(message);

        std::string content_length = std::to_string(body.str().size());

        http::response response;
        response.protocol = http::protocol::HTTP_11;
        response.status_code = http::status_code::OK;
        response.reason_phrase = http::reason_phrase::OK;
        response.headers = {
            { http::header::Content_Type,   http::content_type::json },
            { http::header::Content_Length, std::move(content_length) },
        }; 

        http::response_writer response_writer(_sb, _log);
        response_writer.put_response(response);
        response_writer.put_body(body.str().c_str());

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    inline http_server_event_otransport::http_server_event_otransport(std::streambuf* sb, diag::log_ostream* log)
        : diag_base("abc::net::msg::http_server_event_otransport", log)
        , _sb(sb)
        , _log(log) {
    }


    inline void http_server_event_otransport::send_message(const json::value& message) {
        constexpr const char* suborigin = "send_message()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        // The event stream must be already open by the `http_server_itransport`.
        // The processor may call this method multiple times for the same request.

        std::vector<http::event_message> event_messages;

        // If the message is a string, it is assumed to be a comment.
        if (message.type() == json::value_type::string) {
            event_messages.emplace_back(http::comment_event_message(message.string().c_str()));
        }

        // If the message is an object, each property name is the event type, and the property value, which should be a string, is the event value.
        else if (message.type() == json::value_type::object) {
            for (const json::literal::object::value_type& property : message.object()) {
                const std::string& event_type = property.first;
                const json::value& event_value = property.second;

                diag_base::expect(suborigin, event_value.type() == json::value_type::string, __TAG__, "event_value.type() == json::value_type::string");

                event_messages.emplace_back(http::event_message(event_type.c_str(), event_value.string().c_str()));
            }
        }

        // If the message is an array, each item should also be an array, with two items of type string - the event type and the event value.
        else if (message.type() == json::value_type::array) {
            for (const json::value& item : message.array()) {
                diag_base::expect(suborigin, item.type() == json::value_type::array, __TAG__, "item.type() == json::value_type::array");
                diag_base::expect(suborigin, item.array().size() == 2, __TAG__, "item.array().size() == 2");
                diag_base::expect(suborigin, item.array()[0].type() == json::value_type::string, __TAG__, "item.array()[0].type() == json::value_type::string");
                diag_base::expect(suborigin, item.array()[1].type() == json::value_type::string, __TAG__, "item.array()[1].type() == json::value_type::string");

                const std::string& event_type = item.array()[0].string();
                const std::string& event_value = item.array()[1].string();

                event_messages.emplace_back(http::event_message(event_type.c_str(), event_value.c_str()));
            }
        }

        // No other message types are supported.
        else {
            diag_base::expect(suborigin, false, __TAG__, "Unsupported message type.");
        }

        // Construct the event.
        abc::net::http::event event(std::move(event_messages));

        // Write the event to the stream.
        http::response_writer response_writer(_sb, _log);
        response_writer.put_event(event);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    inline http_server_itransport::http_server_itransport(http::endpoint_config&& config, diag::log_ostream* log)
        : base("abc::net::msg::http_server_itransport", std::move(config), log) {

        constexpr const char* suborigin = "http_server_itransport()";
        base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::expect(suborigin, config.files_prefix.empty(), __TAG__, "config.files_prefix.empty()"); // File requests should be disabled.

        base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void http_server_itransport::set_processor(processor* processor, const char* key) {
        constexpr const char* suborigin = "set_processor()";
        base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::expect(suborigin, processor != nullptr, __TAG__, "processor != nullptr");
        base::expect(suborigin, key != nullptr, __TAG__, "key != nullptr"); // Multiplexing is required for the http transport.

        {
            std::lock_guard<std::mutex> lock(_processor_contexts_mutex);

            std::string key_str(key);
            http_server_processor_map::iterator processor_context_itr = _processor_contexts.find(key_str);
            if (processor_context_itr == _processor_contexts.end()) {
                processor_context_itr = _processor_contexts.emplace(key_str, http_server_processor_context()).first;
            }

            processor_context_itr->second.processor = processor;
        }

        base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void http_server_itransport::process_rest_request(http::server& http, const http::request& request) {
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

        // Get the processor context for the requested path.
        http_server_processor_map::iterator processor_context_itr;
        {
            std::lock_guard<std::mutex> lock(_processor_contexts_mutex);

            processor_context_itr = _processor_contexts.find(request.resource.path);
        }
        base::require(suborigin, __TAG__, processor_context_itr != _processor_contexts.end(), http::status_code::Bad_Request, http::reason_phrase::Bad_Request, http::content_type::text, "There is no processor for the requested path.");

        // POST JSON-RPC request => JSON-RPC response.
        if (request.method == http::method::POST) {
            base::require(suborigin, __TAG__, acceptsJson, http::status_code::Bad_Request, http::reason_phrase::Bad_Request, http::content_type::text, "Must accept JSON.");
            base::require(suborigin, __TAG__, acceptsEventStream, http::status_code::Bad_Request, http::reason_phrase::Bad_Request, http::content_type::text, "Must accept event stream.");

            // Content-Type header is not required.

            // JSON-RPC body.
            json::reader json_reader(static_cast<http::request_reader&>(http).rdbuf(), base::log());
            json::value message = json_reader.get_value();

            json::json_rpc_validator json_rpc_validator(base::log());
            bool isJsonRpc = json_rpc_validator.is_simple_request(message) || json_rpc_validator.is_simple_notification(message) || json_rpc_validator.is_simple_response(message)
                        || json_rpc_validator.is_batch_request(message) || json_rpc_validator.is_batch_response(message);
            base::require(suborigin, __TAG__, isJsonRpc, http::status_code::Bad_Request, http::reason_phrase::Bad_Request, http::content_type::text, "Must be a valid JSON-RPC message.");

            // Prepare the processor context.
            processor_context_itr->second.request_type = http_server_response_type::post_json_rpc;
            processor_context_itr->second.json_rpc_request_id = nullptr;
            if (message.type() == json::value_type::object) {
                json::literal::object::const_iterator id_itr = message.object().find("id");
                if (id_itr != message.object().end()) {
                    processor_context_itr->second.json_rpc_request_id = id_itr->second;
                }
            }

            // Process the message.
            base::expect(suborigin, processor_context_itr->second.processor != nullptr, __TAG__, "processor_context_itr->second.processor != nullptr");
            http_server_response_otransport response_otransport(static_cast<http::response_writer&>(http).rdbuf(), base::log());
            processor_context_itr->second.processor->process_message(message, &response_otransport);
        }

        // GET request => SSE stream.
        else if (request.method == http::method::GET) {
            base::require(suborigin, __TAG__, acceptsEventStream, http::status_code::Bad_Request, http::reason_phrase::Bad_Request, http::content_type::text, "Must accept event stream.");

            // Prepare the processor context.
            processor_context_itr->second.request_type = http_server_response_type::post_json_rpc;
            processor_context_itr->second.last_event_id.clear();
            http::headers::const_iterator last_event_id_header = request.headers.find(http::header::Last_Event_ID);
            if (last_event_id_header != request.headers.end()) {
                processor_context_itr->second.last_event_id = last_event_id_header->second;
            }

            // Send an HTTP response on the current thread.
            abc::net::http::response response;
            response.protocol = abc::net::http::protocol::HTTP_11;
            response.status_code = abc::net::http::status_code::OK;
            response.reason_phrase = abc::net::http::reason_phrase::OK;
            response.headers = abc::net::http::headers {
                { abc::net::http::header::Content_Type,  abc::net::http::content_type::event_stream },
                { abc::net::http::header::Cache_Control, abc::net::http::cache_control::no_cache },
                { abc::net::http::header::Connection,    abc::net::http::connection::keep_alive },
            };

            http.put_response(response);

            // Start a new thread for the event stream.
            std::thread(send_event_stream_thread_func, this, static_cast<http::response_writer&>(http).rdbuf(), &processor_context_itr->second).detach();
        }

        // Bad Request.
        else {
            base::require(suborigin, __TAG__, false, http::status_code::Bad_Request, http::reason_phrase::Bad_Request, http::content_type::text, "The method must be POST or GET.");
        }

        base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void http_server_itransport::send_event_stream_thread_func(http_server_itransport* this_ptr, std::streambuf* sb, http_server_processor_context* processor_context) {
        this_ptr->send_event_stream(sb, processor_context);
    }


    inline void http_server_itransport::send_event_stream(std::streambuf* sb, http_server_processor_context* processor_context) {
        constexpr const char* suborigin = "send_event_stream()";
        base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        base::expect(suborigin, sb != nullptr, __TAG__, "sb != nullptr");
        base::expect(suborigin, processor_context != nullptr, __TAG__, "processor_context != nullptr");
        base::expect(suborigin, processor_context->processor != nullptr, __TAG__, "processor_context->processor != nullptr");

        base::increment_requests_in_progress();

        try {
            http_server_event_otransport event_otransport(sb, base::log());

            processor_context->processor->process_message(json::value(), &event_otransport);
        }
        catch (std::exception& ex) {
            base::put_any(suborigin, diag::severity::optional, __TAG__, "Processor threw an exception - '%s'.", ex.what());
        }

        base::decrement_requests_in_progress();

        base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


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
