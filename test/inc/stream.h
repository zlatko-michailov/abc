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


#pragma once

#include "../../src/stream/stream.h"
#include "../../src/diag/tag.h"

#include "test.h"


template <typename Stream>
inline bool verify_stream_good(test_context& context, const Stream& stream, abc::diag::tag_t tag) {
    bool passed = true;

    passed = context.are_equal(stream.good(), true,  tag, "%u") && passed;
    passed = context.are_equal(stream.eof(),  false, tag, "%u") && passed;
    passed = context.are_equal(stream.fail(), false, tag, "%u") && passed;
    passed = context.are_equal(stream.bad(),  false, tag, "%u") && passed;

    return passed;
}


template <typename Stream>
inline bool verify_stream_eof(test_context& context, const Stream& stream, abc::diag::tag_t tag) {
    bool passed = true;

    passed = context.are_equal(stream.good(), false, tag, "%u") && passed;
    passed = context.are_equal(stream.eof(),  true,  tag, "%u") && passed;
    passed = context.are_equal(stream.fail(), false, tag, "%u") && passed;
    passed = context.are_equal(stream.bad(),  false, tag, "%u") && passed;

    return passed;
}


template <typename Stream>
inline bool verify_stream_good(test_context& context, const Stream& stream, std::size_t expected_gcount, abc::diag::tag_t tag) {
    bool passed = true;

    passed = context.are_equal(stream.gcount(), expected_gcount, tag, "%zu") && passed;
    passed = verify_stream_good(context, stream, tag) && passed;

    return passed;
}


template <typename Stream>
inline bool verify_stream_eof(test_context& context, const Stream& stream, std::size_t expected_gcount, abc::diag::tag_t tag) {
    bool passed = true;

    passed = context.are_equal(stream.gcount(), expected_gcount, tag, "%zu") && passed;
    passed = verify_stream_eof(context, stream, tag) && passed;

    return passed;
}


bool test_istream_move(test_context& context);
bool test_ostream_move(test_context& context);
