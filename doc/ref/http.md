# http

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [http.h](../../src/http.h)
Interface        | [http.i.h](../../src/i/http.i.h)
Tests / Examples | [test/http.cpp](../../test/http.cpp)

These classes provide a _syntactic_ check and generation of `http` request and response streams.
The program is responsible for checking the _semantic_ correctness of the stream as well as for any kind of encoding/decoding that the content of the stream implies.

These classes are not suitable for implementing a general-purpose web server.
Their intent is to be used to implement REST endpoints or clients in a trusted network, e.g. for communicating with a (IoT) device within a LAN.

__Note:__ `abc` does not include a crypto facility that implements TLS/https.
In future, it may provide `streambuf` adapter for a third-party crypto library.

There are four _core_ http stream classes whose names should be self-explanatory:
- `http_request_istream`
- `http_request_ostream`
- `http_response_istream`
- `http_response_ostream`

There are two _convenience_ classes that combine the above core classes as follows:
- `http_client_stream`
  - `http_request_ostream`
  - `http_response_istream`
- `http_server_stream`
  - `http_request_istream`
  - `http_response_ostream`

These http streams are most likely to be connected to a [`socket_streambuf`](socket.md).
However, they could be connected to any other specialization of `std::streambuf` including specializations not provided by `abc`.
