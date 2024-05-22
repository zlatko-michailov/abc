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


#pragma once

#include <cstring>
#include <memory>

#include "../socket.h"
#include "i/socket.i.h"


namespace abc { namespace net { namespace openssl {

    inline tcp_client_socket::tcp_client_socket(bool verify_server, socket::family family, diag::log_ostream* log)
        : base("abc::net::openssl::tcp_client_socket", family, log)
        , _verify_server(verify_server) {

        constexpr const char* suborigin = "tcp_client_socket()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10763, "Begin: verify_server=%d", verify_server);

        const SSL_METHOD *method = TLS_client_method();
        diag_base::require(suborigin, method != nullptr, 0x10764, "::TLS_client_method()");

        _ctx = SSL_CTX_new(method);
        diag_base::require(suborigin, _ctx != nullptr, 0x10765, "::SSL_CTX_new()");

        SSL_CTX_set_verify(_ctx, _verify_server ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, nullptr);

        _ssl = SSL_new(_ctx);
        diag_base::require(suborigin, _ssl != nullptr, 0x10766, "::SSL_new()");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10767, "End:");
    }


    inline tcp_client_socket::tcp_client_socket(tcp_client_socket&& other) noexcept
        : base(std::move(other))
        , _verify_server(other._verify_server)
        , _ctx(other._ctx)
        , _ssl(other._ssl) {

        constexpr const char* suborigin = "tcp_client_socket()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10768, "Begin: _verify_server=%d, _ctx=%p, _ssl=%p", _verify_server, _ctx, _ssl);

        other._verify_server = true;
        other._ctx = nullptr;
        other._ssl = nullptr;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10769, "End:");
    }


