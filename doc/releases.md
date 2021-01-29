# Release Notes

Up to [Documentation](README.md).

## 1.12.1
- No breaking changes.
- Virtual memory:
  - `vmem_pool` bug fix
  - `vmem_iterator`
  - `vmem_list` balance items on insert() and on erase()

## 1.12.0
- No breaking changes.
- Virtual memory:
  - `vmem_pool`
  - `vmem_list`
  - Sample
- Move non-includable files to subfolders. 

## 1.11.0
- No breaking changes.
- Public verification.

## 0.11.0
- No breaking changes.
- Improved documentation.

## 0.10.0
- No breaking changes.
- Reduce the C++ requirement from 17 to 11.
- Support 32-bit.
- Fix build on Ubuntu.
- `http`
  - Enable ignore-case.
- `socket`
  - Bind to _any_ host.
  - Shutdown before closing.
- Basic sample
  - Improve usability.

## 0.9.0
- No breaking changes.
- `endpoint`
  - A simple web server.
- Basic sample
  - A simple app that solves systems of two linear equations of two variables.
  - The app is interacted with using a web browser.

## 0.8.0
- Breaking changes:
  - Refactored `log` and its related entities. Construction and usage are similar, but not the same.
  - The `test_log` shortcut was moved from the product to the test project.
  - Json streams take `Log` as the last template parameter.
  - `log` can only be passed as a raw pointer.
- `log_ostream` is now based on any `std::streambuf`, and thus can send content to any medium for which there is a `std::streambuf`.
`log_ostream` is now a specialization of `table_ostream`.
- `table_ostream` is a generic stream of lines.
- `line_ostream` is a stream over a fixed char buffer that can flush to a `table_ostream`.
`log_ostream` is one specialization of `table_ostream` and `line_ostream`.
Other tabular streams can be created in user space.
- `multifile_streambuf` a derivate of `std::filebuf` that streams out to a sequence of files.
  - `duration_multifilie_streambuf` automatically starts a new file after a given duration.
  - `size_multifilie_streambuf` automatically starts a new file after a given size.

## 0.7.0
- Breaking changes:
  - Removed method `gcount()` from `http_request_ostream` and `http_response_ostream`.
- `json`
  - JSON streams.

## 0.6.0
- No breaking changes.
- `http`
  - _Syntactic_ http streams.
- `ascii`
  - Basic character predicates.

## 0.5.0
- Breaking changes:
  - `socket.h` is no longer in the `posix` subfolder.
  It is in the same folder with all other headers.
  The `posix` subfolder has been removed.
  - `streambuf.h` has been renamed to `buffer_streambuf.h` and class `streambuf_adapter` has been renamed to `buffer_streambuf`.
- Add `socket_streambuf`class to enable creation of streams over sockets.

## 0.4.0
- No breaking changes.
- `socket`, `exception`
  - Enable all `socket` and `exception` classes to take a `log`. This way `abc` classes can log diagnostic info in the app's log.
- `log`
  - Add a `format_binary()` method that formats a byte buffer.
  - Enable all `log_view` class to format byte buffers. This enables sockets to log sent and received bytes.

## 0.3.0
- First promising release. 
