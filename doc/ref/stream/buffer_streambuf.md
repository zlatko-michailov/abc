# buffer_streambuf

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [stream/buffer_streambuf.h](../../../src/stream/buffer_streambuf.h)
Interface        | [stream/i/buffer_streambuf.h](../../../src/stream/i/buffer_streambuf.i.h)
Tests / Examples | [test/buffer_streambuf.cpp](../../../test/buffer_streambuf.cpp)

This is a `std::streambuf` specialization that reads from and writes to a `char` buffer of a fixed size.

This class is heavily used for testing streams.

It also comes handy when `std::thread::id` is used.
The latter can only be sent to a stream.
It doesn't support any other operation.
This `std::streambuf` specialization allows you to get hold of a `std::thread::id` without any heap allocation.
