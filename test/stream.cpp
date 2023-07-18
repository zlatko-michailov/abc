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

#include "inc/stream.h"


namespace abc { namespace test { namespace stream {

    template <typename Stream>
    class test_stream 
        : public Stream {

    public:
        test_stream(std::streambuf* sb)
            : Stream(sb) {
        }

        test_stream(test_stream&& other)
            : Stream(std::move(other)) {
        }

    public:
        void get(char* s, std::size_t n) {
            Stream::read(s, n);
        }

        void put(const char* s) {
            Stream::write(s, std::strlen(s));
        }
    };

    using test_istream = test_stream<abc::istream>;
    using test_ostream = test_stream<abc::ostream>;


    bool test_istream_move(test_context<abc::test::log>& context) {
        char expected[abc::size::_256 + 1] = "first second";
        abc::buffer_streambuf sb(expected, 0, sizeof(expected) - 1, nullptr, 0, 0);

        char actual[abc::size::_256 + 1] = { };

        bool passed = true;

        test_istream is1(&sb);
        is1.get(actual, 6);
        passed = context.are_equal(actual, "first ", 0x1072b) && passed;

        test_istream is2(std::move(is1));
        is1.get(actual, 6);
        passed = context.are_equal(actual, "second", 0x1072c) && passed;

        return passed;
    }


    bool test_ostream_move(test_context<abc::test::log>& context) {
        char actual[abc::size::_256 + 1] = { };
        abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual) - 1);

        bool passed = true;

        test_ostream os1(&sb);
        os1.put("first ");
        os1.flush();
        passed = context.are_equal(actual, "first ", 0x1072d) && passed;

        test_ostream os2(std::move(os1));
        os2.put("second");
        os2.flush();
        passed = context.are_equal(actual, "first second", 0x1072e) && passed;

        return passed;
    }

}}}

