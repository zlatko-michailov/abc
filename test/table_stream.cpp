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


#include <cstdio>

#include "inc/clock.h"
#include "inc/stream.h"
#include "inc/table_stream.h"


using thread_id_line_ostream = abc::line_ostream<17>;
using timestamp_line_ostream = abc::line_ostream<60>;


bool test_line_debug(test_context& context) {
    thread_id_line_ostream thread_id;
    thread_id.put_thread_id(std::this_thread::get_id());

    timestamp_line_ostream timestamp;
    timestamp.put_timestamp(abc::timestamp<test_clock>(), "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u");

    const char binary[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    const char expected_format[] =
        "\n"
        "%s | %16s | 1 |             1111 | origin_1 | suborigin_2 | 1 2 3\n"
        "%s | %16s | 3 |             2222 | origin_3 | suborigin_4 | 5 6 7\n"
        "%s | %16s | 5 |             3333 | origin_5 | suborigin_6 | 0000: 61 62 63 64 65 66 67 68  69 6a 6b 6c 6d 6e 6f 70  abcdefghijklmnop\n"
        "%s | %16s | 5 |             3333 | origin_5 | suborigin_6 | 0010: 71 72 73 74 75 76 77 78  79 7a 41 42 43 44 45 46  qrstuvwxyzABCDEF\n"
        "%s | %16s | 5 |             3333 | origin_5 | suborigin_6 | 0020: 47 48 49 4a 4b 4c 4d 4e  4f 50 51 52 53 54 55 56  GHIJKLMNOPQRSTUV\n"
        "%s | %16s | 5 |             3333 | origin_5 | suborigin_6 | 0030: 57 58 59 5a 30 31 32 33  34 35 36 37 38 39 00     WXYZ0123456789. \n";

    char expected[abc::size::k2 + 1];
    std::snprintf(expected, sizeof(expected), expected_format,
        timestamp.get(), thread_id.get(),
        timestamp.get(), thread_id.get(),
        timestamp.get(), thread_id.get(),
        timestamp.get(), thread_id.get(),
        timestamp.get(), thread_id.get(),
        timestamp.get(), thread_id.get());

    char actual[abc::size::k2 + 1];
    actual[0] = abc::line_ostream<>::endl;
    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 1, sizeof(actual) - 1);
    abc::table_ostream table(&sb);

    bool passed = true;

    {
        abc::diag::debug_line_ostream<abc::size::k2, test_clock> line(&table);
        line.put_any("origin_1", "suborigin_2", abc::diag::severity::critical, 0x1111, "%u %u %u", 1, 2, 3);
        passed = verify_stream_good(context, line, 0x102b4) && passed;

        line.flush();
        line.put_any("origin_3", "suborigin_4", abc::diag::severity::important, 0x2222, "%u %u %u", 5, 6, 7);
        passed = verify_stream_good(context, line, 0x102b5) && passed;
    }

    passed = verify_stream_good(context, table, 0x102b6) && passed;

    {
        abc::diag::debug_line_ostream<abc::size::k2, test_clock> line(&table);
        line.put_binary("origin_5", "suborigin_6", abc::diag::severity::optional, 0x3333, binary, sizeof(binary));
        passed = verify_stream_good(context, line, 0x102b7) && passed;
    }

    passed = verify_stream_good(context, table, 0x102b8) && passed;

    std::ostream seal(&sb);
    seal.put(abc::line_ostream<>::ends);
    seal.flush();
    passed = verify_stream_good(context, table, 0x102b9) && passed;

    passed = context.are_equal(std::strlen(actual), std::strlen(expected), 0x102ba, "%zu");
    passed = context.are_equal(actual, expected, 0x102bb) && passed;

    return passed;
}


