/*
MIT License

Copyright (c) 2018-2020 Zlatko Michailov 

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

#include "socket.i.h"
#include "../exception.h"


namespace abc {

	template <typename LogPtr>
	inline _basic_socket<LogPtr>::_basic_socket(socket::kind_t kind, socket::family_t family, const LogPtr& log_ptr)
		: _basic_socket(socket::handle::invalid, kind, family, log_ptr) {
	}


	template <typename LogPtr>
	inline _basic_socket<LogPtr>::_basic_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family, const LogPtr& log_ptr)
		: _handle(handle)
		, _kind(kind)
		, _family(family)
		, _protocol(kind == socket::kind::stream ? socket::protocol::tcp : socket::protocol::udp)
		, _log_ptr(log_ptr) {
		if (kind != socket::kind::stream && kind != socket::kind::dgram) {
			throw exception<std::logic_error>("kind", 0x6);
		}

		if (family != socket::family::ipv4 && family != socket::family::ipv6) {
			throw exception<std::logic_error>("family", 0x7);
		}
	}


	template <typename LogPtr>
	inline _basic_socket<LogPtr>::_basic_socket(_basic_socket&& other) noexcept {
		_kind = other._kind;
		_family = other._family;
		_protocol = other._protocol;
		_handle = other._handle;
		_log_ptr = std::move(other._log_ptr);

		other._handle = socket::handle::invalid;
	}


	template <typename LogPtr>
	inline _basic_socket<LogPtr>::~_basic_socket() noexcept {
		close();
	}


	template <typename LogPtr>
	inline bool _basic_socket<LogPtr>::is_open() const noexcept {
		return _handle != socket::handle::invalid;
	}


	template <typename LogPtr>
	inline void _basic_socket<LogPtr>::close() noexcept {
		if (is_open()) {
			::close(_handle);

			_handle = socket::handle::invalid;
		}
	}


	template <typename LogPtr>
	inline void _basic_socket<LogPtr>::open() {
		close();

		_handle = ::socket(_family, _kind, _protocol);

		if (!is_open()) {
			throw exception<std::runtime_error>("::socket()", 0x8);
		}
	}


	template <typename LogPtr>
	inline addrinfo	_basic_socket<LogPtr>::hints() const noexcept {
		addrinfo hints = { 0 };

		hints.ai_family		= _family;
		hints.ai_socktype	= _kind;
		hints.ai_protocol	= _protocol;
		hints.ai_flags		= 0;

		return hints;
	}


	template <typename LogPtr>
	inline void _basic_socket<LogPtr>::bind(const char* port) {
		bind(nullptr, port);
	}


	template <typename LogPtr>
	inline void _basic_socket<LogPtr>::bind(const char* host, const char* port) {
		tie(host, port, socket::tie::bind);
	}


	template <typename LogPtr>
	inline void _basic_socket<LogPtr>::tie(const char* host, const char* port, socket::tie_t tt) {
		if (!is_open()) {
			open();
		}
		else if (tt == socket::tie::bind) {
			throw exception<std::runtime_error>("is_open()", 0x9);
		}

		addrinfo hnt = hints();
		addrinfo* hostList = nullptr;

		socket::error_t err = ::getaddrinfo(host, port, &hnt, &hostList);

		if (err != socket::error::none) {
			if (tt == socket::tie::bind) {
				close();
			}

			throw exception<std::runtime_error>("::getaddrinfo()", 0xa);
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

			throw exception<std::runtime_error>("connect()", 0xb);
		}
	}


	template <typename LogPtr>
	inline void _basic_socket<LogPtr>::tie(const socket::address& address, socket::tie_t tt) {
		if (!is_open()) {
			open();
		}
		else if (tt == socket::tie::bind) {
			throw exception<std::runtime_error>("is_open()", 0xc);
		}

		socket::error_t err = tie(address.value, address.size, tt);

		if (err != socket::error::none) {
			throw exception<std::runtime_error>("bind() / connect()", 0xd);
		}
	}


	template <typename LogPtr>
	inline socket::error_t _basic_socket<LogPtr>::tie(const sockaddr& addr, socklen_t addr_len, socket::tie_t tt) {
		if (!is_open()) {
			throw exception<std::runtime_error>("!is_open()", 0xe);
		}

		switch(tt) {
			case socket::tie::bind:
				return ::bind(handle(), &addr, addr_len);

			case socket::tie::connect:
				return ::connect(handle(), &addr, addr_len);

			default:
				throw exception<std::logic_error>("tt", 0xf);
		}

		return socket::error::any;
	}


	template <typename LogPtr>
	inline socket::kind_t _basic_socket<LogPtr>::kind() const noexcept {
		return _kind;
	}


	template <typename LogPtr>
	inline socket::family_t _basic_socket<LogPtr>::family() const noexcept {
		return _family;
	}


	template <typename LogPtr>
	inline socket::protocol_t _basic_socket<LogPtr>::protocol() const noexcept {
		return _protocol;
	}


	template <typename LogPtr>
	inline socket::handle_t _basic_socket<LogPtr>::handle() const noexcept {
		return _handle;
	}


	template <typename LogPtr>
	inline const LogPtr& _basic_socket<LogPtr>::log_ptr() const noexcept {
		return _log_ptr;
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline _client_socket<LogPtr>::_client_socket(socket::kind_t kind, socket::family_t family, const LogPtr& log_ptr)
		: _basic_socket<LogPtr>(kind, family, log_ptr) {
	}


	template <typename LogPtr>
	inline _client_socket<LogPtr>::_client_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family, const LogPtr& log_ptr)
		: _basic_socket<LogPtr>(handle, kind, family, log_ptr) {
	}


	template <typename LogPtr>
	inline void _client_socket<LogPtr>::connect(const char* host, const char* port) {
		this->tie(host, port, socket::tie::connect);
	}


	template <typename LogPtr>
	inline void _client_socket<LogPtr>::connect(const socket::address& address) {
		this->tie(address, socket::tie::connect);
	}


	template <typename LogPtr>
	inline void _client_socket<LogPtr>::send(const void* buffer, std::size_t size, socket::address* address) {
		if (!this->is_open()) {
			throw exception<std::logic_error>("!is_open()", 0x10);
		}

		ssize_t sent_size;
		if (address != nullptr) {
			if (this->kind() != socket::kind::dgram) {
				throw exception<std::logic_error>("!dgram", 0x11);
			}

			sent_size = ::sendto(this->handle(), buffer, size, 0, &address->value, address->size);
		}
		else {
			sent_size = ::send(this->handle(), buffer, size, 0);
		}

		if (sent_size < 0) {
			throw exception<std::runtime_error>("::send()", 0x12);
		}
		else if (sent_size < size) {
			throw exception<std::runtime_error>("::send()", 0x13);
		}
	}


	template <typename LogPtr>
	inline void _client_socket<LogPtr>::receive(void* buffer, std::size_t size, socket::address* address) {
		if (!this->is_open()) {
			throw exception<std::logic_error>("!is_open()", 0x14);
		}

		ssize_t received_size;
		if (address != nullptr) {
			if (this->kind() != socket::kind::dgram) {
				throw exception<std::logic_error>("!dgram", 0x15);
			}

			 received_size = ::recvfrom(this->handle(), buffer, size, 0, &address->value, &address->size);
		}
		else {
			 received_size = ::recv(this->handle(), buffer, size, 0);
		}

		if (received_size < 0) {
			throw exception<std::runtime_error>("::recv()", 0x16);
		}
		else if (received_size < size) {
			throw exception<std::runtime_error>("::recv()", 0x17);
		}
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline udp_socket<LogPtr>::udp_socket(socket::family_t family, const LogPtr& log_ptr)
		: _client_socket<LogPtr>(socket::kind::dgram, family, log_ptr) {
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline tcp_client_socket<LogPtr>::tcp_client_socket(socket::family_t family, const LogPtr& log_ptr)
		: _client_socket<LogPtr>(socket::kind::stream, family, log_ptr) {
	}


	template <typename LogPtr>
	inline tcp_client_socket<LogPtr>::tcp_client_socket(socket::handle_t handle, socket::family_t family, const LogPtr& log_ptr)
		: _client_socket<LogPtr>(handle, socket::kind::stream, family, log_ptr) {
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline tcp_server_socket<LogPtr>::tcp_server_socket(socket::family_t family, const LogPtr& log_ptr)
		: _basic_socket<LogPtr>(socket::kind::stream, family, log_ptr) {
	}


	template <typename LogPtr>
	inline void tcp_server_socket<LogPtr>::listen(socket::backlog_size_t backlog_size) {
		socket::error_t err = ::listen(this->handle(), backlog_size);

		if (err != socket::error::none) {
			throw exception<std::runtime_error>("::listen()", 0x18);
		}
	}


	template <typename LogPtr>
	inline tcp_client_socket<LogPtr> tcp_server_socket<LogPtr>::accept() const {
		socket::handle_t hnd = ::accept(this->handle(), nullptr, nullptr);

		if (hnd == socket::handle::invalid) {
			throw exception<std::runtime_error>("::accept()", 0x19);
		}

		return tcp_client_socket<LogPtr>(hnd, this->family(), this->log_ptr());
	}

}
