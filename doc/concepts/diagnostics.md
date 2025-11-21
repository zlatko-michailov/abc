# Diagnostics

Up to [Documentation](../README.md).

Diagnostics is an important part of running UI-less programs, like services and programs on IoT devices.
That is why it is a core concept in `abc`.

## `abc::diag::log_ostream`
At top-level, there is a [`abc::diag::log_ostream`](../ref/diag/log.md) instance.
The program developer should keep that instance alive, and should pass it to other class instances that support diagnostics.

The job of [`abc::diag::log_ostream`](../ref/diag/log.md) is to put the desired subset of log lines into the underlying [`abc::stream::table_ostream`](../ref/stream/table_stream.md).
It does that by orchestrating a [`abc::diag::log_line_ostream`](../ref/diag/log.md) and a [`abc::diag::log_filter`](../ref/diag/log.md) instance.
If the [`abc::diag::log_filter`](../ref/diag/log.md) decides that the diagnostic entry should be persisted, the [`abc::diag::log_line_ostream`](../ref/diag/log.md) formats it, and  puts it into the underlying [`abc::stream::table_ostream`](../ref/stream/table_stream.md).
Otherwise, the diagnostic entry is dropped off.

## Line
There are three classes that derive from [`abc::diag::log_line_ostream`](../ref/diag/log.md).
Other overrides of [`abc::diag::log_line_ostream`](../ref/diag/log.md) may be added by programs.

### `abc::diag::debug_line_ostream`
[`abc::diag::debug_line_ostream`](../ref/diag/log.md) includes all supported diagnostic fields formatted in fixed-width columns.
This line format is human-readable as is.
It is recommended as a primary choice of override.

### `abc::diag::diag_line_ostream`
[`abc::diag::diag_line_ostream`](../ref/diag/log.md) also includes all supported diagnostic fields, but this time they are formatted without any space padding.
This line format optimizes the amount of bites persisted and transported at the expense of human readability.
An additional tool or script may be needed to make it human-readable.

### `abc::diag::test_line_ostream`
[`abc::diag::test_line_ostream`](../ref/diag/log.md) includes a subset of the input diagnostic data formatted in a way intended for aesthetics rather than for real diagnostics.

## Filter
The purpose of the filter is to skip diagnostic entries based on _origin prefix_ and _severity_.

The provided [`abc::diag::str_log_filter`](../ref/diag/log.md) implements the pure virtual [`abc::diag::log_filter`](../ref/diag/log.md).

An instance of another class may be used as long as that class implements [`abc::diag::log_filter`](../ref/diag/log.md).

## Diagnostic Data
There is a set of "known" fields and an optional free-text tail.
Here are the known fields:

### Origin
Origin is a string that represents the "path" to the place where the event originates.
It looks like "namespace::class::method".

### Severity
Severity is an unsigned integer of type [`abc::diag::severity_t`](../ref/diag/log.md).
The supported values are already defined in `abc::diag::severity`.
The smaller the value, the higher the severity.

### Tag
Tag is an unsigned integer of type [abc::diag::tag_t](../ref/diag/tag.md).
For more information, visit the [Tagging](tagging.md) concept.

## Examples
For examples on how to create diagnostic components, look up the following test and sample files:
- [test/main.cpp](../../test/main.cpp)
- [samples/basic/main.cpp](../../samples/basic/main.cpp)
- [samples/tictactoe/main.cpp](../../samples/tictactoe/main.cpp)
- [samples/connect4/main.cpp](../../samples/connect4/main.cpp)
- [samples/vmem/main.cpp](../../samples/vmem/main.cpp)
- [samples/picar_4wd/main.cpp](../../samples/picar_4wd/main.cpp)

## Further Reading
For guidance and examples on how to code diagnostic calls throughout your program, visit tutorial [How to Log Diagnostics](../tutorials/diagnostics.md).
