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


#pragma once

#include <cstring>
#include <memory>

#include "i/openssl_socket.i.h"
#include "socket.h"


namespace abc {

	template <typename Log>
	inline openssl_tcp_client_socket<Log>::openssl_tcp_client_socket(bool verify_server, socket::family_t family, Log* log)
		: base(family, log)
		, _verify_server(verify_server) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::openssl_tcp_client_socket() >>>");
		}

		const SSL_METHOD *method = TLS_client_method();
		if (method == nullptr) {
			throw exception<std::logic_error, Log>("openssl_tcp_client_socket::openssl_tcp_client_socket() TLS_client_method()", __TAG__, log_local);
		}

		_ctx = SSL_CTX_new(method);
		if (_ctx == nullptr) {
			throw exception<std::logic_error, Log>("openssl_tcp_client_socket::openssl_tcp_client_socket() SSL_CTX_new()", __TAG__, log_local);
		}

		SSL_CTX_set_verify(_ctx, _verify_server ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, nullptr);

		_ssl = SSL_new(_ctx);
		if (_ssl == nullptr) {
			throw exception<std::logic_error, Log>("openssl_tcp_client_socket::openssl_tcp_client_socket() SSL_new()", __TAG__, log_local);
		}

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::openssl_tcp_client_socket() <<<");
		}
	}


	template <typename Log>
	inline openssl_tcp_client_socket<Log>::openssl_tcp_client_socket(openssl_tcp_client_socket&& other) noexcept
		: base(std::move(other))
		, _verify_server(other._verify_server)
		, _ctx(other._ctx)
		, _ssl(other._ssl) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::openssl_tcp_client_socket(move) other._ssl=%p >>>", other._ssl);
		}

		other._verify_server = true;
		other._ctx = nullptr;
		other._ssl = nullptr;

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::openssl_tcp_client_socket() _ssl=%p <<<", _ssl);
		}
	}


	template <typename Log>
	inline openssl_tcp_client_socket<Log>::openssl_tcp_client_socket(socket::fd_t fd, SSL_CTX* ctx, bool verify_server, socket::family_t family, Log* log)
		: base(fd, family, log)
		, _verify_server(verify_server) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::openssl_tcp_client_socket() ctx=%p >>>", ctx);
		}

		_ssl = SSL_new(ctx);
		if (_ssl == nullptr) {
			throw exception<std::logic_error, Log>("openssl_tcp_client_socket::openssl_tcp_client_socket() SSL_new()", __TAG__, log_local);
		}

		int stat = SSL_set_fd(_ssl, fd);
		if (stat <= 0) {
			throw exception<std::logic_error, Log>("openssl_tcp_client_socket::openssl_tcp_client_socket() SSL_set_fd()", __TAG__, log_local);
		}

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::openssl_tcp_client_socket() _ssl=%p <<<", _ssl);
		}
	}


	template <typename Log>
	inline openssl_tcp_client_socket<Log>::~openssl_tcp_client_socket() noexcept {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::~openssl_tcp_client_socket() _ssl=%p >>>", _ssl);
		}

		if (_ssl != nullptr) {
			SSL_shutdown(_ssl);
			SSL_free(_ssl);
			_ssl = nullptr;
		}

		if (_ctx != nullptr) {
			SSL_CTX_free(_ctx);
			_ctx = nullptr;
		}

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::~openssl_tcp_client_socket() <<<");
		}
	}


	template <typename Log>
	inline void openssl_tcp_client_socket<Log>::connect(const char* host, const char* port) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::connect() >>>");
		}

		base::connect(host, port);
		connect_handshake();

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::connect() <<<");
		}
	}


	template <typename Log>
	inline void openssl_tcp_client_socket<Log>::connect(const socket::address& address) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::connect() >>>");
		}

		base::connect(address);
		connect_handshake();

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::connect() <<<");
		}
	}


	template <typename Log>
	inline std::size_t openssl_tcp_client_socket<Log>::send(const void* buffer, std::size_t size) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::send() >>> size=%zu", size);
		}

		if (!base::is_open()) {
			throw exception<std::logic_error, Log>("openssl_tcp_client_socket::send() !is_open()", __TAG__, log_local);
		}

		if (_ssl == nullptr) {
			throw exception<std::logic_error, Log>("openssl_tcp_client_socket::send() !_ssl", __TAG__, log_local);
		}

		int sent_size = SSL_write(_ssl, buffer, (int)size);

		if (sent_size < 0) {
			if (log_local != nullptr) {
				log_local->put_any(category::abc::socket, severity::important, __TAG__, "openssl_tcp_client_socket::send() sent_size=%l", (long)sent_size);
			}

			sent_size = 0;
		}
		else if ((std::size_t)sent_size < size) {
			if (log_local != nullptr) {
				log_local->put_any(category::abc::socket, severity::important, __TAG__, "openssl_tcp_client_socket::send() sent_size=%l", (long)sent_size);
			}
		}

		if (log_local != nullptr) {
			log_local->put_binary(category::abc::socket, severity::abc::debug, __TAG__, buffer, size);
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::send() <<< size=%zu, sent_size=%l", size, sent_size);
		}

		return sent_size;
	}


	template <typename Log>
	inline std::size_t openssl_tcp_client_socket<Log>::receive(void* buffer, std::size_t size) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::receive() >>> size=%zu", size);
		}

		if (!base::is_open()) {
			throw exception<std::logic_error, Log>("openssl_tcp_client_socket::receive() !is_open()", __TAG__, log_local);
		}

		if (_ssl == nullptr) {
			throw exception<std::logic_error, Log>("openssl_tcp_client_socket::receive() !_ssl", __TAG__, log_local);
		}

		int received_size = SSL_read(_ssl, buffer, (int)size);

		if (received_size < 0) {
			if (log_local != nullptr) {
				log_local->put_any(category::abc::socket, severity::important, __TAG__, "openssl_tcp_client_socket::receive() received_size=%l", (long)received_size);
			}

			received_size = 0;
		}
		else if ((std::size_t)received_size < size) {
			if (log_local != nullptr) {
				log_local->put_any(category::abc::socket, severity::important, __TAG__, "openssl_tcp_client_socket::receive() received_size=%l", (long)received_size);
			}
		}

		if (log_local != nullptr) {
			log_local->put_binary(category::abc::socket, severity::abc::debug, __TAG__, buffer, size);
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::receive() <<< size=%zu, received_size=%l", size, received_size);
		}

		return received_size;
	}


	template <typename Log>
	inline void openssl_tcp_client_socket<Log>::connect_handshake() {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::connect_handshake() >>>");
		}

		if (!base::is_open()) {
			throw exception<std::logic_error, Log>("openssl_tcp_client_socket::connect_handshake() !is_open()", __TAG__, log_local);
		}

		if (_ssl == nullptr) {
			throw exception<std::logic_error, Log>("openssl_tcp_client_socket::connect_handshake() !_ssl", __TAG__, log_local);
		}

		int stat = SSL_set_fd(_ssl, base::fd());
		if (stat <= 0) {
			throw exception<std::logic_error, Log>("openssl_tcp_client_socket::connect_handshake() SSL_set_fd()", __TAG__, log_local);
		}

		log_local->put_any(category::abc::socket, severity::important, __TAG__, "Before SSL_connect()");
		int ret = SSL_connect(_ssl);
		log_local->put_any(category::abc::socket, severity::important, __TAG__, "After SSL_connect() ret=%d", ret);

		if (ret != 1) {
			int err = SSL_get_error(_ssl, ret);
			log_local->put_any(category::abc::socket, severity::important, __TAG__, "ERR=%d", err);

			throw exception<std::runtime_error, Log>("openssl_tcp_client_socket::connect_handshake() SSL_connect()", __TAG__, log_local);
		}

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::connect_handshake() <<<");
		}
	}


	// --------------------------------------------------------------


	template <typename Log>
	inline openssl_tcp_server_socket<Log>::openssl_tcp_server_socket(const char* cert_file_path, const char* pkey_file_path, const char* pkey_file_password, bool verify_client, socket::family_t family, Log* log)
		: base(family, log)
		, _verify_client(verify_client) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_server_socket::openssl_tcp_server_socket() >>>");
		}

		std::size_t pkey_file_password_len = std::strlen(pkey_file_password);
		if (pkey_file_password_len >= max_password) {
			throw exception<std::logic_error, Log>("openssl_tcp_server_socket::openssl_tcp_server_socket() pkey_file_password_len", __TAG__, log_local);
		}

		std::strncpy(_pkey_file_password, pkey_file_password, max_password);
		_pkey_file_password[max_password] = '\0';

		const SSL_METHOD *method = TLS_server_method();
		if (method == nullptr) {
			throw exception<std::logic_error, Log>("openssl_tcp_server_socket::openssl_tcp_server_socket() TLS_server_method()", __TAG__, log_local);
		}

		_ctx = SSL_CTX_new(method);
		if (_ctx == nullptr) {
			throw exception<std::logic_error, Log>("openssl_tcp_server_socket::openssl_tcp_server_socket() SSL_CTX_new()", __TAG__, log_local);
		}

		SSL_CTX_set_verify(_ctx, _verify_client ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, nullptr);

		SSL_CTX_set_default_passwd_cb(_ctx, pem_passwd_cb);
		SSL_CTX_set_default_passwd_cb_userdata(_ctx, _pkey_file_password);

		int stat = SSL_CTX_use_certificate_file(_ctx, cert_file_path, SSL_FILETYPE_PEM);
		if (stat <= 0) {
			throw exception<std::logic_error, Log>("openssl_tcp_server_socket::openssl_tcp_server_socket() SSL_CTX_use_certificate_file()", __TAG__, log_local);
		}

	    stat = SSL_CTX_use_PrivateKey_file(_ctx, pkey_file_path, SSL_FILETYPE_PEM);
		if (stat <= 0) {
			throw exception<std::logic_error, Log>("openssl_tcp_server_socket::openssl_tcp_server_socket() SSL_CTX_use_PrivateKey_file()", __TAG__, log_local);
		}

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_server_socket::openssl_tcp_server_socket() <<<");
		}
	}


	template <typename Log>
	inline openssl_tcp_server_socket<Log>::openssl_tcp_server_socket(openssl_tcp_server_socket&& other) noexcept
		: base(std::move(other))
		, _verify_client(other._verify_client)
		, _ctx(other._ctx) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_server_socket::openssl_tcp_server_socket(move) other._ctx=%p >>>", other._ctx);
		}

		std::memmove(_pkey_file_password, other._pkey_file_password, sizeof(_pkey_file_password));

		other._verify_client = false;
		other._ctx = nullptr;
		std::memset(other._pkey_file_password, 0, sizeof(other._pkey_file_password));

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_server_socket::openssl_tcp_server_socket(move) <<<");
		}
	}


	template <typename Log>
	inline openssl_tcp_server_socket<Log>::~openssl_tcp_server_socket() noexcept {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_server_socket::~openssl_tcp_server_socket() >>>");
		}

		if (_ctx != nullptr) {
			SSL_CTX_free(_ctx);
			_ctx = nullptr;
		}

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_server_socket::~openssl_tcp_server_socket() <<<");
		}
	}


	template <typename Log>
	inline openssl_tcp_client_socket<Log> openssl_tcp_server_socket<Log>::accept() const {
		Log* log_local = base::log();

		socket::fd_t fd = base::accept_fd();

		const bool verify_server = false; // This value doesn't matter.
		openssl_tcp_client_socket<Log> openssl_client(fd, _ctx, verify_server, base::family(), base::log());

		int stat = SSL_accept(openssl_client._ssl);
		if (stat <= 0) {
			throw exception<std::logic_error, Log>("openssl_tcp_server_socket::accept() SSL_accept()", __TAG__, log_local);
		}

		return openssl_client;
	}


	template <typename Log>
	inline int openssl_tcp_server_socket<Log>::pem_passwd_cb(char* buf, int size, int /*rwflag*/, void* password) noexcept {
		std::strncpy(buf, static_cast<char*>(password), size);
		buf[size - 1] = '\0';

		return (int)std::strlen(buf);
	}

}