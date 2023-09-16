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


#include "../src/diag/tag.h"

#include "inc/stream.h"
#include "inc/http.h"


#if 0 //// TODO: Remove
template <typename HttpStream>
static bool verify_string(test_context& context, const char* actual, const char* expected, const HttpStream& stream, abc::diag::tag_t tag);

template <typename HttpStream>
static bool verify_binary(test_context& context, const void* actual, const void* expected, std::size_t size, const HttpStream& stream, abc::diag::tag_t tag);
#endif


bool test_http_request_istream_extraspaces(test_context& context) {
    char content[] =
        "GET   http://a.com/b?c=d    HTTP/12.345  \r\n"
        "Name:Value\r\n"
        "Multi_Word-Name:  Value  with   spaces   inside \t \r\n"
        "Multi-Line   :   First line\r\n"
        " Second  line  \r\n"
        "\t    \t  \t    Third  line   \r\n"
        "Trailing-Spaces  :  3 spaces   \r\n"
        "\r\n"
        "  123 \t abc \r x \n";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::net::http_request_istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    std::string method = istream.get_method();
    passed = context.are_equal(method.c_str(), "GET", __TAG__) && passed;
    passed = verify_stream_good(context, istream, std::strlen("GET"), 0x1006a) && passed;

    abc::net::http_resource resource = istream.get_resource();
    passed = context.are_equal(resource.path.c_str(), "http://a.com/b", 0x1006b) && passed;
    passed = context.are_equal(resource.parameters.size(), (std::size_t)1, __TAG__, "%zu") && passed;
    passed = context.are_equal(resource.parameters["c"].c_str(), "d", __TAG__) && passed;
    passed = verify_stream_good(context, istream, std::strlen("http://a.com/b?c=d"), __TAG__) && passed;

    std::string protocol = istream.get_protocol();
    passed = context.are_equal(protocol.c_str(), "HTTP/12.345", __TAG__) && passed;
    passed = verify_stream_good(context, istream, std::strlen("HTTP/12.345"), __TAG__) && passed;

    abc::net::http_headers headers = istream.get_headers();
    passed = context.are_equal(headers.size(), (std::size_t)4, __TAG__, "%zu") && passed;
    passed = context.are_equal(headers["Name"].c_str(), "Value", 0x1006e) && passed;
    passed = context.are_equal(headers["Multi_Word-Name"].c_str(), "Value with spaces inside", 0x10070) && passed;
    passed = context.are_equal(headers["Multi-Line"].c_str(), "First line Second line Third line", 0x10072) && passed;
    passed = context.are_equal(headers["Trailing-Spaces"].c_str(), "3 spaces", 0x1006e) && passed;
    passed = verify_stream_good(context, istream, std::strlen("Name:Value\r\nMulti_Word-Name:Value with spaces inside\r\nMulti-Line:First line Second line Third line\r\nTrailing-Spaces:3 spaces\r\n"), __TAG__) && passed;

    std::string body = istream.get_body(4);
    passed = context.are_equal(body.c_str(), "  12", __TAG__) && passed;
    passed = verify_stream_good(context, istream, std::strlen("  12"), __TAG__) && passed;

    body = istream.get_body(1000);
    passed = context.are_equal(body.c_str(), "3 \t abc \r x \n", __TAG__) && passed;
    passed = verify_stream_eof(context, istream, std::strlen("3 \t abc \r x \n"), __TAG__) && passed;

    return passed;
}


