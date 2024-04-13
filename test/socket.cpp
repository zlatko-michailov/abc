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


#include <cctype>
#include <thread>

#include "inc/http.h"
#include "inc/json.h"
#include "inc/socket.h"


static constexpr const char origin[] = "";
static constexpr const char cert_filename[] = "cert.pem";
static constexpr const char pkey_filename[] = "pkey.pem";
static constexpr const char pkey_password[] = "server";
static constexpr bool verify_client = false;
static constexpr bool verify_server = false;


std::string make_filepath(test_context& context, const char* process_path, const char* filename) {
    constexpr const char* suborigin = "make_filepath";

    context.log()->put_any(origin, suborigin, abc::diag::severity::optional, 0x1079d, "process_path='%s'", process_path);

    std::string filepath = abc::parent_path(process_path);
    filepath.append("/");
    filepath.append(filename);

    context.log()->put_any(origin, suborigin, abc::diag::severity::optional, 0x1079f, "filepath='%s'", filepath.c_str());

    return filepath;
}


bool test_udp_socket(test_context& context) {
    constexpr const char* suborigin = "test_udp_socket";
    constexpr const char* server_port = "31234";
    constexpr const char* request_content = "Some request content.";
    constexpr const char* response_content = "The corresponding response content.";
    bool passed = true;

    abc::net::udp_socket server(abc::net::socket::family::ipv4, context.log());
    server.bind(server_port);

    std::thread client_thread(
        [&passed, &context] () {
        try {
            abc::net::udp_socket client(abc::net::socket::family::ipv4, context.log());
            client.connect("localhost", server_port);

            std::uint16_t content_length = std::strlen(request_content);
            client.send(&content_length, sizeof(std::uint16_t));
            client.send(request_content, content_length);

            client.receive(&content_length, sizeof(std::uint16_t));

            char content[abc::size::k1];
            client.receive(content, content_length);
            content[content_length] = '\0';

            passed = context.are_equal(content, response_content, 0x10028) && passed;
        }
        catch (const std::exception& ex) {
            context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x10029, "client: EXCEPTION: %s", ex.what());
            throw;
        }
    });

    abc::net::socket::address client_address;
    std::uint16_t content_length;
    server.receive(&content_length, sizeof(std::uint16_t), &client_address);

    char content[abc::size::k1];
    server.receive(content, content_length);
    content[content_length] = '\0';

    passed = context.are_equal(content, request_content, 0x1002a) && passed;

    server.connect(client_address);

    content_length = std::strlen(response_content);
    server.send(&content_length, sizeof(std::uint16_t));

    server.send(response_content, content_length);

    client_thread.join();
    return passed;
}


// --------------------------------------------------------------


template <typename ServerSocket, typename ClientSocket>
bool tcp_socket(test_context& context, ServerSocket& server, ClientSocket& client, const char* server_port) {
    constexpr const char* suborigin = "tcp_socket";
    constexpr const char* request_content = "Some request content.";
    constexpr const char* response_content = "The corresponding response content.";
    bool passed = true;

    server.bind(server_port);
    server.listen(5);

    std::thread client_thread(
        [&client, &passed, &context, server_port] () {
        try {
            client.connect("localhost", server_port);

            std::uint16_t content_length = std::strlen(request_content);
            client.send(&content_length, sizeof(std::uint16_t));
            client.send(request_content, content_length);

            client.receive(&content_length, sizeof(std::uint16_t));

            char content[abc::size::k1];
            client.receive(content, content_length);
            content[content_length] = '\0';

            passed = context.are_equal(content, response_content, 0x1002b) && passed;
        }
        catch (const std::exception& ex) {
            context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x1002c, "client: EXCEPTION: %s", ex.what());
            throw;
        }
    });

    ClientSocket connection = server.accept();

    std::uint16_t content_length;
    connection.receive(&content_length, sizeof(std::uint16_t));

    char content[abc::size::k1];
    connection.receive(content, content_length);
    content[content_length] = '\0';

    passed = context.are_equal(content, request_content, 0x1002d) && passed;

    content_length = std::strlen(response_content);
    connection.send(&content_length, sizeof(std::uint16_t));

    connection.send(response_content, content_length);

    client_thread.join();
    return passed;
}


