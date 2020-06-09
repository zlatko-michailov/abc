# abc
Header-only library of essential utilities for C++ development that doesn't use dynamic/heap memory.


[Summary](#Summary)  
[Brief Class Reference](#Brief-Class-Reference)  
[Toolchain and Platform Dependencies](#Toolchain-and-Platform-Dependencies)  
[Try It](#Try-It)  
[Use It](#Use-It)  
[Release Notes](#Release-Notes)  
[Roadmap](#Roadmap)


## Summary
`abc` is a header-only library that contains a few classes that are missing in the `std` library. The key entities are:
- timestamp
- log
- test
- socket (TCP and UDP)
- http streams
- streambuf specializations
- ascii

All classes are provided as headers, and must be compiled in client programs.
There is no precompiled flavor of the library.
That is because `abc` targets devices with pequliar architectures and small memory capacities.

### No Dynamic Memory Allocation
`abc` uses static, caller-provided, buffers instead of dynamically allocating heap memory.
It doesn't use `std::string`, `std::vector`, or any other `std` class that automatically allocates heap memory.
This doesn't prevent apps from using classes like `std::string` or `std::vector`.
It is just a measure to maintain performance and efficiency for apps that care about them.


## Brief Class Reference
### `timestamp`
A thread-safe, lock-free, facility to calculate the date and time by a `std::chrono::time_point`.

### `log`
A flexible output facility. The driving entity is `log`. It takes three other sub-entities:
- `LogContainer`- adapter to the output medium:
	- `log_container::ostream` - for any existing `std::ostream` instance.  
	- `log_container::file` - for a disk file.  
- `LogView` - line formatter:
	- `log_view::debug` - nicely organized columns suitable for a human eye.
	- `log_view::diagg` - condensed columns suitable for shipping of the network.
	- `log_view::test` - balanced information suitable test frameworks.
	- `log_view::blank` - prints no predefined field, only custom content.
- `LogFilter` - decides whether a line should be printed or not:
	- `log_filter::none` - no filtering, i.e. print everything.
	- `log_filter::none` - don't print anything.
	- `log_filter::seveity` - print lines with higher or equal to the given seveirty.

More implementations of each sub-entity could be done in user-space.

### `test`
A simple test framework. It is used to test `abc` itself. It could be used for projects that use `abc`.

### `socket`
__Note__: This entity is only available on POSIX systems where the BSD socket C API is available.

This is a C++ wrapper around the BSD socket C API. The following self-explanatory classes are provided:
- `udp_socket`
- `tcp_server_socket`
- `tcp_client_socket`

### `http` streams
These classes provide a _syntactic_ check of http request and response streams.
The app is responsible for checking the _semantic_ correctness of the stream as well as for any kind of encoding/decoding that the content of the stream implies.

These classes are not suitable for implementing a general-purpose web server.
Their intent is to be used to implement REST end points or clients in a trusted network, e.g. communicating with a (IoT) device on a LAN.

__Note:__ `abc` does not include a crypto ficility that implements TLS/https, nor is there any plan to implement such a facility.

There are four _core_ http stream classes:
- `http_request_istream`
- `http_request_ostream`
- `http_response_istream`
- `http_response_ostream`

There are two _convenience_ classes that combine the above core classes as follows:
- `http_client_stream : http_request_ostream, http_response_istream`
- `http_server_stream : http_request_istream, http_response_ostream`


### `streambuf` Specializations
- `buffer_streambuf`- a `std::streambuf`implementation over a fixed `char` buffer.
This class comes handy when `std::thread::id` is used.
The latter can only be sent to a stream. It doesn't support any other operation.
This `streambuf` specialization allows you to get hold of a `std::thread::id` without any heap allocation.
- `socket_streambuf` - a `std::streambuf`implementation over `_client_socket`, particularly over `tcp_client_socket`.

### `ascii`
Simple predicates to check ASCII code category.
The advantage of these predicates over the `std` ones is that they are explicitly defined as opposed to delegated to the `"C"` locale.
That makes them suitable for implementing protocols like `http`.

### `exception`
An envelope around any exception type.
Its purpose is to log a tag, which identifies the place where this exception was thrown.

### Tagging
Tags are unique 64-bit integers that identify the place in the code where a log entry was created.

1. Pass the `__TAG__` sentinel wherever an API needs a `tag_t` value. Commit your change with that sentinel.
2. At some later point, a designated contributor runs `bin/tag.sh --conf bin/tag.conf` to assign a unique number to each `__TAG__` instance. 


## Toolchain and Platform Dependencies
### GCC
The project is compiled using GCC 9.
It may be possible to compile with an earlier version of GCC or even with a differnt compiler, but that hasn't been tried.

### POSIX
The socket classes use the BSD socket C API.
This project has been tested on Windows 10 with the [Windows Subsystem for Linux](https://docs.microsoft.com/en-us/windows/wsl/install-win10) feature enabled.


## Try It
Clone the repo loclaly.

From the root repo folder, run `make`.
This will compile and run the tests. The product will be located in the `out/abc/<version>/inc` subfolder. 

To see examples of how to use these classes, open the test files.


## Use It
At least for now, the process is manual. From a built repo, copy the `out/abc` subfolder to the location where your project includes its dependencies, e.g. something like `deps/abc`.

Keep an eye on the `abc` [repo](https://github.com/zlatko-michailov/abc) for updates. You can follow the above procedure to install multiple versions of `abc` side by side, so you can easily evaluate new versions.


## Release Notes
### 0.6.0
- No breaking changes.
- `http`
  - Introduce _syntactic_ streams.
- `ascii`
  - Introduce basic predicates.

### 0.5.0
- Breaking changes.
  - `socket.h` is no longer in the `posix` subfolder.
  It is in the same folder with all other headers.
  The `posix` subfolder has been removed.
  - `streambuf.h` has been renamed to `buffer_streambuf.h` and class `streambuf_adapter` has been renamed to `buffer_streambuf`.
- Add `socket_streambuf`class to enable creation of streams over sockets.

### 0.4.0
- No breaking changes.
- `socket`, `exception`
  - Enable all `socket` and `exception` classes to take a `log`. This way `abc` classes can log diagnostic info in the app's log.
- `log`
  - Add a `format_binary()` method that formats a byte buffer.
  - Enable all `log_view` class to format byte buffers. This enables sockets to log sent and received bytes.

### 0.3.0
- First promising release. 


## Roadmap
### 0.7.0
- Introduce `JSON` streams.

### 0.8.0
- Introduce `WebSocket` client and server.
- Introduce `base64` encoding and decoding. (Required for WebSocket.)
- Introduce `SHA-1` hashing. (Required for WebSocket.)