bool test_http_request_istream_bodytext(test_context& context) {
    char content[] =
        "POST http://a.com/bbb/cc/d?x=111&yy=22&zzz=3 HTTP/1.1\r\n"
        "\r\n"
        "{\r\n"
        "  \"foo\": 42,\r\n"
        "  \"bar\": \"qwerty\"\r\n"
        "}";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::net::http_request_istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    const std::size_t binary_line_size      = 10;
    const std::size_t binary_line_remainder =  7;

    std::string method = istream.get_method();
    passed = context.are_equal(method.c_str(), "POST", __TAG__) && passed;
    passed = verify_stream_good(context, istream, std::strlen("POST"), __TAG__) && passed;

    abc::net::http_resource resource = istream.get_resource();
    passed = context.are_equal(resource.path.c_str(), "http://a.com/bbb/cc/d", __TAG__) && passed;
    passed = context.are_equal(resource.parameters.size(), (std::size_t)3, __TAG__, "%zu") && passed;
    passed = context.are_equal(resource.parameters["x"].c_str(), "111", __TAG__) && passed;
    passed = context.are_equal(resource.parameters["yy"].c_str(), "22", __TAG__) && passed;
    passed = context.are_equal(resource.parameters["zzz"].c_str(), "3", __TAG__) && passed;
    passed = verify_stream_good(context, istream, std::strlen("http://a.com/bbb/cc/d?x=111&yy=22&zzz=3"), __TAG__) && passed;

    std::string protocol = istream.get_protocol();
    passed = context.are_equal(protocol.c_str(), "HTTP/1.1", __TAG__) && passed;
    passed = verify_stream_good(context, istream, std::strlen("HTTP/1.1"), __TAG__) && passed;

    abc::net::http_headers headers = istream.get_headers();
    passed = context.are_equal(headers.size(), (std::size_t)0, __TAG__, "%zu") && passed;
    passed = verify_stream_good(context, istream, (std::size_t)0, __TAG__) && passed;

    std::string body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)"{\r\n  \"foo\"", binary_line_size, __TAG__) && passed;
    passed = verify_stream_good(context, istream, binary_line_size, __TAG__) && passed;

    body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)": 42,\r\n  \"", binary_line_size, __TAG__) && passed;
    passed = verify_stream_good(context, istream, binary_line_size, __TAG__) && passed;

    body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)"bar\": \"qwe", binary_line_size, __TAG__) && passed;
    passed = verify_stream_good(context, istream, binary_line_size, __TAG__) && passed;

    body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)"rty\"\r\n}", binary_line_remainder, __TAG__) && passed;
    passed = verify_stream_eof(context, istream, binary_line_remainder, __TAG__) && passed;

    return passed;
}


bool test_http_request_istream_bodybinary(test_context& context) {
    char content[] =
        "POST http://a.com/b?c=d HTTP/1.1\r\n"
        "\r\n"
        "\x01\x05\x10 text \x02\x03\x12 mixed \x04\x18\x19 with \x7f\x80 bytes \xaa\xff";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::net::http_request_istream<test_log*> istream(&sb, context.log());

    bool passed = true;

    const std::size_t binary_line_size      = 16;
    const std::size_t binary_line_remainder =  7;

    std::string method = istream.get_method();
    passed = context.are_equal(method.c_str(), "POST", __TAG__) && passed;
    passed = verify_stream_good(context, istream, std::strlen("POST"), __TAG__) && passed;

    abc::net::http_resource resource = istream.get_resource();
    passed = context.are_equal(resource.path.c_str(), "http://a.com/b", __TAG__) && passed;
    passed = context.are_equal(resource.parameters.size(), (std::size_t)1, __TAG__, "%zu") && passed;
    passed = context.are_equal(resource.parameters["c"].c_str(), "d", __TAG__) && passed;
    passed = verify_stream_good(context, istream, std::strlen("http://a.com/b?c=d"), __TAG__) && passed;

    std::string protocol = istream.get_protocol();
    passed = context.are_equal(protocol.c_str(), "HTTP/1.1", __TAG__) && passed;
    passed = verify_stream_good(context, istream, std::strlen("HTTP/1.1"), __TAG__) && passed;

    abc::net::http_headers headers = istream.get_headers();
    passed = context.are_equal(headers.size(), (std::size_t)0, __TAG__, "%zu") && passed;
    passed = verify_stream_good(context, istream, (std::size_t)0, __TAG__) && passed;

    std::string body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)"\x01\x05\x10 text \x02\x03\x12 mix", binary_line_size, __TAG__) && passed;
    passed = verify_stream_good(context, istream, binary_line_size, __TAG__) && passed;

    body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)"ed \x04\x18\x19 with \x7f\x80 b", binary_line_size, __TAG__) && passed;
    passed = verify_stream_good(context, istream, binary_line_size, __TAG__) && passed;

    body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)"ytes \xaa\xff", binary_line_remainder, __TAG__) && passed;
    passed = verify_stream_eof(context, istream, binary_line_remainder, __TAG__) && passed;

    return passed;
}


