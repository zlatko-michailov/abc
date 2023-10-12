# exception

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [diag/exception.h](../../src/diag/exception.h)
Interface        | [diag/i/exception.h](../../src/diag/i/exception.i.h)
Tests / Examples | n/a

`exception` is a thin wrapper around any exception type.
Its purpose is to log a tagged entry, which identifies the place where this exception was thrown.
The caller could catch the base exception type.

There are several concrete exception types provided for static analysis:
- `assert_error` - derives from `std::logic_error`.
Represents a general assert failure.
- `expect_error` - derives from `assert_error`.
Represents an "expect" (input state) assert failure.
- `ensure_error` - derives from `assert_error`.
Represents an "ensure" (output state) assert failure.
