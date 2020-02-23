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

#include <cstdint>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

namespace abc {

	class udp_client_socket;
	class tcp_client_socket;
	class udp_server_socket;
	class tcp_server_socket;


	namespace socket {
		typedef int				kind_t;
		typedef int				family_t;
		typedef int				protocol_t;
		typedef int				handle_t;
		typedef std::uint8_t	purpose_t;
		typedef int				backlog_size_t;


		namespace kind {
			constexpr kind_t	stream	= SOCK_STREAM;
			constexpr kind_t	dgram	= SOCK_DGRAM;
		}


		namespace family {
			constexpr family_t	ipv4	= AF_INET;
			constexpr family_t	ipv6	= AF_INET6;
		}


		namespace protocol {
			constexpr protocol_t	tcp	= IPPROTO_TCP;
			constexpr protocol_t	udp	= IPPROTO_UDP;
		}


		namespace purpose {
			constexpr purpose_t	client	= 0;
			constexpr purpose_t	server	= 1;
		}


		namespace handle {
			constexpr handle_t	invalid	= -1;
		}
	}


	// --------------------------------------------------------------


	class _basic_socket {
	public:
		_basic_socket(socket::kind_t kind, socket::family_t family, socket::purpose_t purpose);
		_basic_socket(_basic_socket&& other) noexcept = default;

	public:
		~_basic_socket() noexcept;

	protected:
		_basic_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family, socket::purpose_t purpose);

	public:
		bool		is_opened() const noexcept;
		void		close() noexcept;

	protected:
		void				open() noexcept;
		addrinfo			hints() const noexcept;

	protected:
		socket::kind_t		kind() const noexcept;
		socket::family_t	family() const noexcept;
		socket::protocol_t	protocol() const noexcept;
		socket::purpose_t	purpose() const noexcept;
		socket::handle_t	handle() const noexcept;

	private:
		socket::kind_t		_kind;
		socket::family_t	_family;
		socket::protocol_t	_protocol;
		socket::purpose_t	_purpose;
		socket::handle_t	_handle;
	};


	class _connected_socket : public _basic_socket {
	public:
		_connected_socket(socket::kind_t kind, socket::family_t family);
		_connected_socket(_connected_socket&& other) noexcept = default;

	protected:
		_connected_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family);

	public:
		void send(const void* buffer, std::size_t byte_count);
		void receive(void* buffer, std::size_t byte_count);

		////void send_async();
		////void async_async();
	};


	class _client_socket : public _connected_socket {
	public:
		_client_socket(socket::kind_t kind, socket::family_t family);
		_client_socket(_client_socket&& other) noexcept = default;

	protected:
		_client_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family);

	public:
		void connect(const char* host, const char* port);

		////void connect_async();
	};


	class udp_client_socket : public _client_socket {
	public:
		udp_client_socket(socket::family_t family = socket::family::ipv4);
		udp_client_socket(udp_client_socket&& other) noexcept = default;
	};


	class tcp_client_socket : public _client_socket {
	public:
		tcp_client_socket(socket::family_t family = socket::family::ipv4);
		tcp_client_socket(tcp_client_socket&& other) noexcept = default;

	protected:
		friend class tcp_server_socket;
		tcp_client_socket(socket::handle_t handle, socket::family_t family);
	};


	class _server_socket : public _basic_socket {
	public:
		_server_socket(socket::kind_t, socket::family_t family);
		_server_socket(_server_socket&& other) noexcept = default;

	public:
		void bind(const char* port);
	};


	class udp_server_socket : public _server_socket {
	public:
		udp_server_socket(socket::family_t family = socket::family::ipv4);
		udp_server_socket(udp_server_socket&& other) noexcept = default;
	};


	class tcp_server_socket : public _server_socket {
	public:
		tcp_server_socket(socket::family_t family = socket::family::ipv4);
		tcp_server_socket(tcp_server_socket&& other) noexcept = default;

	public:
		void				listen(socket::backlog_size_t backlog_size);
		tcp_client_socket	accept() const;
	};

}
