# Documentation

> If you find a bug on any page, please file an issue at the project's [Issues](../issues).
Provide the title and the link to the page along with as much information as you consider relevant.

- [MIT License](../LICENSE)
- Fundamentals
  - [Features](fund/features.md)
  - [Intent](fund/intent.md)
  - [Principles - collisions with std, exceptions](fund/principles.md)
- Concepts
  - [Media and streams](MediaAndStreams.md)
  - [Tagging](HowTo_Troubleshoot.md)
  - [Diagnostics - log](HowTo_Diagnostics.md)
  - [GUI and REST endpoint](HowTo_GUI.md)
- Getting Started
  - [Dependencies - prepare your dev box](GettingStarted_Dependencies.md)
  - [How to try it - build and examine a clone of the repo](GettingStarted_Trying.md)
  - [How to adopt it - include abc in a bigger program, .i.h files](GettingStarted_Adopting.md)
- Tutorials
  - [GUI and REST endpoint](HowTo_GUI.md)
  - [Troubleshoot - log filter and log lines](HowTo_Troubleshoot.md)
  - [Tagging](HowTo_Troubleshoot.md)
- Class Reference
  - [ascii](ref/ascii.md)
  - [buffer_streambuf](ref/buffer_streambuf.md)
  - [endpoint](ref/endpoint.md)
  - [exception](ref/exception.md)
  - [http streams](ref/http.md)
    - [http_request_istream](ref/http.md)
    - [http_request_ostream](ref/http.md)
    - [http_response_istream](ref/http.md)
    - [http_response_ostresm](ref/http.md)
    - [http_client_stream](ref/http.md)
    - [http_server_stream](ref/http.md)
  - [json streams](ref/json.md)
    - [json_istream](ref/json.md)
    - [json_ostream](ref/json.md)
  - [log streams ***](ref/log.md)
  - [multifile ***](ref/multifile.md)
  - [size](ref/size.md)
  - [sockets ***](ref/socket.md)
  - [streams ***](ref/stream.md)
  - [table ***](ref/table.md)
  - [tag](ref/tag.md)
  - [test](ref/test.md)
  - [timestamp](ref/timestamp.md)
- [Release Notes](ReleaseNotes.md)
- [Roadmap](Roadmap.md)

___
DELETE EVERYTHING FROM HERE DOWN
___


## Summary
`abc` is a header-only library that contains a few classes that are missing in the `std` library. The key entities are:
- socket (TCP and UDP)
- HTTP I/O streams
- JSON I/O streams
- generic table output stream
- log
- test framework
- timestamp

All classes are provided as headers, and must be compiled in client programs.
There is no precompiled flavor of the library.
That is because `abc` targets devices with peculiar architectures and small memory capacities.


### No Dynamic Memory Allocation
`abc` uses static, caller-provided, buffers instead of dynamically allocating heap memory.
It doesn't use `std::string`, `std::vector`, or any other `std` class that automatically allocates heap memory.
This doesn't prevent apps from using classes like `std::string` or `std::vector`.
It is just a measure to maintain performance and efficiency for apps that care about them.



## Toolchain and Platform Dependencies
### GCC
The source code uses C++ 11.
The project should be compilable using GCC 4.8.1 or later. It has been tested with GCC 9.

