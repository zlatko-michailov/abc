# Virtual Memory

Up to [Documentation](../README.md).

Programs may need to manipulate large datasets that don't, or are impractical to, fit in memory.
Typically, programs only work with a small window of the large dataset at any given time.

`abc` provides facilities at two levels that enable programs to work with such large datasets.

## High Level - Data Structures
If the data records that a program needs to deal with are keyless, they can be organized in a linked list.
`abc` provides `abc::vmem::list`, which offers methods very similar to `std::list`.

If `abc::vmem::list`'s balancing policy is not a good fit, `abc` provides the more generic `abc::vmem::container`, which allows the program to implement its own specialization.

If the data records have keys, they can be organized in a map.
`abc` provides `abc::vmem::map`, which offers methods very similar to `std::map`.

For any other kind of data, `abc` provides `abc::vmem::linked`, which is simply a linked list of pages.

Using data structures avoids the hassle of mapping and unmapping individual pages to and from memory.

## Low Level - Pool and Pages
### `abc::vmem::pool`
A `abc::vmem::pool` is a persistent sequence of `abc::vmem::page`s.
When the program needs a page, the pool maps it into memory.
When a page is no longer needed, it is synced back to disk, and eventually unmapped from memory.

A key parameter of a pool is the maximum number of pages it can map to memory at any given time.
This guarantees the maximum amount of memory a pool may use.

### `abc::vmem::page`
A `abc::vmem::page` represents a contiguous `4KB` block.
Each page has a unique position in the pool that never changes.
Thus, every byte in a pool can be accessed using the combination of a page position within the pool and a byte position within the page.

### `abc::vmem::ptr`
A `abc::vmem::ptr` is a smart pointer that keeps the referenced page locked in memory, which prevents the pool from unmapping it.

__Note__: Accessing the memory of a page that is not locked, may cause a process crash. 

### Special Pages
#### Root Page
The root page is at position `0`.
It is reserved for the pool itself to store its own metadata.
A program developer should never need to access this page.

#### Start Page
The start page is at position `1`.
This is the place where a program can store whatever it needs.
Typically, that is the states of the lists and maps stored in this pool.
If a program needs to store more than 4KB of startup metadata, it should break it into pieces smaller than 4KB, and organize them in a `abc::vmem::list` or a `abc::vmem::map`.
Then, only store the `abc::vmem::list_state`/`abc::vmem::map_state` of that list/map in the start page.
