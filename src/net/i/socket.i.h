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
#include <streambuf>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include "../../diag/i/diag_ready.i.h"


namespace abc { namespace net {

    namespace socket {
        enum class kind : int {
            stream = SOCK_STREAM,
            dgram  = SOCK_DGRAM,
        };


        enum class family : int {
            ipv4 = AF_INET,
            ipv6 = AF_INET6,
        };


        enum class protocol : int {
            tcp = IPPROTO_TCP,
            udp = IPPROTO_UDP,
        };


        using fd_t = int;

        namespace fd {
            constexpr fd_t invalid = -1;
        }


        using error_t = int;

        namespace error {
            constexpr error_t none =  0;
            constexpr error_t any  = -1;
        }


        enum class tie : std::uint8_t {
            bind    = 1,
            connect = 2,
        };


        /**
         * @brief Convenience wrapper around `sockaddr`.
         */
        struct address {
            sockaddr  value;
            socklen_t size = sizeof(sockaddr);
        };

        using backlog_size_t = int;
    }


    // --------------------------------------------------------------


    /**
     * @brief         Common socket functionality. Not directly constructable.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class basic_socket
        : protected diag_ready<const char*, LogPtr>  {

    using diag_base = diag_ready<const char*, LogPtr>;

    protected:
        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param kind   Stream or datagram.
         * @param family IPv4 or IPv6.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        basic_socket(const char* origin, socket::kind kind, socket::family family, const LogPtr& log = nullptr);

        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param fd     Socket descriptor.
         * @param kind   Stream or datagram.
         * @param family IPv4 or IPv6.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        basic_socket(const char* origin, socket::fd_t fd, socket::kind kind, socket::family family, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        basic_socket(basic_socket&& other) noexcept;

        /**
         * @brief Deleted.
         */
        basic_socket(const basic_socket& other) = delete;

        /**
         * @brief Destructor.
         */
        ~basic_socket() noexcept;

    public:
        /**
         * @brief Returns whether the socket is open.
         */
        bool is_open() const noexcept;

        /**
         * @brief Closes the socket.
         */
        void close() noexcept;

        /**
         * @brief      Binds the socket to the given port on all host names.
         * @param port Port number (as a string).
         */
        void bind(const char* port);

        /**
         * @brief      Binds the socket to the given port on the given host name.
         * @param host Host name.
         * @param port Port number (as a string).
         */
        void bind(const char* host, const char* port);

    protected:
        /**
         * @brief Opens the socket.
         */
        void open();

        /**
         * @brief Returns the hints needed to obtain the host list.
         */
        addrinfo hints() const noexcept;

        /**
         * @brief      Binds or connects the socket to the given host and port.
         * @param host Host name.
         * @param port Port number (as a string).
         * @param tt   Bind or connect.
         */
        void tie(const char* host, const char* port, socket::tie tt);

        /**
         * @brief         Binds or connects the socket to the given address.
         * @param address Address.
         * @param tt      Bind or connect.
         */
        void tie(const socket::address& address, socket::tie tt);

    private:
        /**
         * @brief             Tries to bind/connect the socket to the given address. (Low level.)
         * @param addr        Low level address.
         * @param addr_length Size of the low level address struct.
         * @param tt 
         * @return            `0` = success. Otherwise = error.
         */
        socket::error_t try_tie(const sockaddr& addr, socklen_t addr_length, socket::tie tt);

    public:
        /**
         * @brief Returns the socket descriptor.
         */
        socket::fd_t fd() const noexcept;

    protected:
        /**
         * @brief Returns a family-specific representation of any host.
         */
        const char* any_host() const noexcept;

        /**
         * @brief Returns the socket kind - stream or datagram.
         */
        socket::kind kind() const noexcept;

        /**
         * @brief Returns the socket family - IPv4 or IPv6.
         */
        socket::family family() const noexcept;

        /**
         * @brief Returns the socket protocol - TCP or UDP.
         */
        socket::protocol protocol() const noexcept;

    private:
        /**
         * @brief The socket kind passed in to the constructor - stream or datagram.
         */
        socket::kind _kind;

        /**
         * @brief The socket family passed in to the constructor - IPv4 or IPv6.
         */
        socket::family _family;

        /**
         * @brief The socket protocol passed in to the constructor - TCP or UDP.
         */
        socket::protocol _protocol;

        /**
         * @brief The socket descriptor.
         */
        socket::fd_t _fd;
    };


    // --------------------------------------------------------------


    /**
     * @brief         Client (data transfer) socket functionality. Not directly constructable.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class client_socket
        : public basic_socket<LogPtr> {

        using base = basic_socket<LogPtr>;

    protected:
        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param kind   Stream or datagram.
         * @param family IPv4 or IPv6.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        client_socket(const char* origin, socket::kind kind, socket::family family, const LogPtr& log = nullptr);

        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param fd     Socket descriptor.
         * @param kind   Stream or datagram.
         * @param family IPv4 or IPv6.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        client_socket(const char* origin, socket::fd_t fd, socket::kind kind, socket::family family, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        client_socket(client_socket&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        client_socket(const client_socket& other) = delete;

    public:
        /**
         * @brief      Connects the socket to the given port on the given host name. Optional for UDP sockets.
         * @param host Host name.
         * @param port Port number (as a string).
         */
        void connect(const char* host, const char* port);

