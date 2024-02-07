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

#include <stdexcept>
#include <memory>
#include <cstdio>

#include "i/socket.i.h"


namespace abc { namespace net {

    template <typename LogPtr>
    inline basic_socket<LogPtr>::basic_socket(const char* origin, socket::kind kind, socket::family family, const LogPtr& log)
        : basic_socket(origin, socket::fd::invalid, kind, family, log) {
    }


    template <typename LogPtr>
    inline basic_socket<LogPtr>::basic_socket(const char* origin, socket::fd_t fd, socket::kind kind, socket::family family, const LogPtr& log)
        : diag_base(copy(origin), log)
        , _kind(kind)
        , _family(family)
        , _protocol(kind == socket::kind::stream ? socket::protocol::tcp : socket::protocol::udp)
        , _fd(fd) {

        constexpr const char* suborigin = "basic_socket()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: fd=%d, kind=%d, family=%d, protocol=%d", (int)fd, (int)kind, (int)family);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10006, "End: %s, %s", _kind == socket::kind::stream ? "tcp" : "udp", _family == socket::family::ipv4 ? "ipv4" : "ipv6");
    }


    template <typename LogPtr>
    inline basic_socket<LogPtr>::basic_socket(basic_socket&& other) noexcept
        : diag_base(std::move(other))
        , _kind(other._kind)
        , _family(other._family)
        , _protocol(other._protocol)
        , _fd(other._fd) {

        constexpr const char* suborigin = "basic_socket()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: fd=%d, kind=%d, family=%d, protocol=%d", (int)other._fd, (int)other._kind, (int)other._family);

        other._fd = socket::fd::invalid;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10007, "End: %s, %s", _kind == socket::kind::stream ? "tcp" : "udp", _family == socket::family::ipv4 ? "ipv4" : "ipv6");
    }


    template <typename LogPtr>
    inline basic_socket<LogPtr>::~basic_socket() noexcept {
        constexpr const char* suborigin = "~basic_socket()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: %s, %s", _kind == socket::kind::stream ? "tcp" : "udp", _family == socket::family::ipv4 ? "ipv4" : "ipv6");

        close();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename LogPtr>
    inline bool basic_socket<LogPtr>::is_open() const noexcept {
        return _fd != socket::fd::invalid;
    }


    template <typename LogPtr>
    inline void basic_socket<LogPtr>::close() noexcept {
        constexpr const char* suborigin = "close()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: fd=%d", _fd);

        if (is_open()) {
            diag_base::put_any(suborigin, diag::severity::optional, 0x10009, "Closing");

            ::shutdown(_fd, SHUT_RDWR);
            ::close(_fd);

            _fd = socket::fd::invalid;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: fd=%d", _fd);
    }


    template <typename LogPtr>
    inline void basic_socket<LogPtr>::open() {
        constexpr const char* suborigin = "open()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1000a, "Begin:");

        close();

        _fd = ::socket((int)_family, (int)_kind, (int)_protocol);

        diag_base::ensure(suborigin, is_open(), 0x1000b, "is_open");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1000c, "End: fd=%d", _fd);
    }


    template <typename LogPtr>
    inline addrinfo basic_socket<LogPtr>::hints() const noexcept {
        addrinfo hints{ };

        hints.ai_family   = (int)_family;
        hints.ai_socktype = (int)_kind;
        hints.ai_protocol = (int)_protocol;
        hints.ai_flags    = 0;

        return hints;
    }


    template <typename LogPtr>
    inline void basic_socket<LogPtr>::bind(const char* port) {
        bind(any_host(), port);
    }


    template <typename LogPtr>
    inline void basic_socket<LogPtr>::bind(const char* host, const char* port) {
        tie(host, port, socket::tie::bind);
    }


    template <typename LogPtr>
    inline void basic_socket<LogPtr>::tie(const char* host, const char* port, socket::tie tt) {
        const char* const tt_str = tt == socket::tie::bind ? "bind" : "connect";

        constexpr const char* suborigin = "tie(host, port)";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1000d, "Begin: %s(host, port)", tt_str);

        diag_base::expect(suborigin, !is_open() || tt == socket::tie::connect, 0x1000e, "is_open");

        if (!is_open()) {
            open();
        }

        addrinfo hnt = hints();
        addrinfo* hostList = nullptr;

        socket::error_t err = ::getaddrinfo(host, port, &hnt, &hostList);

        if (err != socket::error::none) {
            if (tt == socket::tie::bind) {
                close();
            }

            diag_base::template throw_exception<std::runtime_error>(suborigin, 0x1000f, "::getaddrinfo() err=%d", err);
        }

        if (hostList == nullptr) {
            diag_base::put_any(suborigin, diag::severity::important, 0x10798, "%s(host, port), ::getaddrinfo() nullptr", tt_str);
        }

        bool is_done = false;
        for (addrinfo* host = hostList; host != nullptr; host = host->ai_next) {
            err = try_tie(*(host->ai_addr), host->ai_addrlen, tt);

            if (err == socket::error::none) {
                is_done = true;
                break;
            }
        }

        ::freeaddrinfo(hostList);

        if (!is_done) {
            if (tt == socket::tie::bind) {
                close();
            }

            diag_base::template throw_exception<std::runtime_error>(suborigin, 0x1000d, "!is_done");
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1000d, "End: %s", tt_str);
    }


    template <typename LogPtr>
    inline void basic_socket<LogPtr>::tie(const socket::address& address, socket::tie tt) {
        const char* const tt_str = tt == socket::tie::bind ? "bind" : "connect";

        constexpr const char* suborigin = "tie(socket::address)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: %s(socket::address)", tt_str);

        diag_base::expect(suborigin, !is_open() || tt == socket::tie::connect, 0x10012, "!is_open");

        if (!is_open()) {
            open();
        }

        socket::error_t err = try_tie(address.value, address.size, tt);

        if (err != socket::error::none) {
            diag_base::template throw_exception<std::runtime_error>(suborigin, 0x10013, "::getaddrinfo() err=%d", err);
        }

        diag_base::ensure(suborigin, is_open(), __TAG__, "is_open");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: %s(socket::address)", tt_str);
    }


    template <typename LogPtr>
    inline socket::error_t basic_socket<LogPtr>::try_tie(const sockaddr& addr, socklen_t addr_len, socket::tie tt) {
        const char* const tt_str = tt == socket::tie::bind ? "bind" : "connect";

        constexpr const char* suborigin = "try_tie(sockaddr)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: %s(sockaddr)", tt_str);

        diag_base::expect(suborigin, is_open(), 0x10014, "is_open");

        const int on = 1;
        socket::error_t err = socket::error::any;

        switch(tt) {
            case socket::tie::bind:
                ::setsockopt(fd(), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

                err = ::bind(fd(), &addr, addr_len);
                break;

            case socket::tie::connect:
                err = ::connect(fd(), &addr, addr_len);
                break;

            default:
                diag_base::assert(suborigin, false, 0x10015, "tt");
        }

        diag_base::put_binary(suborigin, diag::severity::optional, 0x1079b, addr.sa_data, addr_len);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1079c, "End: %s(sockaddr), err=%d", tt_str, err);

        return err;
    }


    template <typename LogPtr>
    inline const char* basic_socket<LogPtr>::any_host() const noexcept {
        switch (_family) {
            case socket::family::ipv4:
                return "0.0.0.0";

            case socket::family::ipv6:
                return "::";

            default:
                return nullptr;
        }
    }


    template <typename LogPtr>
    inline socket::kind basic_socket<LogPtr>::kind() const noexcept {
        return _kind;
    }


    template <typename LogPtr>
    inline socket::family basic_socket<LogPtr>::family() const noexcept {
        return _family;
    }


    template <typename LogPtr>
    inline socket::protocol basic_socket<LogPtr>::protocol() const noexcept {
        return _protocol;
    }


    template <typename LogPtr>
    inline socket::fd_t basic_socket<LogPtr>::fd() const noexcept {
        return _fd;
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline client_socket<LogPtr>::client_socket(const char* origin, socket::kind kind, socket::family family, const LogPtr& log)
        : basic_socket<LogPtr>(origin, kind, family, log) {
    }


    template <typename LogPtr>
    inline client_socket<LogPtr>::client_socket(const char* origin, socket::fd_t fd, socket::kind kind, socket::family family, const LogPtr& log)
        : basic_socket<LogPtr>(origin, fd, kind, family, log) {
    }


    template <typename LogPtr>
    inline void client_socket<LogPtr>::connect(const char* host, const char* port) {
        base::tie(host, port, socket::tie::connect);
    }


    template <typename LogPtr>
    inline void client_socket<LogPtr>::connect(const socket::address& address) {
        base::tie(address, socket::tie::connect);
    }


    template <typename LogPtr>
    inline std::size_t client_socket<LogPtr>::send(const void* buffer, std::size_t size, socket::address* address) {
        constexpr const char* suborigin = "send()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: size=%zu", size);

        diag_base::expect(suborigin, base::is_open(), 0x10017, "is_open");
        diag_base::expect(suborigin, address == nullptr || base::kind() == socket::kind::dgram, 0x10018, "!address || dgram");

        ssize_t sent_size;
        if (address != nullptr) {
            // dgram
            sent_size = ::sendto(base::fd(), buffer, size, 0, &address->value, address->size);
        }
        else {
            // stream
            sent_size = ::send(base::fd(), buffer, size, 0);
        }

        if (sent_size < 0) {
            diag_base::put_any(suborigin, diag::severity::important, 0x1043f, "sent_size=%ld", (long)sent_size);

            sent_size = 0;
        }
        else if ((std::size_t)sent_size < size) {
            diag_base::put_any(suborigin, diag::severity::important, 0x10440, "sent_size=%ld", (long)sent_size);
        }

        diag_base::put_binary(suborigin, diag::severity::verbose, 0x10066, buffer, size);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: size=%zu, sent_size=%ld", size, (long)sent_size);

        return sent_size;
    }


    template <typename LogPtr>
    inline std::size_t client_socket<LogPtr>::receive(void* buffer, std::size_t size, socket::address* address) {
        constexpr const char* suborigin = "receive()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: size=%zu", size);

        diag_base::expect(suborigin, base::is_open(), 0x1001d, "is_open");
        diag_base::expect(suborigin, address == nullptr || base::kind() == socket::kind::dgram, 0x1001e, "!address || dgram");

        ssize_t received_size;
        if (address != nullptr) {
            // dgram
            received_size = ::recvfrom(base::fd(), buffer, size, 0, &address->value, &address->size);
        }
        else {
            // stream
            received_size = ::recv(base::fd(), buffer, size, 0);
        }

        if (received_size < 0) {
            diag_base::put_any(suborigin, diag::severity::important, 0x10441, "received_size=%ld", (long)received_size);

            received_size = 0;
        }
        else if ((std::size_t)received_size < size) {
            diag_base::put_any(suborigin, diag::severity::important, 0x10442, "size=%zu, received_size=%ld", size, (long)received_size);
        }

        diag_base::put_binary(suborigin, diag::severity::verbose, 0x10067, buffer, size);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: size=%zu, received_size=%ld", size, (long)received_size);

        return received_size;
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline udp_socket<LogPtr>::udp_socket(socket::family family, const LogPtr& log)
        : client_socket<LogPtr>("abc::net::udp_socket", socket::kind::dgram, family, log) {
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline tcp_client_socket<LogPtr>::tcp_client_socket(socket::family family, const LogPtr& log)
        : client_socket<LogPtr>("abc::net::tcp_client_socket", socket::kind::stream, family, log) {
    }


    template <typename LogPtr>
    inline tcp_client_socket<LogPtr>::tcp_client_socket(socket::fd_t fd, socket::family family, const LogPtr& log)
        : client_socket<LogPtr>("abc::net::tcp_client_socket", fd, socket::kind::stream, family, std::move(log)) {
    }


    // --------------------------------------------------------------


    template <typename LogPtr>
    inline tcp_server_socket<LogPtr>::tcp_server_socket(socket::family family, const LogPtr& log)
        : basic_socket<LogPtr>("abc::net::tcp_server_socket", socket::kind::stream, family, log) {
    }


    template <typename LogPtr>
    inline void tcp_server_socket<LogPtr>::listen(socket::backlog_size_t backlog_size) {
        constexpr const char* suborigin = "listen()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10022, "Begin:");

        diag_base::expect(suborigin, base::is_open(), __TAG__, "is_open");

        socket::error_t err = ::listen(base::fd(), backlog_size);

        if (err != socket::error::none) {
            diag_base::template throw_exception<std::runtime_error>(suborigin, __TAG__, "::listen() err=%d", err);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10024, "End:");
    }


    template <typename LogPtr>
    inline tcp_client_socket<LogPtr> tcp_server_socket<LogPtr>::accept() const {
        socket::fd_t fd = accept_fd();

        return tcp_client_socket<LogPtr>(fd, base::family(), base::log());
    }


    template <typename LogPtr>
    inline socket::fd_t tcp_server_socket<LogPtr>::accept_fd() const {
        constexpr const char* suborigin = "accept_fd()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10025, "Begin:");

        socket::fd_t fd = ::accept(base::fd(), nullptr, nullptr);

        if (fd == socket::fd::invalid) {
            diag_base::template throw_exception<std::runtime_error>(suborigin, 0x10026, "::accept()");
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10027, "End:");

        return fd;
    }


    // --------------------------------------------------------------


    template <typename SocketPtr, typename LogPtr>
    inline socket_streambuf<SocketPtr, LogPtr>::socket_streambuf(const SocketPtr& socket, const LogPtr& log)
        : base()
        , diag_base("abc::net::socket_streambuf", log)
        , _socket(socket) {

        constexpr const char* suborigin = "socket_streambuf()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::expect(suborigin, socket != nullptr, 0x10068, "socket");

        setg(&_get_ch, &_get_ch, &_get_ch);
        setp(&_put_ch, &_put_ch + 1);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename Socket, typename Log>
    inline socket_streambuf<Socket, Log>::socket_streambuf(socket_streambuf&& other) noexcept
        : base()
        , diag_base(std::move(other))
        , _socket(std::move(other._socket))
        , _get_ch(other._get_ch)
        , _put_ch(other._put_ch) {

        constexpr const char* suborigin = "socket_streambuf()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        setg(&_get_ch, &_get_ch, &_get_ch);
        setp(&_put_ch, &_put_ch + 1);

        other._socket = nullptr;
        other.setg(nullptr, nullptr, nullptr);
        other.setp(nullptr, nullptr);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename Socket, typename Log>
    inline std::streambuf::int_type socket_streambuf<Socket, Log>::underflow() {
        _socket->receive(&_get_ch, sizeof(char));

        setg(&_get_ch, &_get_ch, &_get_ch + 1);

        return _get_ch;
    }


    template <typename Socket, typename Log>
    inline std::streambuf::int_type socket_streambuf<Socket, Log>::overflow(std::streambuf::int_type ch) {
        _socket->send(&_put_ch, sizeof(char));
        _socket->send(&ch, sizeof(char));

        setp(&_put_ch, &_put_ch + 1);

        return ch;
    }


    template <typename Socket, typename Log>
    inline int socket_streambuf<Socket, Log>::sync() {
        if (pptr() != &_put_ch) {
            _socket->send(&_put_ch, sizeof(char));
        }

        setp(&_put_ch, &_put_ch + 1);

        return 0;
    }

} }
