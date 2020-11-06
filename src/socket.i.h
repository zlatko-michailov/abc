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
#include <streambuf>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include "log.i.h"


namespace abc {

	namespace socket {
		using kind_t = int;

		namespace kind {
			constexpr kind_t	stream	= SOCK_STREAM;
			constexpr kind_t	dgram	= SOCK_DGRAM;
		}


		using family_t = int;

		namespace family {
			constexpr family_t	ipv4	= AF_INET;
			constexpr family_t	ipv6	= AF_INET6;
		}


		using protocol_t = int;

		namespace protocol {
			constexpr protocol_t	tcp	= IPPROTO_TCP;
			constexpr protocol_t	udp	= IPPROTO_UDP;
		}


		using handle_t = int;

		namespace handle {
			constexpr handle_t	invalid	= -1;
		}


		using error_t = int;

		namespace error {
			constexpr error_t	none	=  0;
			constexpr error_t	any		= -1;
		}


		using tie_t = std::uint8_t;

		namespace tie {
			constexpr tie_t		bind	= 1;
			constexpr tie_t		connect	= 2;
		}


		struct address {
			sockaddr		value;
			socklen_t		size = sizeof(sockaddr);
		};

		using backlog_size_t = int;
	}


	// --------------------------------------------------------------


	template <typename Log>
	class _basic_socket {
	protected:
		_basic_socket(socket::kind_t kind, socket::family_t family, Log* log);
		_basic_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family, Log* log);
		_basic_socket(_basic_socket&& other) noexcept;
		_basic_socket(const _basic_socket& other) = delete;

		~_basic_socket() noexcept;

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
		const char*			any_host() const noexcept;
		socket::kind_t		kind() const noexcept;
		socket::family_t	family() const noexcept;
		socket::protocol_t	protocol() const noexcept;
		socket::handle_t	handle() const noexcept;
		Log*				log() const noexcept;

	private:
		socket::kind_t		_kind;
		socket::family_t	_family;
		socket::protocol_t	_protocol;
		socket::handle_t	_handle;
		Log*				_log;
	};


	// --------------------------------------------------------------


	template <typename Log>
	class _client_socket : public _basic_socket<Log> {
		using base = _basic_socket<Log>;

	protected:
		_client_socket(socket::kind_t kind, socket::family_t family, Log* log);
		_client_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family, Log* log);
		_client_socket(_client_socket&& other) noexcept = default;
		_client_socket(const _client_socket& other) = delete;

	public:
		void connect(const char* host, const char* port);
		void connect(const socket::address& address);

		void send(const void* buffer, std::size_t size, socket::address* address = nullptr);
		void receive(void* buffer, std::size_t size, socket::address* address = nullptr);
	};


	// --------------------------------------------------------------


	template <typename Log = null_log>
	class udp_socket : public _client_socket<Log> {
		using base = _client_socket<Log>;

	public:
		udp_socket(socket::family_t family = socket::family::ipv4, Log* log = nullptr);
		udp_socket(Log* log);
		udp_socket(udp_socket&& other) noexcept = default;
		udp_socket(const udp_socket& other) = delete;
	};


	// --------------------------------------------------------------


	template <typename Log>
	class tcp_server_socket;


	template <typename Log = null_log>
	class tcp_client_socket : public _client_socket<Log> {
		using base = _client_socket<Log>;

	public:
		tcp_client_socket(socket::family_t family = socket::family::ipv4, Log* log = nullptr);
		tcp_client_socket(Log* log);
		tcp_client_socket(tcp_client_socket&& other) noexcept = default;
		tcp_client_socket(const tcp_client_socket& other) = delete;

	protected:
		friend class tcp_server_socket<Log>;
		tcp_client_socket(socket::handle_t handle, socket::family_t family, Log* log);
	};


	// --------------------------------------------------------------


	template <typename Log = null_log>
	class tcp_server_socket : public _basic_socket<Log> {
		using base = _basic_socket<Log>;

	public:
		tcp_server_socket(socket::family_t family = socket::family::ipv4, Log* log = nullptr);
		tcp_server_socket(Log* log);
		tcp_server_socket(tcp_server_socket&& other) noexcept = default;
		tcp_server_socket(const tcp_server_socket& other) = delete;

	public:
		void					listen(socket::backlog_size_t backlog_size);
		tcp_client_socket<Log>	accept() const;
	};


	// --------------------------------------------------------------


	template <typename Socket, typename Log = null_log>
	class socket_streambuf : public std::streambuf {
		using base = std::streambuf;

	public:
		socket_streambuf(Socket* socket, Log* log = nullptr);

	protected:
		virtual int_type	underflow() override;
		virtual int_type	overflow(int_type ch) override;
		virtual int			sync() override;

	private:
		Socket*		_socket;
		Log*		_log;
		char		_get_ch;
		char		_put_ch;
	};

}
