# How to Log Diagnostics

Up to [Documentation](../README.md).

> A prerequisite to this tutorial is becoming familiar with these concepts:
>- [Diagnostics](../concepts/diagnostics.md)
>- [Tagging](../concepts/tagging.md)

## Creating a `log_ostream`
The `log_ostream` instance sits at the top of a stack of instances.
> All of those instances must be constructed before and destroyed after any class instance that uses it.

It is up to each program to decide how to make that happen.
Our sample projects are small, and we have access to the `main()` function.
So, we can use local variables on `main()`'s stack.
In larger projects, you may have to allocate these two instances on the heap, and assign them to global variables.
You may also consider using `std::shared_ptr` to automate the lifespan management.

### `std::streambuf`
At the very bottom, there is a plain `std::strambuf`.
This allows for logging to be easily redirected with only changing this parameter.

See [Media and Streams](../concepts/media_and_streams.md) for what `std::streambuf` implementations `abc` provides.
If you don't find the `std::streambuf` over the medium you need, look at the implementations above, and do your own.

### `abc::stream::table_ostream`
A log is a table - a sequence of lines that get pushed through the `std::streambuf`.

```c++
abc::stream::table_ostream table(std::cout.rdbuf());
```

### `abc::diag::log_line_ostream`
Each log entry is a line that get formatted in a chosen way, and gets appended to the `abc::stream::table_ostream`.
See [Diagnostics](../concepts/diagnostics.md) for what formatting options `abc` provides.
Feel free to implement your own `abc::stream::table_ostream` override, if you don't find one with the desired formatting.

```c++
abc::diag::debug_line_ostream<> line(&table);
```

### `abc::diag::log_filter`
The purpose of the filter is to eliminate "noise" entries to make it easier to find the important one.
`abc` provides only one override - `abc::diag::str_log_filter`, which allows entries with a matching origin and equal or higher severity to pass.

```c++
abc::diag::str_log_filter<const char*> filter("my_namespace::", abc::diag::severity::important);
```

### `abc::diag::log_ostream`
The `abc::diag::log_ostream` is the tip of the stack.
When it gets called, it first checks the filter.
If the entry passes the filter, it is sent to the `abc::diag::log_line_ostream`, which formats it, and sends it to the `abc::stream::table_ostream` it is linked to.

```c++
abc::diag::log_ostream log(&line, &filter);
```

### Putting It All Together
The simplest `std::streambuf` we can get is the console, i.e. the one that `std::cout` writes to.
As the example above shows, it is easy to start with, but in production, there is nobody to look at the console.
In that case, it is recommended to use either `abc::stream::duration_multifile_streambuf` or `abc::stream::size_multifile_streambuf`.
Both output to files.
The former starts a new file when the given duration has expired, while the latter starts a new file when the given size has been reached.

For the same of this tutorial, we will use `abc::stream::size_multifile_streambuf`.
We assume that `log_path` is a local variable that contains the path to a folder where we want the log files to be stored.
We will create log files limited to 16KB each.

```c++
abc::stream::size_multifile_streambuf sb(abc::size::k16, path);
abc::stream::table_ostream table(&sb);
abc::diag::debug_line_ostream<> line(&table);
abc::diag::str_log_filter<const char*> filter("my_namespace::", abc::diag::severity::important);
abc::diag::log_ostream log(&line, &filter);
```

## Logging
Log a lot.
Log everywhere state changes.
Log everywhere execution branches.

The cost of each log entry is the cost of an `if` statement.
The value may be priceless.
You can always filter entries in or out, but you cannot get entries unless you logged them.

Prior to version 2.0, the `abc::diag::log_ostream` had to be used directly.
That is still possible, but it is not recommended.

Since version 2.0, the recommended way is to derive from `abc::diag::diag_ready`, which is a thin wrapper around `abc::diag::log_ostream` that keeps the full name of the current class as _origin_.
Then, each method should only pass in its own name as _suborigin_ along with the rest of the logging attributes.

It is recommended to log _callstack_ entries from non-trivial method that mark the begin and end of the method:
```c++
constexpr const char* suborigin = "my_method()";
diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

...

diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
```

If your program needs to throw an exception, it is recommended to do that through the `abc::diag::diag_ready` base, which will log the exception before throwing it, that will contain a unique _tag_ to identify the point of origin.

### SAL
The program's code will be cleaner if you use the SAL methods instead of throwing exceptions directly:
- `expect()` - potentially throws an exception that derives from `std::logic_error`.
This method could be called to check state at method begin or to check results from calls. 
- `ensure()` - potentially throws an exception that derives from `std::logic_error`.
This method could be called to check state at method end.
- `assert()` - potentially throws an exception that derives from `std::logic_error`.
This method could be called in cases where neither `expect()` nor `ensure()` fit (although such cases should not exist).
- `require()` - potentially throws `std::runtime_error` or a custom exception (that preferably derives from `std::runtime_error`.)
While the previous methods represent static code checks, this one represents a runtime check.

### Choosing a Severity
Severity is an integer, but the set of allowed values is very small and already defined.

Choose severities wisely when you log.
Otherwise, filtering by severity may not be effective.

### Tagging
The tag is used to locate the place in the codebase where a given log entry originates from.

When you log entries, always pass in the `__TAG__` sentinel.

Follow the guidance from [Tagging](../concepts/tagging.md) to convert the `__TAG__` sentinels into unique tags.
