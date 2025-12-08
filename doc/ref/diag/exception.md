# exception

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [diag/exception.h](../../../src/diag/exception.h)
Interface        | [diag/i/exception.h](../../../src/diag/i/exception.i.h)
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

Static analysis exception types should not be used for bad input.
Exception types that derive from `std::runtime_error` should be used:
- `input_error` - derives from `std::runtime_error`.
Represents a transient failure.
