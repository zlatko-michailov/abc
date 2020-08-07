/*
MIT License

Copyright (c) 2018 Zlatko Michailov 

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

#include "../src/stream.h"

#include "test.h"


namespace abc { namespace test {

	template <typename Stream>
	inline bool verify_stream(test_context<abc::test::log_ptr>& context, const Stream& stream, tag_t tag) {
		bool passed = true;

		passed = context.are_equal(stream.good(), true, tag, "%u") && passed;
		passed = context.are_equal(stream.eof(), false, tag, "%u") && passed;
		passed = context.are_equal(stream.fail(), false, tag, "%u") && passed;
		passed = context.are_equal(stream.bad(), false, tag, "%u") && passed;

		return passed;
	}


	template <typename Stream>
	inline bool verify_stream(test_context<abc::test::log_ptr>& context, const Stream& stream, std::size_t expected_gcount, tag_t tag) {
		bool passed = true;

		passed = context.are_equal(stream.gcount(), expected_gcount, tag, "%u") && passed;
		passed = verify_stream(context, stream, tag) && passed;

		return passed;
	}

}}