        /**
         * @brief         Connects the socket to the given address. Optional for UDP sockets.
         * @param address Address.
         */
        void connect(const socket::address& address);

        /**
         * @brief         Sends the bytes from the buffer into the socket.
         * @param buffer  Data buffer. 
         * @param size    Buffer size.
         * @param address Remote address. Only needed for UDP sockets if `connect()` wasn't called.
         * @return        The number of bytes sent. `0` = error.
         */
        std::size_t send(const void* buffer, std::size_t size, socket::address* address = nullptr);

        /**
         * @brief         Receives the given number of bytes from the socket, and stores them into the buffer.
         * @param buffer  Data buffer. 
         * @param size    Buffer size.
         * @param address Remote address. Only needed for UDP sockets if `connect()` wasn't called.
         * @return        The number of bytes received. `0` = error.
         */
        std::size_t receive(void* buffer, std::size_t size, socket::address* address = nullptr);
    };


    // --------------------------------------------------------------


    /**
     * @brief         UDP socket functionality.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class udp_socket
        : public client_socket<LogPtr> {

        using base = client_socket<LogPtr>;

    public:
        /**
         * @brief        Constructor.
         * @param family IPv4 or IPv6.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        udp_socket(socket::family family = socket::family::ipv4, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        udp_socket(udp_socket&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        udp_socket(const udp_socket& other) = delete;
    };


    // --------------------------------------------------------------


    template <typename LogPtr>
    class tcp_server_socket;


    /**
     * @brief         TCP client socket functionality.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class tcp_client_socket
        : public client_socket<LogPtr> {

        using base = client_socket<LogPtr>;

    public:
        /**
         * @brief        Constructor.
         * @param family IPv4 or IPv6.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        tcp_client_socket(socket::family family = socket::family::ipv4, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        tcp_client_socket(tcp_client_socket&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        tcp_client_socket(const tcp_client_socket& other) = delete;

    protected:
        friend tcp_server_socket<LogPtr>;

        /**
         * @brief        Internal constructor for accepted connections.
         * @param fd     Descriptor.
         * @param family IPv4 or IPv6.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        tcp_client_socket(socket::fd_t fd, socket::family family, const LogPtr& log);
    };


    // --------------------------------------------------------------


    /**
     * @brief         TCP server socket functionality.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class tcp_server_socket
        : public basic_socket<LogPtr> {

        using base = basic_socket<LogPtr>;

    public:
        /**
         * @brief        Constructor.
         * @param family IPv4 or IPv6.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        tcp_server_socket(socket::family family = socket::family::ipv4, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        tcp_server_socket(tcp_server_socket&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        tcp_server_socket(const tcp_server_socket& other) = delete;

    public:
        /**
         * @brief              Starts listening.
         * @param backlog_size Queue size.
         */
        void listen(socket::backlog_size_t backlog_size);

        /**
         * @brief  Blocks until a client tries to connect.
         * @return New `tcp_client_socket` instance for the new connection.
         */
        tcp_client_socket<LogPtr> accept() const;

    protected:
        /**
         * @brief  Blocks until a client tries to connect.
         * @return The fd of the new connection.
         */
        socket::fd_t accept_fd() const;
    };


    // --------------------------------------------------------------


    /**
     * @brief            `std::streambuf` specialization that is backed by a socket.
     * @tparam SocketPtr Pointer type to `client_socket`.
     * @tparam LogPtr    Pointer type to `log_ostream`.
     */
    template <typename SocketPtr, typename LogPtr = std::nullptr_t>
    class socket_streambuf
        : public std::streambuf
        , protected diag_ready<const char*, LogPtr> {
        
        using base = std::streambuf;
        using diag_base = diag_ready<const char*, LogPtr>;

        constexpr std::size_t retry_count = 5;

    public:
        /**
         * @brief        Constructor.
         * @param socket `client_socket` pointer.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        socket_streambuf(const SocketPtr& socket, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        socket_streambuf(socket_streambuf&& other) noexcept;

        /**
         * @brief Deleted.
         */
        socket_streambuf(const socket_streambuf& other) = delete;

    protected:
        /**
         * @brief  Handler that reads a byte from the socket.
         * @return The byte received.
         */
        virtual int_type underflow() override;

        /**
         * @brief    Handler that sends a byte to the socket.
         * @param ch Byte to be sent.
         * @return   `ch`
         */
        virtual int_type overflow(int_type ch) override;

        /**
         * @brief  Flushes.
         * @return `0`
         */
        virtual int sync() override;

    private:
        /**
         * @brief The `client_socket` pointer passed in to the constructor.
         */
        SocketPtr _socket;

        /**
         * @brief Cached char received.
         */
        char _get_ch;

        /**
         * @brief Cached char to be sent.
         */
        char _put_ch;
    };

} }
