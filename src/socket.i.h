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

	class udp_socket;
	class tcp_client_socket;
	class tcp_server_socket;


	namespace socket {
		typedef int				kind_t;
		typedef int				family_t;
		typedef int				protocol_t;
		typedef int				handle_t;
		typedef int				backlog_size_t;
		typedef int				error_t;
		typedef std::uint8_t	tie_t;


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


		namespace handle {
			constexpr handle_t	invalid	= -1;
		}


		namespace error {
			constexpr error_t	none	=  0;
			constexpr error_t	any		= -1;
		}


		namespace tie {
			constexpr tie_t		bind	= 1;
			constexpr tie_t		connect	= 2;
		}


		struct address {
			sockaddr		value;
			socklen_t		size = sizeof(sockaddr);
		};
	}


	// --------------------------------------------------------------


	class _basic_socket {
	public:
		_basic_socket(socket::kind_t kind, socket::family_t family);
		_basic_socket(_basic_socket&& other) noexcept = default;

	public:
		~_basic_socket() noexcept;

	protected:
		_basic_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family);

	public:
		bool				is_open() const noexcept;
		void				close() noexcept;
		void				bind(const char* port);
		void				bind(const char* host, const char* port);

	protected:
		void				open();
		addrinfo			hints() const noexcept;

		void				tie(const char* host, const char* port, socket::tie_t tt);
		void				tie(const socket::address& address, socket::tie_t tt);

	private:
		socket::error_t		tie(const sockaddr& addr, socklen_t addr_length, socket::tie_t tt);

	protected:
		socket::kind_t		kind() const noexcept;
		socket::family_t	family() const noexcept;
		socket::protocol_t	protocol() const noexcept;
		socket::handle_t	handle() const noexcept;

	private:
		socket::kind_t		_kind;
		socket::family_t	_family;
		socket::protocol_t	_protocol;
		socket::handle_t	_handle;
	};


	class _client_socket : public _basic_socket {
	public:
		_client_socket(socket::kind_t kind, socket::family_t family);
		_client_socket(_client_socket&& other) noexcept = default;

	public:
		void connect(const char* host, const char* port);
		void connect(const socket::address& address);

		////void connect_async();

		void send(const void* buffer, std::size_t size);
		void receive(void* buffer, std::size_t size, socket::address* address = nullptr);

		////void send_async();
		////void async_async();
	};


	class udp_socket : public _client_socket {
	public:
		udp_socket(socket::family_t family = socket::family::ipv4);
		udp_socket(udp_socket&& other) noexcept = default;
	};


	class tcp_client_socket : public _client_socket {
	public:
		tcp_client_socket(socket::family_t family = socket::family::ipv4);
		tcp_client_socket(tcp_client_socket&& other) noexcept = default;

	protected:
		friend class tcp_server_socket;
		tcp_client_socket(socket::handle_t handle, socket::family_t family);
	};


	class tcp_server_socket : public _basic_socket {
	public:
		tcp_server_socket(socket::family_t family = socket::family::ipv4);
		tcp_server_socket(tcp_server_socket&& other) noexcept = default;

	public:
		void				listen(socket::backlog_size_t backlog_size);
		tcp_client_socket	accept() const;
	};

}