#if 0 ////
bool test_http_request_istream_realworld_01(test_context<abc::test::log>& context) {
    char content[] =
        "GET https://en.cppreference.com/w/cpp/io/basic_streambuf HTTP/1.1\r\n"
        "Host: en.cppreference.com\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate, br\r\n"
        "Connection: keep-alive\r\n"
        "Cookie: __utma=165123437.761011328.1578550293.1590821219.1590875063.126; __utmz=165123437.1581492299.50.2.utmcsr=bing|utmccn=(organic)|utmcmd=organic|utmctr=(not%20provided); _bsap_daycap=407621%2C407621%2C408072%2C408072%2C408072%2C408072; _bsap_lifecap=407621%2C407621%2C408072%2C408072%2C408072%2C408072; __utmc=165123437\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "Cache-Control: max-age=0\r\n"
        "\r\n";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::http_request_istream<abc::test::log> istream(&sb, context.log);

    char buffer[1024];
    bool passed = true;

    istream.get_method(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "GET", istream, 0x10085) && passed;

    istream.get_resource(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "https://en.cppreference.com/w/cpp/io/basic_streambuf", istream, 0x10086) && passed;

    istream.get_protocol(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "HTTP/1.1", istream, 0x10087) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Host", istream, 0x10088) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "en.cppreference.com", istream, 0x10089) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "User-Agent", istream, 0x1008a) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0", istream, 0x1008b) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Accept", istream, 0x1008c) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8", istream, 0x1008d) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Accept-Language", istream, 0x1008e) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "en-US,en;q=0.5", istream, 0x1008f) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Accept-Encoding", istream, 0x10090) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "gzip, deflate, br", istream, 0x10091) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Connection", istream, 0x10092) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "keep-alive", istream, 0x10093) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Cookie", istream, 0x10094) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "__utma=165123437.761011328.1578550293.1590821219.1590875063.126; __utmz=165123437.1581492299.50.2.utmcsr=bing|utmccn=(organic)|utmcmd=organic|utmctr=(not%20provided); _bsap_daycap=407621%2C407621%2C408072%2C408072%2C408072%2C408072; _bsap_lifecap=407621%2C407621%2C408072%2C408072%2C408072%2C408072; __utmc=165123437", istream, 0x10095) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Upgrade-Insecure-Requests", istream, 0x10096) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "1", istream, 0x10097) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Cache-Control", istream, 0x10098) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "max-age=0", istream, 0x10099) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "", istream, 0x1009a) && passed;

    return passed;
}


// --------------------------------------------------------------


bool test_http_request_istream_resource(test_context<abc::test::log>& context, const char* resource, const char* path, const char* parameter_name, const char* expected_parameter_value, abc::tag_t tag);


bool test_http_request_istream_resource_01(test_context<abc::test::log>& context) {
    return test_http_request_istream_resource(context, "/path", "/path", "p3", nullptr, 0x103b6);
}


bool test_http_request_istream_resource_02(test_context<abc::test::log>& context) {
    return test_http_request_istream_resource(context, "/path?", "/path", "p3", nullptr, 0x103b7);
}


