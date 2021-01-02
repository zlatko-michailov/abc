/*
MIT License

Copyright (c) 2018-2021 Zlatko Michailov 

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

#include "../src/http.h"

#include "test.h"
#include "stream.h"


namespace abc { namespace test { namespace http {

	bool test_http_request_istream_extraspaces(test_context<abc::test::log>& context);
	bool test_http_request_istream_bodytext(test_context<abc::test::log>& context);
	bool test_http_request_istream_bodybinary(test_context<abc::test::log>& context);
	bool test_http_request_istream_realworld_01(test_context<abc::test::log>& context);

	bool test_http_request_ostream_bodytext(test_context<abc::test::log>& context);
	bool test_http_request_ostream_bodybinary(test_context<abc::test::log>& context);

	bool test_http_response_istream_extraspaces(test_context<abc::test::log>& context);
	bool test_http_response_istream_realworld_01(test_context<abc::test::log>& context);
	bool test_http_response_istream_realworld_02(test_context<abc::test::log>& context);

	bool test_http_response_ostream_bodytext(test_context<abc::test::log>& context);
	bool test_http_response_ostream_bodybinary(test_context<abc::test::log>& context);

}}}

