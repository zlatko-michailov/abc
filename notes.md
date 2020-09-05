# v0.8
## Done
- Add a base `_stream<Stream>` that exposes the state getters.
  - Implement a single set of stream state verification methods.
  - Derive `http_` streams from the base `_stream`.
  - Derive `table_` and `line_` from the base `_stream`.
- Test log lines.
- Convert `LineStream` to `Line`.
- Convert `FilterPtr` to `Filter`. 
- Convert `LogPtr` to `Log*` for all classes.
- Convert `SocketPtr` to `Socket`.
- `json` make <Log> the last template parameter.
- `multifile` make <Log> the last template parameter.
- `duration_multifile_streambuf`
- `size_multifile_streambuf`
- README
  - Dependency - zip. 
  - Try It - step-by-step instructions
  - Use It - step-by-step instructions
  - Organize class reference - Media, Streams, Utilities

## To Do
- Tag.
- pack
  - Include bin/tag*
  - Make a shell script?

## Postponed
- Improve code style
- `socket_streambuf` and `multifile_streambuf` should take a <Size> template parameter to buffer I/O.
- `socket_streambuf` and `multifile_streambuf` should take a <Size> template parameter to buffer I/O.

# v1.8
## Done
## To Do
- Samples
- Internationalization

# v1.9
## Done
## To Do
- WebSocket (SHA-1, base64)

# v1.10
## Done
## To Do
- ring - single writer, multiple readers
- file-backed buffer

---

- conf
- flight (killbit)
- uuid (request/correlation)
- usage
- ring
- then


PID PPID PS
ps -el | sed -E -e 's/^. +. +([[:digit:]]+) +([[:digit:]]+) +.+ +([[:alpha:]]+)$/\1 \2 \3/'