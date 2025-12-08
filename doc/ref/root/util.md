# util

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [util.h](../../src/root/util.h)
Interface        | [util.h](../../src/root/util.h)
Tests / Examples | n/a

These are possibly unrelated facilities that are too small to warrant a dedicated headers.

## copy
Since `abc` class instances take arguments as move references, `copy()` is a convenience function that creates a temporary copy of the passed in const reference.
