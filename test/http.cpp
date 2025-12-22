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
#include "inc/http.h"


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

    abc::stream::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::net::http::request_istream istream(&sb, context.log());

    bool passed = true;

    std::string method = istream.get_method();
    passed = context.are_equal(method.c_str(), "GET", 0x10af2) && passed;
    passed = verify_stream_good(context, istream, std::strlen("GET"), 0x1006a) && passed;

    abc::net::http::resource resource = istream.get_resource();
    passed = context.are_equal(resource.path.c_str(), "http://a.com/b", 0x1006b) && passed;
    passed = context.are_equal(resource.query.size(), (std::size_t)1, 0x10af3, "%zu") && passed;
    passed = context.are_equal(resource.query["c"].c_str(), "d", 0x10af4) && passed;
    passed = verify_stream_good(context, istream, std::strlen("http://a.com/b?c=d"), 0x10af5) && passed;

    std::string protocol = istream.get_protocol();
    passed = context.are_equal(protocol.c_str(), "HTTP/12.345", 0x10af6) && passed;
    passed = verify_stream_good(context, istream, std::strlen("HTTP/12.345"), 0x10af7) && passed;

    abc::net::http::headers headers = istream.get_headers();
    passed = context.are_equal(headers.size(), (std::size_t)4, 0x10af8, "%zu") && passed;
    passed = context.are_equal(headers["Name"].c_str(), "Value", 0x1006e) && passed;
    passed = context.are_equal(headers["Multi_Word-Name"].c_str(), "Value with spaces inside", 0x10070) && passed;
    passed = context.are_equal(headers["Multi-Line"].c_str(), "First line Second line Third line", 0x10072) && passed;
    passed = context.are_equal(headers["Trailing-Spaces"].c_str(), "3 spaces", 0x1006e) && passed;
    passed = verify_stream_good(context, istream, std::strlen("Name:Value\r\nMulti_Word-Name:Value with spaces inside\r\nMulti-Line:First line Second line Third line\r\nTrailing-Spaces:3 spaces\r\n"), 0x10af9) && passed;

    std::string body = istream.get_body(4);
    passed = context.are_equal(body.c_str(), "  12", 0x10afa) && passed;
    passed = verify_stream_good(context, istream, std::strlen("  12"), 0x10afb) && passed;

    body = istream.get_body(1000);
    passed = context.are_equal(body.c_str(), "3 \t abc \r x \n", 0x10afc) && passed;
    passed = verify_stream_eof(context, istream, std::strlen("3 \t abc \r x \n"), 0x10afd) && passed;

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

    abc::stream::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::net::http::request_istream istream(&sb, context.log());

    bool passed = true;

    const std::size_t binary_line_size      = 10;
    const std::size_t binary_line_remainder =  7;

    std::string method = istream.get_method();
    passed = context.are_equal(method.c_str(), "POST", 0x10afe) && passed;
    passed = verify_stream_good(context, istream, std::strlen("POST"), 0x10aff) && passed;

    abc::net::http::resource resource = istream.get_resource();
    passed = context.are_equal(resource.path.c_str(), "http://a.com/bbb/cc/d", 0x10b00) && passed;
    passed = context.are_equal(resource.query.size(), (std::size_t)3, 0x10b01, "%zu") && passed;
    passed = context.are_equal(resource.query["x"].c_str(), "111", 0x10b02) && passed;
    passed = context.are_equal(resource.query["yy"].c_str(), "22", 0x10b03) && passed;
    passed = context.are_equal(resource.query["zzz"].c_str(), "3", 0x10b04) && passed;
    passed = verify_stream_good(context, istream, std::strlen("http://a.com/bbb/cc/d?x=111&yy=22&zzz=3"), 0x10b05) && passed;

    std::string protocol = istream.get_protocol();
    passed = context.are_equal(protocol.c_str(), "HTTP/1.1", 0x10b06) && passed;
    passed = verify_stream_good(context, istream, std::strlen("HTTP/1.1"), 0x10b07) && passed;

    abc::net::http::headers headers = istream.get_headers();
    passed = context.are_equal(headers.size(), (std::size_t)0, 0x10b08, "%zu") && passed;
    passed = verify_stream_good(context, istream, (std::size_t)0, 0x10b09) && passed;

    std::string body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)"{\r\n  \"foo\"", binary_line_size, 0x10b0a) && passed;
    passed = verify_stream_good(context, istream, binary_line_size, 0x10b0b) && passed;

    body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)": 42,\r\n  \"", binary_line_size, 0x10b0c) && passed;
    passed = verify_stream_good(context, istream, binary_line_size, 0x10b0d) && passed;

    body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)"bar\": \"qwe", binary_line_size, 0x10b0e) && passed;
    passed = verify_stream_good(context, istream, binary_line_size, 0x10b0f) && passed;

    body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)"rty\"\r\n}", binary_line_remainder, 0x10b10) && passed;
    passed = verify_stream_eof(context, istream, binary_line_remainder, 0x10b11) && passed;

    return passed;
}


bool test_http_request_istream_bodybinary(test_context& context) {
    char content[] =
        "POST http://a.com/b?c=d HTTP/1.1\r\n"
        "\r\n"
        "\x01\x05\x10 text \x02\x03\x12 mixed \x04\x18\x19 with \x7f\x80 bytes \xaa\xff";

    abc::stream::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::net::http::request_istream istream(&sb, context.log());

    bool passed = true;

    const std::size_t binary_line_size      = 16;
    const std::size_t binary_line_remainder =  7;

    std::string method = istream.get_method();
    passed = context.are_equal(method.c_str(), "POST", 0x10b12) && passed;
    passed = verify_stream_good(context, istream, std::strlen("POST"), 0x10b13) && passed;

    abc::net::http::resource resource = istream.get_resource();
    passed = context.are_equal(resource.path.c_str(), "http://a.com/b", 0x10b14) && passed;
    passed = context.are_equal(resource.query.size(), (std::size_t)1, 0x10b15, "%zu") && passed;
    passed = context.are_equal(resource.query["c"].c_str(), "d", 0x10b16) && passed;
    passed = verify_stream_good(context, istream, std::strlen("http://a.com/b?c=d"), 0x10b17) && passed;

    std::string protocol = istream.get_protocol();
    passed = context.are_equal(protocol.c_str(), "HTTP/1.1", 0x10b18) && passed;
    passed = verify_stream_good(context, istream, std::strlen("HTTP/1.1"), 0x10b19) && passed;

    abc::net::http::headers headers = istream.get_headers();
    passed = context.are_equal(headers.size(), (std::size_t)0, 0x10b1a, "%zu") && passed;
    passed = verify_stream_good(context, istream, (std::size_t)0, 0x10b1b) && passed;

    std::string body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)"\x01\x05\x10 text \x02\x03\x12 mix", binary_line_size, 0x10b1c) && passed;
    passed = verify_stream_good(context, istream, binary_line_size, 0x10b1d) && passed;

    body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)"ed \x04\x18\x19 with \x7f\x80 b", binary_line_size, 0x10b1e) && passed;
    passed = verify_stream_good(context, istream, binary_line_size, 0x10b1f) && passed;

    body = istream.get_body(binary_line_size);
    passed = context.are_equal((const void*)body.c_str(), (const void *)"ytes \xaa\xff", binary_line_remainder, 0x10b20) && passed;
    passed = verify_stream_eof(context, istream, binary_line_remainder, 0x10b21) && passed;

    return passed;
}


