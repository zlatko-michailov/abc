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
	inline openssl_tcp_client_socket<Log>::openssl_tcp_client_socket(bool is_security_enabled, socket::family_t family, Log* log)
		: base(family, log)
		, _is_security_enabled(is_security_enabled) {
	}


	template <typename Log>
	inline openssl_tcp_client_socket<Log>::openssl_tcp_client_socket(Log* log)
		: openssl_tcp_client_socket<Log>(true, socket::family::ipv4, log) {
	}


	template <typename Log>
	inline openssl_tcp_client_socket<Log>::openssl_tcp_client_socket(openssl_tcp_client_socket&& other) noexcept
		: base(std::move(other))
		, _is_security_enabled(other._is_security_enabled)
		, _ssl(other._ssl) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::openssl_tcp_client_socket(move) other._ssl=%p >>>", other._ssl);
		}

		other._ssl = nullptr;

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::openssl_tcp_client_socket() _ssl=%p <<<", _ssl);
		}
	}


	template <typename Log>
	inline openssl_tcp_client_socket<Log>::openssl_tcp_client_socket(socket::fd_t fd, SSL_CTX* ctx, bool is_security_enabled, socket::family_t family, Log* log)
		: base(fd, family, log)
		, _is_security_enabled(is_security_enabled) {
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

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_client_socket::~openssl_tcp_client_socket() <<<");
		}
	}


	template <typename Log>
	inline std::size_t openssl_tcp_client_socket<Log>::send(const void* buffer, std::size_t size, socket::address* address) {
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

		int sent_size;
		if (address != nullptr) {
			throw exception<std::logic_error, Log>("openssl_tcp_client_socket::send() address", __TAG__, log_local);
		}
		else {
			sent_size = SSL_write(_ssl, buffer, (int)size);
		}

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


	// --------------------------------------------------------------


	template <typename Log>
	inline openssl_tcp_server_socket<Log>::openssl_tcp_server_socket(const char* cert_file_path, const char* pkey_file_path, const char* pkey_file_password, bool is_security_enabled, Log* log)
		: openssl_tcp_server_socket<Log>(cert_file_path, pkey_file_path, pkey_file_password, is_security_enabled, socket::family::ipv4, log) {
	}


	template <typename Log>
	inline openssl_tcp_server_socket<Log>::openssl_tcp_server_socket(const char* cert_file_path, const char* pkey_file_path, const char* pkey_file_password, bool is_security_enabled, socket::family_t family, Log* log)
		: base(family, log)
		, _is_security_enabled(is_security_enabled) {
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
		, _is_security_enabled(other._is_security_enabled)
		, _ctx(other._ctx) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, __TAG__, "openssl_tcp_server_socket::openssl_tcp_server_socket(move) other._ctx=%p >>>", other._ctx);
		}

		std::memmove(_pkey_file_password, other._pkey_file_password, sizeof(_pkey_file_password));

		other._is_security_enabled = false;
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

		openssl_tcp_client_socket<Log> openssl_client(fd, _ctx, _is_security_enabled, base::family(), base::log());

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
