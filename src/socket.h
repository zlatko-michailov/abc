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
#include "exception.h"


namespace abc {

	inline _basic_socket::_basic_socket(socket::kind_t kind, socket::family_t family)
		: _basic_socket(socket::handle::invalid, kind, family) {
	}

	inline _basic_socket::_basic_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family)
		: _handle(handle)
		, _kind(kind)
		, _family(family)
		, _protocol(kind == socket::kind::stream ? socket::protocol::tcp : socket::protocol::udp) {
		if (kind != socket::kind::stream && kind != socket::kind::dgram) {
			throw exception<std::logic_error>("kind", __TAG__);
		}

		if (family != socket::family::ipv4 && family != socket::family::ipv6) {
			throw exception<std::logic_error>("family", __TAG__);
		}
	}


	inline _basic_socket::~_basic_socket() noexcept {
		close();
	}


	inline bool _basic_socket::is_open() const noexcept {
		return _handle != socket::handle::invalid;
	}


	inline void _basic_socket::close() noexcept {
		if (is_open()) {
			::close(_handle);

			_handle = socket::handle::invalid;
		}
	}


	inline void _basic_socket::open() {
		close();

		_handle = ::socket(_family, _kind, _protocol);

		if (!is_open()) {
			throw exception<std::runtime_error>("::socket()", __TAG__);
		}
	}


	inline addrinfo	_basic_socket::hints() const noexcept {
		addrinfo hints = { 0 };

		hints.ai_family		= _family;
		hints.ai_socktype	= _kind;
		hints.ai_protocol	= _protocol;
		hints.ai_flags		= 0;

		return hints;
	}


	inline void _basic_socket::tie(const char* host, const char* port, socket::tie_t tt) {
		if (!is_open()) {
			open();
		}
		else if (tt == socket::tie::bind) {
			throw exception<std::runtime_error>("is_open()", __TAG__);
		}

		addrinfo hnt = hints();
		addrinfo* hostList = nullptr;

		socket::error_t err = ::getaddrinfo(host, port, &hnt, &hostList);

		if (err != socket::error::none) {
			if (tt == socket::tie::bind) {
				close();
			}

			throw exception<std::runtime_error>("::getaddrinfo()", __TAG__);
		}

		bool is_done = false;
		for (addrinfo* host = hostList; host != nullptr; host = host->ai_next) {
			err = tie(*(host->ai_addr), host->ai_addrlen, tt);

			if (err == socket::error::none) {
				is_done = true;
				break; // Success
			}
		}

		::freeaddrinfo(hostList);

		if (!is_done) {
			if (tt == socket::tie::bind) {
				close();
			}

			throw exception<std::runtime_error>("connect()", __TAG__);
		}
	}


	inline void _basic_socket::tie(const socket::address& address, socket::tie_t tt) {
		if (!is_open()) {
			open();
		}
		else if (tt == socket::tie::bind) {
			throw exception<std::runtime_error>("is_open()", __TAG__);
		}

		socket::error_t err = tie(address.value, address.size, tt);

		if (err != socket::error::none) {
			throw exception<std::runtime_error>("bind() / connect()", __TAG__);
		}
	}


	inline socket::error_t _basic_socket::tie(const sockaddr& addr, socklen_t addr_len, socket::tie_t tt) {
		if (!is_open()) {
			throw exception<std::runtime_error>("!is_open()", __TAG__);
		}

		switch(tt) {
			case socket::tie::bind:
				return ::bind(handle(), &addr, addr_len);

			case socket::tie::connect:
				return ::connect(handle(), &addr, addr_len);

			default:
				throw exception<std::logic_error>("tt", __TAG__);
		}

		return socket::error::any;
	}


	inline socket::kind_t _basic_socket::kind() const noexcept {
		return _kind;
	}


	inline socket::family_t _basic_socket::family() const noexcept {
		return _family;
	}

	inline socket::protocol_t _basic_socket::protocol() const noexcept {
		return _protocol;
	}


	inline socket::handle_t _basic_socket::handle() const noexcept {
		return _handle;
	}


	// --------------------------------------------------------------


	inline _connected_socket::_connected_socket(_basic_socket& socket) noexcept
		: _socket(socket) {
	}


	inline void _connected_socket::send(const void* buffer, std::size_t byte_count) {
		if (!_socket.is_open()) {
			throw exception<std::logic_error>("!is_open()", __TAG__);
		}

		ssize_t sent_byte_count = ::send(_socket.handle(), buffer, byte_count, 0);

		if (sent_byte_count < 0) {
			throw exception<std::runtime_error>("::send()", __TAG__);
		}
		else if (sent_byte_count < byte_count) {
			throw exception<std::runtime_error>("::send()", __TAG__);
		}
	}


	inline void _connected_socket::receive(void* buffer, std::size_t byte_count, socket::address* address) {
		if (!_socket.is_open()) {
			throw exception<std::logic_error>("!is_open()", __TAG__);
		}

		ssize_t received_byte_count;
		if (address != nullptr) {
			 received_byte_count = ::recvfrom(_socket.handle(), buffer, byte_count, 0, &address->value, &address->size);
		}
		else {
			 received_byte_count = ::recv(_socket.handle(), buffer, byte_count, 0);
		}

		if (received_byte_count < 0) {
			throw exception<std::runtime_error>("::recv()", __TAG__);
		}
		else if (received_byte_count < byte_count) {
			throw exception<std::runtime_error>("::recv()", __TAG__);
		}
	}


	// --------------------------------------------------------------


	inline _client_socket::_client_socket(_basic_socket& socket) noexcept
		: _connected_socket(socket) {
	}


	inline void _client_socket::connect(const char* host, const char* port) {
		_socket.tie(host, port, socket::tie::connect);
	}


	inline void _client_socket::connect(const socket::address& address) {
		_socket.tie(address, socket::tie::connect);
	}


	// --------------------------------------------------------------


	inline _server_socket::_server_socket(_basic_socket& socket) noexcept
		: _socket(socket) {
	}


	inline void _server_socket::bind(const char* port) {
		_socket.tie(nullptr, port, socket::tie::bind);
	}


	// --------------------------------------------------------------


	inline udp_socket::udp_socket(socket::family_t family)
		: _basic_socket(socket::kind::dgram, family)
		, _client_socket(static_cast<_basic_socket&>(*this))
		, _server_socket(static_cast<_basic_socket&>(*this)) {
	}


	// --------------------------------------------------------------


	inline udp_client_socket::udp_client_socket(socket::family_t family)
		: _basic_socket(socket::kind::dgram, family)
		, _client_socket(static_cast<_basic_socket&>(*this)) {
	}


	// --------------------------------------------------------------


	inline tcp_client_socket::tcp_client_socket(socket::family_t family)
		: _basic_socket(socket::kind::stream, family)
		, _client_socket(static_cast<_basic_socket&>(*this)) {
	}


	inline tcp_client_socket::tcp_client_socket(socket::handle_t handle, socket::family_t family)
		: _basic_socket(handle, socket::kind::dgram, family)
		, _client_socket(static_cast<_basic_socket&>(*this)) {
	}


	// --------------------------------------------------------------


	inline udp_server_socket::udp_server_socket(socket::family_t family)
		: _basic_socket(socket::kind::dgram, family)
		, _connected_socket(static_cast<_basic_socket&>(*this))
		, _server_socket(static_cast<_basic_socket&>(*this)) {
	}


	// --------------------------------------------------------------


	inline tcp_server_socket::tcp_server_socket(socket::family_t family)
		: _basic_socket(socket::kind::stream, family)
		, _server_socket(static_cast<_basic_socket&>(*this)) {
	}


	inline void tcp_server_socket::listen(socket::backlog_size_t backlog_size) {
		int err = ::listen(_socket.handle(), backlog_size);

		if (err != 0) {
			throw exception<std::runtime_error>("::listen()", __TAG__);
		}
	}


	inline tcp_client_socket tcp_server_socket::accept() const {
		socket::handle_t hnd = ::accept(_socket.handle(), nullptr, nullptr);

		if (hnd == socket::handle::invalid) {
			throw exception<std::runtime_error>("::accept()", __TAG__);
		}

		return tcp_client_socket(hnd, _socket.family());
	}

}