bool test_http_request_istream_realworld_01(test_context& context) {
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

    abc::stream::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::net::http::request_istream istream(&sb, context.log());

    bool passed = true;

    std::string method = istream.get_method();
    passed = context.are_equal(method.c_str(), "GET", 0x10b22) && passed;
    passed = verify_stream_good(context, istream, std::strlen("GET"), 0x10b23) && passed;

    abc::net::http::resource resource = istream.get_resource();
    passed = context.are_equal(resource.path.c_str(), "https://en.cppreference.com/w/cpp/io/basic_streambuf", 0x10b24) && passed;
    passed = context.are_equal(resource.query.size(), (std::size_t)0, 0x10b25, "%zu") && passed;
    passed = verify_stream_good(context, istream, std::strlen("https://en.cppreference.com/w/cpp/io/basic_streambuf"), 0x10b26) && passed;

    std::string protocol = istream.get_protocol();
    passed = context.are_equal(protocol.c_str(), "HTTP/1.1", 0x10b27) && passed;
    passed = verify_stream_good(context, istream, std::strlen("HTTP/1.1"), 0x10b28) && passed;

    abc::net::http::headers headers = istream.get_headers();
    passed = context.are_equal(headers.size(), (std::size_t)9, 0x10b29, "%zu") && passed;
    passed = context.are_equal(headers["Host"].c_str(), "en.cppreference.com", 0x10b2a) && passed;
    passed = context.are_equal(headers["User-Agent"].c_str(), "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0", 0x10b2b) && passed;
    passed = context.are_equal(headers["Accept"].c_str(), "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8", 0x10b2c) && passed;
    passed = context.are_equal(headers["Accept-Language"].c_str(), "en-US,en;q=0.5", 0x10b2d) && passed;
    passed = context.are_equal(headers["Accept-Encoding"].c_str(), "gzip, deflate, br", 0x10b2e) && passed;
    passed = context.are_equal(headers["Connection"].c_str(), "keep-alive", 0x10b2f) && passed;
    passed = context.are_equal(headers["Cookie"].c_str(), "__utma=165123437.761011328.1578550293.1590821219.1590875063.126; __utmz=165123437.1581492299.50.2.utmcsr=bing|utmccn=(organic)|utmcmd=organic|utmctr=(not%20provided); _bsap_daycap=407621%2C407621%2C408072%2C408072%2C408072%2C408072; _bsap_lifecap=407621%2C407621%2C408072%2C408072%2C408072%2C408072; __utmc=165123437", 0x10b30) && passed;
    passed = context.are_equal(headers["Upgrade-Insecure-Requests"].c_str(), "1", 0x10b31) && passed;
    passed = context.are_equal(headers["Cache-Control"].c_str(), "max-age=0", 0x10b32) && passed;
    passed = verify_stream_good(context, istream, std::strlen("Host:en.cppreference.com\r\nUser-Agent:Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0\r\nAccept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nAccept-Language:en-US,en;q=0.5\r\nAccept-Encoding:gzip, deflate, br\r\nConnection:keep-alive\r\nCookie:__utma=165123437.761011328.1578550293.1590821219.1590875063.126; __utmz=165123437.1581492299.50.2.utmcsr=bing|utmccn=(organic)|utmcmd=organic|utmctr=(not%20provided); _bsap_daycap=407621%2C407621%2C408072%2C408072%2C408072%2C408072; _bsap_lifecap=407621%2C407621%2C408072%2C408072%2C408072%2C408072; __utmc=165123437\r\nUpgrade-Insecure-Requests:1\r\nCache-Control:max-age=0\r\n"), 0x10b33) && passed;

    return passed;
}


// --------------------------------------------------------------


bool test_http_request_istream_resource(test_context& context, const char* expected_resource, const char* expected_path, std::size_t expected_parameter_count, const char* expected_parameter_name, const char* expected_parameter_value, const char* expected_fragment);


bool test_http_request_istream_resource_01(test_context& context) {
    return test_http_request_istream_resource(context, "/pa%20th", "/pa th", 0, "p3", nullptr, "");
}


bool test_http_request_istream_resource_02(test_context& context) {
    return test_http_request_istream_resource(context, "/path?", "/path", 0, "p3", nullptr, "");
}


bool test_http_request_istream_resource_03(test_context& context) {
    return test_http_request_istream_resource(context, "/path?p1=123&p2&p%203=4%202", "/path", 3, "p 3", "4 2", "");
}


bool test_http_request_istream_resource_04(test_context& context) {
    return test_http_request_istream_resource(context, "/path?p1=123&p2&p3=42%20&p4=56", "/path", 4, "p3", "42 ", "");
}


bool test_http_request_istream_resource_05(test_context& context) {
    return test_http_request_istream_resource(context, "/path#fr%20ag", "/path", 0, "p3", nullptr, "fr ag");
}


bool test_http_request_istream_resource_06(test_context& context) {
    return test_http_request_istream_resource(context, "/path%20?#frag%20", "/path ", 0, "p3", nullptr, "frag ");
}


bool test_http_request_istream_resource_07(test_context& context) {
    return test_http_request_istream_resource(context, "/path?p1&p2=&p3=12#frag", "/path", 3, "p3", "12", "frag");
}


bool test_http_request_istream_resource_08(test_context& context) {
    return test_http_request_istream_resource(context, "/path?#", "/path", 0, "p3", nullptr, "");
}


bool test_http_request_istream_resource_09(test_context& context) {
    return test_http_request_istream_resource(context, "/path#%20", "/path", 0, "p3", nullptr, " ");
}


bool test_http_request_istream_resource_10(test_context& context) {
    return test_http_request_istream_resource(context, "/path?=12&=34&p3=56", "/path", 1, "p3", "56", "");
}


bool test_http_request_istream_resource(test_context& context, const char* expected_resource, const char* expected_path, std::size_t expected_parameter_count, const char* expected_parameter_name, const char* expected_parameter_value, const char* expected_fragment) {
    std::string content("GET ");
    content.append(expected_resource);
    content.append(" ");

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::http::request_istream istream(&sb, context.log());

    bool passed = true;

    std::string method = istream.get_method();
    passed = context.are_equal(method.c_str(), "GET", 0x10b34) && passed;

    abc::net::http::resource resource = istream.get_resource();
    passed = context.are_equal(resource.path.c_str(), expected_path, 0x10b35) && passed;
    passed = context.are_equal(resource.query.size(), expected_parameter_count, 0x10b36, "%zu") && passed;
    if (expected_parameter_value != nullptr) {
        passed = context.are_equal(resource.query[expected_parameter_name].c_str(), expected_parameter_value, 0x10b37) && passed;
    }
    else {
        passed = context.are_equal(resource.query.find(expected_parameter_name) == resource.query.end(), true, 0x10b38, "%d") && passed;
    }
    passed = context.are_equal(resource.fragment.c_str(), expected_fragment, 0x10b39) && passed;

    return passed;
}


