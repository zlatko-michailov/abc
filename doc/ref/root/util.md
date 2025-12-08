# util

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [root/util.h](../../src/root/util.h)
Interface        | [root/util.h](../../src/root/util.h)
Tests / Examples | n/a

These are possibly unrelated facilities that are too small to warrant dedicated headers.

## `copy`
Since `abc` class instances take arguments as move references, `copy()` is a convenience function that creates a temporary copy of a const reference, so that it could be moved.

## String Unifiers
A set of common purpose functions like length and emptiness that work on `cost char*` or `const std::string&`.

## Container Equality Comparers
Comparers whether two container have the same items in the same order.

## `parent_path`
Removes the last segment of the given path.

## `less_i`
Ignore-case string comparer.