bool test_http_request_istream_resource_03(test_context<abc::test::log>& context) {
    return test_http_request_istream_resource(context, "/path?p1=123&p2&p3=42", "/path", "p3", "42", 0x103b8);
}


bool test_http_request_istream_resource_04(test_context<abc::test::log>& context) {
    return test_http_request_istream_resource(context, "/path?p1=123&p2&p3=42&p4=56", "/path", "p3", "42", 0x103b9);
}


bool test_http_request_istream_resource(test_context<abc::test::log>& context, const char* resource, const char* path, const char* /*parameter_name*/, const char* expected_parameter_value, abc::tag_t /*tag*/) {
    constexpr std::size_t buffer_size = 200;
    char buffer[buffer_size + 1];
    std::memset(buffer, 'x', buffer_size);


    const std::size_t resource_len = std::strlen(resource);
    std::memcpy(buffer, resource, resource_len + 1);
    buffer[resource_len + 1] = '\0';

    bool passed = true;
    abc::http_request_istream<abc::test::log>::split_resource(buffer, buffer_size);
    passed = context.are_equal(buffer, path, 0x103ba) && passed;

    const char* parameter_value = abc::http_request_istream<abc::test::log>::get_resource_parameter(buffer, buffer_size, "p3");
    if (expected_parameter_value != nullptr) {
        passed = context.are_equal(parameter_value, expected_parameter_value, 0x103bb) && passed;
    }
    else {
        passed = context.are_equal<void*>((void*)parameter_value, (void*)nullptr, 0x103bc, "%p") && passed;
    }

    return passed;
}


// --------------------------------------------------------------


bool test_http_request_ostream_bodytext(test_context<abc::test::log>& context) {
    const char expected[] =
        "POST http://a.com/b?c=d HTTP/1.1\r\n"
        "Simple-Header-Name: Simple-Header-Value\r\n"
        "List: items separated by a single space\r\n"
        "Multi-Line: first line second line third line\r\n"
        "\r\n"
        "{\r\n"
        "  \"foo\": 42,\r\n"
        "  \"bar\": \"qwerty\"\r\n"
        "}";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::http_request_ostream<abc::test::log> ostream(&sb, context.log);

    bool passed = true;

    ostream.put_method("POST");
    passed = verify_stream(context, ostream, 0x1009b) && passed;

    ostream.put_resource("http://a.com/b?c=d");
    passed = verify_stream(context, ostream, 0x1009c) && passed;

    ostream.put_protocol("HTTP/1.1");
    passed = verify_stream(context, ostream, 0x1009d) && passed;

    ostream.put_header_name("Simple-Header-Name");
    passed = verify_stream(context, ostream, 0x1009e) && passed;

    ostream.put_header_value("Simple-Header-Value");
    passed = verify_stream(context, ostream, 0x1009f) && passed;

    ostream.put_header_name("List");
    passed = verify_stream(context, ostream, 0x100a0) && passed;

    ostream.put_header_value(" \t items  \t\t  separated   by \t  a\t\tsingle space\t");
    passed = verify_stream(context, ostream, 0x100a1) && passed;

    ostream.put_header_name("Multi-Line");
    passed = verify_stream(context, ostream, 0x100a2) && passed;

    ostream.put_header_value("first line \r\n  \t  second  line\t \r\n\tthird line\t");
    passed = verify_stream(context, ostream, 0x100a3) && passed;

    ostream.end_headers();

    ostream.put_body("{\r\n");
    passed = verify_stream(context, ostream, 0x100a4) && passed;

    ostream.put_body("  \"foo\": 42,\r\n");
    passed = verify_stream(context, ostream, 0x100a5) && passed;

    ostream.put_body("  \"bar\": \"qwerty\"\r\n");
    passed = verify_stream(context, ostream, 0x100a6) && passed;

    ostream.put_body("}");
    passed = verify_stream(context, ostream, 0x100a7) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x100a8) && passed;

    return passed;
}