bool test_line_diag(test_context& context) {
    thread_id_line_ostream thread_id;
    thread_id.put_thread_id(std::this_thread::get_id());

    timestamp_line_ostream timestamp;
    timestamp.put_timestamp(abc::timestamp<test_clock>(), "%4.4u-%2.2u-%2.2uT%2.2u:%2.2u:%2.2u.%3.3uZ");

    const char binary[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    const char expected_format[] =
        "\n"
        "%s,%s,1,1111,origin_1,suborigin_2,1 2 3\n"
        "%s,%s,3,2222,origin_3,suborigin_4,5 6 7\n"
        "%s,%s,5,3333,origin_5,suborigin_6,0000: 61 62 63 64 65 66 67 68  69 6a 6b 6c 6d 6e 6f 70  abcdefghijklmnop\n"
        "%s,%s,5,3333,origin_5,suborigin_6,0010: 71 72 73 74 75 76 77 78  79 7a 41 42 43 44 45 46  qrstuvwxyzABCDEF\n"
        "%s,%s,5,3333,origin_5,suborigin_6,0020: 47 48 49 4a 4b 4c 4d 4e  4f 50 51 52 53 54 55 56  GHIJKLMNOPQRSTUV\n"
        "%s,%s,5,3333,origin_5,suborigin_6,0030: 57 58 59 5a 30 31 32 33  34 35 36 37 38 39 00     WXYZ0123456789. \n";

    char expected[abc::size::k2 + 1];
    std::snprintf(expected, sizeof(expected), expected_format,
        timestamp.get(), thread_id.get(),
        timestamp.get(), thread_id.get(),
        timestamp.get(), thread_id.get(),
        timestamp.get(), thread_id.get(),
        timestamp.get(), thread_id.get(),
        timestamp.get(), thread_id.get());

    char actual[abc::size::k2 + 1];
    actual[0] = abc::line_ostream<>::endl;
    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 1, sizeof(actual) - 1);
    abc::table_ostream table(&sb);

    bool passed = true;

    {
        abc::diag::diag_line_ostream<abc::size::k2, test_clock> line(&table);
        line.put_any("origin_1", "suborigin_2", abc::diag::severity::critical, 0x1111, "%u %u %u", 1, 2, 3);
        passed = verify_stream_good(context, line, 0x102bc) && passed;

        line.flush();
        line.put_any("origin_3", "suborigin_4", abc::diag::severity::important, 0x2222, "%u %u %u", 5, 6, 7);
        passed = verify_stream_good(context, line, 0x102bd) && passed;
    }

    passed = verify_stream_good(context, table, 0x102be) && passed;

    {
        abc::diag::diag_line_ostream<abc::size::k2, test_clock> line(&table);
        line.put_binary("origin_5", "suborigin_6", abc::diag::severity::optional, 0x3333, binary, sizeof(binary));
        passed = verify_stream_good(context, line, 0x102bf) && passed;
        passed = verify_stream_good(context, table, 0x102c0) && passed;
    }

    passed = verify_stream_good(context, table, 0x102c1) && passed;

    std::ostream seal(&sb);
    seal.put(abc::line_ostream<>::ends);
    seal.flush();
    passed = verify_stream_good(context, table, 0x102c2) && passed;

    passed = context.are_equal(std::strlen(actual), std::strlen(expected), 0x102c3, "%zu");
    passed = context.are_equal(actual, expected, 0x102c4) && passed;

    return passed;
}


bool test_line_test(test_context& context) {
    timestamp_line_ostream timestamp;
    timestamp.put_timestamp(abc::timestamp<test_clock>(), "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u");

    const char binary[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    const char expected_format[] =
        "\n"
        "%s 1 2 3\n"
        "%s     5 6 7\n"
        "%s         0000: 61 62 63 64 65 66 67 68  69 6a 6b 6c 6d 6e 6f 70  abcdefghijklmnop\n"
        "%s         0010: 71 72 73 74 75 76 77 78  79 7a 41 42 43 44 45 46  qrstuvwxyzABCDEF\n"
        "%s         0020: 47 48 49 4a 4b 4c 4d 4e  4f 50 51 52 53 54 55 56  GHIJKLMNOPQRSTUV\n"
        "%s         0030: 57 58 59 5a 30 31 32 33  34 35 36 37 38 39 00     WXYZ0123456789. \n";

    char expected[abc::size::k2 + 1];
    std::snprintf(expected, sizeof(expected), expected_format,
        timestamp.get(),
        timestamp.get(),
        timestamp.get(),
        timestamp.get(),
        timestamp.get(),
        timestamp.get());

    char actual[abc::size::k2 + 1];
    actual[0] = abc::line_ostream<>::endl;
    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 1, sizeof(actual) - 1);
    abc::table_ostream table(&sb);

    bool passed = true;

    {
        abc::diag::test_line_ostream<abc::size::k2, test_clock> line(&table);
        line.put_any("origin_1", "suborigin_2", abc::diag::severity::critical, 0x1111, "%u %u %u", 1, 2, 3);
        passed = verify_stream_good(context, line, 0x102c5) && passed;

        line.flush();
        line.put_any("origin_3", "suborigin_4", abc::diag::severity::important, 0x2222, "%u %u %u", 5, 6, 7);
        passed = verify_stream_good(context, line, 0x102c6) && passed;
    }

    passed = verify_stream_good(context, table, 0x102c7) && passed;

    {
        abc::diag::test_line_ostream<abc::size::k2, test_clock> line(&table);
        line.put_binary("origin_5", "suborigin_6", abc::diag::severity::optional, 0x3333, binary, sizeof(binary));
        passed = verify_stream_good(context, line, 0x102c8) && passed;
    }

    passed = verify_stream_good(context, table, 0x102c9) && passed;

    std::ostream seal(&sb);
    seal.put(abc::line_ostream<>::ends);
    seal.flush();
    passed = verify_stream_good(context, table, 0x102ca) && passed;

    passed = context.are_equal(std::strlen(actual), std::strlen(expected), 0x102cb, "%zu");
    passed = context.are_equal(actual, expected, 0x102cc) && passed;

    return passed;
}