// --------------------------------------------------------------


bool http_request_reader(test_context& context, bool use_headers, bool use_body);


bool test_http_request_reader_none(test_context& context) {
    return http_request_reader(context, false, false);
}


bool test_http_request_reader_headers(test_context& context) {
    return http_request_reader(context, true, false);
}


bool test_http_request_reader_body(test_context& context) {
    return http_request_reader(context, false, true);
}


bool test_http_request_reader_headers_body(test_context& context) {
    return http_request_reader(context, true, true);
}


bool http_request_reader(test_context& context, bool use_headers, bool use_body) {
    char content_body[] =
        "{\r\n"
        "  \"foo\": 42,\r\n"
        "  \"bar\": \"qwerty\"\r\n"
        "}";

    std::string content = "POST http://a.com/bbb/cc/d?x=111&yy=22&zzz=3 HTTP/1.1\r\n";

    if (use_headers) {
        content.append(
            "Host: en.cppreference.com\r\n"
            "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
            "Accept-Language: en-US,en;q=0.5\r\n");
    }
    content.append("\r\n");

    if (use_body) {
        content.append(content_body);
    }

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::http::request_reader reader(&sb, context.log());

    bool passed = true;

    abc::net::http::request request = reader.get_request();
    std::string body = reader.get_body(abc::size::k1);

    passed = context.are_equal(request.method.c_str(), "POST", 0x10b3a) && passed;

    passed = context.are_equal(request.resource.path.c_str(), "http://a.com/bbb/cc/d", 0x10b3b) && passed;
    passed = context.are_equal(request.resource.query.size(), (std::size_t)3, 0x10b3c, "%zu") && passed;
    passed = context.are_equal(request.resource.query["x"].c_str(), "111", 0x10b3d) && passed;
    passed = context.are_equal(request.resource.query["yy"].c_str(), "22", 0x10b3e) && passed;
    passed = context.are_equal(request.resource.query["zzz"].c_str(), "3", 0x10b3f) && passed;

    passed = context.are_equal(request.protocol.c_str(), "HTTP/1.1", 0x10b40) && passed;

    if (use_headers) {
        passed = context.are_equal(request.headers.size(), (std::size_t)4, 0x10b41, "%zu") && passed;
        passed = context.are_equal(request.headers["Host"].c_str(), "en.cppreference.com", 0x10b42) && passed;
        passed = context.are_equal(request.headers["User-Agent"].c_str(), "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0", 0x10b43) && passed;
        passed = context.are_equal(request.headers["Accept"].c_str(), "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8", 0x10b44) && passed;
        passed = context.are_equal(request.headers["Accept-Language"].c_str(), "en-US,en;q=0.5", 0x10b45) && passed;
    }
    else {
        passed = context.are_equal(request.headers.size(), (std::size_t)0, 0x10b46, "%zu") && passed;
    }

    if (use_body) {
        passed = context.are_equal(body.c_str(), content_body, 0x10b47) && passed;
    }
    else {
        passed = context.are_equal(body.c_str(), "", 0x10b48) && passed;
    }

    return passed;
}


// --------------------------------------------------------------


bool test_http_request_ostream_bodytext(test_context& context) {
    const char expected[] =
        "POST http://a.com/b?c=d HTTP/1.1\r\n"
        "List: items separated by a single space\r\n"
        "Multi-Line: first line second line third line\r\n"
        "Simple-Header-Name: Simple-Header-Value\r\n"
        "\r\n"
        "{\r\n"
        "  \"foo\": 42,\r\n"
        "  \"bar\": \"qwerty\"\r\n"
        "}";

    std::stringbuf sb(std::ios_base::out);

    abc::net::http::request_ostream ostream(&sb, context.log());

    bool passed = true;

    ostream.put_method("POST");
    passed = verify_stream_good(context, ostream, 0x1009b) && passed;

    ostream.put_resource("http://a.com/b?c=d");
    passed = verify_stream_good(context, ostream, 0x1009c) && passed;

    ostream.put_protocol("HTTP/1.1");
    passed = verify_stream_good(context, ostream, 0x1009d) && passed;

    abc::net::http::headers headers = {
        { "Simple-Header-Name", "Simple-Header-Value" },
        { "List", " \t items  \t\t  separated   by \t  a\t\tsingle space\t" },
        { "Multi-Line", "first line \r\n  \t  second  line\t \r\n\tthird line\t" },
    };

    ostream.put_headers(headers);
    passed = verify_stream_good(context, ostream, 0x10b49) && passed;

    ostream.put_body("{\r\n");
    passed = verify_stream_good(context, ostream, 0x100a4) && passed;

    ostream.put_body("  \"foo\": 42,\r\n");
    passed = verify_stream_good(context, ostream, 0x100a5) && passed;

    ostream.put_body("  \"bar\": \"qwerty\"\r\n");
    passed = verify_stream_good(context, ostream, 0x100a6) && passed;

    ostream.put_body("}");
    passed = verify_stream_good(context, ostream, 0x100a7) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x100a8) && passed;

    return passed;
}


bool test_http_request_ostream_bodybinary(test_context& context) {
    const char expected[] =
        "GET http://a.com/b?c=d HTTP/1.1\r\n"
        "List: items separated by a single space\r\n"
        "Multi-Line: first line second line third line\r\n"
        "Simple-Header-Name: Simple-Header-Value\r\n"
        "\r\n"
        "\x01\x04\x10\x1f"
        "\x20\x70\x7f"
        "\x80\xa5\xb8\xcc\xdd\xff";

    std::stringbuf sb(std::ios_base::out);

    abc::net::http::request_ostream ostream(&sb, context.log());

    bool passed = true;

    ostream.put_method("GET");
    passed = verify_stream_good(context, ostream, 0x100a9) && passed;

    ostream.put_resource("http://a.com/b?c=d");
    passed = verify_stream_good(context, ostream, 0x100aa) && passed;

    ostream.put_protocol("HTTP/1.1");
    passed = verify_stream_good(context, ostream, 0x100ab) && passed;

    abc::net::http::headers headers = {
        { "Simple-Header-Name", "Simple-Header-Value" },
        { "List", " \t items  \t\t  separated   by \t  a\t\tsingle space\t" },
        { "Multi-Line", "first line \r\n  \t  second  line\t \r\n\tthird line\t" },
    };

    ostream.put_headers(headers);
    passed = verify_stream_good(context, ostream, 0x100a3) && passed;

    ostream.put_body("\x01\x04\x10\x1f");
    passed = verify_stream_good(context, ostream, 0x100ae) && passed;

    ostream.put_body("\x20\x70\x7f");
    passed = verify_stream_good(context, ostream, 0x100af) && passed;

    ostream.put_body("\x80\xa5\xb8\xcc\xdd\xff");
    passed = verify_stream_good(context, ostream, 0x100b0) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x100b1) && passed;

    return passed;
}


// --------------------------------------------------------------


bool test_http_request_ostream_resource(test_context& context, abc::net::http::resource&& actual_resource, const char* expected_resource);


