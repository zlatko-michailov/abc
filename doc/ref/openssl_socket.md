# openssl_socket

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [openssl_socket.h](../../src/openssl_socket.h)
Interface        | [openssl_socket.i.h](../../src/i/openssl_socket.i.h)
Tests / Examples | [test/socket.cpp](../../test/socket.cpp)

__Note__: See the [socket](socket.md) limitations on availability.
Additionally, this functionality requires `OpenSSL` to be installed.

`abc` does not implement any cryptographic functionality, nor does it distribute `OpenSSL` or any other crypto or TLS library.

The main purpose of these socket classes is to prove that it is possible to support TLS on top of the existing `abc` sockets as well as to provide an example how to do it.

`OpenSSL` was chosen because it is probably the most popular TLS implementation and it is freely available on Linux distributions

`openssl_tcp_server_socket` is a TCP server socket that supports TLS.
It can _listen_ and _accept_ TLS client connections.

`openssl_tcp_client_socket` is TCP client socket that supports TLS.
It can _connect_ to a TLS server and can _send_ and _receive_ bytes over the encrypted connection.
