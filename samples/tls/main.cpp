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


int pem_passwd_cb(char* buf, int size, int /*rwflag*/, void* /*password*/) {
	memset(buf, 0, size);
	strncpy(buf, "server", size);
	buf[size - 1] = '\0';

	return(strlen(buf));
}


SSL_CTX* new_server_ssl_ctx (char path[], std::size_t prog_path_len_1, const char* cert_path, const char* pkey_path, log_ostream& log) {
	const SSL_METHOD *method = TLS_server_method();
	if (method == nullptr) {
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "ERR: method=%p", method);
		return nullptr;
	}

	SSL_CTX *ctx = SSL_CTX_new(method);
	if (ctx == nullptr) {
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "ERR: ctx=%p", ctx);
		return nullptr;
	}

	SSL_CTX_set_default_passwd_cb(ctx, pem_passwd_cb);

	std::strcpy(path + prog_path_len_1, cert_path);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "cert_path='%s'", path);

    int stat = SSL_CTX_use_certificate_file(ctx, path, SSL_FILETYPE_PEM);
	if (stat <= 0) {
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "ERR: use_cert stat=%d", stat);
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return nullptr;
	}

	std::strcpy(path + prog_path_len_1, pkey_path);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "cert_path='%s'", path);

    stat = SSL_CTX_use_PrivateKey_file(ctx, path, SSL_FILETYPE_PEM);
	if (stat <= 0) {
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "ERR: use_pkey stat=%d", stat);
		ERR_print_errors_fp(stderr);
		SSL_CTX_free(ctx);
		return nullptr;
	}

	return ctx;
}


int accept_client(SSL* ssl, abc::tcp_client_socket<log_ostream>& client, log_ostream& log) {
	int stat = SSL_set_fd(ssl, client.handle());
	if (stat <= 0) {
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "ERR: set_fd stat=%d", stat);
		ERR_print_errors_fp(stderr);
		return stat;
	}

	stat = SSL_accept(ssl);
	if (stat <= 0) {
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "ERR: accept stat=%d", stat);
		ERR_print_errors_fp(stderr);
		return stat;
	}

	return 1;
}


void server(char* path, std::size_t prog_path_len_1, const char* cert_path, const char* pkey_path, log_ostream& log) {

	const char* port = "31241";
	abc::tcp_server_socket<log_ostream> server(&log);
	server.bind(port);
	server.listen(5);
	
	abc::tcp_client_socket<log_ostream> client = server.accept();


	SSL_CTX *ctx = new_server_ssl_ctx(path, prog_path_len_1, cert_path, pkey_path, log);
	if (ctx == nullptr) {
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "ERR: new_server_ctx=%p", ctx);
		return;
	}

	SSL *ssl = SSL_new(ctx);
	if (ctx == nullptr) {
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "ERR: new_ssl=%p", ssl);
		SSL_CTX_free(ctx);
		return;
	}

	int stat = accept_client(ssl, client, log);
	if (stat == 1) {
		const char welcome[] = ">>> Welcome to abc!\n";
		SSL_write(ssl, welcome, sizeof(welcome));
	}


	std::cout << "Press ENTER to shut down server socket..." << std::endl;
	std::cin.get();

	SSL_shutdown(ssl);
	SSL_free(ssl);
	
	SSL_CTX_free(ctx);
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


	server(path, prog_path_len_1, cert_path, pkey_path, log);


	std::cout << "Press ENTER to exit..." << std::endl;
	std::cin.get();

	return 0;
}
