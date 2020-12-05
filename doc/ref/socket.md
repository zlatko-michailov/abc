# socket

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [socket.h](../../src/socket.h)
Interface        | [socket.i.h](../../src/socket.i.h)
Tests / Examples | [test/socket.cpp](../../test/socket.cpp)

__Note__: This medium is only available on POSIX systems where the BSD socket C API is available.
These are C++ wrappers around the BSD socket C API.

`socket_streambuf` is a `std::streambuf` specialization that reads from and writes to a `_client_socket` - mainly a `tcp_client_socket`, but possibly a `udp_socket`.

`tcp_server_socket` is a TCP server socket.
It can _listen_ and _accept_ client connections.

`tcp_client_socket` is TCP client socket.
It can _send_ and _receive_ bytes.

Since UDP sockets are connectionless, they are symmetric - there is no client or server.
Each side can _send_, and _receive_ bytes.
`udp_socket` represents UDP sockets.
