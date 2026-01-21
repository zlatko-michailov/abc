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


#include <string>

#include "inc/multifile_streambuf.h"


template <typename Streambuf>
bool test_move(Streambuf& sb1, test_context& context) {
    std::string path(sb1.path());

    std::ostream out1(&sb1);
    out1.write("one ", 4);
    out1.flush();

    Streambuf sb2(std::move(sb1));
    std::ostream out2(&sb2);
    out2.write("two ", 4);
    out2.flush();


    std::filebuf sbin;
    sbin.open(path, std::ios_base::in);
    std::istream in(&sbin);
    char actual[8 + 1] = { };
    in.read(actual, 8);

    bool passed = true;

    passed = context.are_equal(actual, "one two ", 0x1072a) && passed;

    return passed;
}


bool test_multifile_streambuf_move(test_context& context) {
    abc::stream::multifile_streambuf<std::chrono::system_clock> sb1("out/test", std::ios_base::out);
    return test_move(sb1, context);
}


bool test_duration_multifile_streambuf_move(test_context& context) {
    abc::stream::duration_multifile_streambuf<std::chrono::system_clock> sb1(std::chrono::minutes(1), "out/test", std::ios_base::out);
    return test_move(sb1, context);
}


bool test_size_multifile_streambuf_move(test_context& context) {
    abc::stream::size_multifile_streambuf<std::chrono::system_clock> sb1(abc::size::k1, "out/test", std::ios_base::out);
    return test_move(sb1, context);
}
