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
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../../src/log.h"
#include "../../src/socket.h"


using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;


int pem_passwd_cb(char* buf, int size, int /*rwflag*/, void* /*password*/)
{
	memset(buf, 0, size);
	strncpy(buf, "server", size);
	buf[size - 1] = '\0';

	return(strlen(buf));
}


int main(int /*argc*/, const char* argv[]) {
	// Create a log.
	abc::log_filter filter(abc::severity::abc::debug);
	log_ostream log(std::cout.rdbuf(), &filter);

	// Use the path to this program to build the path to the pool file.
	constexpr std::size_t max_path = abc::size::k1;
	char path[max_path];
	path[0] = '\0';

	constexpr const char cert_path[] = "cert.pem";
	std::size_t cert_path_len = std::strlen(cert_path); 

	constexpr const char pkey_path[] = "pkey.pem";
	std::size_t pkey_path_len = std::strlen(pkey_path); 

	const char* prog_last_separator = std::strrchr(argv[0], '/');
	std::size_t prog_path_len = 0;
	std::size_t prog_path_len_1 = 0;

	if (prog_last_separator != nullptr) {
		prog_path_len = prog_last_separator - argv[0];
		prog_path_len_1 = prog_path_len + 1;
		std::size_t full_path_len = prog_path_len_1 + std::max(cert_path_len, pkey_path_len);

		if (full_path_len >= max_path) {
			log.put_any(abc::category::abc::samples, abc::severity::critical, __TAG__,
				"This sample allows paths up to %zu chars. The path to this process is %zu chars. To continue, either move the current dir closer to the process, or increase the path limit in main.cpp.",
				max_path, full_path_len);

			return 1;
		}

		std::strncpy(path, argv[0], prog_path_len_1);
	}


	const char* port = "31241";
	abc::tcp_server_socket<log_ostream> server(&log);
	server.bind(port);
	server.listen(5);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Listening on port %s", port);

	abc::tcp_client_socket<log_ostream> client = server.accept();
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Client accepted");

	int stat;
	const SSL_METHOD *method = TLS_server_method();
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "method %p", method);

	SSL_CTX *ctx = SSL_CTX_new(method);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "ctx %p", ctx);

	SSL_CTX_set_default_passwd_cb(ctx, pem_passwd_cb);

	std::strcpy(path + prog_path_len_1, cert_path);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "cert_path='%s'", path);

    stat = SSL_CTX_use_certificate_file(ctx, path, SSL_FILETYPE_PEM);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "use_cert %s(%d)", (stat == 1 ? "OK" : "ERR"), stat);
	ERR_print_errors_fp(stderr);

	std::strcpy(path + prog_path_len_1, pkey_path);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "cert_path='%s'", path);

    stat = SSL_CTX_use_PrivateKey_file(ctx, path, SSL_FILETYPE_PEM);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "use_privkey %s(%d)", (stat == 1 ? "OK" : "ERR"), stat);
 	ERR_print_errors_fp(stderr);

	SSL *ssl = SSL_new(ctx);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "ssl %p", ssl);

	stat = SSL_set_fd(ssl, client.handle());
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "set_fd %s(%d)", (stat == 1 ? "OK" : "ERR"), stat);


	stat = SSL_accept(ssl);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "accept %s(%d)", (stat == 1 ? "OK" : "ERR"), stat);

	std::cout << "Press ENTER to finish..." << std::endl;
	std::cin.get();

	SSL_free(ssl);
	SSL_CTX_free(ctx);

	return 0;
}
