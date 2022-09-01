/*
MIT License

Copyright (c) 2018-2022 Zlatko Michailov 

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


		/**
		 * @brief				Convenience wrapper around `sockaddr`.
		 */
		struct address {
			sockaddr			value;
			socklen_t			size = sizeof(sockaddr);
		};

		using backlog_size_t = int;
	}


	// --------------------------------------------------------------


	/**
	 * @brief					Common socket functionality. Not directly constructable.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log>
	class basic_socket {
	protected:
		/**
		 * @brief				Constructor.
		 * @param kind			Stream or datagram.
		 * @param family		IPv4 or IPv6.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		basic_socket(socket::kind_t kind, socket::family_t family, Log* log);

		/**
		 * @brief				Constructor.
		 * @param handle		Socket handle.
		 * @param kind			Stream or datagram.
		 * @param family		IPv4 or IPv6.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		basic_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family, Log* log);

		/**
		 * @brief				Move constructor.
		 */
		basic_socket(basic_socket&& other) noexcept;

		/**
		 * @brief				Deleted.
		 */
		basic_socket(const basic_socket& other) = delete;

		/**
		 * @brief				Destructor.
		 */
		~basic_socket() noexcept;

	public:
		/**
		 * @brief				Returns whether the socket is open.
		 */
		bool is_open() const noexcept;

		/**
		 * @brief				Closes the socket.
		 */
		void close() noexcept;

		/**
		 * @brief				Binds the socket to the given port on all host names.
		 * @param port			Port number (as a string).
		 */
		void bind(const char* port);

		/**
		 * @brief				Binds the socket to the given port on the given host name.
		 * @param host			Host name.
		 * @param port			Port number (as a string).
		 */
		void bind(const char* host, const char* port);

	protected:
		/**
		 * @brief				Opens the socket.
		 */
		void open();

		/**
		 * @brief				Returns the hints needed to obtain the host list.
		 */
		addrinfo hints() const noexcept;

		/**
		 * @brief				Binds or connects the socket to the given host and port.
		 * @param host			Host name.
		 * @param port			Port number (as a string).
		 * @param tt			Bind or connect.
		 */
		void tie(const char* host, const char* port, socket::tie_t tt);

		/**
		 * @brief				Binds or connects the socket to the given address.
		 * @param address		Address.
		 * @param tt			Bind or connect.
		 */
		void tie(const socket::address& address, socket::tie_t tt);

	private:
		/**
		 * @brief				Binds or connects the socket to the given address. (Low level.)
		 * @param addr			Low level address.
		 * @param addr_length	Size of the low level address struct.
		 * @param tt 
		 * @return				`0` = success. `1` = error.
		 */
		socket::error_t tie(const sockaddr& addr, socklen_t addr_length, socket::tie_t tt);

	protected:
		/**
		 * @brief				Returns a family-specific representation of any host.
		 */
		const char* any_host() const noexcept;

		/**
		 * @brief				Returns the socket kind - stream or datagram.
		 */
		socket::kind_t kind() const noexcept;

		/**
		 * @brief				Returns the socket family - IPv4 or IPv6.
		 */
		socket::family_t family() const noexcept;

		/**
		 * @brief				Returns the socket protocol - TCP or UDP.
		 */
		socket::protocol_t protocol() const noexcept;

		/**
		 * @brief				Returns the socket handle.
		 */
		socket::handle_t handle() const noexcept;

		/**
		 * @brief				Returns the Log pointer.
		 */
		Log* log() const noexcept;

	private:
		/**
		 * @brief				The socket kind passed in to the constructor - stream or datagram.
		 */
		socket::kind_t _kind;

		/**
		 * @brief				The socket family passed in to the constructor - IPv4 or IPv6.
		 */
		socket::family_t _family;

		/**
		 * @brief				The socket protocol passed in to the constructor - TCP or UDP.
		 */
		socket::protocol_t _protocol;

		/**
		 * @brief				The socket handle.
		 */
		socket::handle_t _handle;

		/**
		 * @brief				The Log pointer passed in to the constructor.
		 */
		Log* _log;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					Client (data transfer) socket functionality. Not directly constructable.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log>
	class client_socket : public basic_socket<Log> {
		using base = basic_socket<Log>;

	protected:
		/**
		 * @brief				Constructor.
		 * @param kind			Stream or datagram.
		 * @param family		IPv4 or IPv6.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		client_socket(socket::kind_t kind, socket::family_t family, Log* log);

		/**
		 * @brief				Constructor.
		 * @param handle		Socket handle.
		 * @param kind			Stream or datagram.
		 * @param family		IPv4 or IPv6.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		client_socket(socket::handle_t handle, socket::kind_t kind, socket::family_t family, Log* log);

		/**
		 * @brief				Move constructor.
		 */
		client_socket(client_socket&& other) noexcept = default;

		/**
		 * @brief				Deleted.
		 */
		client_socket(const client_socket& other) = delete;

	public:
		/**
		 * @brief				Connects the socket to the given port on the given host name. Optional for UDP sockets.
		 * @param host			Host name.
		 * @param port			Port number (as a string).
		 */
		void connect(const char* host, const char* port);

		/**
		 * @brief				Connects the socket to the given address. Optional for UDP sockets.
		 * @param address		Address.
		 */
		void connect(const socket::address& address);

		/**
		 * @brief				Sends the bytes from the buffer into the socket.
		 * @param buffer		Data buffer. 
		 * @param size			Buffer size.
		 * @param address		Remote address. Only needed for UDP sockets if `connect()` wasn't called.
		 * @return				The number of bytes sent. `0` = error.
		 */
		std::size_t send(const void* buffer, std::size_t size, socket::address* address = nullptr);

		/**
		 * @brief				Receives the given number of bytes from the socket, and stores them into the buffer.
		 * @param buffer		Data buffer. 
		 * @param size			Buffer size.
		 * @param address		Remote address. Only needed for UDP sockets if `connect()` wasn't called.
		 * @return				The number of bytes received. `0` = error.
		 */
		std::size_t receive(void* buffer, std::size_t size, socket::address* address = nullptr);
	};


	// --------------------------------------------------------------


	/**
	 * @brief					UDP socket functionality.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log = null_log>
	class udp_socket : public client_socket<Log> {
		using base = client_socket<Log>;

	public:
		/**
		 * @brief				Constructor.
		 * @param family		IPv4 or IPv6.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		udp_socket(socket::family_t family = socket::family::ipv4, Log* log = nullptr);

		/**
		 * @brief				Constructor.
		 * @details				IPv4 is assumed.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		udp_socket(Log* log);

		/**
		 * @brief				Move constructor.
		 */
		udp_socket(udp_socket&& other) noexcept = default;

		/**
		 * @brief				Deleted.
		 */
		udp_socket(const udp_socket& other) = delete;
	};


	// --------------------------------------------------------------


	template <typename Log>
	class tcp_server_socket;


	/**
	 * @brief					TCP client socket functionality.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log = null_log>
	class tcp_client_socket : public client_socket<Log> {
		using base = client_socket<Log>;

	public:
		/**
		 * @brief				Constructor.
		 * @param family		IPv4 or IPv6.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		tcp_client_socket(socket::family_t family = socket::family::ipv4, Log* log = nullptr);

		/**
		 * @brief				Constructor.
		 * @details				IPv4 is assumed.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		tcp_client_socket(Log* log);

		/**
		 * @brief				Move constructor.
		 */
		tcp_client_socket(tcp_client_socket&& other) noexcept = default;

		/**
		 * @brief				Deleted.
		 */
		tcp_client_socket(const tcp_client_socket& other) = delete;

	protected:
		friend tcp_server_socket<Log>;

		/**
		 * @brief				Internal constructor for accepted connections.
		 * @param handle		Handle
		 * @param family		IPv4 or IPv6.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		tcp_client_socket(socket::handle_t handle, socket::family_t family, Log* log);
	};


	// --------------------------------------------------------------


	/**
	 * @brief					TCP server socket functionality.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log = null_log>
	class tcp_server_socket : public basic_socket<Log> {
		using base = basic_socket<Log>;

	public:
		/**
		 * @brief				Constructor.
		 * @param family		IPv4 or IPv6.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		tcp_server_socket(socket::family_t family = socket::family::ipv4, Log* log = nullptr);

		/**
		 * @brief				Constructor.
		 * @details				IPv4 is assumed.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		tcp_server_socket(Log* log);

		/**
		 * @brief				Move constructor.
		 */
		tcp_server_socket(tcp_server_socket&& other) noexcept = default;

		/**
		 * @brief				Deleted.
		 */
		tcp_server_socket(const tcp_server_socket& other) = delete;

	public:
		/**
		 * @brief				Starts listening.
		 * @param backlog_size	Queue size.
		 */
		void listen(socket::backlog_size_t backlog_size);

		/**
		 * @brief				Blocks until a client tries to connect.
		 * @return				New `tcp_client_socket` instance for the new connection.
		 */
		tcp_client_socket<Log> accept() const;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					`std::streambuf` specialization that is backed by a socket.
	 * @tparam Socket			`client_socket`.
	 * @tparam Log				Logging facility.
	 */
	template <typename Socket, typename Log = null_log>
	class socket_streambuf : public std::streambuf {
		using base = std::streambuf;

	public:
		/**
		 * @brief				Constructor.
		 * @param socket		Pointer to a `client_socket` instance.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		socket_streambuf(Socket* socket, Log* log = nullptr);

		/**
		 * @brief				Move constructor.
		 */
		socket_streambuf(socket_streambuf&& other) noexcept;

		/**
		 * @brief				Deleted.
		 */
		socket_streambuf(const socket_streambuf& other) = delete;

	protected:
		/**
		 * @brief				Handler that reads a byte from the socket.
		 * @return				The byte received.
		 */
		virtual int_type underflow() override;

		/**
		 * @brief				Handler that sends a byte to the socket.
		 * @param ch			Byte to be sent.
		 * @return				`ch`
		 */
		virtual int_type overflow(int_type ch) override;

		/**
		 * @brief				Flushes.
		 * @return				`0`
		 */
		virtual int sync() override;

	private:
		/**
		 * @brief				The `client_socket` pointer passed in to the constructor.
		 */
		Socket* _socket;

		/**
		 * @brief				The Log pointer passed in to the constructor.
		 */
		Log* _log;

		/**
		 * @brief				Cached char received.
		 */
		char _get_ch;

		/**
		 * @brief				Cached char to be sent.
		 */
		char _put_ch;
	};

}