bool test_tcp_socket(test_context& context) {
    abc::net::tcp_server_socket server(abc::net::socket::family::ipv4, context.log());
    abc::net::tcp_client_socket client(abc::net::socket::family::ipv4, context.log());

    return tcp_socket(context, server, client, "31001");
}


bool test_openssl_tcp_socket(test_context& context) {
    bool passed = true;

#ifdef __ABC__OPENSSL
    std::string cert_path = make_filepath(context, context.process_path, cert_filename);
    std::string pkey_path = make_filepath(context, context.process_path, pkey_filename);

    abc::net::openssl::tcp_server_socket server(cert_path.c_str(), pkey_path.c_str(), pkey_password, verify_client, abc::net::socket::family::ipv4, context.log());
    abc::net::openssl::tcp_client_socket client(verify_server, abc::net::socket::family::ipv4, context.log());

    passed = tcp_socket(context, server, client, "31002") && passed;
#else
    passed = context.are_equal(0, 0, 0x107a0) && passed;
#endif

    return passed;
}


// --------------------------------------------------------------


template <typename ServerSocket, typename ClientSocket>
bool tcp_socket_stream_move(test_context& context, ServerSocket& server, ClientSocket& client1, const char* server_port) {
    constexpr const char* suborigin = "tcp_socket_stream_move";
    constexpr const char* request_content = "Some request line.";
    constexpr const char* response_content = "The corresponding response line.";
    bool passed = true;

    server.bind(server_port);
    server.listen(5);

    std::thread client_thread(
        [&passed, &context, &client1, server_port] () {
        try {
            client1.connect("localhost", server_port);

            ClientSocket client2(std::move(client1));

            abc::net::socket_streambuf<ClientSocket*> sb1(&client2, context.log());
            std::ostream client_out(&sb1);

            client_out << request_content << "\n";
            client_out.flush();

            abc::net::socket_streambuf<ClientSocket*> sb2(std::move(sb1));
            std::istream client_in(&sb2);

            char content[abc::size::k1];
            client_in.getline(content, sizeof(content) - 1);
            passed = context.are_equal(content, response_content, 0x10037) && passed;
        }
        catch (const std::exception& ex) {
            context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x10038, "client: EXCEPTION: %s", ex.what());
            throw;
        }
    });

    ClientSocket connection = server.accept();

    abc::net::socket_streambuf<ClientSocket*> sb(&connection, context.log());
    std::istream connection_in(&sb);
    std::ostream connection_out(&sb);

    char content[abc::size::k1];
    connection_in.getline(content, sizeof(content) - 1);
    passed = context.are_equal(content, request_content, 0x10039) && passed;

    connection_out << response_content << "\n";
    connection_out.flush();

    client_thread.join();
    return passed;
}


bool test_tcp_socket_stream_move(test_context& context) {
    abc::net::tcp_server_socket server(abc::net::socket::family::ipv4, context.log());
    abc::net::tcp_client_socket client(abc::net::socket::family::ipv4, context.log());

    return tcp_socket_stream_move(context, server, client, "31003");
}


bool test_openssl_tcp_socket_stream_move(test_context& context) {
    bool passed = true;

#ifdef __ABC__OPENSSL
    std::string cert_path = make_filepath(context, context.process_path, cert_filename);
    std::string pkey_path = make_filepath(context, context.process_path, pkey_filename);

    abc::net::openssl::tcp_server_socket server(cert_path.c_str(), pkey_path.c_str(), pkey_password, verify_client, abc::net::socket::family::ipv4, context.log());
    abc::net::openssl::tcp_client_socket client(verify_server, abc::net::socket::family::ipv4, context.log());

    passed = tcp_socket_stream_move(context, server, client, "31004") && passed;
#else
    passed = context.are_equal(0, 0, 0x107a1) && passed;
#endif

    return passed;
}


// --------------------------------------------------------------


static constexpr const char request_path[] = "/scope/v1.0/api";
static constexpr const char request_header_name[] = "Request-Header-Name";
static constexpr const char request_header_value[] = "Request-Header-Value";
static constexpr const char response_header_name[] = "Response-Header-Name";
static constexpr const char response_header_value[] = "Response-Header-Value";

