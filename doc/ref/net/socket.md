# socket

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [net/socket.h](../../../src/net/socket.h)
Interface        | [net/i/socket.i.h](../../../src/net/i/socket.i.h)
Tests / Examples | [test/socket.cpp](../../../test/socket.cpp)

__Note__: This medium is only available on POSIX systems where the POSIX socket C API is available.
These are C++ wrappers around the POSIX socket C API.

`socket_streambuf` is a `std::streambuf` specialization that reads from and writes to a `client_socket` - mainly a `tcp_client_socket`, but possibly a `udp_socket`.

`tcp_server_socket` is a TCP server socket.
It can _listen_ and _accept_ client connections.

`tcp_client_socket` is a TCP client socket.
It can _connect_ to a server and can _send_ and _receive_ bytes.

Since UDP sockets are connectionless, they are symmetric - there is no client or server.
Each side can _send_, and _receive_ bytes.
`udp_socket` represents UDP sockets.
