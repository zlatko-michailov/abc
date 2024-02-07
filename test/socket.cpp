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
#include <mutex>
#include <condition_variable>

#include "inc/http.h"
#include "inc/json.h"
#include "inc/socket.h"


static constexpr const char origin[] = "";
#if 0 //// TODO:
static constexpr std::size_t max_path_size = abc::size::k1;
static constexpr const char cert_filename[] = "cert.pem";
static constexpr const char pkey_filename[] = "pkey.pem";
static constexpr const char pkey_password[] = "server";
static constexpr bool verify_client = false;
static constexpr bool verify_server = false;


bool make_filepath(test_context& context, char* filepath, std::size_t filepath_size, const char* process_path, const char* filename) {
    constexpr const char* suborigin = "make_filepath";

    context.log()->put_any(origin, suborigin, abc::diag::severity::optional, 0x1079d, "process_path='%s'", process_path);

    std::size_t filename_len = std::strlen(filename); 
    const char* process_last_separator = std::strrchr(process_path, '/');
    std::size_t process_dir_len = 0;

    if (process_last_separator != nullptr) {
        process_dir_len = process_last_separator - process_path;
        std::size_t filepath_len = process_dir_len + 1 + filename_len;

        if (filepath_len >= filepath_size) {
            context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x1079e, "filepath_len=%zu >= filepath_size=%zu", filepath_len, filepath_size);

            return false;
        }

        std::strncpy(filepath, process_path, process_dir_len + 1);
    }

    std::strcpy(filepath + process_dir_len + 1, filename);
    context.log()->put_any(origin, suborigin, abc::diag::severity::optional, 0x1079f, "filepath='%s'", filepath);

    return true;
}
#endif //// TODO:


bool test_udp_socket(test_context& context) {
    constexpr const char* suborigin = "test_udp_socket";
    constexpr const char* server_port = "31234";
    constexpr const char* request_content = "Some request content.";
    constexpr const char* response_content = "The corresponding response content.";
    bool passed = true;

    abc::net::udp_socket<test_log*> server(abc::net::socket::family::ipv4, context.log());
    server.bind(server_port);

    std::thread client_thread(
        [&passed, &context] () {
        try {
            abc::net::udp_socket<test_log*> client(abc::net::socket::family::ipv4, context.log());
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
    abc::net::tcp_server_socket<test_log*> server(abc::net::socket::family::ipv4, context.log());
    abc::net::tcp_client_socket<test_log*> client(abc::net::socket::family::ipv4, context.log());

    return tcp_socket(context, server, client, "31001");
}


#if 0 //// TODO:
bool test_openssl_tcp_socket(test_context<abc::test::log>& context) {
    bool passed = true;

#ifdef __ABC__OPENSSL
    char cert_path[max_path_size];
    passed = passed && make_filepath(context, cert_path, max_path_size, context.process_path, cert_filename);

    char pkey_path[max_path_size];
    passed = passed && make_filepath(context, pkey_path, max_path_size, context.process_path, pkey_filename);

    if (passed) {
        abc::openssl_tcp_server_socket<abc::test::log> server(cert_path, pkey_path, pkey_password, verify_client, abc::socket::family::ipv4, context.log);
        abc::openssl_tcp_client_socket<abc::test::log> client(verify_server, abc::socket::family::ipv4, context.log);

        passed = passed && tcp_socket(context, server, client, "31002");
    }
#else
    passed = context.are_equal(0, 0, 0x107a0);
#endif
    return passed;
}
#endif //// TODO:


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

            abc::net::socket_streambuf<ClientSocket*, test_log*> sb1(&client2, context.log());
            std::ostream client_out(&sb1);

            client_out << request_content << "\n";
            client_out.flush();

            abc::net::socket_streambuf<ClientSocket*, test_log*> sb2(std::move(sb1));
            std::istream client_in(&sb2);

            char content[abc::size::k1];
            client_in.getline(content, sizeof(content) - 1);
            passed = context.are_equal(content, response_content, 0x10037) && passed;
        }
        catch (const std::exception& ex) {
            context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x10038, "client: EXCEPTION: %s", ex.what());
        }
    });

    ClientSocket connection = server.accept();

    abc::net::socket_streambuf<ClientSocket*, test_log*> sb(&connection, context.log());
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
    abc::net::tcp_server_socket<test_log*> server(abc::net::socket::family::ipv4, context.log());
    abc::net::tcp_client_socket<test_log*> client(abc::net::socket::family::ipv4, context.log());

    return tcp_socket_stream_move(context, server, client, "31003");
}


