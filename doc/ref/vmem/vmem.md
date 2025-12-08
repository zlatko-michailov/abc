# vmem

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [vmem.h](../../src/vmem.h)
Interface        | [vmem.i.h](../../src/i/vmem.i.h)
Tests / Examples | [test/vmem.cpp](../../test/vmem.cpp)

`vmem_linked` is a linked list of opaque pages.
This is the foundation of all provided data structures.

`vmem_container` is a _balanced_ list of items.
It is a `vmem_linked` where each page contains items.
The `vmem_container`applies a chosen balancing policy.
Balancing guarantees at least 50% space efficiency even when items are inserted to or erased from random positions.

`vmem_list` is a specialization of `vmem_container` where inserts everywhere except to the end as well as all erases are subject to balancing.
As long as items are only appended to the end, this structure fills up pages fully.

`vmem_stack` is a specialization of `vmem_container` where no insert or erase is subject to balancing.
While inserting to and erasing from the end of a `vmem_list` would not cause balancing, it is safer to use `vmem_stack`, which is guaranteed not to balance.

`vmem_map` implements an efficient access to key-value pairs, similar to `std::map`.
Internally, `vmem_map` uses a `vmem_stack` of `vmem_container`s.
It could be used as an example how to build a new data structure using `vmem_container` and its specializations.

If the desired data structure cannot be implemented using `vmem_container`(s), the developer should try using the `vmem_linked`.


At the lowest level of the virtual memory framework are `vmem_pool`, `vmem_page`, and `vmem_ptr`.

`vmem_pool` represents a mapping between a memory window and a disk file.
Pages are persisted on disk, so content is retained across restarts.

`vmem_page` is used to load a page from disk into memory, and to unload it when the instance is destroyed.

`vmem_ptr` does everything `vmem_page` does plus pointing at a specific byte offset on the page.
 
