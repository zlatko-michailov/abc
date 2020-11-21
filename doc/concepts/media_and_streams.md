# Media and Streams

Up to [Documentation](../README.md).

The `std` library provides a nice separation between media and streams.
Streams are stateless formatters, while media are the actual containers.
The idea is that any stream can operate on any medium.

The notion of media is not easily noticable, because there are only two concrete implementations with unclear names - `stringbuf` and `filebuf`.
The really important class, though, is the semi-abstract `streambuf`.
There a three small methods that a medium may have to override to become operational.

`abc` provides several media and streams that could be mixed and matched.
Follow the links to read more about each class.

## Media
- [`buffer_streambuf`](../ref/buffer_streambuf.md) - a char buffer of a fixed length.
- [`socket_streambuf`](../ref/socket_streambuf.md) - a TCP or UDP socket.
- [`multifile_streambuf`](../ref/multifile_streambuf.md) - a file that when reopened creates a new file with a timestamp in the name. There two specializations:
  - [`dusration_multifile_streambuf`](../ref/duration_multifile_streambuf.md) - automatically reopens at a given time interval.
  - [`size_multifile_streambuf`](../ref/size_multifile_streambuf.md) - automatically reopens when the file's size exceed a given limit.

## Streams
- [`http`](../ref/http.md) - a group of streams:
  - [`http_request_istream`](../ref/http.md)
  - [`http_request_ostream`](../ref/http.md)
  - [`http_response_istream`](../ref/http.md)
  - [`http_response_ostream`](../ref/http.md)
  - [`http_server_stream`](../ref/http.md) - a convenience combination of `http_request_istream` and `http_response_ostream`.
  - [`http_client_stream`](../ref/http.md) - a convenience combination of `http_request_ostream` and `http_response_istream`.
- [`json`](../ref/json.md) - a group of streams:
  - [`json_istream`](../ref/json.md)
  - [`json_ostream`](../ref/json.md)
- [`table_ostream`](../ref/table.md) - an output stream of lines.
  - [`log_ostream`](../ref/log.md) - a specialization for logging.
- [`line_ostream`](../ref/line.md) - an output stream of chars.
  - [`debug_line_ostream`](../ref/log.md) - a specialization for human-readable diagnostics.
   - [`diag_line_ostream`](../ref/log.md) - a specialization for compact diagnostics.
  - [`test_line_ostream`](../ref/log.md) - a specialization for unit testing.
