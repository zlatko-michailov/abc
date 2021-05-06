# How to Use Virtual Memory

Up to [Documentation](../README.md).

> A prerequisite to this tutorial is becoming familiar with this concept:
>- [Virtual Memory](../concepts/vmem.md)
>- [Diagnostics](../concepts/diagnostics.md)

## Notes on Class Layouts
### Field Alignment
The compiler may add some padding between fields, so that each field is aligned to a processor word for faster access.
Thus, the memory layout of a class on a 64-bit system may be different from the memory layout of the same class on a 32-bit system. 
This optimization becomes problematic when an instance has to be copied to and from disk.

To create predictable class layouts, "pack" the classes that get persisted to a byte:

``` c++
#pragma pack(push, 1)

// Class definitions...

#pragma pack(pop)
```

### Types of Undefined Width
Similarly, avoid using types like `int`, `long long`, `size_t` in classes that will be exchanged with the disk.
They may have a different width on different platforms.
Instead, use fixed-width types like `int32_t`, `int64_t`, etc.

## Alias Classes
Each class template has two or three parameters, which makes type names long.
It is advisable to create shorter aliases to those long type names.

## Create a `log_filter` and a `log_ostream`
It is recommended to pass in a `log_ostream` to the `vmem_` instances.
Visit the [How to Log Diagnostics](diagnostics.md) tutorial if needed.

## Create a `vmem_pool` Instance

``` c++
// Construct a pool instance.
// If the file doesn't exist, the pool will be initialized.
// If the fie exists, it should be a valid pool.
abc::vmem_pool<8, log_ostream> pool(path, &log);
```

The template parameter `8` represents the maximum number of 4KB pages this `vmem_pool` may map in memory at any given time.
You should choose a number that fits both your use case and the capacity of your device.
Choosing a number below `3` is strongly discouraged.

## Access the Start Page
The start page (at position `1`) is dedicated to program metadata.
This where states are stored.

__IMPORTANT__: The `vmem_page` instance locks the corresponding page, and prevents it from getting unmapped.
Once that instance is destroyed, the pool may unmap it.
Thus, attempting to access its memory after the instance has been destroyed may lead to unpredictable behavior including a process crash.

``` c++
// Map and lock the start page in memory.
vmem_page start_page(&pool, abc::vmem_page_pos_start, &log);
vmem_start_page* start_page_ptr = reinterpret_cast<vmem_start_page*>(start_page.ptr());
```

Define `vmem_start_page` to reflect the layout you want to start with.
For instance, if you want to have two lists, do something like this:
``` c++
#pragma pack(push, 1)

using vmem_start_page = struct {
	abc::vmem_list_state	list1;
	abc::vmem_list_state	list2;
};

#pragma pack(pop)
```

Remember to pack the layout to a byte, and mind the 4KB limit.

## Using Large Data Sets through Lists
If you need to work with uniform records of data, organize them in a data structure like `vmem_list` or `vmem_map`.

### Traversing a List
``` c++
for (vmem_list::iterator itr = list.begin(); itr != list.end(); itr++) {
	// Do something with *itr
}
```

### Adding Items to a List
``` c++
vmem_list::iterator itr = list.end(); // ... or any other position
list.insert(itr, item);
```

### Removing Items from a List
``` c++
vmem_list::iterator itr = list.rend(); // ... or any other position
itr = list.erase(itr);
```

### Traversing a Map
``` c++
for (vmem_map::iterator itr = map.begin(); itr != map.end(); itr++) {
	// Do something with *itr
}
```

### Adding Items to a Map
``` c++
map.insert(abc::vmem_map_value(key, value));
```

### Removing Items from a Map
``` c++
map.erase(key);
```