bool test_http_request_ostream_resource_01(test_context& context) {
    abc::net::http::resource actual_resource;
    actual_resource.path = "/fi rst/sec ond";

    return test_http_request_ostream_resource(context, std::move(actual_resource), "/fi%20rst/sec%20ond");
}


bool test_http_request_ostream_resource_02(test_context& context) {
    abc::net::http::resource actual_resource;
    actual_resource.path = "/pa th";
    actual_resource.query["p 1"] = "val 1";
    actual_resource.query["p 2"] = "";
    actual_resource.query["p 3"] = "val 3";

    return test_http_request_ostream_resource(context, std::move(actual_resource), "/pa%20th?p%201=val%201&p%202&p%203=val%203");
}


bool test_http_request_ostream_resource_03(test_context& context) {
    abc::net::http::resource actual_resource;
    actual_resource.path = "/pa th";
    actual_resource.fragment = "fr ag";

    return test_http_request_ostream_resource(context, std::move(actual_resource), "/pa%20th#fr%20ag");
}


bool test_http_request_ostream_resource_04(test_context& context) {
    abc::net::http::resource actual_resource;
    actual_resource.path = "/pa th";
    actual_resource.query["p 1"] = "val 1";
    actual_resource.query["p 2"] = "";
    actual_resource.query["p 3"] = "val 3";
    actual_resource.fragment = "fr ag";

    return test_http_request_ostream_resource(context, std::move(actual_resource), "/pa%20th?p%201=val%201&p%202&p%203=val%203#fr%20ag");
}


bool test_http_request_ostream_resource(test_context& context, abc::net::http::resource&& actual_resource, const char* expected_resource) {
    std::string expected = "GET ";
    expected.append(expected_resource);
    expected.append(" HTTP/1.1\r\n\r\n");

    std::stringbuf sb(std::ios_base::out);

    abc::net::http::request_ostream ostream(&sb, context.log());

    bool passed = true;

    ostream.put_method("GET");
    passed = verify_stream_good(context, ostream, 0x10b4a) && passed;

    ostream.put_resource(actual_resource);
    passed = verify_stream_good(context, ostream, 0x10b4b) && passed;

    ostream.put_protocol("HTTP/1.1");
    passed = verify_stream_good(context, ostream, 0x10b4c) && passed;

    ostream.end_headers();

    passed = context.are_equal(sb.str().c_str(), expected.c_str(), expected.length(), 0x10b4d) && passed;

    return passed;
}


// --------------------------------------------------------------


bool http_request_writer(test_context& context, bool use_headers, bool use_body);


bool test_http_request_writer_none(test_context& context) {
    return http_request_writer(context, false, false);
}


bool test_http_request_writer_headers(test_context& context) {
    return http_request_writer(context, true, false);
}


bool test_http_request_writer_body(test_context& context) {
    return http_request_writer(context, false, true);
}


bool test_http_request_writer_headers_body(test_context& context) {
    return http_request_writer(context, true, true);
}


