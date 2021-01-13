# vmem

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [vmem.h](../../src/vmem.h)
Interface        | [vmem.i.h](../../src/i/vmem.i.h)
Tests / Examples | [test/vmem.cpp](../../test/vmem.cpp)

It is recommended to use the high-level data structures - `vmem_list` and `vmem_map`.

> __NOTE__: `vmem_map` is not yet available.

If the developer needs to implement a data structure that is not provided by `abc`, they can do so by using the low-level concepts `vmem_pool`, `vmem_page`, and `vmem_ptr`.
