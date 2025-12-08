# vector_streambuf

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [stream/vector_streambuf.h](../../../src/stream/vector_streambuf.h)
Interface        | [stream/i/vector_streambuf.h](../../../src/stream/i/vector_streambuf.i.h)
Tests / Examples | [test/vector_streambuf.cpp](../../../test/vector_streambuf.cpp)

This is a `std::streambuf` specialization that reads from and writes to an `std::vector<char>`.

This class is very similar to `abc::stream::buffer_streambuf` with an added benefit of dynamically expanding storage at the expense of heap reallocations.