bool http_request_writer(test_context& context, bool use_headers, bool use_body) {
    abc::net::http::request request;

    request.method = "POST";
    request.resource.path = "http://a.com/bbb/cc/d";
    request.resource.query["x"] = "111";
    request.resource.query["yy"] = "22";
    request.resource.query["zzz"] = "3";

    std::string expected = "POST http://a.com/bbb/cc/d?x=111&yy=22&zzz=3 HTTP/1.1\r\n";

    if (use_headers) {
        request.headers["Host"] = "en.cppreference.com";
        request.headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0";
        request.headers["Accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8";
        request.headers["Accept-Language"] = "en-US,en;q=0.5";

        expected.append(
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
            "Accept-Language: en-US,en;q=0.5\r\n"
            "Host: en.cppreference.com\r\n"
            "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0\r\n");
    }
    expected.append("\r\n");

    std::stringbuf sb(std::ios_base::out);

    abc::net::http::request_writer writer(&sb, context.log());

    writer.put_request(request);

    if (use_body) {
        const char expected_body[] =
            "{\r\n"
            "  \"foo\": 42,\r\n"
            "  \"bar\": \"qwerty\"\r\n"
            "}";

        writer.put_body(expected_body);

        expected.append(expected_body);
    }

    bool passed = true;

    passed = context.are_equal(sb.str().c_str(), expected.c_str(), 0x10b4e) && passed;

    return passed;
}


// --------------------------------------------------------------


bool test_http_response_istream_extraspaces(test_context& context) {
    char content[] =
        "HTTP/12.345  789  \t  Something went wrong  \r\n"
        "Header-Name:Header-Value\r\n"
        "\r\n";

    abc::stream::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::net::http::response_istream istream(&sb, context.log());

    bool passed = true;

    std::string protocol = istream.get_protocol();
    passed = context.are_equal(protocol.c_str(), "HTTP/12.345", 0x10b4f) && passed;
    passed = verify_stream_good(context, istream, std::strlen("HTTP/12.345"), 0x10b50) && passed;

    abc::net::http::status_code_t status = istream.get_status_code();
    passed = context.are_equal(status, (abc::net::http::status_code_t)789, 0x10b51, "%u") && passed;
    passed = verify_stream_good(context, istream, std::strlen("789"), 0x10b52) && passed;

    std::string phrase = istream.get_reason_phrase();
    passed = context.are_equal(phrase.c_str(), "Something went wrong  ", 0x100b4) && passed;
    passed = verify_stream_good(context, istream, std::strlen("Something went wrong  "), 0x10b53) && passed;

    abc::net::http::headers headers = istream.get_headers();
    passed = context.are_equal(headers.size(), (std::size_t)1, 0x10b54, "%zu") && passed;
    passed = context.are_equal(headers["Header-Name"].c_str(), "Header-Value", 0x1006e) && passed;
    passed = verify_stream_good(context, istream, std::strlen("Header-Name:Header-Value\r\n"), 0x10b55) && passed;

    std::string body = istream.get_body(10);
    passed = context.are_equal(body.c_str(), "", 0x10b56) && passed;
    passed = verify_stream_eof(context, istream, 0, 0x10b57) && passed;

    return passed;
}


bool test_http_response_istream_realworld_01(test_context& context) {
    char content[] =
        "HTTP/1.1 302\r\n"
        "Set-Cookie: ADRUM_BTa=R:59|g:a2345a60-c557-41f0-8cd9-0ee876b70b76; Max-Age=30; Expires=Sun, 31-May-2020 01:27:14 GMT; Path=/\r\n"
        "Cache-Control: no-cache, no-store, max-age=0, must-revalidate\r\n"
        "Location: https://xerxes-sub.xerxessecure.com/xerxes-jwt/init?state=eyJlbmMiOiJBMTI4R0NNIiwiYWxnIjoiUlNBLU9BRVAtMjU2In0.\r\n"
        "Content-Length: 0\r\n"
        "Date: Sun, 31 May 2020 01:26:44 GMT\r\n"
        "\r\n";

    abc::stream::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::net::http::response_istream istream(&sb, context.log());

    bool passed = true;

    std::string protocol = istream.get_protocol();
    passed = context.are_equal(protocol.c_str(), "HTTP/1.1", 0x10b58) && passed;
    passed = verify_stream_good(context, istream, std::strlen("HTTP/1.1"), 0x10b59) && passed;

    abc::net::http::status_code_t status = istream.get_status_code();
    passed = context.are_equal(status, (abc::net::http::status_code_t)302, 0x10b5a, "%u") && passed;
    passed = verify_stream_good(context, istream, std::strlen("302"), 0x10b5b) && passed;

    std::string phrase = istream.get_reason_phrase();
    passed = context.are_equal(phrase.c_str(), "", 0x10b5c) && passed;
    passed = verify_stream_good(context, istream, 0, 0x10b5d) && passed;

    abc::net::http::headers headers = istream.get_headers();
    passed = context.are_equal(headers.size(), (std::size_t)5, 0x10b5e, "%zu") && passed;
    passed = context.are_equal(headers["Set-Cookie"].c_str(), "ADRUM_BTa=R:59|g:a2345a60-c557-41f0-8cd9-0ee876b70b76; Max-Age=30; Expires=Sun, 31-May-2020 01:27:14 GMT; Path=/", 0x10b5f) && passed;
    passed = context.are_equal(headers["Cache-Control"].c_str(), "no-cache, no-store, max-age=0, must-revalidate", 0x10b60) && passed;
    passed = context.are_equal(headers["Location"].c_str(), "https://xerxes-sub.xerxessecure.com/xerxes-jwt/init?state=eyJlbmMiOiJBMTI4R0NNIiwiYWxnIjoiUlNBLU9BRVAtMjU2In0.", 0x10b61) && passed;
    passed = context.are_equal(headers["Content-Length"].c_str(), "0", 0x10b62) && passed;
    passed = context.are_equal(headers["Date"].c_str(), "Sun, 31 May 2020 01:26:44 GMT", 0x10b63) && passed;
    passed = verify_stream_good(context, istream, std::strlen("Set-Cookie:ADRUM_BTa=R:59|g:a2345a60-c557-41f0-8cd9-0ee876b70b76; Max-Age=30; Expires=Sun, 31-May-2020 01:27:14 GMT; Path=/\r\nCache-Control:no-cache, no-store, max-age=0, must-revalidate\r\nLocation:https://xerxes-sub.xerxessecure.com/xerxes-jwt/init?state=eyJlbmMiOiJBMTI4R0NNIiwiYWxnIjoiUlNBLU9BRVAtMjU2In0.\r\nContent-Length:0\r\nDate:Sun, 31 May 2020 01:26:44 GMT\r\n"), 0x10b64) && passed;

    std::string body = istream.get_body(10);
    passed = context.are_equal(body.c_str(), "", 0x10b65) && passed;
    passed = verify_stream_eof(context, istream, 0, 0x10b66) && passed;

    return passed;
}


bool test_http_response_istream_realworld_02(test_context& context) {
    char content[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Access-Control-Expose-Headers: X-Content-Type-Options,Cache-Control,Pragma,ContextId,Content-Length,Connection,MS-CV,Date\r\n"
        "Content-Length: 205\r\n"
        "\r\n"
        "{\"next\":\"https://centralus.notifications.teams.microsoft.com/users/8:orgid:66c7bbfd-e15c-4257-ad6b-867c195de604/endpoints/0bf687c1-c864-45df-891a-90f548dee242/events/poll?cursor=1590886559&epfs=srt&sca=2\"}\r\n"
        "\r\n";

    abc::stream::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::net::http::response_istream istream(&sb, context.log());

    bool passed = true;

    std::string protocol = istream.get_protocol();
    passed = context.are_equal(protocol.c_str(), "HTTP/1.1", 0x10b67) && passed;
    passed = verify_stream_good(context, istream, std::strlen("HTTP/1.1"), 0x10b68) && passed;

    abc::net::http::status_code_t status = istream.get_status_code();
    passed = context.are_equal(status, (abc::net::http::status_code_t)200, 0x10b69, "%u") && passed;
    passed = verify_stream_good(context, istream, std::strlen("200"), 0x10b6a) && passed;

    std::string phrase = istream.get_reason_phrase();
    passed = context.are_equal(phrase.c_str(), "OK", 0x10b6b) && passed;
    passed = verify_stream_good(context, istream, std::strlen("OK"), 0x10b6c) && passed;

    abc::net::http::headers headers = istream.get_headers();
    passed = context.are_equal(headers.size(), (std::size_t)3, 0x10b6d, "%zu") && passed;
    passed = context.are_equal(headers["Content-Type"].c_str(), "application/json; charset=utf-8", 0x10b6e) && passed;
    passed = context.are_equal(headers["Access-Control-Expose-Headers"].c_str(), "X-Content-Type-Options,Cache-Control,Pragma,ContextId,Content-Length,Connection,MS-CV,Date", 0x10b6f) && passed;
    passed = context.are_equal(headers["Content-Length"].c_str(), "205", 0x10b70) && passed;
    passed = verify_stream_good(context, istream, std::strlen("Content-Type:application/json; charset=utf-8\r\nAccess-Control-Expose-Headers:X-Content-Type-Options,Cache-Control,Pragma,ContextId,Content-Length,Connection,MS-CV,Date\r\nContent-Length:205\r\n"), 0x10b71) && passed;

    std::string body = istream.get_body(200);
    passed = context.are_equal(body.c_str(), "{\"next\":\"https://centralus.notifications.teams.microsoft.com/users/8:orgid:66c7bbfd-e15c-4257-ad6b-867c195de604/endpoints/0bf687c1-c864-45df-891a-90f548dee242/events/poll?cursor=1590886559&epfs=srt&sc", 0x10b72) && passed;
    passed = verify_stream_good(context, istream, 200, 0x10b73) && passed;

    body = istream.get_body(20);
    passed = context.are_equal(body.c_str(), "a=2\"}\r\n\r\n", 0x10b74) && passed;
    passed = verify_stream_eof(context, istream, 9, 0x10b75) && passed;

    return passed;
}


// --------------------------------------------------------------


bool http_response_reader(test_context& context, bool use_headers, bool use_body);


bool test_http_response_reader_none(test_context& context) {
    return http_response_reader(context, false, false);
}


bool test_http_response_reader_headers(test_context& context) {
    return http_response_reader(context, true, false);
}


bool test_http_response_reader_body(test_context& context) {
    return http_response_reader(context, false, true);
}


bool test_http_response_reader_headers_body(test_context& context) {
    return http_response_reader(context, true, true);
}


bool http_response_reader(test_context& context, bool use_headers, bool use_body) {
    char content_body[] =
        "{\r\n"
        "  \"foo\": 42,\r\n"
        "  \"bar\": \"qwerty\"\r\n"
        "}";

    std::string content = "HTTP/1.1 200 OK\r\n";

    if (use_headers) {
        content.append(
            "Access-Control-Expose-Headers: X-Content-Type-Options,Cache-Control,Pragma,ContextId,Content-Length,Connection,MS-CV,Date\r\n"
            "Content-Length: 205\r\n"
            "Content-Type: application/json; charset=utf-8\r\n");
    }
    content.append("\r\n");

    if (use_body) {
        content.append(content_body);
    }

    std::stringbuf sb(content, std::ios_base::in);

    abc::net::http::response_reader reader(&sb, context.log());

    bool passed = true;

    abc::net::http::response response = reader.get_response();
    std::string body = reader.get_body(abc::size::k1);

    passed = context.are_equal(response.protocol.c_str(), "HTTP/1.1", 0x10b76) && passed;
    passed = context.are_equal((unsigned)response.status_code, 200U, 0x10b77, "%u") && passed;
    passed = context.are_equal(response.reason_phrase.c_str(), "OK", 0x10b78) && passed;

    if (use_headers) {
        passed = context.are_equal(response.headers.size(), (std::size_t)3, 0x10b79, "%zu") && passed;
        passed = context.are_equal(response.headers["Access-Control-Expose-Headers"].c_str(), "X-Content-Type-Options,Cache-Control,Pragma,ContextId,Content-Length,Connection,MS-CV,Date", 0x10b7a) && passed;
        passed = context.are_equal(response.headers["Content-Length"].c_str(), "205", 0x10b7b) && passed;
        passed = context.are_equal(response.headers["Content-Type"].c_str(), "application/json; charset=utf-8", 0x10b7c) && passed;
    }
    else {
        passed = context.are_equal(response.headers.size(), (std::size_t)0, 0x10b7d, "%zu") && passed;
    }

    if (use_body) {
        passed = context.are_equal(body.c_str(), content_body, 0x10b7e) && passed;
    }
    else {
        passed = context.are_equal(body.c_str(), "", 0x10b7f) && passed;
    }

    return passed;
}


// --------------------------------------------------------------


bool test_http_response_ostream_bodytext(test_context& context) {
    const char expected[] =
        "HTTP/1.1 200 OK\r\n"
        "List: foo bar foobar\r\n"
        "Simple: simple\r\n"
        "\r\n"
        "First line\r\n"
        "  Second line\r\n"
        "\tThird line";

    std::stringbuf sb(std::ios_base::out);

    abc::net::http::response_ostream ostream(&sb, context.log());

    bool passed = true;

    ostream.put_protocol("HTTP/1.1");
    passed = verify_stream_good(context, ostream, 0x100d2) && passed;

    ostream.put_status_code(200);
    passed = verify_stream_good(context, ostream, 0x100d3) && passed;

    ostream.put_reason_phrase("OK");
    passed = verify_stream_good(context, ostream, 0x100d4) && passed;

    abc::net::http::headers headers = {
        { "List", "foo bar foobar" },
        { "Simple", "simple" },
    };

    ostream.put_headers(headers);
    passed = verify_stream_good(context, ostream, 0x100d5) && passed;

    ostream.put_body("First line\r\n");
    passed = verify_stream_good(context, ostream, 0x100d9) && passed;

    ostream.put_body("  Second line\r\n");
    passed = verify_stream_good(context, ostream, 0x100da) && passed;

    ostream.put_body("\tThird line\r\n");
    passed = verify_stream_good(context, ostream, 0x100db) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x100dc) && passed;

    return passed;
}


bool test_http_response_ostream_bodybinary(test_context& context) {
    const char expected[] =
        "HTTP/1.1 789 Something went wrong \r\n"
        "Multi-Line-List: aaa bbbb ccc ddd\r\n"
        "\r\n"
        "\x03\x07\x13\x16\x19"
        "\x20\x24\x35\x46\x57\x71\x7f"
        "\x80\x89\xa5\xb6\xc7\xff";

    std::stringbuf sb(std::ios_base::out);

    abc::net::http::response_ostream ostream(&sb, context.log());

    bool passed = true;

    ostream.put_protocol("HTTP/1.1");
    passed = verify_stream_good(context, ostream, 0x100dd) && passed;

    ostream.put_status_code(789);
    passed = verify_stream_good(context, ostream, 0x100de) && passed;

    ostream.put_reason_phrase("Something went wrong ");
    passed = verify_stream_good(context, ostream, 0x100df) && passed;

    abc::net::http::headers headers = {
        { "Multi-Line-List", "aaa bbbb ccc ddd" },
    };

    ostream.put_headers(headers);
    passed = verify_stream_good(context, ostream, 0x100d5) && passed;

    ostream.put_body("\x03\x07\x13\x16\x19");
    passed = verify_stream_good(context, ostream, 0x100e2) && passed;

    ostream.put_body("\x20\x24\x35\x46\x57\x71\x7f");
    passed = verify_stream_good(context, ostream, 0x100e3) && passed;

    ostream.put_body("\x80\x89\xa5\xb6\xc7\xff");
    passed = verify_stream_good(context, ostream, 0x100e4) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x100e5) && passed;

    return passed;
}


