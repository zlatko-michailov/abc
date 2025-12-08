# vmem

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [vmem/*.h](../../../src/vmem/pool.h)
Interface        | [vmem/i/*.i.h](../../../src/vmem/i/pool.i.h)
Tests / Examples | [test/vmem.cpp](../../../test/vmem.cpp)

## Foundation
`pool` represents a mapping between a memory window and a disk file.
Pages are persisted on disk, so content is retained across restarts.

`page` is used to load a page from disk into memory, and to unload it when the instance is destroyed.

`ptr` does everything `page` does plus pointing at a specific byte offset on the page.

## Containers 
`linked` is a linked list of opaque pages.
This is the foundation of all provided containers.

`container` is a _balanced_ list of items.
It is a `linked` where each page contains items.
The `container`applies a chosen balancing policy.
Balancing guarantees at least 50% space efficiency even when items are inserted to or erased from random positions.

`list` is a specialization of `container` where inserts everywhere except to the end, as well as all erases, are subject to balancing.
As long as items are only appended to the end, this container fills up pages fully.

`stack` is a specialization of `container` where no insert or erase is subject to balancing.
While inserting to and erasing from the end of a `list` would not cause balancing, it is safer to use `stack`, which is guaranteed not to balance.

`map` implements an efficient access to key-value pairs, similar to `std::map`.
Internally, `map` uses a `stack` of `container`s.
It could be used as an example how to build a new container using `container` and its specializations.

If the desired container cannot be implemented using `container`(s), the developer should try using `linked`.
