# How to Use Virtual Memory

Up to [Documentation](../README.md).

> A prerequisite to this tutorial is becoming familiar with these concepts:
>- [Virtual Memory](../concepts/vmem.md)
>- [Diagnostics](../concepts/diagnostics.md)

## Notes on Class Layouts
### Field Alignment
The compiler may add some padding between fields, so that each field is aligned to a processor word for faster access.
Thus, the memory layout of a class on a 64-bit system may be different from the memory layout of the same class on a 32-bit system. 
This optimization becomes problematic when an class instance has to be copied from memory to disk and vice versa.

To create predictable class layouts, "pack" the classes that get persisted to a byte:

```c++
#pragma pack(push, 1)

// Class definitions...

#pragma pack(pop)
```

### Types of Undefined Width
Similarly, avoid using types like `int`, `long long`, `size_t`, etc. in classes that will be saved on disk.
They may have a different width on different platforms.
Instead, use fixed-width types like `int32_t`, `int64_t`, etc.

## Create an `abc::diag::log_ostream`
It is recommended to pass in an `abc::diag::log_ostream` to the `abc::vmem::*` instances.
Visit the [How to Log Diagnostics](diagnostics.md) tutorial if needed.

## Create an `abc::vmem::pool_config`
An `abc::vmem::pool` could be configured by quite a few parameters, and more may be added in future.
All of those parameters are bundled in a class - `abc::vmem::pool_config`.

The `abc::vmem::pool_config` constructor may require some rationalization:

```c++
pool_config(
    const char* file_path,
    std::size_t max_mapped_page_count = abc::size::max,
    bool sync_pages_on_unlock = false,
    bool sync_locked_pages_on_destroy = false);
```
- `file_path` - Path to the pool file.
- `max_mapped_page_count` - Maximum number of mapped pages at the same time. Default: `abc::size::max`, i.e. no limit.
- `sync_pages_on_unlock` - Pages always get synced to disk before they get unmapped from memory.
When this parameter is `true`, pages get synced to disk every time their lock count drops to `0`.
More frequent saves would reduce data loss in case of a process crash at the cost of performance.
- `sync_locked_pages_on_destroy` - When a page is locked, it is in use, and it is likely "dirty".
Saving it in such a state may lead to inconsistencies.
However, at shutdown, a page may still appear locked (due to a bug or some other reason).
In such cases it may be better to save that page.
When this parameter is `true`, locked pages get synced to disk when the pool is destroyed.

## Create an `abc::vmem::pool`

```c++
// Create a log.
abc::stream::table_ostream table(std::cout.rdbuf());
abc::diag::debug_line_ostream<> line(&table);
abc::diag::str_log_filter<const char *> filter("", abc::diag::severity::important);
abc::diag::log_ostream log(&line, &filter);

// Use the path to this program to build the path to the pool file.
std::string process_dir = abc::parent_path(argv[0]);
std::string pool_path = process_dir + "/pool.vmem";

// Construct a pool instance.
// If the file doesn't exist, the pool will be initialized.
// If the fie exists, it should be a valid pool.
abc::vmem::pool_config config(
    pool_path.c_str(), // Path to the pool file.
    8                  // Max mapped pages. Maxed memory: 8 x 4KB = 32KB
);
abc::vmem::pool pool(std::move(config), &log);
```

## Do Not Tamper with the Root Page
The root page (at position `0`) is for `abc` metadata.
Do not tamper with it.

## Access the Start Page
The start page (at position `1`) is for program metadata.
This is where container states are stored.

__Note__: If a pool needs to contain many containers whose states do not fit on the start page, use a container whose items are container states.

__Note__: The `abc::vmem::page` instance locks the corresponding page, and prevents it from getting unmapped.
Once that instance is destroyed, the pool may unmap the page.
Thus, attempting to access its memory after the instance has been destroyed may lead to unpredictable behavior including a process crash.

```c++
// IMPORTANT: Ensure a predictable layout of the data on disk!
#pragma pack(push, 1)

// Max 4 items per vmem page.
using list_item = struct {
    std::uint64_t                 data; // Use types with predictable size!
    std::array<std::uint8_t, 900> dummy;
};

// Define the start page layout.
using start_page_layout = struct {
    abc::vmem::list_state   list1;
    abc::vmem::list_state   list2;
    abc::vmem::list_state   list3;
    abc::vmem::string_state str1;
    abc::vmem::string_state str2;
};

#pragma pack(pop)

// Map and lock the start page in memory.
abc::vmem::page start_page(&pool, abc::vmem::page_pos_start, &log);
start_page_layout* start_page_ptr = reinterpret_cast<start_page_layout*>(start_page.ptr());
```

## Using Large Data Sets through Lists
If you need to work with uniform records of data, organize them in a data structure like `abc::vmem::list` or `abc::vmem::map`.

### Traversing a List
```c++
for (abc::vmem::list::iterator itr = list.begin(); itr != list.end(); itr++) {
    // Do something with *itr
}
```

### Adding Items to a List
```c++
abc::vmem::list::iterator itr = list.end(); // ... or any other position
list.insert(itr, item);
```

### Removing Items from a List
```c++
abc::vmem::list::iterator itr = list.rend(); // ... or any other position
itr = list.erase(itr);
```

### Traversing a Map
```c++
for (abc::vmem::map::iterator itr = map.begin(); itr != map.end(); itr++) {
    // Do something with *itr
}
```

### Adding Items to a Map
```c++
map.insert(abc::vmem::map_value(key, value));
```

### Removing Items from a Map
```c++
map.erase(key);
```