#if 0 //// TODO:
bool test_openssl_tcp_socket_stream_move(test_context<abc::test::log>& context) {
    bool passed = true;

#ifdef __ABC__OPENSSL
    char cert_path[max_path_size];
    passed = passed && make_filepath(context, cert_path, max_path_size, context.process_path, cert_filename);

    char pkey_path[max_path_size];
    passed = passed && make_filepath(context, pkey_path, max_path_size, context.process_path, pkey_filename);

    if (passed) {
        abc::openssl_tcp_server_socket<abc::test::log> server(cert_path, pkey_path, pkey_password, verify_client, abc::socket::family::ipv4, context.log);
        abc::openssl_tcp_client_socket<abc::test::log> client(verify_server, abc::socket::family::ipv4, context.log);

        passed = passed && tcp_socket_stream_move(context, server, client, "31004");
    }
#else
    passed = context.are_equal(0, 0, 0x107a1);
#endif
    return passed;
}
#endif //// TODO:


// --------------------------------------------------------------


#if 0 //// TODO:
static constexpr const char protocol[] = "HTTP/1.1";
static constexpr const char request_method[] = "POST";
static constexpr const char request_resource[] = "/scope/v1.0/api";
static constexpr const char request_header_name[] = "Request-Header-Name";
static constexpr const char request_header_value[] = "Request-Header-Value";
static constexpr abc::net::http::status_code_t response_status_code = 200;
static constexpr const char response_reason_phrase[] = "OK";
static constexpr const char response_header_name[] = "Response-Header-Name";
static constexpr const char response_header_value[] = "Response-Header-Value";

template <typename ClientSocket>
void http_json_stream_client(bool& passed, test_context& context, ClientSocket& client, const char* server_port) {
    constexpr const char* suborigin = "http_json_stream_client";

    try {
        client.connect("localhost", server_port);

        abc::net::socket_streambuf<ClientSocket*, test_log*> sb(&client, context.log());
        abc::net::http::client<test_log*> http(&sb, context.log());

        // Send request
        {
            abc::net::http::request request;
            request.method = request_method;
            request.resource = request_resource;
            request.protocol = protocol;
            request.headers = {
                { request_header_name, request_header_value },
            };

            http.put_request(request);

            abc::net::json::value<test_log*> body = 
                abc::net::json::literal::object {
                    { "param", "foo" },
                };

            abc::net::json::writer<test_log*> json(&sb, context.log());
            json.put_value(body)
        }

        // Receive response
        {
            abc::net::http::response response = http.get_response();

            passed = context.are_equal(response.protocol.c_str(), protocol, 0x100e9) && passed;
            passed = context.are_equal(response.status_code, response_status_code, 0x100ea, "%u") && passed;
            passed = context.are_equal(response.reason_phrase.c_str(), response_reason_phrase, 0x100eb) && passed;
            passed = context.are_equal(response.headers.size(), 1, 0x100ec, "%zu") && passed;
            passed = context.are_equal(response.headers[response_header_name].c_str(), response_header_value, 0x100ed) && passed;

            abc::net::json::reader<test_log*> json(&sb, context.log());
            abc::net::json::value<test_log*> body = json.get_value();

            passed = context.are_equal(body.type(), abc::net::json::value_type::object, 0x102a0, "%u") && passed;
            passed = context.are_equal(body.object().size(), 2, 0x102a1, "%zu") && passed;
            passed = context.are_equal(body.object()["n"].type(), abc::net::json::value_type::number, 0x102a2, "%u") && passed;
            passed = context.are_equal(body.object()["n"].number(), 42.0, 0x102a3, "%f") && passed;
            passed = context.are_equal(body.object()["s"].type(), abc::net::json::value_type::string, 0x102a4, "%u") && passed;
            passed = context.are_equal(body.object()["s"].string().c_str(), "bar", 0x102a5) && passed;
        }
    }
    catch (const std::exception& ex) {
        context.log()->put_any(origin, suborigin, abc::diag::severity::important, 0x100f0, "client: EXCEPTION: %s", ex.what());
    }
}


