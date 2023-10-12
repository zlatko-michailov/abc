# diag_ready

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [diag/diag_ready.h](../../src/diag/diag_ready.h)
Interface        | [diag/i/diag_ready.i.h](../../src/diag/i/diag_ready.i.h)
Tests / Examples | n/a

`diag_ready` is a convenience accessor to logging facilities.
When a class derives from `diag_ready`, it can directly call the logging methods without checking whether a `log_ostream` has been provided or not.

In addition to logging, this class offers facilities to throw exceptions as well as to assert static analysis conditions.