bool test_http_request_ostream_bodybinary(test_context<abc::test::log>& context) {
    const char expected[] =
        "GET http://a.com/b?c=d HTTP/1.1\r\n"
        "Multi-Line: second line third line\r\n"
        "\r\n"
        "\x01\x04\x10\x1f"
        "\x20\x70\x7f"
        "\x80\xa5\xb8\xcc\xdd\xff";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::http_request_ostream<abc::test::log> ostream(&sb, context.log);

    bool passed = true;

    ostream.put_method("GET");
    passed = verify_stream(context, ostream, 0x100a9) && passed;

    ostream.put_resource("http://a.com/b?c=d");
    passed = verify_stream(context, ostream, 0x100aa) && passed;

    ostream.put_protocol("HTTP/1.1");
    passed = verify_stream(context, ostream, 0x100ab) && passed;

    ostream.put_header_name("Multi-Line");
    passed = verify_stream(context, ostream, 0x100ac) && passed;

    ostream.put_header_value("\r\n\tsecond line\t\r\n third  line      ");
    passed = verify_stream(context, ostream, 0x100ad) && passed;

    ostream.end_headers();

    ostream.put_body("\x01\x04\x10\x1f");
    passed = verify_stream(context, ostream, 0x100ae) && passed;

    ostream.put_body("\x20\x70\x7f");
    passed = verify_stream(context, ostream, 0x100af) && passed;

    ostream.put_body("\x80\xa5\xb8\xcc\xdd\xff");
    passed = verify_stream(context, ostream, 0x100b0) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x100b1) && passed;

    return passed;
}


// --------------------------------------------------------------


bool test_http_response_istream_extraspaces(test_context<abc::test::log>& context) {
    char content[] =
        "HTTP/12.345  789  \t  Something went wrong  \r\n"
        "Header-Name:Header-Value\r\n"
        "\r\n";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::http_response_istream<abc::test::log> istream(&sb, context.log);

    char buffer[101];
    bool passed = true;

    istream.get_protocol(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "HTTP/12.345", istream, 0x100b2) && passed;

    istream.get_status_code(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "789", istream, 0x100b3) && passed;

    istream.get_reason_phrase(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Something went wrong  ", istream, 0x100b4) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Header-Name", istream, 0x100b5) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Header-Value", istream, 0x100b6) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "", istream, 0x100b7) && passed;

    return passed;
}


bool test_http_response_istream_realworld_01(test_context<abc::test::log>& context) {
    char content[] =
        "HTTP/1.1 302\r\n"
        "Set-Cookie: ADRUM_BTa=R:59|g:a2345a60-c557-41f0-8cd9-0ee876b70b76; Max-Age=30; Expires=Sun, 31-May-2020 01:27:14 GMT; Path=/\r\n"
        "Cache-Control: no-cache, no-store, max-age=0, must-revalidate\r\n"
        "Location: https://xerxes-sub.xerxessecure.com/xerxes-jwt/init?state=eyJlbmMiOiJBMTI4R0NNIiwiYWxnIjoiUlNBLU9BRVAtMjU2In0.\r\n"
        "Content-Length: 0\r\n"
        "Date: Sun, 31 May 2020 01:26:44 GMT\r\n"
        "\r\n";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::http_response_istream<abc::test::log> istream(&sb, context.log);

    char buffer[201];
    bool passed = true;

    istream.get_protocol(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "HTTP/1.1", istream, 0x100b8) && passed;

    istream.get_status_code(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "302", istream, 0x100b9) && passed;

    istream.get_reason_phrase(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "", istream, 0x100ba) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Set-Cookie", istream, 0x100bb) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "ADRUM_BTa=R:59|g:a2345a60-c557-41f0-8cd9-0ee876b70b76; Max-Age=30; Expires=Sun, 31-May-2020 01:27:14 GMT; Path=/", istream, 0x100bc) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Cache-Control", istream, 0x100bd) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "no-cache, no-store, max-age=0, must-revalidate", istream, 0x100be) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Location", istream, 0x100bf) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "https://xerxes-sub.xerxessecure.com/xerxes-jwt/init?state=eyJlbmMiOiJBMTI4R0NNIiwiYWxnIjoiUlNBLU9BRVAtMjU2In0.", istream, 0x100c0) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Content-Length", istream, 0x100c1) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "0", istream, 0x100c2) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Date", istream, 0x100c3) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Sun, 31 May 2020 01:26:44 GMT", istream, 0x100c4) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "", istream, 0x100c5) && passed;

    return passed;
}


