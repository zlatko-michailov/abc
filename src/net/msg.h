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

    inline streambuf_transport::streambuf_transport(const char* origin, std::streambuf* sb_in, std::streambuf* sb_out, diag::log_ostream* log)
        : transport()
        , daemon(copy(origin), log)
        , _sb_in(sb_in)
        , _sb_out(sb_out)
        , _log(log)
        , _strm_in(sb_in) {
    }


    inline streambuf_transport::streambuf_transport(std::streambuf* sb_in, std::streambuf* sb_out, diag::log_ostream* log)
        : streambuf_transport("abc::net::msg::streambuf_transport", sb_in, sb_out, log) {
    }


    inline void streambuf_transport::send_message(const json::value& message, const char* key) {
        constexpr const char* suborigin = "send_message()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::expect(suborigin, key == nullptr, __TAG__, "key == nullptr"); // Multiplexing is not supported for streambuf transport.

        json::writer writer(_sb_out, _log);
        writer.put_value(message);

        json::ostream ostream(_sb_out, _log);
        ostream.put_lf();
        ostream.flush();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void streambuf_transport::set_message_processor(message_processor* processor, const char* key) {
        constexpr const char* suborigin = "set_message_processor()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");
    
        diag_base::expect(suborigin, processor != nullptr, __TAG__, "processor != nullptr");
        diag_base::expect(suborigin, key == nullptr, __TAG__, "key == nullptr"); // Multiplexing is not supported for streambuf transport.

        _processor = processor;

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void streambuf_transport::on_idle() {
        constexpr const char* suborigin = "on_idle()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::expect(suborigin, _processor != nullptr, __TAG__, "_processor != nullptr");

        // Do not crash on bad input.
        try {
            std::string line;
            std::getline(_strm_in, line);

            // Parse the line as a JSON value.
            std::stringbuf sb_line(line, std::ios::in);
            json::reader json_reader(&sb_line, _log);
            json::value message = json_reader.get_value();

            // Process the message.
            _processor->process_message(message);
        }
        catch (const diag::input_error& ex) {
            diag_base::put_any(suborigin, diag::severity::important, __TAG__, "Input error: %s", ex.what());
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    // --------------------------------------------------------------


    inline console_transport::console_transport(diag::log_ostream* log)
        : streambuf_transport("abc::net::msg::console_transport", std::cin.rdbuf(), std::cout.rdbuf(), log) {
    }


    // --------------------------------------------------------------


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