template <typename ClientSocket>
void http_json_stream_client(bool& passed, test_context& context, ClientSocket& client, const char* server_port) {
    constexpr const char* suborigin = "http_json_stream_client";

    try {
        client.connect("localhost", server_port);

        abc::net::socket_streambuf<ClientSocket*> sb(&client, context.log());
        abc::net::http::client http(&sb, context.log());

        // Send request
        {
            abc::net::http::request request;
            request.method = abc::net::http::method::POST;
            request.resource.path = request_path;
            request.protocol = abc::net::http::protocol::HTTP_11;
            request.headers = {
                { request_header_name, request_header_value },
            };

            http.put_request(request);

            abc::net::json::value body = 
                abc::net::json::literal::object {
                    { "param", "foo" },
                };

            abc::net::json::writer json(&sb, context.log());
            json.put_value(body);
        }

        // Receive response
        {
            abc::net::http::response response = http.get_response();

            passed = context.are_equal(response.protocol.c_str(), abc::net::http::protocol::HTTP_11, 0x100e9) && passed;
            passed = context.are_equal(response.status_code, abc::net::http::status_code::OK, 0x100ea, "%u") && passed;
            passed = context.are_equal(response.reason_phrase.c_str(), abc::net::http::reason_phrase::OK, 0x100eb) && passed;
            passed = context.are_equal(response.headers.size(), (std::size_t)1, 0x100ec, "%zu") && passed;
            passed = context.are_equal(response.headers[response_header_name].c_str(), response_header_value, 0x100ed) && passed;

            abc::net::json::reader json(&sb, context.log());
            abc::net::json::value body = json.get_value();

            passed = context.are_equal(body.type(), abc::net::json::value_type::object, 0x102a0, "%u") && passed;
            passed = context.are_equal(body.object().size(), (std::size_t)2, 0x102a1, "%zu") && passed;
            passed = context.are_equal(body.object()["n"].type(), abc::net::json::value_type::number, 0x102a2, "%u") && passed;
            passed = context.are_equal(body.object()["n"].number(), 42.0, 0x102a3, "%f") && passed;
            passed = context.are_equal(body.object()["s"].type(), abc::net::json::value_type::string, 0x102a4, "%u") && passed;
            passed = context.are_equal(body.object()["s"].string().c_str(), "bar", 0x102a5) && passed;
        }
    }
    catch (const std::exception& ex) {
        context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x100f0, "client: EXCEPTION: %s", ex.what());
        throw;
    }
}


void http_json_stream_server_request(bool& passed, test_context& context, abc::net::http::server& http) {
    constexpr const char* suborigin = "http_json_stream_server_request";

    try {
        // Receive request
        abc::net::http::request request = http.get_request();

        passed = context.are_equal(request.method.c_str(), abc::net::http::method::POST, 0x100f2) && passed;
        passed = context.are_equal(request.resource.path.c_str(), request_path, 0x100f3) && passed;
        passed = context.are_equal(request.protocol.c_str(), abc::net::http::protocol::HTTP_11, 0x100f4) && passed;
        passed = context.are_equal(request.headers.size(), (std::size_t)1, 0x100f5, "%zu") && passed;
        passed = context.are_equal(request.headers[request_header_name].c_str(), request_header_value, 0x100f6) && passed;
    }
    catch (const std::exception& ex) {
        context.log()->put_any(origin, suborigin, abc::diag::severity::important, __TAG__, "client: EXCEPTION: %s", ex.what());
        throw;
    }
}


void http_json_stream_server_body_and_response(bool& passed, test_context& context, abc::net::http::server& http) {
    constexpr const char* suborigin = "http_json_stream_server_body_and_response";

    try {
        std::streambuf* sb = static_cast<const abc::net::http::request_reader&>(http).rdbuf();

        // Receive request
        {
            abc::net::json::reader json(sb, context.log());
            abc::net::json::value body = json.get_value();

            passed = context.are_equal(body.type(), abc::net::json::value_type::object, 0x102aa, "%u") && passed;
            passed = context.are_equal(body.object().size(), (std::size_t)1, 0x102ab, "%zu") && passed;
            passed = context.are_equal(body.object()["param"].type(), abc::net::json::value_type::string, 0x102ac, "%u") && passed;
            passed = context.are_equal(body.object()["param"].string().c_str(), "foo", 0x102ae) && passed;
        }

        // Send response
        {
            abc::net::http::response response;
            response.protocol = abc::net::http::protocol::HTTP_11;
            response.status_code = abc::net::http::status_code::OK;
            response.reason_phrase = abc::net::http::reason_phrase::OK;
            response.headers = {
                { response_header_name, response_header_value },
            };

            http.put_response(response);

            abc::net::json::writer json(sb, context.log());

            abc::net::json::value body = 
                abc::net::json::literal::object {
                    { "n", 42.0 },
                    { "s", "bar" },
                };

            json.put_value(body);
        }
    }
    catch (const std::exception& ex) {
        context.log()->put_any(origin, suborigin, abc::diag::severity::important, __TAG__, "client: EXCEPTION: %s", ex.what());
        throw;
    }
}


