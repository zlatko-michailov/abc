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
    test_processor processor(context);

    std::stringbuf sb_in(
        "{ \"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [21, 12], \"id\": 1 }\n"
        "[ { \"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [21, 12], \"id\": 1 }, { \"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [45, 34], \"id\": 2 } ]\n"
        "{ \"jsonrpc\": \"2.0\", \"method\": \"notify\", \"params\": [21, 12] }\n"
        "{ \"jsonrpc\": \"2.0\", \"result\": 9, \"id\": 1 }\n"
        "{ \"jsonrpc\": \"2.0\", \"error\": {\"code\": -32001, \"message\": \"Method not found\"}, \"id\": 2 }\n"
        "[ { \"jsonrpc\": \"2.0\", \"result\": 9, \"id\": 1 }, { \"jsonrpc\": \"2.0\", \"error\": {\"code\": -32001, \"message\": \"Method not found\"}, \"id\": 2 } ]\n"
        "{ \"jsonrpc\": \"2.0\", \"method\": \"stop\" }\n",
        std::ios::in
    );

    std::stringbuf dummy_sb_out(std::ios::out);

    abc::net::msg::streambuf_transport transport(&sb_in, &dummy_sb_out, context.log());
    transport.set_message_processor(&processor);

    processor.set_transport_daemon(&transport);

    std::future<void> future = transport.start_async();
    future.wait();

    context.are_equal(processor.passed(), true, __TAG__, "%d");
    context.are_equal(processor.message_count(), (std::size_t)7, __TAG__, "%zu");

    return processor.passed();
}

