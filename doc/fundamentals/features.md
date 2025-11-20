# Features
Up to [Documentation](../README.md).

`abc` builds on existing `std` concepts, and implements some essential features that are missing in the `std` library.

Following are some highlights.
For details, explore the [Class Reference](../README.md#class-reference).

## HTTP Endpoint
`abc` enables C++ programs to stand up an HTTP endpoints, which allows C++ programs to have a GUI and a REST API surface, even if such programs are running on remote devices.

One HTTP endpoint can serve both static files and REST requests.
Static files are used for implementing GUI - HTML, images, JavaScript, etc.
The JavaScript can make REST requests back to the endpoint and to eventually update the GUI.

## Socket
A group of `*_socket` classes that wrap around the POSIX socket API is provided.

This module allows library consumers to plug in their own wrappers around other socket APIs, e.g. TLS socket implementations.
An OpenSSL wrapper is provided as an example.

`abc::net::tcp_client_socket_streambuf` is provided, which allows `std::istream`, `std::ostream`, and any other stream that derives from them, to operate over `abc::net::tcp_client_socket`.

## HTTP Protocol
A group of stream classes that parse and generate HTTP requests and HTTP responses is provided.

While these streams primarily target `abc::net::tcp_client_socket_streambuf`, they can also work with any class that derives from `std::streambuf`.
This way, HTTP communication over other media, e.g. a third-party TLS streambuf, can be implemented.

## JSON Serialization
Similarly to HTTP, a pair of streams is provided that can read and write JSON content to and from any streambuf.

## Virtual Memory
Virtual memory enables programs to manipulate large data sets that don't fit in memory.

It is implemented at two levels - at the lower level, there is a persistent `abc::vmem::pool` that maps and unmaps `abc::vmem::page` instances to and from memory as they are needed.
At the higher level, there are data structures - `abc::vmem::list`, `abc::vmem::stack`, and `abc::vmem::map` as well as the more generic `abc::vmem::container` and `abc::vmem::linked` that allow adding of custom data structures.

## Diagnostics
All library primitives have the ability to log diagnostics if a `abc::diag::log_ostream` is provided. 

User classes can derive from `abc::diag::diag_ready`, which simplifies diagnostics logging and offers some SAL functionality like `expect()` and `ensure()`.

Each method of each library class logs a "Begin:" and "End:" entry, so the call stack could be tracked down if needed.

## Timestamp
Prior to C++ 20, there is no thread-safe way to obtain a date from a `std::chrono::time_point`.
Therefore, a `abc::timestamp` class is provided for this very purpose.

## GPIO
A group of classes that wraps around GPIO as well as around SMBus (a.k.a. I2C), is also provided.

There are two levels of GPIO and SMBus primitives.
Low level - GPIO chip, GPIO line, SMBus controller, SMBus target.
High level - specific peripherals, e.g. `abc::gpio::ultrasonic`, `abc::smbus::servo`, `abc::smbus::motor`, etc.
