# Features
Up to [Documentation](../README.md).

`abc` builds on existing `std` concepts, and implements some essential features that are missing in the `std` library.

## HTTP Endpoint
The main benefit from `abc` is that it enables every C++ program to stand up an HTTP endpoint, which allows the program to have a GUI and a REST API surface, even if the program is running on a remote device.
One HTTP endpoint can serve both static files and REST requests.
Static files are used for implementing GUI - HTML, images, JavaScript, etc.
The JavaScript can make REST requests back to the endpoint and to eventually update the GUI.

## Socket
A group of `socket` classes, that wraps around the BSD socket API, is provided.

Additionally, a `socket_streambuf` is provided, which allows `std::istream`, `std::ostream`, and any other stream that derives from them, to operate over `abc::tcp_client_socket` and `abc::udp_socket`.

## HTTP Protocol
A group of stream classes that parse and generate HTTP requests and HTTP responses.
While these streams primarily target `abc::socket_streambuf`, they can also work with any class that derives from `std::streambuf`.
This way, you can implement HTTP communication over other media, e.g. a third-party TLS streambuf.

## JSON Serialization
Similarly to HTTP, a pair of streams is provided that can read and write JSON content to and from any streambuf.

## Virtual Memory
Virtual memory enables programs to manipulate large data sets that don't fit in memory.
It is implemented at two levels - at the lower level, there is a persistent `vmem_pool` that maps and unmaps `vmem_pages` to and from memory as they are needed.
At the higher level, there are data structures - `vmem_list` and `vmem_map` (the latter is not yet available) that implement the corresponding functionality by abstracting away the mapping and unmapping of pages.

## Diagnostics
Programs misbehave sometimes, and when they do, we learn about it later, eventually from users.
To be able to find out what happened during the execution of a program, `abc` provides a `log` stream where methods can record relevant information.
Moreover, every `abc` class takes a log parameter and records diagnostic information.
Thus, a program can obtain such diagnostic information about the library's performance.

## Timestamp
Prior to C++ 20, there is no thread-safe way to obtain a date from a `std::chrono::time_point`.
Therefore, a `timestamp` class is provided for this very purpose.
