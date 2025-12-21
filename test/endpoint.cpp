/*
MIT License

Copyright (c) 2018-2025 Zlatko Michailov 

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

#include "../src/diag/tag.h"

#include "inc/stream.h"
#include "inc/endpoint.h"


bool test_endpoint_event_message(test_context& context, const abc::net::http::event_message& event_message, const char* expected, abc::diag::tag_t tag);
bool test_endpoint_event(test_context& context, const abc::net::http::event& event, const char* expected, abc::diag::tag_t tag);


bool test_endpoint_event_messages(test_context& context) {
    bool passed = true;

    {
        abc::net::http::comment_event_message event_message("This is a comment.");
        const char* expected = ": This is a comment.\n";

        passed = test_endpoint_event_message(context, event_message, expected, __TAG__) && passed;
    }

    {
        abc::net::http::type_event_message event_message("test");
        const char* expected = "event: test\n";

        passed = test_endpoint_event_message(context, event_message, expected, __TAG__) && passed;
    }

    {
        abc::net::http::data_event_message event_message("This is some data.");
        const char* expected = "data: This is some data.\n";

        passed = test_endpoint_event_message(context, event_message, expected, __TAG__) && passed;
    }

    {
        abc::net::http::id_event_message event_message("ABC123");
        const char* expected = "id: ABC123\n";

        passed = test_endpoint_event_message(context, event_message, expected, __TAG__) && passed;
    }

    {
        abc::net::http::retry_event_message event_message(456);
        const char* expected = "retry: 456\n";

        passed = test_endpoint_event_message(context, event_message, expected, __TAG__) && passed;
    }

    return passed;
}


bool test_endpoint_events_1(test_context& context) {
    bool passed = true;

    {
        abc::net::http::event event({
            abc::net::http::comment_event_message("This is a comment."),
        });
        const char* expected = ": This is a comment.\n\n";

        passed = test_endpoint_event(context, event, expected, __TAG__) && passed;
    }

    {
        abc::net::http::event event({
            abc::net::http::type_event_message("test"),
        });
        const char* expected = "event: test\n\n";

        passed = test_endpoint_event(context, event, expected, __TAG__) && passed;
    }

    {
        abc::net::http::event event({
            abc::net::http::data_event_message("This is some data."),
        });
        const char* expected = "data: This is some data.\n\n";

        passed = test_endpoint_event(context, event, expected, __TAG__) && passed;
    }

    {
        abc::net::http::event event({
            abc::net::http::id_event_message("ABC123"),
        });
        const char* expected = "id: ABC123\n\n";

        passed = test_endpoint_event(context, event, expected, __TAG__) && passed;
    }

    {
        abc::net::http::event event({
            abc::net::http::retry_event_message(456),
        });
        const char* expected = "retry: 456\n\n";

        passed = test_endpoint_event(context, event, expected, __TAG__) && passed;
    }

    return passed;
}


bool test_endpoint_events_N(test_context& context) {
    bool passed = true;

    {
        abc::net::http::event event({
            abc::net::http::comment_event_message("This is a comment."),
            abc::net::http::type_event_message("test"),
            abc::net::http::data_event_message("This is some data."),
            abc::net::http::id_event_message("ABC123"),
            abc::net::http::retry_event_message(456),
        });
        const char* expected =
            ": This is a comment.\n"
            "event: test\n"
            "data: This is some data.\n"
            "id: ABC123\n"
            "retry: 456\n"
            "\n";

        passed = test_endpoint_event(context, event, expected, __TAG__) && passed;
    }

    {
        abc::net::http::event event({
            abc::net::http::comment_event_message("Event 1:"),
            abc::net::http::type_event_message("test"),
            abc::net::http::id_event_message("1"),
            abc::net::http::data_event_message("Message 1.1"),
            abc::net::http::data_event_message("Message 1.2"),
            abc::net::http::data_event_message("Message 1.3"),
        });
        const char* expected =
            ": Event 1:\n"
            "event: test\n"
            "id: 1\n"
            "data: Message 1.1\n"
            "data: Message 1.2\n"
            "data: Message 1.3\n"
            "\n";

        passed = test_endpoint_event(context, event, expected, __TAG__) && passed;
    }

    return passed;
}

bool test_endpoint_server_events_1(test_context& context);
bool test_endpoint_server_events_N(test_context& context);


bool test_endpoint_event_message(test_context& context, const abc::net::http::event_message& event_message, const char* expected, abc::diag::tag_t tag) {
    bool passed = true;

    std::stringbuf sb(std::ios_base::out);
    event_message.send(&sb);

    passed = context.are_equal(sb.str().c_str(), expected, tag) && passed;

    return passed;
}


bool test_endpoint_event(test_context& context, const abc::net::http::event& event, const char* expected, abc::diag::tag_t tag) {
    bool passed = true;

    std::stringbuf sb(std::ios_base::out);
    event.send(&sb);

    passed = context.are_equal(sb.str().c_str(), expected, tag) && passed;

    return passed;
}


