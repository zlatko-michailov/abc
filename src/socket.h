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

	inline _basic_socket::_basic_socket(socket::kind_t kind, socket::family_t family, socket::purpose_t purpose)
		: _basic_socket(socket::handle::invalid, kind, family, purpose) {
	}

	inline _basic_socket::_basic_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family, socket::purpose_t purpose)
		: _handle(handle)
		, _kind(kind)
		, _family(family)
		, _protocol(kind == socket::kind::stream ? socket::protocol::tcp : socket::protocol::udp)
		, _purpose(purpose) {
		if (kind != socket::kind::stream && kind != socket::kind::dgram) {
			throw exception<std::logic_error>("kind", __TAG__);
		}

		if (family != socket::family::ipv4 && family != socket::family::ipv6) {
			throw exception<std::logic_error>("family", __TAG__);
		}

		if (purpose != socket::purpose::client && purpose != socket::purpose::server) {
			throw exception<std::logic_error>("purpose", __TAG__);
		}
	}


	inline _basic_socket::~_basic_socket() noexcept {
		std::cout << "Destroying handle=" << _handle << std::endl;
		close();
	}


	inline bool _basic_socket::is_opened() const noexcept {
		return _handle != socket::handle::invalid;
	}


	inline void _basic_socket::close() noexcept {
		if (is_opened()) {
			::close(_handle);
			_handle = socket::handle::invalid;
		}
	}


	inline void _basic_socket::open() noexcept {
		close();

		_handle = ::socket(_family, _kind, _protocol);
	}


	inline addrinfo	_basic_socket::hints() const noexcept {
		addrinfo hints = { 0 };

		hints.ai_family		= _family;
		hints.ai_socktype	= _kind;
		hints.ai_protocol	= _protocol;
		hints.ai_flags		= AI_CANONNAME;

		return hints;
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


	inline socket::purpose_t _basic_socket::purpose() const noexcept {
		return _purpose;
	}


	inline socket::handle_t _basic_socket::handle() const noexcept {
		return _handle;
	}


	// --------------------------------------------------------------


	inline _connected_socket::_connected_socket(socket::kind_t kind, socket::family_t family)
		: _basic_socket(kind, family, socket::purpose::client) {
	}


	inline _connected_socket::_connected_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family)
		: _basic_socket(handle, kind, family, socket::purpose::client) {
	}


	inline void _connected_socket::send(const void* buffer, std::size_t byte_count) {
		if (!is_opened()) {
			throw exception<std::logic_error>("!is_opened()", __TAG__);
		}

		ssize_t sent_byte_count = ::send(handle(), buffer, byte_count, 0);

		if (sent_byte_count < 0) {
			throw exception<std::runtime_error>("::send()", __TAG__);
		}
		else if (sent_byte_count < byte_count) {
			throw exception<std::runtime_error>("::send()", __TAG__);
		}
	}


	inline void _connected_socket::receive(void* buffer, std::size_t byte_count) {
		if (!is_opened()) {
			throw exception<std::logic_error>("!is_opened()", __TAG__);
		}

		ssize_t received_byte_count = ::recv(handle(), buffer, byte_count, 0);

		if (received_byte_count < 0) {
			throw exception<std::runtime_error>("::recv()", __TAG__);
		}
		else if (received_byte_count < byte_count) {
			throw exception<std::runtime_error>("::recv()", __TAG__);
		}
	}


	// --------------------------------------------------------------


	inline _client_socket::_client_socket(socket::kind_t kind, socket::family_t family)
		: _connected_socket(kind, family) {
	}


	inline _client_socket::_client_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family)
		: _connected_socket(handle, kind, family) {
	}


	inline void _client_socket::connect(const char* host, const char* port) {
		addrinfo hnt = hints();
		addrinfo* hostList = nullptr;

		int err = ::getaddrinfo(host, port, &hnt, &hostList);

		if (err != 0) {
			throw exception<std::runtime_error>("::getaddrinfo()", __TAG__);
		}

		for (addrinfo* host = hostList; host != nullptr; host = host->ai_next) {
			open();

			if (is_opened()) {
				err = ::connect(handle(), host->ai_addr, host->ai_addrlen);

				if (err == 0) {
					break; // Success
				}

				close();
			}
		}

		::freeaddrinfo(hostList);

		if (!is_opened()) {
			throw exception<std::runtime_error>("connect()", __TAG__);
		}
	}


	// --------------------------------------------------------------


	inline udp_client_socket::udp_client_socket(socket::family_t family)
		: _client_socket(socket::kind::dgram, family) {
	}


	// --------------------------------------------------------------


	inline tcp_client_socket::tcp_client_socket(socket::family_t family)
		: _client_socket(socket::kind::stream, family) {
	}


	inline tcp_client_socket::tcp_client_socket(socket::handle_t handle, socket::family_t family)
		: _client_socket(handle, socket::kind::stream, family) {
	}


	// --------------------------------------------------------------


	inline _server_socket::_server_socket(socket::kind_t kind, socket::family_t family)
		: _basic_socket(kind, family, socket::purpose::server) {
	}


	inline void _server_socket::bind(const char* port) {
		addrinfo hnt = hints();
		addrinfo* hostList = nullptr;

		int err = ::getaddrinfo(nullptr, port, &hnt, &hostList);

		if (err != 0) {
			throw exception<std::runtime_error>("::getaddrinfo()", __TAG__);
		}

		for (addrinfo* host = hostList; host != nullptr; host = host->ai_next) {
			open();

			if (is_opened()) {
				err = ::bind(handle(), host->ai_addr, host->ai_addrlen);

				if (err == 0) {
					break; // Success
				}

				close();
			}
		}

		::freeaddrinfo(hostList);

		if (!is_opened()) {
			throw exception<std::runtime_error>("bind()", __TAG__);
		}
	}


	// --------------------------------------------------------------


	inline udp_server_socket::udp_server_socket(socket::family_t family)
		: _server_socket(socket::kind::dgram, family) {
	}


	// --------------------------------------------------------------


	inline tcp_server_socket::tcp_server_socket(socket::family_t family)
		: _server_socket(socket::kind::stream, family) {
	}


	inline void tcp_server_socket::listen(socket::backlog_size_t backlog_size) {
		int err = ::listen(handle(), backlog_size);

		if (err != 0) {
			throw exception<std::runtime_error>("::listen()", __TAG__);
		}
	}


	inline tcp_client_socket tcp_server_socket::accept() const {
		socket::handle_t hnd = ::accept(handle(), nullptr, nullptr);

		if (hnd == socket::handle::invalid) {
			throw exception<std::runtime_error>("::accept()", __TAG__);
		}

		return std::move(tcp_client_socket(hnd, family()));
	}

}