bool test_http_response_ostream_bodynone(test_context& context) {
    const char expected[] =
        "HTTP/1.1 200 OK\r\n"
        "List: foo bar foobar\r\n"
        "Simple: simple\r\n"
        "\r\n";

    std::stringbuf sb(std::ios_base::out);

    abc::net::http::response_ostream ostream(&sb, context.log());

    bool passed = true;

    abc::net::http::headers headers = {
        { "List", "foo bar foobar" },
        { "Simple", "simple" },
    };

    ostream.put_protocol("HTTP/1.1");
    ostream.put_status_code(200);
    ostream.put_reason_phrase("OK");
    ostream.put_headers(headers);
    passed = verify_stream_good(context, ostream, 0x10b80) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10b81) && passed;

    return passed;
}


// --------------------------------------------------------------


bool http_response_writer(test_context& context, bool use_headers, bool use_body);


bool test_http_response_writer_none(test_context& context) {
    return http_response_writer(context, false, false);
}


bool test_http_response_writer_headers(test_context& context) {
    return http_response_writer(context, true, false);
}


bool test_http_response_writer_body(test_context& context) {
    return http_response_writer(context, false, true);
}


bool test_http_response_writer_headers_body(test_context& context) {
    return http_response_writer(context, true, true);
}


bool http_response_writer(test_context& context, bool use_headers, bool use_body) {
    abc::net::http::response response;

    response.status_code = 200;
    response.reason_phrase = "OK";

    std::string expected = "HTTP/1.1 200 OK\r\n";

    if (use_headers) {
        response.headers["List"] = "foo bar foobar";
        response.headers["Simple"] = "simple";

        expected.append(
            "List: foo bar foobar\r\n"
            "Simple: simple\r\n");
    }
    expected.append("\r\n");

    std::stringbuf sb(std::ios_base::out);

    abc::net::http::response_writer writer(&sb, context.log());

    writer.put_response(response);

    if (use_body) {
        const char expected_body[] =
            "{\r\n"
            "  \"foo\": 42,\r\n"
            "  \"bar\": \"qwerty\"\r\n"
            "}";

        writer.put_body(expected_body);

        expected.append(expected_body);
    }

    bool passed = true;

    passed = context.are_equal(sb.str().c_str(), expected.c_str(), 0x10b82) && passed;

    return passed;
}


bool http_response_writer_event_message(test_context& context, const abc::net::http::event_message& event_message, const char* expected, abc::diag::tag_t tag);
bool http_response_writer_event(test_context& context, const abc::net::http::event& event, const char* expected, abc::diag::tag_t tag);


