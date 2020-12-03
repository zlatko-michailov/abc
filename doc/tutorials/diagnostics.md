# How to Log Diagnostics

Up to [Documentation](../README.md).

> A prerequisite to this tutorial is becoming familiar with these concepts:
>- [Diagnostics](../concepts/diagnostics.md)
>- [Tagging](../concepts/tagging.md)

## Creating a `log_filter` and a `log_ostream`
The `log_ostream` instance must be constructed before and destroyed after any class instance that uses it.
The `log_filter` instance must be constructed before and destroyed after the `log_ostream` instance.
Therefore, planning the lifetime of these two instances is important.

Since our sample projects are small, and we have access to the `main()` function, we can use local variables on `main()`'s stack.
In larger projects, you may have to allocate these two instances on the heap, and assign them to global variables.

We must construct the `log_filter` instance first.
That is simple - all we have to pass in is a severity level.
We can always adjust that if we decide we need more or less log entries:
``` c++
// Allow entries of severity 'important' or higher from all components.
abc::log_filter filter(abc::severity::important);
```

To construct the `log_ostream` instance, we must pick a `line_ostream` format, and we have to pass in two other instances - a `streambuf` and a `log_filter`.

Let's pick `debug_line_ostream` as a line format.
To simplify our type definitions, we can define this shortcut:
``` c++
using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;
```

We already have a `log_filter`.
We need to choose a `streambuf`.

The simplest `streambuf` we can get is the console, i.e. the one that `cout` writes to:
``` c++
// Write to the console.
log_ostream log(std::cout.rdbuf(), &filter);
```

The console is easy to start with, but in production, there is nobody to look at it.
In that case, it is recommended to use either `duration_multifile_streambuf` or `size_multifile_streambuf`.
Both represent a file.
The former starts a new file when the given size is reached, while the latter starts a new file when the given duration has expired.

To use a `size_multifile_streambuf`:
``` c++
// We assume 'path' is a local variable that contains the path from where the program is running.

// Create files limited to 16KB each.
abc::size_multifile_streambuf sb(abc::size::k16, path);
log_ostream log(&sb, &filter);
``` 

## Logging
Log a lot.
Log everywhere state changes.
Log everywhere execution branches.

The cost of each log entry is the cost of an `if` statement.
The value may be priceless.
You can always filter entries in or out, but you can get entries unless you logged them.

``` c++
log->put_any(abc::category::my, abc::severity::optional, __TAG__, "REST: Sending status=%s", status);
```

### Choosing a Category
The category of a log entry is a factor you can use to filter out entries.
The value is an integer of type `severity_t` between `0` and `severity::abc - 1`.
It is up to you to decide what constitutes a category - a single class, a part of a class, or several classes.

### Choosing a Severity
Severity is also an integer, but the set of allowed values is very small and already defined.

Filtering log entries by severity is very useful when investigating an issue - start with a value of `severity::critical` and lower it until you find the thread where the issue occured.
So spend some time to devise a severity strategy, and stick to it throughout your program.

### Tagging
The tag is not another factor - it is rather the combination of all the other factors.
It is rarely used to search for, unless you want to verify a particular hypothesis.
The tag is primarily used to locate the place in the codebase where a given entry was logged.

When you log entries, always pass in the `__TAG__` sentinel.

Follow the guidance from the [Tagging](../concepts/tagging.md) conceptual page to convert the `__TAG__` sentinels into unique tags.
