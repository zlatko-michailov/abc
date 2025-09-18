/*
MIT License

Copyright (c) 2018-2024 Zlatko Michailov 

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


#include "inc/vector_streambuf.h"


static bool test_vector_streambuf(test_context& context, const char* text, std::size_t initial_capacity);


bool test_vector_streambuf_1_char(test_context& context) {
    return test_vector_streambuf(context, "x", 50);
}


bool test_vector_streambuf_N_chars(test_context& context) {
    return test_vector_streambuf(context, "This is a slightly longer text", 200);
}


bool test_vector_streambuf_N_chars_grow(test_context& context) {
    return test_vector_streambuf(context, "This is an even longer text with an extension", 10);
}


bool test_vector_streambuf_move(test_context& context) {
    const char* expected = "Test move constructor";

    bool passed = true;

    abc::stream::vector_streambuf sb1(abc::size::_256);
    std::ostream out(&sb1);
    out.write(expected, std::strlen(expected) + 1);
    passed = context.are_equal(sb1.vector().data(), expected, __TAG__) && passed;

    abc::stream::vector_streambuf sb2(std::move(sb1));
    std::istream in(&sb2);
    char actual2[abc::size::_256 + 1];
    in.read(actual2, std::strlen(expected) + 1);
    passed = context.are_equal(actual2, expected, __TAG__) && passed;

    return passed;
}


bool test_vector_streambuf(test_context& context, const char* text, std::size_t initial_capacity) {
    abc::stream::vector_streambuf sb(initial_capacity);

    std::ostream out(&sb);
    
    std::size_t text_len = std::strlen(text);
    std::size_t total_capacity = text_len + 1;
    std::size_t first_len = total_capacity > initial_capacity ? initial_capacity : text_len;

    std::size_t i = 0;
    for (; i < first_len; i++) {
        out.put(text[i]);
    }

    if (total_capacity > initial_capacity) {
        sb.ensure_capacity(total_capacity);

        for (; i < text_len; i++) {
            out.put(text[i]);
        }
    }

    out.put(abc::ascii::ends);

    return context.are_equal(sb.vector().data(), text, __TAG__);
}