template <typename ServerSocket, typename ClientSocket>
bool tcp_socket_http_json_stream(test_context& context, ServerSocket& server, ClientSocket& client, const char* server_port) {
    bool passed = true;

    server.bind(server_port);
    server.listen(5);

    std::thread client_thread(
        [&passed, &context, &client, server_port] () {
        http_json_stream_client(passed, context, client, server_port);
    });

    ClientSocket connection = server.accept();

    abc::net::socket_streambuf<ClientSocket*> sb(&connection, context.log());
    abc::net::http::server http(&sb, context.log());

    http_json_stream_server_request(passed, context, http);
    http_json_stream_server_body_and_response(passed, context, http);

    client_thread.join();
    return passed;
}


bool test_tcp_socket_http_json_stream(test_context& context) {
    abc::net::tcp_server_socket server(abc::net::socket::family::ipv4, context.log());
    abc::net::tcp_client_socket client(abc::net::socket::family::ipv4, context.log());

    return tcp_socket_http_json_stream(context, server, client, "31005");
}


bool test_openssl_tcp_socket_http_json_stream(test_context& context) {
    bool passed = true;

#ifdef __ABC__OPENSSL
    std::string cert_path = make_filepath(context, context.process_path, cert_filename);
    std::string pkey_path = make_filepath(context, context.process_path, pkey_filename);

    abc::net::openssl::tcp_server_socket server(cert_path.c_str(), pkey_path.c_str(), pkey_password, verify_client, abc::net::socket::family::ipv4, context.log());
    abc::net::openssl::tcp_client_socket client(verify_server, abc::net::socket::family::ipv4, context.log());

    passed = tcp_socket_http_json_stream(context, server, client, "31006") && passed;
#else
    passed = context.are_equal(0, 0, 0x107a2) && passed;
#endif

    return passed;
}


// --------------------------------------------------------------


template <typename ServerSocket, typename ClientSocket>
class test_endpoint_base
    : public abc::net::http::endpoint<ServerSocket, ClientSocket> {

    using base = abc::net::http::endpoint<ServerSocket, ClientSocket>;

protected:
    test_endpoint_base(const char* origin, bool& passed, test_context& context, abc::net::http::endpoint_config&& config, abc::diag::log_ostream* log);

protected:
    virtual void process_rest_request(abc::net::http::server& http, const abc::net::http::request& request) override;

protected:
    bool& _passed;
    test_context& _context;
};


template <typename ServerSocket, typename ClientSocket>
inline test_endpoint_base<ServerSocket, ClientSocket>::test_endpoint_base(const char* origin, bool& passed, test_context& context, abc::net::http::endpoint_config&& config, abc::diag::log_ostream* log)
    : base(origin, std::move(config), log)

    , _passed(passed)
    , _context(context) {
}


template <typename ServerSocket, typename ClientSocket>
inline void test_endpoint_base<ServerSocket, ClientSocket>::process_rest_request(abc::net::http::server& http, const abc::net::http::request& request) {
    _passed = _context.are_equal(request.method.c_str(), abc::net::http::method::POST, 0x107a3) && _passed;
    _passed = _context.are_equal(request.resource.path.c_str(), request_path, 0x107a4) && _passed;

    http_json_stream_server_body_and_response(_passed, _context, http);

    base::set_shutdown_requested();
}


// --------------------------------------------------------------