bool test_http_response_istream_realworld_02(test_context<abc::test::log>& context) {
    char content[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Access-Control-Expose-Headers: X-Content-Type-Options,Cache-Control,Pragma,ContextId,Content-Length,Connection,MS-CV,Date\r\n"
        "Content-Length: 205\r\n"
        "\r\n"
        "{\"next\":\"https://centralus.notifications.teams.microsoft.com/users/8:orgid:66c7bbfd-e15c-4257-ad6b-867c195de604/endpoints/0bf687c1-c864-45df-891a-90f548dee242/events/poll?cursor=1590886559&epfs=srt&sca=2\"}\r\n"
        "\r\n";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::http_response_istream<abc::test::log> istream(&sb, context.log);

    char buffer[201];
    bool passed = true;

    istream.get_protocol(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "HTTP/1.1", istream, 0x100c6) && passed;

    istream.get_status_code(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "200", istream, 0x100c7) && passed;

    istream.get_reason_phrase(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "OK", istream, 0x100c8) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Content-Type", istream, 0x100c9) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "application/json; charset=utf-8", istream, 0x100ca) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Access-Control-Expose-Headers", istream, 0x100cb) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "X-Content-Type-Options,Cache-Control,Pragma,ContextId,Content-Length,Connection,MS-CV,Date", istream, 0x100cc) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "Content-Length", istream, 0x100cd) && passed;

    istream.get_header_value(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "205", istream, 0x100ce) && passed;

    istream.get_header_name(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "", istream, 0x100cf) && passed;

    istream.get_body(buffer, 200);
    passed = verify_binary(context, buffer, "{\"next\":\"https://centralus.notifications.teams.microsoft.com/users/8:orgid:66c7bbfd-e15c-4257-ad6b-867c195de604/endpoints/0bf687c1-c864-45df-891a-90f548dee242/events/poll?cursor=1590886559&epfs=srt&sc", 200, istream, 0x100d0) && passed;

    istream.get_body(buffer, 5);
    passed = verify_binary(context, buffer, "a=2\"}", 5, istream, 0x100d1) && passed;

    return passed;
}


// --------------------------------------------------------------


bool test_http_response_ostream_bodytext(test_context<abc::test::log>& context) {
    const char expected[] =
        "HTTP/1.1 200 OK\r\n"
        "Simple: simple\r\n"
        "List: foo bar foobar\r\n"
        "\r\n"
        "First line\r\n"
        "  Second line\r\n"
        "\tThird line";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::http_response_ostream<abc::test::log> ostream(&sb, context.log);

    bool passed = true;

    ostream.put_protocol("HTTP/1.1");
    passed = verify_stream(context, ostream, 0x100d2) && passed;

    ostream.put_status_code("200");
    passed = verify_stream(context, ostream, 0x100d3) && passed;

    ostream.put_reason_phrase("OK");
    passed = verify_stream(context, ostream, 0x100d4) && passed;

    ostream.put_header_name("Simple");
    passed = verify_stream(context, ostream, 0x100d5) && passed;

    ostream.put_header_value("simple");
    passed = verify_stream(context, ostream, 0x100d6) && passed;

    ostream.put_header_name("List");
    passed = verify_stream(context, ostream, 0x100d7) && passed;

    ostream.put_header_value("foo    bar\t\t\tfoobar   \t  \t \t ");
    passed = verify_stream(context, ostream, 0x100d8) && passed;

    ostream.end_headers();

    ostream.put_body("First line\r\n");
    passed = verify_stream(context, ostream, 0x100d9) && passed;

    ostream.put_body("  Second line\r\n");
    passed = verify_stream(context, ostream, 0x100da) && passed;

    ostream.put_body("\tThird line\r\n");
    passed = verify_stream(context, ostream, 0x100db) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x100dc) && passed;

    return passed;
}