bool test_http_response_writer_event_messages(test_context& context) {
    bool passed = true;

    {
        abc::net::http::comment_event_message event_message("This is a comment.");
        const char* expected = ": This is a comment.\r\n";

        passed = http_response_writer_event_message(context, event_message, expected, __TAG__) && passed;
    }

    {
        abc::net::http::type_event_message event_message("test");
        const char* expected = "event: test\r\n";

        passed = http_response_writer_event_message(context, event_message, expected, __TAG__) && passed;
    }

    {
        abc::net::http::data_event_message event_message("This is some data.");
        const char* expected = "data: This is some data.\r\n";

        passed = http_response_writer_event_message(context, event_message, expected, __TAG__) && passed;
    }

    {
        abc::net::http::id_event_message event_message("ABC123");
        const char* expected = "id: ABC123\r\n";

        passed = http_response_writer_event_message(context, event_message, expected, __TAG__) && passed;
    }

    {
        abc::net::http::retry_event_message event_message(456);
        const char* expected = "retry: 456\r\n";

        passed = http_response_writer_event_message(context, event_message, expected, __TAG__) && passed;
    }

    return passed;
}


bool test_http_response_writer_events_1(test_context& context) {
    bool passed = true;

    {
        abc::net::http::event event({
            abc::net::http::comment_event_message("This is a comment."),
        });
        const char* expected = ": This is a comment.\r\n\r\n";

        passed = http_response_writer_event(context, event, expected, __TAG__) && passed;
    }

    {
        abc::net::http::event event({
            abc::net::http::type_event_message("test"),
        });
        const char* expected = "event: test\r\n\r\n";

        passed = http_response_writer_event(context, event, expected, __TAG__) && passed;
    }

    {
        abc::net::http::event event({
            abc::net::http::data_event_message("This is some data."),
        });
        const char* expected = "data: This is some data.\r\n\r\n";

        passed = http_response_writer_event(context, event, expected, __TAG__) && passed;
    }

    {
        abc::net::http::event event({
            abc::net::http::id_event_message("ABC123"),
        });
        const char* expected = "id: ABC123\r\n\r\n";

        passed = http_response_writer_event(context, event, expected, __TAG__) && passed;
    }

    {
        abc::net::http::event event({
            abc::net::http::retry_event_message(456),
        });
        const char* expected = "retry: 456\r\n\r\n";

        passed = http_response_writer_event(context, event, expected, __TAG__) && passed;
    }

    return passed;
}


bool test_http_response_writer_events_N(test_context& context) {
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
            ": This is a comment.\r\n"
            "event: test\r\n"
            "data: This is some data.\r\n"
            "id: ABC123\r\n"
            "retry: 456\r\n"
            "\r\n";

        passed = http_response_writer_event(context, event, expected, __TAG__) && passed;
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
            ": Event 1:\r\n"
            "event: test\r\n"
            "id: 1\r\n"
            "data: Message 1.1\r\n"
            "data: Message 1.2\r\n"
            "data: Message 1.3\r\n"
            "\r\n";

        passed = http_response_writer_event(context, event, expected, __TAG__) && passed;
    }

    return passed;
}


bool http_response_writer_event_message(test_context& context, const abc::net::http::event_message& event_message, const char* expected, abc::diag::tag_t tag) {
    bool passed = true;

    std::stringbuf sb(std::ios_base::out);

    abc::net::http::response_writer writer(&sb, context.log());

    writer.put_event_message(event_message);

    passed = context.are_equal(sb.str().c_str(), expected, tag) && passed;

    return passed;
}


bool http_response_writer_event(test_context& context, const abc::net::http::event& event, const char* expected, abc::diag::tag_t tag) {
    bool passed = true;

    std::stringbuf sb(std::ios_base::out);

    abc::net::http::response_writer writer(&sb, context.log());

    writer.put_event(event);

    passed = context.are_equal(sb.str().c_str(), expected, tag) && passed;

    return passed;
}


// --------------------------------------------------------------


bool test_http_request_istream_move(test_context& context) {
    char content[] =
        "GET https://en.cppreference.com/w/cpp/io/basic_streambuf HTTP/1.1\r\n"
        "\r\n";

    abc::stream::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::net::http::request_istream istream1(&sb, context.log());

    bool passed = true;

    std::string method = istream1.get_method();
    passed = context.are_equal(method.c_str(), "GET", 0x10b83) && passed;
    passed = verify_stream_good(context, istream1, std::strlen("GET"), 0x10b84) && passed;

    abc::net::http::request_istream istream2(std::move(istream1));

    abc::net::http::resource resource = istream2.get_resource();
    passed = context.are_equal(resource.path.c_str(), "https://en.cppreference.com/w/cpp/io/basic_streambuf", 0x10b85) && passed;
    passed = context.are_equal(resource.query.size(), (std::size_t)0, 0x10b86, "%zu") && passed;
    passed = verify_stream_good(context, istream2, std::strlen("https://en.cppreference.com/w/cpp/io/basic_streambuf"), 0x10b87) && passed;

    return passed;
}


bool test_http_request_ostream_move(test_context& context) {
    const char expected[] =
        "POST http://a.com/b?c=d HTTP/1.1\r\n";

    std::stringbuf sb(std::ios_base::out);

    abc::net::http::request_ostream ostream1(&sb, context.log());

    bool passed = true;

    ostream1.put_method("POST");
    passed = verify_stream_good(context, ostream1, 0x10717) && passed;

    abc::net::http::request_ostream ostream2(std::move(ostream1));

    ostream2.put_resource("http://a.com/b?c=d");
    passed = verify_stream_good(context, ostream2, 0x10718) && passed;

    ostream2.put_protocol("HTTP/1.1");
    passed = verify_stream_good(context, ostream2, 0x10719) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x1071a) && passed;

    return passed;
}


bool test_http_response_istream_move(test_context& context) {
    char content[] =
        "HTTP/1.1 302\r\n"
        "\r\n";

    abc::stream::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    abc::net::http::response_istream istream1(&sb, context.log());

    bool passed = true;

    std::string protocol = istream1.get_protocol();
    passed = context.are_equal(protocol.c_str(), "HTTP/1.1", 0x10b88) && passed;
    passed = verify_stream_good(context, istream1, std::strlen("HTTP/1.1"), 0x10b89) && passed;

    abc::net::http::response_istream istream2(std::move(istream1));

    abc::net::http::status_code_t status = istream2.get_status_code();
    passed = context.are_equal(status, (abc::net::http::status_code_t)302, 0x10b8a, "%u") && passed;
    passed = verify_stream_good(context, istream2, std::strlen("302"), 0x10b8b) && passed;

    return passed;
}


bool test_http_response_ostream_move(test_context& context) {
    const char expected[] =
        "HTTP/1.1 200 OK\r\n";

    std::stringbuf sb(std::ios_base::out);

    abc::net::http::response_ostream ostream1(&sb, context.log());

    bool passed = true;

    ostream1.put_protocol("HTTP/1.1");
    passed = verify_stream_good(context, ostream1, 0x1071d) && passed;

    abc::net::http::response_ostream ostream2(std::move(ostream1));

    ostream2.put_status_code(200);
    passed = verify_stream_good(context, ostream2, 0x1071e) && passed;

    ostream2.put_reason_phrase("OK");
    passed = verify_stream_good(context, ostream2, 0x1071f) && passed;

    passed = context.are_equal(sb.str().c_str(), expected, std::strlen(expected), 0x10720) && passed;

    return passed;
}