class test_http_endpoint
    : public test_endpoint_base<abc::net::tcp_server_socket, abc::net::tcp_client_socket> {

    using base = test_endpoint_base<abc::net::tcp_server_socket, abc::net::tcp_client_socket>;
    using diag_base = abc::diag::diag_ready<const char*>;

public:
    test_http_endpoint(bool& passed, test_context& context, abc::net::http::endpoint_config&& config, abc::diag::log_ostream* log);

protected:
    virtual abc::net::tcp_server_socket create_server_socket() override;
};


inline test_http_endpoint::test_http_endpoint(bool& passed, test_context& context, abc::net::http::endpoint_config&& config, abc::diag::log_ostream* log)
    : base("test_http_endpoint", passed, context, std::move(config), log) {
}


inline abc::net::tcp_server_socket test_http_endpoint::create_server_socket() {
    return abc::net::tcp_server_socket(abc::net::socket::family::ipv4, diag_base::log());
}


// --------------------------------------------------------------


#ifdef __ABC__OPENSSL
class test_https_endpoint
    : public test_endpoint_base<abc::net::openssl::tcp_server_socket, abc::net::openssl::tcp_client_socket> {

    using base = test_endpoint_base<abc::net::openssl::tcp_server_socket, abc::net::openssl::tcp_client_socket>;
    using diag_base = abc::diag::diag_ready<const char*>;

public:
    test_https_endpoint(bool verify_client, bool& passed, test_context& context, abc::net::http::endpoint_config&& config, abc::diag::log_ostream* log);

protected:
    virtual abc::net::openssl::tcp_server_socket create_server_socket() override;

protected:
    bool _verify_client;
};


inline test_https_endpoint::test_https_endpoint(bool verify_client, bool& passed, test_context& context, abc::net::http::endpoint_config&& config, abc::diag::log_ostream* log)
    : base("test_https_endpoint", passed, context, std::move(config), log)
    , _verify_client(verify_client) {
}


inline abc::net::openssl::tcp_server_socket test_https_endpoint::create_server_socket() {
    return abc::net::openssl::tcp_server_socket(base::config().cert_file_path.c_str(), base::config().pkey_file_path.c_str(), base::config().pkey_file_password.c_str(), _verify_client, abc::net::socket::family::ipv4, diag_base::log());
}
#endif


// --------------------------------------------------------------


template <typename Endpoint, typename ClientSocket>
bool endpoint_json_stream(bool& passed, test_context& context, Endpoint& endpoint, ClientSocket& client, const char* server_port) {
    std::future<void> done = endpoint.start_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread client_thread(
        [&passed, &context, &client, server_port] () {
        http_json_stream_client(passed, context, client, server_port);
    });

    client_thread.join();
    done.wait();

    return passed;
}


bool test_http_endpoint_json_stream(test_context& context) {
    bool passed = true;

    abc::net::http::endpoint_config config(
        "31007",                // port
        5,                      // listen_queue_size
        context.process_path,   // root_dir (Note: No trailing slash!)
        "/resources/"           // files_prefix
    );

    test_http_endpoint endpoint(passed, context, std::move(config), context.log());

    abc::net::tcp_client_socket client(abc::net::socket::family::ipv4, context.log());

    passed = endpoint_json_stream(passed, context, endpoint, client, config.port.c_str()) && passed;

    return passed;
}


bool test_https_endpoint_json_stream(test_context& context) {
    bool passed = true;

    std::string cert_path = make_filepath(context, context.process_path, cert_filename);
    std::string pkey_path = make_filepath(context, context.process_path, pkey_filename);

#ifdef __ABC__OPENSSL
    abc::net::http::endpoint_config config(
        "31008",              // port
        5,                    // listen_queue_size
        context.process_path, // root_dir (Note: No trailing slash!)
        "/resources/",        // files_prefix
        cert_path.c_str(),
        pkey_path.c_str(),
        pkey_password
    );

    test_https_endpoint endpoint(verify_client, passed, context, std::move(config), context.log());

    abc::net::openssl::tcp_client_socket client(verify_server, abc::net::socket::family::ipv4, context.log());

    passed = endpoint_json_stream(passed, context, endpoint, client, config.port.c_str()) && passed;
#else
    passed = context.are_equal(0, 0, 0x107a5) && passed;
#endif

    return passed;
}
