/*
MIT License

Copyright (c) 2018-2020 Zlatko Michailov 

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

#include "table.h"
#include "clock.h"
#include "stream.h"


namespace abc { namespace test { namespace table {

	using thread_id_line_ostream = abc::line_ostream<17>;
	using timestamp_line_ostream = abc::line_ostream<60>;


	bool test_table_line_debug(test_context<abc::test::log>& context) {
		thread_id_line_ostream thread_id;
		thread_id.put_thread_id(std::this_thread::get_id());

		timestamp_line_ostream timestamp;
		timestamp.put_timestamp(abc::timestamp<clock>(), "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u");

		const char binary[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

		const char expected_format[] =
			"\n"
			"%s | %16s | 8003 | 1 |             1111 | 1 2 3\n"
			"%s | %16s | 8004 | 3 |             2222 | 5 6 7\n"
			"%s | %16s | 8005 | 4 |             3333 | 0000: 61 62 63 64 65 66 67 68  69 6a 6b 6c 6d 6e 6f 70  abcdefghijklmnop\n"
			"%s | %16s | 8005 | 4 |             3333 | 0010: 71 72 73 74 75 76 77 78  79 7a 41 42 43 44 45 46  qrstuvwxyzABCDEF\n"
			"%s | %16s | 8005 | 4 |             3333 | 0020: 47 48 49 4a 4b 4c 4d 4e  4f 50 51 52 53 54 55 56  GHIJKLMNOPQRSTUV\n"
			"%s | %16s | 8005 | 4 |             3333 | 0030: 57 58 59 5a 30 31 32 33  34 35 36 37 38 39 00     WXYZ0123456789. \n";

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
			abc::debug_line_ostream<abc::size::k2, abc::test::clock> line(&table);
			line.put_any(abc::category::abc::socket, abc::severity::critical, 0x1111, "%u %u %u", 1, 2, 3);
			passed = verify_stream(context, line, 0x102b4) && passed;

			line.flush();
			line.put_any(abc::category::abc::http, abc::severity::important, 0x2222, "%u %u %u", 5, 6, 7);
			passed = verify_stream(context, line, 0x102b5) && passed;
		}
		passed = verify_stream(context, table, 0x102b6) && passed;
		{
			abc::debug_line_ostream<abc::size::k2, abc::test::clock> line(&table);
			line.put_binary(abc::category::abc::json, abc::severity::optional, 0x3333, binary, sizeof(binary));
			passed = verify_stream(context, line, 0x102b7) && passed;
		}
		passed = verify_stream(context, table, 0x102b8) && passed;

		std::ostream seal(&sb);
		seal.put(abc::line_ostream<>::ends);
		seal.flush();
		passed = verify_stream(context, table, 0x102b9) && passed;

		passed = context.are_equal(std::strlen(actual), std::strlen(expected), 0x102ba, "%zu");
		passed = context.are_equal(actual, expected, 0x102bb) && passed;

		return passed;
	}


	bool test_table_line_diag(test_context<abc::test::log>& context) {
		thread_id_line_ostream thread_id;
		thread_id.put_thread_id(std::this_thread::get_id());

		timestamp_line_ostream timestamp;
		timestamp.put_timestamp(abc::timestamp<clock>(), "%4.4u-%2.2u-%2.2uT%2.2u:%2.2u:%2.2u.%3.3uZ");

		const char binary[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

		const char expected_format[] =
			"\n"
			"%s,%s,8003,1,1111,1 2 3\n"
			"%s,%s,8004,3,2222,5 6 7\n"
			"%s,%s,8005,4,3333,0000: 61 62 63 64 65 66 67 68  69 6a 6b 6c 6d 6e 6f 70  abcdefghijklmnop\n"
			"%s,%s,8005,4,3333,0010: 71 72 73 74 75 76 77 78  79 7a 41 42 43 44 45 46  qrstuvwxyzABCDEF\n"
			"%s,%s,8005,4,3333,0020: 47 48 49 4a 4b 4c 4d 4e  4f 50 51 52 53 54 55 56  GHIJKLMNOPQRSTUV\n"
			"%s,%s,8005,4,3333,0030: 57 58 59 5a 30 31 32 33  34 35 36 37 38 39 00     WXYZ0123456789. \n";

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
			abc::diag_line_ostream<abc::size::k2, abc::test::clock> line(&table);
			line.put_any(abc::category::abc::socket, abc::severity::critical, 0x1111, "%u %u %u", 1, 2, 3);
			passed = verify_stream(context, line, 0x102bc) && passed;

			line.flush();
			line.put_any(abc::category::abc::http, abc::severity::important, 0x2222, "%u %u %u", 5, 6, 7);
			passed = verify_stream(context, line, 0x102bd) && passed;
		}
		passed = verify_stream(context, table, 0x102be) && passed;
		{
			abc::diag_line_ostream<abc::size::k2, abc::test::clock> line(&table);
			line.put_binary(abc::category::abc::json, abc::severity::optional, 0x3333, binary, sizeof(binary));
			passed = verify_stream(context, line, 0x102bf) && passed;
			passed = verify_stream(context, table, 0x102c0) && passed;
		}
		passed = verify_stream(context, table, 0x102c1) && passed;

		std::ostream seal(&sb);
		seal.put(abc::line_ostream<>::ends);
		seal.flush();
		passed = verify_stream(context, table, 0x102c2) && passed;

		passed = context.are_equal(std::strlen(actual), std::strlen(expected), 0x102c3, "%zu");
		passed = context.are_equal(actual, expected, 0x102c4) && passed;

		return passed;
	}


	bool test_table_line_test(test_context<abc::test::log>& context) {
		timestamp_line_ostream timestamp;
		timestamp.put_timestamp(abc::timestamp<clock>(), "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u");

		const char binary[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

		const char expected_format[] =
			"\n"
			"%s 1 2 3\n"
			"%s     5 6 7\n"
			"%s       0000: 61 62 63 64 65 66 67 68  69 6a 6b 6c 6d 6e 6f 70  abcdefghijklmnop\n"
			"%s       0010: 71 72 73 74 75 76 77 78  79 7a 41 42 43 44 45 46  qrstuvwxyzABCDEF\n"
			"%s       0020: 47 48 49 4a 4b 4c 4d 4e  4f 50 51 52 53 54 55 56  GHIJKLMNOPQRSTUV\n"
			"%s       0030: 57 58 59 5a 30 31 32 33  34 35 36 37 38 39 00     WXYZ0123456789. \n";

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
			abc::test_line_ostream<abc::size::k2, abc::test::clock> line(&table);
			line.put_any(abc::category::abc::socket, abc::severity::critical, 0x1111, "%u %u %u", 1, 2, 3);
			passed = verify_stream(context, line, 0x102c5) && passed;

			line.flush();
			line.put_any(abc::category::abc::http, abc::severity::important, 0x2222, "%u %u %u", 5, 6, 7);
			passed = verify_stream(context, line, 0x102c6) && passed;
		}
		passed = verify_stream(context, table, 0x102c7) && passed;
		{
			abc::test_line_ostream<abc::size::k2, abc::test::clock> line(&table);
			line.put_binary(abc::category::abc::json, abc::severity::optional, 0x3333, binary, sizeof(binary));
			passed = verify_stream(context, line, 0x102c8) && passed;
		}
		passed = verify_stream(context, table, 0x102c9) && passed;

		std::ostream seal(&sb);
		seal.put(abc::line_ostream<>::ends);
		seal.flush();
		passed = verify_stream(context, table, 0x102ca) && passed;

		passed = context.are_equal(std::strlen(actual), std::strlen(expected), 0x102cb, "%zu");
		passed = context.are_equal(actual, expected, 0x102cc) && passed;

		return passed;
	}

}}}

