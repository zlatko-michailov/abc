# http

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [net/http.h](../../../src/net/http.h)
Interface        | [net/i/http.i.h](../../../src/net/i/http.i.h)
Tests / Examples | [test/http.cpp](../../../test/http.cpp)

## Readers and Writers
These are higher-level entities:
- `request_reader`
- `request_writer`
- `response_reader`
- `response_writer`

They read/write whole requests/responses.
Thus, only minimum knowledge of the http protocol is required.

## `client` and `server` 
These are _convenience_ classes that simply combine the corresponding readers and writers:
- `client`
  - `request_writer`
  - `response_reader`
- `server`
  - `request_reader`
  - `response_writer`

## Streams
These are lower-level entities that read and write http streams.
The caller must have sufficient knowledge of the protocol to read/write elements in the correct order.

There are four http stream classes:
- `request_istream`
- `request_ostream`
- `response_istream`
- `response_ostream`

## Media
Streams and readers/writers are most likely to be connected to a [`socket_streambuf`](socket.md).
However, they could be connected to any other specialization of `std::streambuf` including specializations not provided by `abc`.

## Note on `request_ostream`
The http protocol states that the client should provide a `Host` header.
Class `request_ostream` cannot create this header implicitly, because the value is given to the connection facility, not to this class.

Therefore, __the caller is responsible for explicitly adding a `Host` header__.

## RFC Reference
| Title          | Link
| ---            | ---
| HTTP/1.1       | https://datatracker.ietf.org/doc/html/rfc9112
| HTTP Semantics | https://datatracker.ietf.org/doc/html/rfc9110
| URI            | https://datatracker.ietf.org/doc/html/rfc3986
