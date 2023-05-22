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

#include <cstdint>
#include <openssl/ssl.h>

#include "../size.h"
#include "socket.i.h"
#include "log.i.h"


namespace abc {

	template <typename Log>
	class openssl_tcp_server_socket;


	/**
	 * @brief					TCP client socket functionality with OpenSSL encryption.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log = null_log>
	class openssl_tcp_client_socket : public tcp_client_socket<Log> {
		using base = tcp_client_socket<Log>;

	public:
		/**
		 * @brief				Constructor.
		 * @param verify_server	Allows the client to accept self-signed certificates.
		 * @param family		IPv4 or IPv6.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		openssl_tcp_client_socket(bool verify_server = true, socket::family_t family = socket::family::ipv4, Log* log = nullptr);

		/**
		 * @brief				Constructor.
		 * @details				Security is enabled and IPv4 is assumed.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		openssl_tcp_client_socket(Log* log);

		/**
		 * @brief				Move constructor.
		 */
		openssl_tcp_client_socket(openssl_tcp_client_socket&& other) noexcept;

		/**
		 * @brief				Deleted.
		 */
		openssl_tcp_client_socket(const openssl_tcp_client_socket& other) = delete;

		/**
		 * @brief				Destructor.
		 */
		virtual ~openssl_tcp_client_socket() noexcept;

	public:
		/**
		 * @brief				Connects the socket to the given port on the given host name.
		 * @param host			Host name.
		 * @param port			Port number (as a string).
		 */
		void connect(const char* host, const char* port);

		/**
		 * @brief				Connects the socket to the given address.
		 * @param address		Address.
		 */
		void connect(const socket::address& address);

		/**
		 * @brief				Sends the bytes from the buffer into the socket.
		 * @param buffer		Data buffer. 
		 * @param size			Buffer size.
		 * @return				The number of bytes sent. `0` = error.
		 */
		std::size_t send(const void* buffer, std::size_t size);

		/**
		 * @brief				Receives the given number of bytes from the socket, and stores them into the buffer.
		 * @param buffer		Data buffer. 
		 * @param size			Buffer size.
		 * @return				The number of bytes received. `0` = error.
		 */
		std::size_t receive(void* buffer, std::size_t size);

	protected:
		friend openssl_tcp_server_socket<Log>;

		/**
		 * @brief				Internal constructor for accepted connections.
		 * @param fd			Descriptor.
		 * @param ctx			OpenSSL server context.
		 * @param verify_server	Allows the client to accept self-signed certificates.
		 * @param family		IPv4 or IPv6.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		openssl_tcp_client_socket(socket::fd_t fd, SSL_CTX* ctx, bool verify_server, socket::family_t family, Log* log);

		/**
		 * @brief				Does the TLS handshake after the base socket has been connected.
		 */
		void connect_handshake();

	private:
		/**
		 * @brief				Whether full security is enabled.
		 */
		bool _verify_server = true;

		/**
		 * @brief				OpenSSL context.
		 */
		SSL_CTX* _ctx = nullptr;

		/**
		 * @brief				OpenSSL state specific to this connection.
		 */
		SSL* _ssl = nullptr;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					TCP server socket functionality with OpenSSL encryption.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log = null_log>
	class openssl_tcp_server_socket : public tcp_server_socket<Log> {
		using base = tcp_server_socket<Log>;

	public:
		/**
		 * @brief				Constructor.
		 * @param cert_file_path Path to the certificate file. This is most typically public/unencrypted.
		 * @param pkey_file_path Path to the private key file. This is typically password-encrypted.
		 * @param pkey_file_password Password for the private key file.
		 * @param verify_client	Allows the server to require client a certificate.
		 * @param family		IPv4 or IPv6.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		openssl_tcp_server_socket(const char* cert_file_path, const char* pkey_file_path, const char* pkey_file_password, bool verify_client = false, socket::family_t family = socket::family::ipv4, Log* log = nullptr);

		/**
		 * @brief				Constructor.
		 * @details				IPv4 is assumed.
		 * @param cert_file_path Path to the certificate file. This is most typically public/unencrypted.
		 * @param pkey_file_path Path to the private key file. This is typically password-encrypted.
		 * @param pkey_file_password Password for the private key file.
		 * @param verify_client	Allows the server to require client a certificate.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		openssl_tcp_server_socket(const char* cert_file_path, const char* pkey_file_path, const char* pkey_file_password, bool verify_client, Log* log);

		/**
		 * @brief				Move constructor.
		 */
		openssl_tcp_server_socket(openssl_tcp_server_socket&& other) noexcept;

		/**
		 * @brief				Deleted.
		 */
		openssl_tcp_server_socket(const openssl_tcp_server_socket& other) = delete;

		/**
		 * @brief				Destructor.
		 */
		virtual ~openssl_tcp_server_socket() noexcept;

	public:
		/**
		 * @brief				Blocks until a client tries to connect.
		 * @return				New `openssl_tcp_client_socket` instance for the new connection.
		 */
		openssl_tcp_client_socket<Log> accept() const;

	private:
		/**
		 * @brief				Callback passed to `SSL_CTX_set_default_passwd_cb()`.
		 */
		static int pem_passwd_cb(char* buf, int size, int rwflag, void* password) noexcept;

	private:
		static constexpr std::size_t max_password = size::_256;

		/**
		 * @brief				Password for the private key file.
		 */
		char _pkey_file_password[max_password + 1];

		/**
		 * @brief				Allows the server to enforce security, e.g. to require client a certificate.
		 */
		bool _verify_client = false;

		/**
		 * @brief				OpenSSL context.
		 */
		SSL_CTX* _ctx = nullptr;
	};

}
