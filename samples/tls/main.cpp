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


#include <cstring>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "../../src/log.h"
#include "../../src/openssl_socket.h"


using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;


void server(const char* cert_path, const char* pkey_path, const char* password, log_ostream* log, std::condition_variable* scenario_cond) {
	const char* port = "31241";
	bool verify_client = false;
	int queue_size = 5;

	abc::openssl_tcp_server_socket<log_ostream> openssl_server(cert_path, pkey_path, password, verify_client, abc::socket::family::ipv4, log);

	openssl_server.bind(port);
	openssl_server.listen(queue_size);
	
	// accept() blocks. Unblock the client thread now.
	scenario_cond->notify_one();

	abc::openssl_tcp_client_socket<log_ostream> openssl_connection = openssl_server.accept();

	const char hello[] = ">>> Welcome to abc!\n";
	uint len = sizeof(hello) - 1;
	openssl_connection.send(&len, 2);
	openssl_connection.send(hello, len);

	char message[100 + 1];
	std::memset(message, 0, sizeof(message));
	len = 0;
	openssl_connection.receive(&len, 2);
	openssl_connection.receive(message, len);
	log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "SERVER: %u:%s", len, message);

	std::cout << "Press ENTER to shut down server socket..." << std::endl;
	std::cin.get();
}


void client(log_ostream* log, std::mutex* scenario_mutex, std::condition_variable* scenario_cond) {
	const char* port = "31241";
	bool verify_server = false;
	const char* host = "localhost";

	// Block until the server starts listening.
	std::unique_lock<std::mutex> lock(*scenario_mutex);
	scenario_cond->wait(lock);

	abc::openssl_tcp_client_socket<log_ostream> openssl_client(verify_server, abc::socket::family::ipv4, log);

	openssl_client.connect(host, port);

	uint len = 0;
	openssl_client.receive(&len, 2);

	char message[100 + 1];
	std::memset(message, 0, sizeof(message));
	openssl_client.receive(message, len);
	log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "CLIENT: %u:%s", len, message);

	const char hi[] = "<<< Thanks.";
	len = sizeof(hi) - 1;
	openssl_client.send(&len, 2);
	openssl_client.send(hi, len);

	std::cout << "Press ENTER to close client socket..." << std::endl;
	std::cin.get();
}


int main(int /*argc*/, const char* argv[]) {
	// Create a log.
	abc::log_filter filter(abc::severity::abc::debug);
	log_ostream log(std::cout.rdbuf(), &filter);

	// Use the path to this program to build the path to the pool file.
	constexpr std::size_t max_path = abc::size::k1;

	char cert_path[max_path];
	cert_path[0] = '\0';

	char pkey_path[max_path];
	pkey_path[0] = '\0';

	constexpr const char cert_file[] = "cert.pem";
	std::size_t cert_file_len = std::strlen(cert_file); 

	constexpr const char pkey_file[] = "pkey.pem";
	std::size_t pkey_file_len = std::strlen(pkey_file); 

	const char* prog_last_separator = std::strrchr(argv[0], '/');
	std::size_t prog_path_len = 0;
	std::size_t prog_path_len_1 = 0;

	if (prog_last_separator != nullptr) {
		prog_path_len = prog_last_separator - argv[0];
		prog_path_len_1 = prog_path_len + 1;
		std::size_t full_path_len = prog_path_len_1 + std::max(cert_file_len, pkey_file_len);

		if (full_path_len >= max_path) {
			log.put_any(abc::category::abc::samples, abc::severity::critical, __TAG__,
				"This sample allows paths up to %zu chars. The path to this process is %zu chars. To continue, either move the current dir closer to the process, or increase the path limit in main.cpp.",
				max_path, full_path_len);

			return 1;
		}

		std::strncpy(cert_path, argv[0], prog_path_len_1);
		std::strncpy(pkey_path, argv[0], prog_path_len_1);
	}

	std::strcpy(cert_path + prog_path_len_1, cert_file);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "cert_path='%s'", cert_path);

	std::strcpy(pkey_path + prog_path_len_1, pkey_file);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "pkey_path='%s'", pkey_path);

	std::mutex scenario_mutex;
	std::condition_variable scenario_cond;
	std::thread server_thread(server, cert_path, pkey_path, "server", &log, &scenario_cond);
	std::thread client_thread(client, &log, &scenario_mutex, &scenario_cond);

	server_thread.join();
	client_thread.join();

	return 0;
}
