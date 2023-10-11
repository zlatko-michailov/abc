# table

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [table_stream.h](../../src/table_stream.h)
Interface        | [i/table_stream.i.h](../../src/i/table_stream.i.h)
Tests / Examples | [test/table_stream.cpp](../../test/table_stream.cpp)

`table_ostream` is a base class that represents a stream of formatted lines.

`line_ostream` is a base class that does formatting of individual values into a single line.
It provides formatting of commonly used values like `std::thread::id`, `timestamp`, and a binary buffer, as well as general `std::sprint()` formatting.

Each formatted line is sent to a `table_ostream`.