bool test_table_move(test_context& context) {
    char actual[abc::size::_256 + 1] = { };
    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual) - 1);

    bool passed = true;

    abc::table_ostream os1(&sb);
    os1.put_line("first\n");
    os1.flush();
    passed = context.are_equal(actual, "first\n", 0x10731) && passed;

    abc::table_ostream os2(std::move(os1));
    os2.put_line("second\n");
    os2.flush();
    passed = context.are_equal(actual, "first\nsecond\n", 0x10732) && passed;

    return passed;
}


bool test_log_move(test_context& context) {
    using Filter = test_log_filter;
    using Line = abc::diag::debug_line_ostream<>;
    using Log = abc::diag::log_ostream<Line, Filter*>;

    Filter filter("", abc::diag::severity::optional);

    char actual[abc::size::_256 + 1] = { };
    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual) - 1);

    bool passed = true;

    Log os1(&sb, &filter);
    os1.put_line("third\n");
    os1.flush();
    passed = context.are_equal(actual, "third\n", 0x10733) && passed;

    Log os2(std::move(os1));
    os2.put_line("fourth\n");
    os2.flush();
    passed = context.are_equal(actual, "third\nfourth\n", 0x10734) && passed;

    return passed;
}


bool test_line_move(test_context& context) {
    char actual[abc::size::_256 + 1] = { };
    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual) - 1);

    bool passed = true;

    abc::table_ostream table(&sb);
    abc::line_ostream<> os1(&table);
    os1.put_any("first");
    os1.flush();
    passed = context.are_equal(actual, "first\n", 0x10735) && passed;

    abc::line_ostream<> os2(std::move(os1));
    os2.put_any("second");
    os2.flush();
    passed = context.are_equal(actual, "first\nsecond\n", 0x10736) && passed;

    return passed;
}


template <typename Line>
bool _test_line_move(test_context& context, const char* line1_pattern, const char* line2_pattern) {
    using Filter = test_log_filter;
    using Log = abc::diag::log_ostream<Line, Filter*>;

    Filter filter("", abc::diag::severity::optional);

    thread_id_line_ostream thread_id;
    thread_id.put_thread_id(std::this_thread::get_id());

    char actual[abc::size::k1 + 1] = { };
    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual) - 1);

    char expected[abc::size::k1 + 1];

    bool passed = true;

    Log table(&sb, &filter);
    Line os1(&table);
    os1.put_any("origin_1", "suborigin_2", abc::diag::severity::critical, 0x01, "first");
    os1.flush();

    std::snprintf(expected, sizeof(expected), line1_pattern, thread_id.get());
    passed = context.are_equal(actual, expected, 0x10737) && passed;

    Line os2(std::move(os1));
    os2.put_any("origin_3", "suborigin_4", abc::diag::severity::important, 0x02, "second");
    os2.flush();

    std::snprintf(expected, sizeof(expected), line2_pattern, thread_id.get(), thread_id.get());
    passed = context.are_equal(actual, expected, 0x10738) && passed;

    return passed;
}


bool test_line_debug_move(test_context& context) {
    using Line = abc::diag::debug_line_ostream<abc::size::k1, test_clock>;

    const char* line1_pattern = "2020-10-15 12:34:56.789 | %16s | 1 |                1 | origin_1 | suborigin_2 | first\n";
    const char* line2_pattern = "2020-10-15 12:34:56.789 | %16s | 1 |                1 | origin_1 | suborigin_2 | first\n2020-10-15 12:34:56.789 | %16s | 3 |                2 | origin_3 | suborigin_4 | second\n";

    return _test_line_move<Line>(context, line1_pattern, line2_pattern);
}


bool test_line_diag_move(test_context& context) {
    using Line = abc::diag::diag_line_ostream<abc::size::k1, test_clock>;

    const char* line1_pattern = "2020-10-15T12:34:56.789Z,%s,1,1,origin_1,suborigin_2,first\n";
    const char* line2_pattern = "2020-10-15T12:34:56.789Z,%s,1,1,origin_1,suborigin_2,first\n2020-10-15T12:34:56.789Z,%s,3,2,origin_3,suborigin_4,second\n";

    return _test_line_move<Line>(context, line1_pattern, line2_pattern);
}


bool test_line_test_move(test_context& context) {
    using Line = abc::diag::test_line_ostream<abc::size::k1, test_clock>;

    const char* line1_pattern = "2020-10-15 12:34:56.789 first\n";
    const char* line2_pattern = "2020-10-15 12:34:56.789 first\n2020-10-15 12:34:56.789     second\n";

    return _test_line_move<Line>(context, line1_pattern, line2_pattern);
}