template <typename ClientSocket>
void http_json_stream_server(bool& passed, test_context& context, ClientSocket& connection) {
    try {
        abc::net::socket_streambuf<ClientSocket, test_log*> sb(&connection, context.log());
        abc::net::http::server<test_log*> http(&sb, context.log());

        // Receive request
        {
            abc::net::http::request request = http.get_request();

            passed = context.are_equal(request.method.c_str(), request_method, 0x100f2) && passed;
            passed = context.are_equal(request.resource.path.c_str(), request_resource, 0x100f3) && passed;
            passed = context.are_equal(request.protocol.c_str(), protocol, 0x100f4) && passed;
            passed = context.are_equal(request.headers.size(), 1, 0x100f5, "%zu") && passed;
            passed = context.are_equal(request.headers[request_header_name].c_str(), request_header_value, 0x100f6) && passed;

            abc::net::json::reader<test_log*> json(static_cast<abc::net::http::request_reader<test_log*>&>(http).rdbuf(), context.log());
            abc::net::json::value<test_log*> body = json.get_value();

            passed = context.are_equal(body.type(), abc::net::json::value_type::object, 0x102aa, "%u") && passed;
            passed = context.are_equal(body.object().size(), 1, 0x102ab, "%zu") && passed;
            passed = context.are_equal(body.object()["param"].type(), abc::net::json::value_type::string, 0x102ac, "%u") && passed;
            passed = context.are_equal(body.object()["param"].string().c_str(), "foo", 0x102ae) && passed;
        }

        // Send response
        {
            abc::net::http::response response;
            response.protocol = protocol;
            response.status_code = response_status_code;
            response.reason_phrase = response_reason_phrase;
            response.headers = {
                { response_header_name, response_header_value },
            };

            http.put_response(response);

            abc::net::json::writer<test_log*> json(static_cast<abc::net::http::response_writer<test_log*>&>(http).rdbuf(), context.log());

            abc::net::json::value<test_log*> body = 
                abc::net::json::literal::object {
                    { "n", 42.0 },
                    { "s", "bar" },
                };

            json.put_value(body);
        }
    }
    catch (const std::exception& ex) {
        context.log()->put_any(origin, suborigin, abc::diag::severity::important, __TAG__, "client: EXCEPTION: %s", ex.what());
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

    http_json_stream_server(passed, context, connection);

    client_thread.join();
    return passed;
}


bool test_tcp_socket_http_json_stream(test_context& context) {
    abc::net::tcp_server_socket<test_log*> server(abc::net::socket::family::ipv4, context.log());
    abc::net::tcp_client_socket<test_log*> client(abc::net::socket::family::ipv4, context.log());

    return tcp_socket_http_json_stream(context, server, client, "31005");
}


bool test_openssl_tcp_socket_http_json_stream(test_context<abc::test::log>& context) {
    bool passed = true;

#ifdef __ABC__OPENSSL
    char cert_path[max_path_size];
    passed = passed && make_filepath(context, cert_path, max_path_size, context.process_path, cert_filename);

    char pkey_path[max_path_size];
    passed = passed && make_filepath(context, pkey_path, max_path_size, context.process_path, pkey_filename);

    if (passed) {
        abc::openssl_tcp_server_socket<abc::test::log> server(cert_path, pkey_path, pkey_password, verify_client, abc::socket::family::ipv4, context.log);
        abc::openssl_tcp_client_socket<abc::test::log> client(verify_server, abc::socket::family::ipv4, context.log);

        passed = passed && tcp_socket_http_json_stream(context, server, client, "31006");
    }
#else
    passed = context.are_equal(0, 0, 0x107a2);
#endif
    return passed;
}


// --------------------------------------------------------------


template <typename ServerSocket, typename ClientSocket, typename Limits, typename Log>
class test_endpoint : public abc::endpoint<ServerSocket, ClientSocket, Limits, Log> {
    using base = abc::endpoint<ServerSocket, ClientSocket, Limits, Log>;

public:
    test_endpoint(bool& passed, test_context<Log>& context, abc::endpoint_config* config, Log* log);

protected:
    virtual void process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) override;

protected:
    bool& _passed;
    test_context<Log>& _context;
};


template <typename ServerSocket, typename ClientSocket, typename Limits, typename Log>
inline test_endpoint<ServerSocket, ClientSocket, Limits, Log>::test_endpoint(bool& passed, test_context<Log>& context, abc::endpoint_config* config, Log* log)
    : base(config, log)
    , _passed(passed)
    , _context(context) {
}


template <typename ServerSocket, typename ClientSocket, typename Limits, typename Log>
inline void test_endpoint<ServerSocket, ClientSocket, Limits, Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
    _passed = _context.are_equal(method, request_method, 0x107a3) && _passed;
    _passed = _context.are_equal(resource, request_resource, 0x107a4) && _passed;

    http_json_stream_server(_passed, _context, http);

    base::set_shutdown_requested();
}


// --------------------------------------------------------------


template <typename Limits, typename Log>
class test_http_endpoint : public test_endpoint<abc::tcp_server_socket<Log>, abc::tcp_client_socket<Log>, Limits, Log> {
    using base = test_endpoint<abc::tcp_server_socket<Log>, abc::tcp_client_socket<Log>, Limits, Log>;

public:
    test_http_endpoint(bool& passed, test_context<Log>& context, abc::endpoint_config* config, Log* log);

protected:
    virtual abc::tcp_server_socket<Log>    create_server_socket() override;
};