bool test_http_response_ostream_bodybinary(test_context<abc::test::log>& context) {
    const char expected[] =
        "HTTP/1.1 789 Something went wrong \r\n"
        "Multi-Line-List: aaa bbbb ccc ddd\r\n"
        "\r\n"
        "\x03\x07\x13\x16\x19"
        "\x20\x24\x35\x46\x57\x71\x7f"
        "\x80\x89\xa5\xb6\xc7\xff";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    abc::http_response_ostream<abc::test::log> ostream(&sb, context.log);

    bool passed = true;

    ostream.put_protocol("HTTP/1.1");
    passed = verify_stream(context, ostream, 0x100dd) && passed;

    ostream.put_status_code("789");
    passed = verify_stream(context, ostream, 0x100de) && passed;

    ostream.put_reason_phrase("Something went wrong ");
    passed = verify_stream(context, ostream, 0x100df) && passed;

    ostream.put_header_name("Multi-Line-List");
    passed = verify_stream(context, ostream, 0x100e0) && passed;

    ostream.put_header_value("\r\n  \r\n\taaa  \t bbb\r\n\t\t\tccc\tddd");
    passed = verify_stream(context, ostream, 0x100e1) && passed;

    ostream.end_headers();

    ostream.put_body("\x03\x07\x13\x16\x19");
    passed = verify_stream(context, ostream, 0x100e2) && passed;

    ostream.put_body("\x20\x24\x35\x46\x57\x71\x7f");
    passed = verify_stream(context, ostream, 0x100e3) && passed;

    ostream.put_body("\x80\x89\xa5\xb6\xc7\xff");
    passed = verify_stream(context, ostream, 0x100e4) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x100e5) && passed;

    return passed;
}


// --------------------------------------------------------------


template <typename HttpStream>
bool http_request_istream_move(test_context<abc::test::log>& context) {
    char content[] =
        "GET https://en.cppreference.com/w/cpp/io/basic_streambuf HTTP/1.1\r\n"
        "\r\n";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    HttpStream istream1(&sb, context.log);

    char buffer[1024];
    bool passed = true;

    istream1.get_method(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "GET", static_cast<abc::http_request_istream<abc::test::log>&>(istream1), 0x10715) && passed;

    HttpStream istream2(std::move(istream1));

    istream2.get_resource(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "https://en.cppreference.com/w/cpp/io/basic_streambuf", static_cast<abc::http_request_istream<abc::test::log>&>(istream2), 0x10716) && passed;

    return passed;
}


template <typename HttpStream>
bool http_request_ostream_move(test_context<abc::test::log>& context) {
    const char expected[] =
        "POST http://a.com/b?c=d HTTP/1.1\r\n";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    HttpStream ostream1(&sb, context.log);

    bool passed = true;

    ostream1.put_method("POST");
    passed = verify_stream(context, static_cast<abc::http_request_ostream<abc::test::log>&>(ostream1), 0x10717) && passed;

    HttpStream ostream2(std::move(ostream1));

    ostream2.put_resource("http://a.com/b?c=d");
    passed = verify_stream(context, static_cast<abc::http_request_ostream<abc::test::log>&>(ostream2), 0x10718) && passed;

    ostream2.put_protocol("HTTP/1.1");
    passed = verify_stream(context, static_cast<abc::http_request_ostream<abc::test::log>&>(ostream2), 0x10719) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x1071a) && passed;

    return passed;
}


