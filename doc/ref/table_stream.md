# table

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [table.h](../../src/table.h)
Interface        | [table.i.h](../../src/i/table.i.h)
Tests / Examples | [test/table.cpp](../../test/table.cpp)

`table_ostream` is a base class that represents a stream of formatted lines.

`line_ostream` is a base class that does formatting of individual values into a single line.
It provides formatting of commonly used values like `std::thread::id`, `timestamp`, and a binary buffer, as well as general `std::sprint()` formatting.

Each formatted line is sent to a `table_ostream`.
