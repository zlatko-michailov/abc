# json

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [json.h](../../../src/net/json.h)
Interface        | [json.i.h](../../../src/net/i/json.i.h)
Tests / Examples | [test/json.cpp](../../../test/json.cpp)

## Reader and Writer
These are higher-level entities:
- `reader`
- `writer`

They read/write whole `value`s.
Thus, only minimum knowledge of the JSON format is required.

## Streams
These are lower-level entities that read and write JSON streams.
The caller must have sufficient knowledge of the JSON format to read/write elements in the correct order.

These classes enable sequential parsing and generation of JSON payloads:
- `istream`
- `ostream`

## Media
Streams and readers/writers are most likely to be connected to a [`socket_streambuf`](socket.md).
However, they could be connected to any other specialization of `std::streambuf` including specializations not provided by `abc`.
