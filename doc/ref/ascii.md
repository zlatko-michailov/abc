# ascii

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [ascii.h](../../src/ascii.h)
Interface        | [ascii.h](../../src/ascii.h)
Tests / Examples | n/a

Simple predicates to check ASCII code category.
The advantage of these predicates over the `std` ones is that they are explicitly defined as opposed to delegated to the `"C"` locale.
That makes them suitable for implementing protocols like HTTP.
