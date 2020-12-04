# `buffer_streambuf`

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [buffer_streambuf.h](../../src/buffer_streambuf.h)
Interface        | [buffer_streambuf.h](../../src/buffer_streambuf.h)
Tests / Examples | [test/streambuf.cpp](../../test/streambuf.cpp)

## `buffer_streambuf`
This is a `std::streambuf` specialization that reads from and writes to a fixed `char` buffer.

This class is heavily used for testing streams.

It also comes handy when `std::thread::id` is used.
The latter can only be sent to a stream.
It doesn't support any other operation.
This `std::streambuf` specialization allows you to get hold of a `std::thread::id` without any heap allocation.
