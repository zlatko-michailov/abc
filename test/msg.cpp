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


#include <sstream>

#include "inc/msg.h"


bool test_streambuf_transport(test_context& context) {
    bool passed = true;

    // Since we don't have a blocking memory streambuf, we cannot easily connect a sender and a receiver that work simultaneously.
    // Instead, we connect the sender to a medium streambuf, and then connect the receiver to the same medium streambuf.

    std::stringbuf sender_sb_in(
        "{ \"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [21, 12], \"id\": 1 }\n"
        "[ { \"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [21, 12], \"id\": 1 }, { \"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [45, 34], \"id\": 2 } ]\n"
        "{ \"jsonrpc\": \"2.0\", \"method\": \"notify\", \"params\": [21, 12] }\n"
        "{ \"jsonrpc\": \"2.0\", \"result\": 9, \"id\": 1 }\n"
        "{ \"jsonrpc\": \"2.0\", \"error\": {\"code\": -32001, \"message\": \"Method not found\"}, \"id\": 2 }\n"
        "[ { \"jsonrpc\": \"2.0\", \"result\": 9, \"id\": 1 }, { \"jsonrpc\": \"2.0\", \"error\": {\"code\": -32001, \"message\": \"Method not found\"}, \"id\": 2 } ]\n"
        "{ \"jsonrpc\": \"2.0\", \"method\": \"stop\" }\n"
    );
    std::stringbuf medium_sb;
    std::stringbuf receiver_sb_out;

    // Sender - set up.
    abc::net::msg::streambuf_transport sender_transport(&sender_sb_in, &medium_sb, context.log());
    test_processor sender_processor(context);
    sender_transport.set_message_processor(&sender_processor);
    sender_processor.set_transport_daemon(&sender_transport);

    // Sender - execute.
    std::future<void> sender_future = sender_transport.start_async();
    sender_future.wait();

    // Receiver - ser up.
    abc::net::msg::streambuf_transport receiver_transport(&medium_sb, &receiver_sb_out, context.log());
    test_processor receiver_processor(context);
    receiver_transport.set_message_processor(&receiver_processor);
    receiver_processor.set_transport_daemon(&receiver_transport);

    // Receiver - execute.
    std::future<void> receiver_future = receiver_transport.start_async();
    receiver_future.wait();

    passed = context.are_equal(sender_processor.passed(), true, __TAG__, "%d") && passed;
    passed = context.are_equal(sender_processor.message_count(), (std::size_t)7, __TAG__, "%zu") && passed;

    passed = context.are_equal(receiver_processor.passed(), true, __TAG__, "%d") && passed;
    passed = context.are_equal(receiver_processor.message_count(), (std::size_t)7, __TAG__, "%zu") && passed;

    passed = sender_processor.passed() && passed;
    passed = receiver_processor.passed() && passed;

    return passed;
}