template <typename Limits, typename Log>
inline test_http_endpoint<Limits, Log>::test_http_endpoint(bool& passed, test_context<Log>& context, abc::endpoint_config* config, Log* log)
    : base(passed, context, config, log) {
}


template <typename Limits, typename Log>
inline abc::tcp_server_socket<Log> test_http_endpoint<Limits, Log>::create_server_socket() {
    return abc::tcp_server_socket<Log>(abc::socket::family::ipv4, base::_log);
}


// --------------------------------------------------------------


#ifdef __ABC__OPENSSL
template <typename Limits, typename Log>
class test_https_endpoint : public test_endpoint<abc::openssl_tcp_server_socket<Log>, abc::openssl_tcp_client_socket<Log>, Limits, Log> {
    using base = test_endpoint<abc::openssl_tcp_server_socket<Log>, abc::openssl_tcp_client_socket<Log>, Limits, Log>;

public:
    test_https_endpoint(const char* cert_path, const char* pkey_path, const char* pkey_password, bool verify_client,
                        bool& passed, test_context<Log>& context, abc::endpoint_config* config, Log* log);

protected:
    virtual abc::openssl_tcp_server_socket<Log>    create_server_socket() override;

protected:
    const char* _cert_path;
    const char* _pkey_path;
    const char* _pkey_password;
    bool _verify_client;
};


template <typename Limits, typename Log>
inline test_https_endpoint<Limits, Log>::test_https_endpoint(const char* cert_path, const char* pkey_path, const char* pkey_password, bool verify_client,
                                                            bool& passed, test_context<Log>& context, abc::endpoint_config* config, Log* log)
    : base(passed, context, config, log)
    , _cert_path(cert_path)
    , _pkey_path(pkey_path)
    , _pkey_password(pkey_password)
    , _verify_client(verify_client) {
}


template <typename Limits, typename Log>
inline abc::openssl_tcp_server_socket<Log> test_https_endpoint<Limits, Log>::create_server_socket() {
    return abc::openssl_tcp_server_socket<Log>(_cert_path, _pkey_path, _pkey_password, _verify_client, abc::socket::family::ipv4, base::_log);
}
#endif


// --------------------------------------------------------------


template <typename Endpoint, typename ClientSocket>
bool endpoint_json_stream(bool& passed, test_context<abc::test::log>& context, Endpoint& endpoint, ClientSocket& client, const char* server_port) {
    std::future<void> done = endpoint.start_async();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::thread client_thread([&passed, &context, &client, server_port] () {
        http_json_stream_client(passed, context, client, server_port);
    });

    client_thread.join();
    done.wait();

    return passed;
}


bool test_http_endpoint_json_stream(test_context<abc::test::log>& context) {
    bool passed = true;

    abc::endpoint_config config(
        "31007",                // port
        5,                        // listen_queue_size
        context.process_path,    // root_dir (Note: No trailing slash!)
        "/resources/"            // files_prefix
    );
    test_http_endpoint<abc::endpoint_limits, abc::test::log> endpoint(passed, context, &config, context.log);
    abc::tcp_client_socket<abc::test::log> client(abc::socket::family::ipv4, context.log);

    passed = endpoint_json_stream(passed, context, endpoint, client, config.port) && passed;

    return passed;
}


bool test_https_endpoint_json_stream(test_context<abc::test::log>& context) {
    bool passed = true;

#ifdef __ABC__OPENSSL
    abc::endpoint_config config(
        "31008",                // port
        5,                        // listen_queue_size
        context.process_path,    // root_dir (Note: No trailing slash!)
        "/resources/"            // files_prefix
    );

    char cert_path[max_path_size];
    passed = passed && make_filepath(context, cert_path, max_path_size, context.process_path, cert_filename);

    char pkey_path[max_path_size];
    passed = passed && make_filepath(context, pkey_path, max_path_size, context.process_path, pkey_filename);

    if (passed) {
        test_https_endpoint<abc::endpoint_limits, abc::test::log> endpoint(cert_path, pkey_path, pkey_password, verify_client, passed, context, &config, context.log);
        abc::openssl_tcp_client_socket<abc::test::log> client(verify_server, abc::socket::family::ipv4, context.log);

        passed = passed && endpoint_json_stream(passed, context, endpoint, client, config.port);
    }
#else
    passed = context.are_equal(0, 0, 0x107a5);
#endif
    return passed;
}
#endif //// TODO:


