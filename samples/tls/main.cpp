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


#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "../../src/diag/log.h"
#include "../../src/net/openssl/socket.h"


using log_table = abc::stream::table_ostream;
using log_line = abc::diag::debug_line_ostream<>;
using log_filter = abc::diag::str_log_filter<const char*>;
using log_ostream = abc::diag::log_ostream;


constexpr const char* origin = "tls_sample";


void server(const char* cert_path, const char* pkey_path, const char* password, log_ostream* log, std::condition_variable* scenario_cond) {
    constexpr const char* suborigin = "server()";

    const char* port = "31241";
    bool verify_client = false;
    int queue_size = 5;

    abc::net::openssl::tcp_server_socket openssl_server(cert_path, pkey_path, password, verify_client, abc::net::socket::family::ipv4, log);

    openssl_server.bind(port);
    openssl_server.listen(queue_size);
    
    // accept() blocks. Unblock the client thread now.
    scenario_cond->notify_one();

    std::unique_ptr<abc::net::tcp_client_socket> openssl_connection = openssl_server.accept();

    const char hello[] = ">>> Welcome to abc!";
    uint len = sizeof(hello) - 1;
    openssl_connection->send(&len, 2);
    openssl_connection->send(hello, len);

    char message[100 + 1] { };
    len = 0;
    openssl_connection->receive(&len, 2);
    openssl_connection->receive(message, len);
    log->put_any(origin, suborigin, abc::diag::severity::important, 0x1075e, "Received: (%u)'%s'", len, message);

    std::cout << "Press ENTER to shut down server socket..." << std::endl;
    std::cin.get();
}


void client(log_ostream* log, std::mutex* scenario_mutex, std::condition_variable* scenario_cond) {
    constexpr const char* suborigin = "client()";

    const char* port = "31241";
    bool verify_server = false;
    const char* host = "localhost";

    // Block until the server starts listening.
    std::unique_lock<std::mutex> lock(*scenario_mutex);
    scenario_cond->wait(lock);

    abc::net::openssl::tcp_client_socket openssl_client(verify_server, abc::net::socket::family::ipv4, log);

    openssl_client.connect(host, port);

    uint len = 0;
    openssl_client.receive(&len, 2);

    char message[100 + 1] { };
    openssl_client.receive(message, len);
    log->put_any(origin, suborigin, abc::diag::severity::important, 0x1075f, "Received: (%u)'%s'", len, message);

    const char hi[] = "<<< Thanks.";
    len = sizeof(hi) - 1;
    openssl_client.send(&len, 2);
    openssl_client.send(hi, len);

    std::cout << "Press ENTER to close client socket..." << std::endl;
    std::cin.get();
}


int main(int /*argc*/, const char* argv[]) {
    constexpr const char* suborigin = "main()";

    // Create a log.
    log_table table(std::cout.rdbuf());
    log_line line(&table);
    log_filter filter("", abc::diag::severity::important);
    log_ostream log(&line, &filter);

    // Create cert and pkey paths.
    std::string process_dir = abc::parent_path(argv[0]);

    std::string cert_path = process_dir;
    cert_path.append("/cert.pem");
    log.put_any(origin, suborigin, abc::diag::severity::optional, 0x10761, "cert_path='%s'", cert_path.c_str());

    std::string pkey_path = process_dir;
    pkey_path.append("/pkey.pem");
    log.put_any(origin, suborigin, abc::diag::severity::optional, 0x10762, "pkey_path='%s'", pkey_path.c_str());

    // Run the client and server simultaneously.
    std::mutex scenario_mutex;
    std::condition_variable scenario_cond;
    std::thread server_thread(server, cert_path.c_str(), pkey_path.c_str(), "server", &log, &scenario_cond);
    std::thread client_thread(client, &log, &scenario_mutex, &scenario_cond);

    server_thread.join();
    client_thread.join();

    return 0;
}
