# Media and Streams

Up to [Documentation](../README.md).

The `std` library provides a nice separation between media and streams.
Streams are stateless formatters, while media are the actual containers.
The idea is that any stream can operate on any medium.

The notion of media is not easily noticeable, because there are only two concrete implementations with unclear names - `stringbuf` and `filebuf`.
The really important class, though, is the semi-abstract `streambuf`.
There a three small methods that a medium may have to override to become operational.

`abc` provides several media and streams that could be mixed and matched.
Follow the links to read more about each class.

## Media
- [`abc::stream::buffer_streambuf`](../ref/stream/buffer_streambuf.md) - a char buffer of a fixed length.
- [`abc::stream::vector_streambuf`](../ref/stream/vector_streambuf.md) - a `std::vector<char>`.
- [`abc::stream::multifile_streambuf`](../ref/stream/multifile_streambuf.md) - a file that when reopened creates a new file with a timestamp in the name. There two specializations:
  - [`abc::stream::duration_multifile_streambuf`](../ref/stream/duration_multifile_streambuf.md) - automatically reopens at a given time interval.
  - [`abc::stream::size_multifile_streambuf`](../ref/stream/size_multifile_streambuf.md) - automatically reopens when the file's size exceed a given limit.
- [`abc::net::tcp_client_socket_streambuf`](../ref/net/tcp_client_socket_streambuf.md) - a TCP socket.

## Streams
- [`abc::net::http`](../ref/net/http.md) - a group of streams.
Notice that using these streams directly requires some knowledge on the HTTP protocol.
It is recommended to use the corresponding readers and writers instead.
  - [`abc::net::http::request_istream`](../ref/net/http.md)
  - [`abc::net::http::request_ostream`](../ref/net/http.md)
  - [`abc::net::http::response_istream`](../ref/net/http.md)
  - [`abc::net::http::response_ostream`](../ref/net/http.md)
- [`abc::net::json`](../ref/net/json.md) - a group of streams.
Notice that using these streams directly requires some knowledge on the JSON format.
It is recommended to use the corresponding readers and writers instead.
  - [`abc::net::json::istream`](../ref/net/json.md)
  - [`abc::net::json::ostream`](../ref/net/json.md)
- [`abc::stream::table_ostream`](../ref/stream/table_stream.md) - an output stream of lines.
  - [`abc::diag::log_ostream`](../ref/diag/log.md) - a specialization for logging.
- [`abc::stream::line_ostream`](../ref/stream/table_stream.md) - an output stream of chars within a line.
  - [`abc::diag::log_line_ostream`](../ref/diag/log.md) - a line output stream interface for diagnostic purposes.
  - [`abc::diag::debug_line_ostream`](../ref/diag/log.md) - a specialization for human-readable diagnostics.
  - [`abc::diag::diag_line_ostream`](../ref/diag/log.md) - a specialization for compact diagnostics.
  - [`abc::diag::test_line_ostream`](../ref/diag/log.md) - a specialization for unit testing.

## Readers and Writers
For streams that require knowledge on the corresponding protocol/format, like HTTP and JSON, `abc` provides readers and writers that can read and write whole values.
- [`abc::net::http`](../ref/net/http.md)
  - [`abc::net::http::request_reader`](../ref/net/http.md)
  - [`abc::net::http::request_writer`](../ref/net/http.md)

 For convenience, `abc` provides the following combinations:
- [`abc::net::http::server`](../ref/net/http.md) - combines:
  - [`abc::net::http::request_reader`](../ref/net/http.md)
  - [`abc::net::http::response_writer`](../ref/net/http.md)
- [`abc::net::http::client`](../ref/net/http.md) - combines:
  - [`abc::net::http::request_writer`](../ref/net/http.md)
  - [`abc::net::http::response_reader`](../ref/net/http.md)