template <typename HttpStream>
bool http_response_istream_move(test_context<abc::test::log>& context) {
    char content[] =
        "HTTP/1.1 302\r\n"
        "\r\n";

    abc::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    HttpStream istream1(&sb, context.log);

    char buffer[1024];
    bool passed = true;

    istream1.get_protocol(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "HTTP/1.1", static_cast<abc::http_response_istream<abc::test::log>&>(istream1), 0x1071b) && passed;

    HttpStream istream2(std::move(istream1));

    istream2.get_status_code(buffer, sizeof(buffer));
    passed = verify_string(context, buffer, "302", static_cast<abc::http_response_istream<abc::test::log>&>(istream2), 0x1071c) && passed;

    return passed;
}


template <typename HttpStream>
bool http_response_ostream_move(test_context<abc::test::log>& context) {
    const char expected[] =
        "HTTP/1.1 200 OK\r\n";

    char actual [1024 + 1];

    abc::buffer_streambuf sb(nullptr, 0, 0, actual, 0, sizeof(actual));

    HttpStream ostream1(&sb, context.log);

    bool passed = true;

    ostream1.put_protocol("HTTP/1.1");
    passed = verify_stream(context, static_cast<abc::http_response_ostream<abc::test::log>&>(ostream1), 0x1071d) && passed;

    HttpStream ostream2(std::move(ostream1));

    ostream2.put_status_code("200");
    passed = verify_stream(context, static_cast<abc::http_response_ostream<abc::test::log>&>(ostream2), 0x1071e) && passed;

    ostream2.put_reason_phrase("OK");
    passed = verify_stream(context, static_cast<abc::http_response_ostream<abc::test::log>&>(ostream2), 0x1071f) && passed;

    passed = context.are_equal(actual, expected, std::strlen(expected), 0x10720) && passed;

    return passed;
}


bool test_http_request_istream_move(test_context<abc::test::log>& context) {
    return http_request_istream_move<abc::http_request_istream<abc::test::log>>(context);
}


bool test_http_request_ostream_move(test_context<abc::test::log>& context) {
    return http_request_ostream_move<abc::http_request_ostream<abc::test::log>>(context);
}


bool test_http_response_istream_move(test_context<abc::test::log>& context) {
    return http_response_istream_move<abc::http_response_istream<abc::test::log>>(context);
}


bool test_http_response_ostream_move(test_context<abc::test::log>& context) {
    return http_response_ostream_move<abc::http_response_ostream<abc::test::log>>(context);
}


bool test_http_client_stream_move(test_context<abc::test::log>& context) {
    bool passed = true;

    passed = http_request_ostream_move<abc::http_client_stream<abc::test::log>>(context) && passed;
    passed = http_response_istream_move<abc::http_client_stream<abc::test::log>>(context) && passed;

    return passed;
}


bool test_http_server_stream_move(test_context<abc::test::log>& context) {
    bool passed = true;

    passed = http_request_istream_move<abc::http_server_stream<abc::test::log>>(context) && passed;
    passed = http_response_ostream_move<abc::http_server_stream<abc::test::log>>(context) && passed;

    return passed;
}
#endif


// --------------------------------------------------------------


#if 0 //// TODO: Remove
template <typename HttpStream>
static bool verify_string(test_context& context, const char* actual, const char* expected, const HttpStream& stream, abc::diag::tag_t tag) {
    bool passed = true;

    passed = context.are_equal(actual, expected, tag) && passed;
    passed = verify_stream_good(context, stream, std::strlen(expected), tag) && passed;

    return passed;
}


template <typename HttpStream>
static bool verify_binary(test_context& context, const void* actual, const void* expected, std::size_t size, const HttpStream& stream, abc::diag::tag_t tag) {
    bool passed = true;

    passed = context.are_equal(actual, expected, size, tag) && passed;
    passed = verify_stream_good(context, stream, size, tag) && passed;

    return passed;
}
#endif