template <typename HttpRequestReader>
bool http_request_reader_move(test_context& context) {
    char content[] =
        "POST https://en.cppreference.com/w/cpp/io/basic_streambuf HTTP/1.1\r\n"
        "List: foo bar foobar\r\n"
        "Simple: simple\r\n"
        "\r\n"
        "{\r\n"
        "  \"foo\": 42,\r\n"
        "  \"bar\": \"qwerty\"\r\n"
        "}";

    char content_body[] =
        "{\r\n"
        "  \"foo\": 42,\r\n"
        "  \"bar\": \"qwerty\"\r\n"
        "}";

    abc::stream::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    HttpRequestReader reader1(&sb, context.log());

    bool passed = true;

    abc::net::http::request request = reader1.get_request();
    passed = context.are_equal(request.method.c_str(), "POST", 0x10b8c) && passed;
    passed = context.are_equal(request.resource.path.c_str(), "https://en.cppreference.com/w/cpp/io/basic_streambuf", 0x10b8d) && passed;
    passed = context.are_equal(request.resource.query.size(), (std::size_t)0, 0x10b8e, "%zu") && passed;
    passed = context.are_equal(request.headers.size(), (std::size_t)2, 0x10b8f, "%zu") && passed;
    passed = context.are_equal(request.headers["List"].c_str(), "foo bar foobar", 0x10b90) && passed;
    passed = context.are_equal(request.headers["Simple"].c_str(), "simple", 0x10b91) && passed;

    HttpRequestReader reader2(std::move(reader1));

    std::string body = reader2.get_body(abc::size::k1);
    passed = context.are_equal(body.c_str(), content_body, 0x10b92) && passed;

    return passed;
}


template <typename HttpRequestWriter>
bool http_request_writer_move(test_context& context) {
    const char expected[] = 
        "POST http://a.com/bbb/cc/d?x=111&yy=22&zzz=3 HTTP/1.1\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Host: en.cppreference.com\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0\r\n"
        "\r\n"
        "{\r\n"
        "  \"foo\": 42,\r\n"
        "  \"bar\": \"qwerty\"\r\n"
        "}";

    const char body[] =
        "{\r\n"
        "  \"foo\": 42,\r\n"
        "  \"bar\": \"qwerty\"\r\n"
        "}";

    abc::net::http::request request;

    request.method = "POST";
    request.resource.path = "http://a.com/bbb/cc/d";
    request.resource.query["x"] = "111";
    request.resource.query["yy"] = "22";
    request.resource.query["zzz"] = "3";
    request.headers["Host"] = "en.cppreference.com";
    request.headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0";
    request.headers["Accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8";
    request.headers["Accept-Language"] = "en-US,en;q=0.5";

    std::stringbuf sb(std::ios_base::out);

    HttpRequestWriter writer1(&sb, context.log());

    writer1.put_request(request);

    HttpRequestWriter writer2(std::move(writer1));

    writer2.put_body(body);

    bool passed = true;

    passed = context.are_equal(sb.str().c_str(), expected, 0x10b93) && passed;

    return passed;
}


template <typename HttpResponseReader>
bool http_response_reader_move(test_context& context) {
    char content[] =
        "HTTP/1.1 911 What's your emergency?\r\n"
        "List: foo bar foobar\r\n"
        "Simple: simple\r\n"
        "\r\n"
        "{\r\n"
        "  \"foo\": 42,\r\n"
        "  \"bar\": \"qwerty\"\r\n"
        "}";

    char content_body[] =
        "{\r\n"
        "  \"foo\": 42,\r\n"
        "  \"bar\": \"qwerty\"\r\n"
        "}";

    abc::stream::buffer_streambuf sb(content, 0, std::strlen(content), nullptr, 0, 0);

    HttpResponseReader reader1(&sb, context.log());

    bool passed = true;

    abc::net::http::response response = reader1.get_response();
    passed = context.are_equal(response.status_code, (abc::net::http::status_code_t)911, 0x10b94, "%u") && passed;
    passed = context.are_equal(response.reason_phrase.c_str(), "What's your emergency?", 0x10b95) && passed;
    passed = context.are_equal(response.headers.size(), (std::size_t)2, 0x10b96, "%zu") && passed;
    passed = context.are_equal(response.headers["List"].c_str(), "foo bar foobar", 0x10b97) && passed;
    passed = context.are_equal(response.headers["Simple"].c_str(), "simple", 0x10b98) && passed;

    HttpResponseReader reader2(std::move(reader1));

    std::string body = reader2.get_body(abc::size::k1);
    passed = context.are_equal(body.c_str(), content_body, 0x10b99) && passed;

    return passed;
}


template <typename HttpResponseWriter>
bool http_response_writer_move(test_context& context) {
    const char expected[] = 
        "HTTP/1.1 911 What's your emergency?\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Host: en.cppreference.com\r\n"
        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0\r\n"
        "\r\n"
        "{\r\n"
        "  \"foo\": 42,\r\n"
        "  \"bar\": \"qwerty\"\r\n"
        "}";

    const char body[] =
        "{\r\n"
        "  \"foo\": 42,\r\n"
        "  \"bar\": \"qwerty\"\r\n"
        "}";

    abc::net::http::response response;

    response.status_code = 911;
    response.reason_phrase = "What's your emergency?";
    response.headers["Host"] = "en.cppreference.com";
    response.headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0) Gecko/20100101 Firefox/76.0";
    response.headers["Accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8";
    response.headers["Accept-Language"] = "en-US,en;q=0.5";

    std::stringbuf sb(std::ios_base::out);

    HttpResponseWriter writer1(&sb, context.log());

    writer1.put_response(response);

    HttpResponseWriter writer2(std::move(writer1));

    writer2.put_body(body);

    bool passed = true;

    passed = context.are_equal(sb.str().c_str(), expected, 0x10b9a) && passed;

    return passed;
}


bool test_http_request_reader_move(test_context& context) {
    return http_request_reader_move<abc::net::http::request_reader>(context);
}


bool test_http_request_writer_move(test_context& context) {
    return http_request_writer_move<abc::net::http::request_writer>(context);
}


bool test_http_response_reader_move(test_context& context) {
    return http_response_reader_move<abc::net::http::response_reader>(context);
}


bool test_http_response_writer_move(test_context& context) {
    return http_response_writer_move<abc::net::http::response_writer>(context);
}


bool test_http_client_move(test_context& context) {
    bool passed = true;

    passed = http_request_writer_move<abc::net::http::client>(context) && passed;
    passed = http_response_reader_move<abc::net::http::client>(context) && passed;

    return passed;
}


bool test_http_server_move(test_context& context) {
    bool passed = true;

    passed = http_request_reader_move<abc::net::http::server>(context) && passed;
    passed = http_response_writer_move<abc::net::http::server>(context) && passed;

    return passed;
}
