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


#include "../../src/root/ascii.h"
#include "../../src/diag/log.h"
#include "../../src/net/json.h"
#include "../../src/net/http.h"
#include "../../src/net/endpoint.h"



class equations_endpoint
    : public abc::net::http::endpoint {

    using base = abc::net::http::endpoint;

public:
    equations_endpoint(abc::net::http::endpoint_config&& config, abc::diag::log_ostream* log);

protected:
    virtual std::unique_ptr<abc::net::tcp_server_socket> create_server_socket() override;
    virtual void process_rest_request(abc::net::http::server& http, const abc::net::http::request& request) override;
};


// --------------------------------------------------------------


inline equations_endpoint::equations_endpoint(abc::net::http::endpoint_config&& config, abc::diag::log_ostream* log)
    : base("equations_endpoint", std::move(config), log) {
}


inline std::unique_ptr<abc::net::tcp_server_socket> equations_endpoint::create_server_socket() {
    return std::unique_ptr<abc::net::tcp_server_socket>(new abc::net::tcp_server_socket(abc::net::socket::family::ipv4, base::log()));
}


inline void equations_endpoint::process_rest_request(abc::net::http::server& http, const abc::net::http::request& request) {
    constexpr const char* suborigin = "process_rest_request()";
    base::put_any(suborigin, abc::diag::severity::callstack, 0x102f1, "Begin:");

    // Support a graceful shutdown.
    if (abc::ascii::are_equal_i(request.method.c_str(), abc::net::http::method::POST) && abc::ascii::are_equal_i(request.resource.path.c_str(), "/shutdown")) {
        base::set_shutdown_requested();

        base::send_simple_response(http, abc::net::http::status_code::OK, abc::net::http::reason_phrase::OK, abc::net::http::content_type::text, "Server is shuting down...", 0x102ce);

        base::put_any(suborigin, abc::diag::severity::callstack, 0x107b7, "Return: 200");
        return;
    }

    // If the resource is not /problem, return 404.
    if (!abc::ascii::are_equal_i(request.resource.path.c_str(), "/problem")) {
        base::send_simple_response(http, abc::net::http::status_code::Not_Found, abc::net::http::reason_phrase::Not_Found, abc::net::http::content_type::text, "The requested resource was not found.", 0x102cf);

        base::put_any(suborigin, abc::diag::severity::callstack, 0x107b8, "Return: 404");
        return;
    }

    // If the method is not POST, return 405.
    if (!abc::ascii::are_equal_i(request.method.c_str(), abc::net::http::method::POST)) {
        base::send_simple_response(http, abc::net::http::status_code::Method_Not_Allowed, abc::net::http::reason_phrase::Method_Not_Allowed, abc::net::http::content_type::text, "POST is the only supported method for resource '/problem'.", 0x102d0);

        base::put_any(suborigin, abc::diag::severity::callstack, 0x107b9, "Return: 405");
        return;
    }

    // Require header Content-Type: application/json
    abc::net::http::headers::const_iterator content_type_itr = request.headers.find(abc::net::http::header::Content_Type);
    if (content_type_itr == request.headers.cend()) {
        base::send_simple_response(http, abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "The Content-Type header was not supplied.", 0x107ba);

        base::put_any(suborigin, abc::diag::severity::callstack, 0x107bb, "Return: 400 (No Content-Type)");
        return;
    }
    if (!abc::ascii::are_equal_i(content_type_itr->second.c_str(), abc::net::http::content_type::json)) {
        base::send_simple_response(http, abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "The Content-Type header was not supplied.", 0x107bc);

        base::put_any(suborigin, abc::diag::severity::callstack, 0x107bd, "Return: 400 (Wrong Content-Type)");
        return;
    }

    std::streambuf* request_body_sb = static_cast<const abc::net::http::request_reader&>(http).rdbuf();
    abc::net::json::reader json_reader(request_body_sb, base::log());
    abc::net::json::value input_value = json_reader.get_value();

    if (input_value.type() != abc::net::json::value_type::object
        || input_value.object().size() != 2
        || input_value.object().find("a") == input_value.object().cend()
            || input_value.object()["a"].type() != abc::net::json::value_type::array
            || input_value.object()["a"].array().size() != 2
            || input_value.object()["a"].array()[0].type() != abc::net::json::value_type::array
            || input_value.object()["a"].array()[0].array().size() != 2
            || input_value.object()["a"].array()[0].array()[0].type() != abc::net::json::value_type::number
            || input_value.object()["a"].array()[0].array()[1].type() != abc::net::json::value_type::number
            || input_value.object()["a"].array()[1].type() != abc::net::json::value_type::array
            || input_value.object()["a"].array()[1].array().size() != 2
            || input_value.object()["a"].array()[1].array()[0].type() != abc::net::json::value_type::number
            || input_value.object()["a"].array()[1].array()[1].type() != abc::net::json::value_type::number
        || input_value.object().find("b") == input_value.object().cend()
            || input_value.object()["b"].type() != abc::net::json::value_type::array
            || input_value.object()["b"].array().size() != 2
            || input_value.object()["b"].array()[0].type() != abc::net::json::value_type::number
            || input_value.object()["b"].array()[1].type() != abc::net::json::value_type::number) {
        const char* const invalid_json = "An invalid JSON payload was supplied. Must be {\"a\": [ [1, 2], [3, 4] ], \"b\": [5, 6] }.";
        base::send_simple_response(http, abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, invalid_json, 0x107be);

        base::put_any(suborigin, abc::diag::severity::callstack, 0x107bf, "Return: 400 (Wrong JSON payload)");
        return;
    }

    // Here's where we store the parsed JSON input.
    double a[2][2] = {
        {
            input_value.object()["a"].array()[0].array()[0].number(),
            input_value.object()["a"].array()[0].array()[1].number(),
        },
        {
            input_value.object()["a"].array()[1].array()[0].number(),
            input_value.object()["a"].array()[1].array()[1].number(),
        },
    };

    double b[2] = {
        input_value.object()["b"].array()[0].number(),
        input_value.object()["b"].array()[1].number(),
    };

    // Now, let's solve the system.
    double det = (a[0][0] * a[1][1]) - (a[0][1] * a[1][0]);
    double det_x = (b[0] * a[1][1]) - (a[0][1] * b[1]);
    double det_y = (a[0][0] * b[1]) - (b[0] * a[1][0]);

    double status = -1;
    double x = 0;
    double y = 0;

    if (det != 0) {
        // 1 solution
        status = 1;
        x = det_x / det;
        y = det_y / det;
    }
    else if (det_x != 0 || det_y != 0) {
        // 0 solutions
        status = 0;
    }
    else {
        // inf. solutions
        status = 2;
    }

    abc::net::json::value output_value = 
        abc::net::json::literal::object {
            { "status", status },
            { "x", x },
            { "y", y },
        };

    // Write the JSON to a char buffer, so we can calculate the Content-Length before we start sending the body.
    char body[abc::size::k1 + 1] { };
    abc::stream::buffer_streambuf response_body_sb(nullptr, 0, 0, body, 0, sizeof(body));
    abc::net::json::writer json_writer(&response_body_sb, base::log());
    json_writer.put_value(output_value);

    char content_length[abc::size::_32 + 1];
    std::snprintf(content_length, sizeof(content_length), "%zu", std::strlen(body));

    // Send the http response
    base::put_any(suborigin, abc::diag::severity::optional, 0x107c0, "Sending response 200");

    abc::net::http::response response;
    response.protocol = abc::net::http::protocol::HTTP_11;
    response.status_code = abc::net::http::status_code::OK;
    response.reason_phrase = abc::net::http::reason_phrase::OK;
    response.headers = abc::net::http::headers {
        { abc::net::http::header::Content_Type,   abc::net::http::content_type::json },
        { abc::net::http::header::Content_Length, content_length },
    };

    http.put_response(response);
    http.put_body(body);

    base::put_any(suborigin, abc::diag::severity::callstack, 0x102d9, "End:");
}
