# json

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [json.h](../../src/json.h)
Interface        | [json.i.h](../../src/i/json.i.h)
Tests / Examples | [test/json.cpp](../../test/json.cpp)

These classes enable sequential parsing and generation of JSON payloads:
- `json_istream`
- `json_ostream`

It is possible to skip over the entire value.
That value could be anything - primitive, array, or object.