### POSIX
The socket classes use the BSD socket C API.
This project has been tested on Windows 10 with the [Windows Subsystem for Linux](https://docs.microsoft.com/en-us/windows/wsl/install-win10) feature enabled.

### zip
zip is needed to pack and to unpack product releases.



## Try It
Clone the repo:
```sh
mkdir abc
git clone https://github.com/zlatko-michailov/abc.git abc
```

Build the project and run the tests:
```sh
cd abc
make
```

The product source code is in `src`.
The test source code is in `test`.

Each entity is implemented in `src/<entity>.h`.
This is the file that should be included from an app's source file.

Complex entities have their interface defined in `src/<entity>.i.h`.
This file could be used by app developers to examine the entity's interface.

The best way to start learning how to use the classes is to pick a test file, to change something in it, and to see how the behavior changes.

Also see [Troubleshoot](#Troubleshoot). 



## Use It
It is recommended to follow the steps from [Try It](#Try-It) to clone the repo and to build the project.
This would verify the code is intact.

After a successful build, the product will be located in: `out/abc`.
Copy that whole tree to a location where your project includes its dependencies.

Alternatively, you can download and unzip the product archive (`abc_<version>.zip`) from the [latest release](https://github.com/zlatko-michailov/abc/releases/latest).

Keep an eye on the `abc` [repo](https://github.com/zlatko-michailov/abc) for updates.

__Note__: You may install multiple versions of `abc` side by side, so you can easily evaluate new versions.



## Troubleshoot
### Log Filter
By default, the test log filter only allows entries of severity `critical` or higher.

To see more output, lower that severity level.
The severity levels are defined in `src/log.i.h`. The lower the level, the more the output. To enable logging from the product, set the severity level to `abc`.

The test log filter is created in the `main()` method in file `test/main.cpp`:
```c++
	abc::test::log_filter filter(abc::severity::critical);
```

### Log Line
By default, the test log uses `test_line_ostream`, which doesn't contain some properties like `thread ID` and `tag`.
To see more properties, switch to the `diag_line_ostream`.
This is done in `test/test.h`.

### Manual Intervention
Every test method has a `context` parameter, which has a `log` property.
You can use `put_any()` or `put_line()` to add to the output.

You may be able to do the same from product classes too as most classes take a `log` parameter.



## Class Reference

### Media


#### Char Buffer
Purpose          | File
---------------- | ----
Include          | [__buffer_streambuf.h__](src/buffer_streambuf.h)
Interface        | [buffer_streambuf.h](src/buffer_streambuf.h)
Tests / Examples | [test/streambuf.cpp](test/streambuf.cpp)

##### `buffer_streambuf`
This is a `std::streambuf` specialization that read from and writes to a fixed `char` buffer.

This class is heavily used for testing streams.

It also comes handy when `std::thread::id` is used.
The latter can only be sent to a stream. It doesn't support any other operation.
This `std::streambuf` specialization allows you to get hold of a `std::thread::id` without any heap allocation.


#### Socket
Purpose          | File
---------------- | ----
Include          | [__socket.h__](src/socket.h)
Interface        | [socket.i.h](src/socket.i.h)
Tests / Examples | [test/socket.cpp](test/socket.cpp)

__Note__: This medium is only available on POSIX systems where the BSD socket C API is available.
This is a C++ wrapper around the BSD socket C API.

##### `socket_streambuf`
This is a `std::streambuf` specialization that reads from and writes to a `_client_socket` - mainly a `tcp_client_socket`, but possibly a `udp_socket`.

##### `udp_socket`
Since UDP sockets are connectionless, they are symmetric - there is no client or server.
Each side can _send_, and _receive_ bytes.

##### `tcp_server_socket`
A TCP server socket.
It can _listen_ and _accept_ client connections.

##### `tcp_client_socket`
A TCP client socket can _send_ and _receive_ bytes.


#### Multifile
Purpose          | File
---------------- | ----
Include          | [__multifile.h__](src/multifile.h)
Interface        | [multifile.i.h](src/multifile.i.h)
Tests / Examples | n/a

##### `multifile_streambuf`
This class is a specialization of `std::filebuf`.
It is the base class that implements writing to a sequence of files.
Each file's name matches the UTC time down to one second.
Therefore, a `multifile_streambuf` must not be _reopened_ more than once within one second.

##### `duration_multifile_streambuf`
This is a specialization of `multifile_streambuf` where the instance is automatically _reopened_ when a given duration has passed.
_Reopening_ is done only on `sync()`, which happens when `flush()` is called on overlying stream.

##### `size_multifile_streambuf`
This is a specialization of `multifile_streambuf` where the instance is automatically _reopened_ when a given size has been exceeded.
_Reopening_ is done only on `sync()`, which happens when `flush()` is called on overlying stream.


### Streams
Streams in C++ are abstract formatters.
Each stream instance connects to s `std::streambuf` implementation.
See [Media](#Media).


#### HTTP
Purpose          | File
---------------- | ----
Include          | [__http.h__](src/http.h)
Interface        | [http.i.h](src/http.i.h)
Tests / Examples | [test/http.cpp](test/http.cpp)

These classes provide a _syntactic_ check and generation of `http` request and response streams.
The app is responsible for checking the _semantic_ correctness of the stream as well as for any kind of encoding/decoding that the content of the stream implies.

These classes are not suitable for implementing a general-purpose web server.
Their intent is to be used to implement REST end points or clients in a trusted network, e.g. communicating with a (IoT) device on a LAN.

__Note:__ `abc` does not include a crypto facility that implements TLS/https, nor is there any plan to implement such a facility.

There are four _core_ http stream classes whose names should be self-explanatory:
- `http_request_istream`
- `http_request_ostream`
- `http_response_istream`
- `http_response_ostream`

There are two _convenience_ classes that combine the above core classes as follows:
- `http_client_stream` : `http_request_ostream`, `http_response_istream`
- `http_server_stream` : `http_request_istream`, `http_response_ostream`

These http streams are most likely to be connected to a `socket_streambuf`.
However, they could be connected to any other specialization of `std::streambuf` including specializations not provided by `abc`.


#### JSON
Purpose          | File
---------------- | ----
Include          | [__json.h__](src/json.h)
Interface        | [json.i.h](src/json.i.h)
Tests / Examples | [test/json.cpp](test/json.cpp)

These classes enable sequential parsing and generation of JSON payloads:
- `json_istream`
- `json_ostream`

It is possible to skip over the entire value of a property that you are not interested in.
That value could be anything - primitive, array, or object.


#### Table
Purpose          | File
---------------- | ----
Include          | [__table.h__](src/table.h)
Interface        | [table.i.h](src/table.i.h)
Tests / Examples | [test/table.cpp](test/table.cpp)

##### `table_ostream`
This is a base class that represents a stream of formatted lines.

##### `line_ostream`
This is a base class that does formatting of individual values into a single line.
It provides formatting of commonly used values like `std::thread::id`, `timestamp`, and a binary buffer, as well as general `std::sprint()` formatting.

Each formatted line is sent to a `table_ostream`.


#### Log
Purpose          | File
---------------- | ----
Include          | [__log.h__](src/log.h)
Interface        | [log.i.h](src/log.i.h)
Tests / Examples | [test/main.cpp](test/main.cpp)

##### `log_ostream`
This is a specialization of `table_ostream`.
It takes a filter to skip lines with a lower than the specified severity.

##### `debug_line_ostream`
This is a specialization of `line_ostream` that has the most fields and is human readable.
This class is recommended for investigation of issues.

##### `diag_line_ostream`
This is a specialization of `line_ostream` that also has the most fields, but there is no extra space to reduce volume.
This class is recommended for automatic collection and shipment to a remote location.

##### `test_line_ostream`
This is a specialization of `line_ostream` that has the minimum number of fields.
This class is recommended for test suites.


### Utilities

#### `endpoint`
Purpose          | File
---------------- | ----
Include          | [__endpoint.h__](src/endpoint.h)
Interface        | [endpoint.i.h](src/endpoint.i.h)
Tests / Examples | [samples/basic/*](sample/basic/main.cpp)

This class implements a simple web server using `socket`, `http`, and `json`.
It can serve both file resources as well as REST.
This way, every `abc` app can be interacted with using a web browser.



#### `test`
Purpose          | File
---------------- | ----
Include          | [__test.h__](src/test.h)
Interface        | [test.h](src/test.h)
Tests / Examples | [test/*.cpp](test/main.cpp)

This is an implementation of a simple test framework.
It is used to test `abc` itself.


#### `timestamp`
Purpose          | File
---------------- | ----
Include          | [__timestamp.h__](src/timestamp.h)
Interface        | [timestamp.i.h](src/timestamp.i.h)
Tests / Examples | [test/timestamp.cpp](test/timestamp.cpp)

This is a thread-safe, lock-free, facility to calculate the date and time by a `std::chrono::time_point`.


#### `ascii`
Purpose          | File
---------------- | ----
Include          | [__ascii.h__](src/ascii.h)
Interface        | [ascii.h](src/ascii.h)
Tests / Examples | n/a

Simple predicates to check ASCII code category.
The advantage of these predicates over the `std` ones is that they are explicitly defined as opposed to delegated to the `"C"` locale.
That makes them suitable for implementing protocols like HTTP.


#### `exception`
Purpose          | File
---------------- | ----
Include          | [__exception.h__](src/exception.h)
Interface        | [exception.h](src/exception.h)
Tests / Examples | n/a

A wrapper around any exception type.
Its purpose is to log a tag, which identifies the place where this exception was thrown.


### Tagging
Purpose          | File
---------------- | ----
Include          | [__tag.h__](src/tag.h)
Interface        | n/a
Tests / Examples | n/a

Tags are unique 64-bit integers that are used to correlate a log entry with the place in the code where it was created.

1. Pass the `__TAG__` sentinel wherever an API needs a `tag_t` value. Commit your change with that sentinel.
2. At some later point, a designated contributor runs `bin/tag.sh --conf bin/tag.conf`, which assigns a unique number to each `__TAG__` instance. 



## Release Notes
### 0.9.0
- No breaking changes.
- `endpoint`
  - A simple web server.
- Basic sample
  - A simple app that solves systems of two linear equations of two variables.
  - The app is interacted with using a web browser.

### 0.8.0
- Breaking changes.
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

### 0.7.0
- Breaking changes.
  - Removed method `gcount()` from `http_request_ostream` and `http_response_ostream`.
- `json`
  - JSON streams.

### 0.6.0
- No breaking changes.
- `http`
  - _Syntactic_ http streams.
- `ascii`
  - Basic character predicates.

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
### 0.10.0
- Tic Tac Toe sample.

### 0.11.0
- Connect Four sample.

### 0.12.0
- Internationalization, if needed.

### 1.9.0
- Introduce `WebSocket` client and server.
- Introduce `base64` encoding and decoding. (Required for WebSocket.)
- Introduce `SHA-1` hashing. (Required for WebSocket.)