    inline tcp_client_socket::tcp_client_socket(socket::fd_t fd, SSL_CTX* ctx, bool verify_server, socket::family family, diag::log_ostream* log)
        : base("abc::net::openssl::tcp_client_socket", fd, family, log)
        , _verify_server(verify_server) {

        constexpr const char* suborigin = "tcp_client_socket()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1076a, "Begin: _verify_server=%d, ctx=%p", _verify_server, ctx);

        diag_base::expect(suborigin, ctx != nullptr, __TAG__, "ctx != nullptr");

        _ssl = SSL_new(ctx);
        diag_base::require(suborigin, _ssl != nullptr, 0x1076b, "::SSL_new()");

        int stat = SSL_set_fd(_ssl, fd);
        diag_base::require(suborigin, stat > 0, 0x1076c, "::SSL_set_fd()");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1076d, "End: _ssl=%p", _ssl);
    }


    inline tcp_client_socket::~tcp_client_socket() noexcept {
        constexpr const char* suborigin = "~tcp_client_socket()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1076e, "Begin: _ssl=%p, _ctx=%p", _ssl, _ctx);

        if (_ssl != nullptr) {
            SSL_shutdown(_ssl);
            SSL_free(_ssl);
            _ssl = nullptr;
        }

        if (_ctx != nullptr) {
            SSL_CTX_free(_ctx);
            _ctx = nullptr;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1076f, "End:");
    }


    inline void tcp_client_socket::connect(const char* host, const char* port) {
        constexpr const char* suborigin = "connect()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10770, "Begin: host='%s', port='%s'", host, port);

        diag_base::expect(suborigin, host != nullptr, __TAG__, "host != nullptr");
        diag_base::expect(suborigin, port != nullptr, __TAG__, "host != nullptr");

        base::connect(host, port);
        connect_handshake();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10771, "End:");
    }


    inline void tcp_client_socket::connect(const socket::address& address) {
        constexpr const char* suborigin = "connect()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10772, "Begin:");

        base::connect(address);
        connect_handshake();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10773, "End:");
    }


    inline std::size_t tcp_client_socket::send(const void* buffer, std::size_t size) {
        constexpr const char* suborigin = "send()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10774, "Begin: size=%zu", size);

        diag_base::expect(suborigin, base::is_open(), 0x10775, "is_open");
        diag_base::expect(suborigin, _ssl != nullptr, 0x10776, "_ssl != nullptr");

        int sent_size = SSL_write(_ssl, buffer, (int)size);

        if (sent_size < 0) {
            diag_base::put_any(suborigin, diag::severity::important, 0x10777, "sent_size=%l", (long)sent_size);

            sent_size = 0;
        }
        else if ((std::size_t)sent_size < size) {
            diag_base::put_any(suborigin, diag::severity::important, 0x10778, "sent_size=%l", (long)sent_size);
        }

        diag_base::put_binary(suborigin, diag::severity::verbose, 0x10779, buffer, size);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1077a, "End: size=%zu, sent_size=%l", size, sent_size);

        return sent_size;
    }


    inline std::size_t tcp_client_socket::receive(void* buffer, std::size_t size) {
        constexpr const char* suborigin = "send()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1077b, "Begin: size=%zu", size);

        diag_base::expect(suborigin, base::is_open(), 0x1077c, "is_open");
        diag_base::expect(suborigin, _ssl != nullptr, 0x1077d, "_ssl != nullptr");

        int received_size = SSL_read(_ssl, buffer, (int)size);

        if (received_size < 0) {
            diag_base::put_any(suborigin, diag::severity::important, 0x1077e, "sent_size=%l", (long)received_size);

            received_size = 0;
        }
        else if ((std::size_t)received_size < size) {
            diag_base::put_any(suborigin, diag::severity::important, 0x1077f, "sent_size=%l", (long)received_size);
        }

        diag_base::put_binary(suborigin, diag::severity::verbose, 0x10780, buffer, size);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10781, "End: size=%zu, received_size=%l", size, received_size);

        return received_size;
    }


    inline void tcp_client_socket::connect_handshake() {
        constexpr const char* suborigin = "connect_handshake()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10782, "Begin:");

        diag_base::expect(suborigin, base::is_open(), 0x10783, "is_open");
        diag_base::expect(suborigin, _ssl != nullptr, 0x10784, "_ssl != nullptr");

        int stat = SSL_set_fd(_ssl, base::fd());
        diag_base::require(suborigin, stat > 0, 0x10785, "::SSL_set_fd()");

        diag_base::put_any(suborigin, diag::severity::important, 0x10786, "Before ::SSL_connect()");
        int ret = SSL_connect(_ssl);
        diag_base::put_any(suborigin, diag::severity::important, 0x10787, "After ::SSL_connect() ret=%d", ret);

        if (ret != 1) {
            int err = SSL_get_error(_ssl, ret);
            diag_base::put_any(suborigin, diag::severity::important, 0x10788, "err=%d", err);

            diag_base::require(suborigin, false, 0x10789, "::SSL_connect()");
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1078a, "End:");
    }


    // --------------------------------------------------------------


    inline tcp_server_socket::tcp_server_socket(const char* cert_file_path, const char* pkey_file_path, const char* pkey_file_password, bool verify_client, socket::family family, diag::log_ostream* log)
        : base("abc::net::openssl::tcp_server_socket", family, log)
        , _verify_client(verify_client) {

        constexpr const char* suborigin = "tcp_server_socket()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1078b, "Begin:");

        diag_base::expect(suborigin, cert_file_path != nullptr, __TAG__, "is_open");
        diag_base::expect(suborigin, pkey_file_path != nullptr, __TAG__, "is_open");
        diag_base::expect(suborigin, pkey_file_password != nullptr, __TAG__, "is_open");

        _pkey_file_password = pkey_file_password;

        const SSL_METHOD *method = TLS_server_method();
        diag_base::require(suborigin, method != nullptr, 0x1078d, "::TLS_server_method()");

        _ctx = SSL_CTX_new(method);
        diag_base::require(suborigin, _ctx != nullptr, 0x1078e, "::SSL_CTX_new()");

        SSL_CTX_set_verify(_ctx, _verify_client ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, nullptr);

        SSL_CTX_set_default_passwd_cb(_ctx, pem_passwd_cb);
        SSL_CTX_set_default_passwd_cb_userdata(_ctx, (void*)_pkey_file_password.c_str());

        int stat = SSL_CTX_use_certificate_file(_ctx, cert_file_path, SSL_FILETYPE_PEM);
        diag_base::require(suborigin, stat > 0, 0x1078f, "::SSL_CTX_use_certificate_file()");

        stat = SSL_CTX_use_PrivateKey_file(_ctx, pkey_file_path, SSL_FILETYPE_PEM);
        diag_base::require(suborigin, stat > 0, 0x10790, "::SSL_CTX_use_certificate_file()");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10791, "End:");
    }


    inline tcp_server_socket::tcp_server_socket(tcp_server_socket&& other) noexcept
        : base(std::move(other))
        , _pkey_file_password(std::move(other._pkey_file_password))
        , _verify_client(other._verify_client)
        , _ctx(other._ctx) {

        constexpr const char* suborigin = "tcp_server_socket()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10792, "Begin:");

        other._verify_client = false;
        other._ctx = nullptr;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10793, "End:");
    }


    inline tcp_server_socket::~tcp_server_socket() noexcept {
        constexpr const char* suborigin = "~tcp_server_socket()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10794, "Begin:");

        if (_ctx != nullptr) {
            SSL_CTX_free(_ctx);
            _ctx = nullptr;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10795, "End:");
    }


    inline std::unique_ptr<net::tcp_client_socket> tcp_server_socket::accept() const {
        constexpr const char* suborigin = "accept()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        socket::fd_t fd = base::accept_fd();
        diag_base::put_any(suborigin, diag::severity::optional, __TAG__, "fd=%d", (int)fd);

        const bool verify_server = false; // This value doesn't matter.
        std::unique_ptr<tcp_client_socket> openssl_client(new tcp_client_socket(fd, _ctx, verify_server, base::family(), diag_base::log()));

        int stat = SSL_accept(openssl_client->_ssl);
        diag_base::require(suborigin, stat > 0, 0x10796, "::SSL_accept()");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        return openssl_client;
    }


    inline int tcp_server_socket::pem_passwd_cb(char* buf, int size, int /*rwflag*/, void* password) noexcept {
        std::strncpy(buf, (const char*)password, size);
        buf[size - 1] = '\0';

        return (int)std::strlen(buf);
    }

} } }
