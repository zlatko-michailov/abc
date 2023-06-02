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

#include "exception.h"
#include "tag.h"
#include "i/socket.i.h"


namespace abc {

	template <typename Log>
	inline basic_socket<Log>::basic_socket(socket::kind_t kind, socket::family_t family, Log* log)
		: basic_socket(socket::fd::invalid, kind, family, log) {
	}


	template <typename Log>
	inline basic_socket<Log>::basic_socket(socket::fd_t fd, socket::kind_t kind, socket::family_t family, Log* log)
		: _kind(kind)
		, _family(family)
		, _protocol(kind == socket::kind::stream ? socket::protocol::tcp : socket::protocol::udp)
		, _fd(fd)
		, _log(log) {
		if (kind != socket::kind::stream && kind != socket::kind::dgram) {
			throw exception<std::logic_error, Log>("basic_socket::basic_socket(kind)", 0x10004, log);
		}

		if (family != socket::family::ipv4 && family != socket::family::ipv6) {
			throw exception<std::logic_error, Log>("basic_socket::basic_socket(family)", 0x10005, log);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::socket, severity::abc::debug, 0x10006, "basic_socket::basic_socket() %s, %s", _kind == socket::kind::stream ? "tcp" : "udp", _family == socket::family::ipv4 ? "ipv4" : "ipv6");
		}
	}


	template <typename Log>
	inline basic_socket<Log>::basic_socket(basic_socket&& other) noexcept {
		_kind = other._kind;
		_family = other._family;
		_protocol = other._protocol;
		_fd = other._fd;
		_log = std::move(other._log);

		other._fd = socket::fd::invalid;

		if (_log != nullptr) {
			_log->put_any(category::abc::socket, severity::abc::debug, 0x10007, "basic_socket::basic_socket(move) %s, %s, fd=%d", _kind == socket::kind::stream ? "tcp" : "udp", _family == socket::family::ipv4 ? "ipv4" : "ipv6", _fd);
		}
	}


	template <typename Log>
	inline basic_socket<Log>::~basic_socket() noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::socket, severity::abc::debug, 0x10008, "basic_socket::~basic_socket() %s, %s, is_open=%s", _kind == socket::kind::stream ? "tcp" : "udp", _family == socket::family::ipv4 ? "ipv4" : "ipv6", is_open() ? "true" : "false");
		}

		close();
	}


	template <typename Log>
	inline bool basic_socket<Log>::is_open() const noexcept {
		return _fd != socket::fd::invalid;
	}


	template <typename Log>
	inline void basic_socket<Log>::close() noexcept {
		if (is_open()) {
			if (_log != nullptr) {
				_log->put_any(category::abc::socket, severity::abc::debug, 0x10009, "basic_socket::close()");
			}

			::shutdown(_fd, SHUT_RDWR);
			::close(_fd);

			_fd = socket::fd::invalid;
		}
	}


	template <typename Log>
	inline void basic_socket<Log>::open() {
		if (_log != nullptr) {
			_log->put_any(category::abc::socket, severity::abc::debug, 0x1000a, "basic_socket::open() start");
		}

		close();

		_fd = ::socket(_family, _kind, _protocol);

		if (!is_open()) {
			throw exception<std::runtime_error, Log>("basic_socket::open() ::socket()", 0x1000b, _log);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::socket, severity::abc::debug, 0x1000c, "basic_socket::open() done");
		}
	}


	template <typename Log>
	inline addrinfo	basic_socket<Log>::hints() const noexcept {
		addrinfo hints{ };

		hints.ai_family		= _family;
		hints.ai_socktype	= _kind;
		hints.ai_protocol	= _protocol;
		hints.ai_flags		= 0;

		return hints;
	}


	template <typename Log>
	inline void basic_socket<Log>::bind(const char* port) {
		bind(any_host(), port);
	}


	template <typename Log>
	inline void basic_socket<Log>::bind(const char* host, const char* port) {
		tie(host, port, socket::tie::bind);
	}


	template <typename Log>
	inline void basic_socket<Log>::tie(const char* host, const char* port, socket::tie_t tt) {
		if (_log != nullptr) {
			_log->put_any(category::abc::socket, severity::abc::debug, 0x1000d, "basic_socket::tie() >>> %s", tt == socket::tie::bind ? "bind" : "connect");
		}

		if (!is_open()) {
			open();
		}
		else if (tt == socket::tie::bind) {
			throw exception<std::runtime_error, Log>("basic_socket::tie() is_open()", 0x1000e, _log);
		}

		addrinfo hnt = hints();
		addrinfo* hostList = nullptr;

		socket::error_t err = ::getaddrinfo(host, port, &hnt, &hostList);

		if (err != socket::error::none) {
			if (tt == socket::tie::bind) {
				close();
			}

			throw exception<std::runtime_error, Log>("basic_socket::tie() ::getaddrinfo()", 0x1000f, _log);
		}

		bool is_done = false;
		for (addrinfo* host = hostList; host != nullptr; host = host->ai_next) {
			err = tie(*(host->ai_addr), host->ai_addrlen, tt);

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

			throw exception<std::runtime_error, Log>("basic_socket::tie() bind()/connect()", 0x10010, _log);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::socket, severity::abc::optional, 0x10011, "basic_socket::tie() <<< %s", tt == socket::tie::bind ? "bind" : "connect");
		}
	}


	template <typename Log>
	inline void basic_socket<Log>::tie(const socket::address& address, socket::tie_t tt) {
		if (!is_open()) {
			open();
		}
		else if (tt == socket::tie::bind) {
			throw exception<std::runtime_error, Log>("basic_socket::tie() is_open()", 0x10012, _log);
		}

		socket::error_t err = tie(address.value, address.size, tt);

		if (err != socket::error::none) {
			throw exception<std::runtime_error, Log>("basic_socket::tie() bind() / connect()", 0x10013, _log);
		}
	}


	template <typename Log>
	inline socket::error_t basic_socket<Log>::tie(const sockaddr& addr, socklen_t addr_len, socket::tie_t tt) {
		if (!is_open()) {
			throw exception<std::runtime_error, Log>("basic_socket::tie() !is_open()", 0x10014, _log);
		}

		const int on = 1;

		switch(tt) {
			case socket::tie::bind:
				::setsockopt(fd(), SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

				return ::bind(fd(), &addr, addr_len);

			case socket::tie::connect:
				return ::connect(fd(), &addr, addr_len);

			default:
				throw exception<std::logic_error, Log>("basic_socket::tie(tt)", 0x10015, _log);
		}

		return socket::error::any;
	}


	template <typename Log>
	inline const char* basic_socket<Log>::any_host() const noexcept {
		switch (_family) {
			case socket::family::ipv4:
				return "0.0.0.0";

			case socket::family::ipv6:
				return "::";

			default:
				return nullptr;
		}
	}


	template <typename Log>
	inline socket::kind_t basic_socket<Log>::kind() const noexcept {
		return _kind;
	}


	template <typename Log>
	inline socket::family_t basic_socket<Log>::family() const noexcept {
		return _family;
	}


	template <typename Log>
	inline socket::protocol_t basic_socket<Log>::protocol() const noexcept {
		return _protocol;
	}


	template <typename Log>
	inline socket::fd_t basic_socket<Log>::fd() const noexcept {
		return _fd;
	}


	template <typename Log>
	inline Log* basic_socket<Log>::log() const noexcept {
		return _log;
	}


	// --------------------------------------------------------------


	template <typename Log>
	inline client_socket<Log>::client_socket(socket::kind_t kind, socket::family_t family, Log* log)
		: basic_socket<Log>(kind, family, log) {
	}


	template <typename Log>
	inline client_socket<Log>::client_socket(socket::fd_t fd, socket::kind_t kind, socket::family_t family, Log* log)
		: basic_socket<Log>(fd, kind, family, log) {
	}


	template <typename Log>
	inline void client_socket<Log>::connect(const char* host, const char* port) {
		base::tie(host, port, socket::tie::connect);
	}


	template <typename Log>
	inline void client_socket<Log>::connect(const socket::address& address) {
		base::tie(address, socket::tie::connect);
	}


	template <typename Log>
	inline std::size_t client_socket<Log>::send(const void* buffer, std::size_t size, socket::address* address) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, 0x10016, "client_socket::send() >>> size=%zu", size);
		}

		if (!base::is_open()) {
			throw exception<std::logic_error, Log>("client_socket::send() !is_open()", 0x10017, log_local);
		}

		ssize_t sent_size;
		if (address != nullptr) {
			if (base::kind() != socket::kind::dgram) {
				throw exception<std::logic_error, Log>("client_socket::send() !dgram", 0x10018, log_local);
			}

			sent_size = ::sendto(base::fd(), buffer, size, 0, &address->value, address->size);
		}
		else {
			sent_size = ::send(base::fd(), buffer, size, 0);
		}

		if (sent_size < 0) {
			if (log_local != nullptr) {
				log_local->put_any(category::abc::socket, severity::important, 0x1043f, "client_socket::send() sent_size=%l", (long)sent_size);
			}

			sent_size = 0;
		}
		else if ((std::size_t)sent_size < size) {
			if (log_local != nullptr) {
				log_local->put_any(category::abc::socket, severity::important, 0x10440, "client_socket::send() sent_size=%l", (long)sent_size);
			}
		}

		if (log_local != nullptr) {
			log_local->put_binary(category::abc::socket, severity::abc::debug, 0x10066, buffer, size);
			log_local->put_any(category::abc::socket, severity::abc::debug, 0x1001b, "client_socket::send() <<< size=%zu, sent_size=%l", size, sent_size);
		}

		return sent_size;
	}


	template <typename Log>
	inline std::size_t client_socket<Log>::receive(void* buffer, std::size_t size, socket::address* address) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, 0x1001c, "client_socket::receive() >>> size=%zu", size);
		}

		if (!base::is_open()) {
			throw exception<std::logic_error, Log>("client_socket::receive() !is_open()", 0x1001d, log_local);
		}

		ssize_t received_size;
		if (address != nullptr) {
			if (base::kind() != socket::kind::dgram) {
				throw exception<std::logic_error, Log>("client_socket::receive() !dgram", 0x1001e, log_local);
			}

			 received_size = ::recvfrom(base::fd(), buffer, size, 0, &address->value, &address->size);
		}
		else {
			 received_size = ::recv(base::fd(), buffer, size, 0);
		}

		if (received_size < 0) {
			if (log_local != nullptr) {
				log_local->put_any(category::abc::socket, severity::important, 0x10441, "client_socket::receive() received_size=%l", (long)received_size);
			}

			received_size = 0;
		}
		else if ((std::size_t)received_size < size) {
			if (log_local != nullptr) {
				log_local->put_any(category::abc::socket, severity::important, 0x10442, "client_socket::receive() received_size=%l", (long)received_size);
			}
		}

		if (log_local != nullptr) {
			log_local->put_binary(category::abc::socket, severity::abc::debug, 0x10067, buffer, size);
			log_local->put_any(category::abc::socket, severity::abc::debug, 0x10021, "client_socket::receive() <<< size=%zu, received_size=%l", size, received_size);
		}

		return received_size;
	}


	// --------------------------------------------------------------


	template <typename Log>
	inline udp_socket<Log>::udp_socket(socket::family_t family, Log* log)
		: client_socket<Log>(socket::kind::dgram, family, log) {
	}


	// --------------------------------------------------------------


	template <typename Log>
	inline tcp_client_socket<Log>::tcp_client_socket(socket::family_t family, Log* log)
		: client_socket<Log>(socket::kind::stream, family, log) {
	}


	template <typename Log>
	inline tcp_client_socket<Log>::tcp_client_socket(socket::fd_t fd, socket::family_t family, Log* log)
		: client_socket<Log>(fd, socket::kind::stream, family, log) {
	}


	// --------------------------------------------------------------


	template <typename Log>
	inline tcp_server_socket<Log>::tcp_server_socket(socket::family_t family, Log* log)
		: basic_socket<Log>(socket::kind::stream, family, log) {
	}


	template <typename Log>
	inline void tcp_server_socket<Log>::listen(socket::backlog_size_t backlog_size) {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, 0x10022, "tcp_server_socket::listen() >>>");
		}

		socket::error_t err = ::listen(base::fd(), backlog_size);

		if (err != socket::error::none) {
			throw exception<std::runtime_error, Log>("tcp_server_socket::listen() ::listen()", 0x10023, log_local);
		}

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, 0x10024, "tcp_server_socket::listen() <<<");
		}
	}


	template <typename Log>
	inline tcp_client_socket<Log> tcp_server_socket<Log>::accept() const {
		socket::fd_t fd = accept_fd();

		return tcp_client_socket<Log>(fd, base::family(), base::log());
	}


	template <typename Log>
	inline socket::fd_t tcp_server_socket<Log>::accept_fd() const {
		Log* log_local = base::log();
		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::debug, 0x10025, "tcp_server_socket::accept() >>>");
		}

		socket::fd_t fd = ::accept(base::fd(), nullptr, nullptr);

		if (fd == socket::fd::invalid) {
			throw exception<std::runtime_error, Log>("tcp_server_socket::accept() ::accept()", 0x10026, log_local);
		}

		if (log_local != nullptr) {
			log_local->put_any(category::abc::socket, severity::abc::optional, 0x10027, "tcp_server_socket::accept() <<<");
		}

		return fd;
	}


	// --------------------------------------------------------------


	template <typename Socket, typename Log>
	inline socket_streambuf<Socket, Log>::socket_streambuf(Socket* socket, Log* log)
		: base()
		, _socket(socket)
		, _log(log) {
		if (socket == nullptr) {
			throw exception<std::logic_error, Log>("socket_streambuf::socket_streambuf(socket)", 0x10068, _log);
		}

		setg(&_get_ch, &_get_ch, &_get_ch);
		setp(&_put_ch, &_put_ch + 1);
	}


	template <typename Socket, typename Log>
	inline socket_streambuf<Socket, Log>::socket_streambuf(socket_streambuf&& other) noexcept
		: base()
		, _socket(other._socket)
		, _log(other._log)
		, _get_ch(other._get_ch)
		, _put_ch(other._put_ch) {
		setg(&_get_ch, &_get_ch, &_get_ch);
		setp(&_put_ch, &_put_ch + 1);

		other._socket = nullptr;
		other.setg(nullptr, nullptr, nullptr);
		other.setp(nullptr, nullptr);
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

}
