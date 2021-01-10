# Virtual Memory

Up to [Documentation](../README.md).

Programs may need to manipulate large data sets that don't, or are impractical to, fit in memory.
Typically, programs only work with a small window of the large data set at any given time.

`abc` provides facilities at two levels that address the above problem.

## High Level - Data Structures
If the data records that a program needs to deal with are keyless, they can be organized in a linked list.
`abc` provides `vmem_list`, which offers methods very similar to `std::list`.

If the data records have keys, they can be organized in a map.
`abc` will soon provide `vmem_map`.

Using data structures avoids the hustle of mapping and unmapping individual pages to and from memory.

## Low Level - Pool and Pages
### `vmem_pool`
A `vmem_pool` is a persistent sequence of `vmem_page`s.
When the program needs a page, the pool maps it to memory.
When a page is no longer needed, it is synced back to disk, and eventually unmapped from memory.

A key parameter of a pool is the maximum number of pages it can map to memory at any given time.
This guarantees the maximum amount of memory a pool may use.

### `vmem_page`
A `vmem_page` represents a contiguous `4KB` block.
Each page has a unique position in the pool that never changes.
Thus, every byte in a pool can be accessed using the combination of a page position within the pool and a byte position within the page.

### `vmem_ptr`
A `vmem_ptr` is a smart pointer that keeps the referenced page locked in memory, which prevents the pool from unmapping it.

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
If a program needs to store more than 4KB of startup metadata, it should break it into a pieces smaller than 4KB, and organize them in a `vmem_list`.
Then, only store the `vmem_list_state` of that list in the start page.
