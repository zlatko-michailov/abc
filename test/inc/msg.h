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

#include "../../src/root/size.h"
#include "../../src/net/msg.h"

#include "test.h"


class test_processor
    : public abc::net::msg::processor {

public:
    test_processor(test_context& context)
        : _daemon(nullptr)
        , _validator(context.log())
        , _passed(true)
        , _message_count(0) {
    }

public:
    virtual void process_message(const abc::net::json::value& message, abc::net::msg::otransport* otransport) override {
        // Count the messages so we can verify intput.
        _message_count++;
        
        // Make sure all exchanged messages are valid.
        _passed = ( _validator.is_simple_request(message)
                || _validator.is_simple_notification(message)
                || _validator.is_simple_response(message)
                || _validator.is_batch_request(message)
                || _validator.is_batch_response(message)
                || _validator.is_error_response(message)
            )
            && _passed;

        // Forward the same message.
        otransport->send_message(message);

        // Stop the daemon if the message is a "stop" request.
        if (message.type() == abc::net::json::value_type::object) {
            abc::net::json::literal::object obj = message.object();
            abc::net::json::literal::object::const_iterator method_itr = obj.find("method");
            if (method_itr != obj.end()) {
                abc::net::json::literal::string name = method_itr->second.string();
                if (name == "stop") {
                    _daemon->request_stop();
                }
            }
        }
    }

public:
    void set_daemon_to_stop(abc::net::daemon* daemon) noexcept {
        _daemon = daemon;
    }

    bool passed() const noexcept {
        return _passed;
    }

    std::size_t message_count() const noexcept {
        return _message_count;
    }

private:
    abc::net::daemon* _daemon;
    abc::net::json::json_rpc_validator _validator;
    bool _passed;
    std::size_t _message_count;
};


bool test_streambuf_transport(test_context& context);
