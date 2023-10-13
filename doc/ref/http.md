# http

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [net/http.h](../../src/net/http.h)
Interface        | [net/i/http.i.h](../../src/net/i/http.i.h)
Tests / Examples | [test/http.cpp](../../test/http.cpp)

These classes read and write `http` streams.
It is recommended to read/write the whole request/response, and then - the body, if applicable.
This way, the caller does not need to know too much about the protocol.

If the caller chooses to read/write individual elements, the caller must have sufficient knowledge of the protocol to read/write elements in the exact order.

There are four _core_ http stream classes whose names should be self-explanatory:
- `request_istream`
- `request_ostream`
- `response_istream`
- `response_ostream`

There are two _convenience_ classes that combine the above core classes as follows:
- `client_stream`
  - `request_ostream`
  - `response_istream`
- `server_stream`
  - `request_istream`
  - `response_ostream`

These http streams are most likely to be connected to a [`socket_streambuf`](socket.md).
However, they could be connected to any other specialization of `std::streambuf` including specializations not provided by `abc`.

## Note on `request_ostream`
The `http` protocol states that the client should provide a `Host` header.
Class `request_ostream` cannot create this header implicitly, because the value is given to the connection facility, not to this class.


Therefore, __the caller is responsible for explicitly adding a `Host` header__.

## RFC Reference
| Title          | Link
| ---            | ---
| HTTP/1.1       | https://datatracker.ietf.org/doc/html/rfc9112
| HTTP Semantics | https://datatracker.ietf.org/doc/html/rfc9110
| URI            | https://datatracker.ietf.org/doc/html/rfc3986
