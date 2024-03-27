# Diagnostics

Up to [Documentation](../README.md).

Diagnostics is an important part of running a service.
That is why it is a core concept in `abc`.

## `log_ostream`
At top-level, there is a [`log_ostream`](../ref/log.md) instance.
The program developer should keep that instance alive, and should pass it to other class instances that support diagnostics.

The job of [`log_ostream`](../ref/log.md) is to put the desired subset of log lines into the underlying [`table_ostream`](../ref/table_stream.md).
It does that by orchestrating a [`log_line_ostream`](../ref/log.md) and a [`log_filter`](../ref/log.md) instance.
If the [`log_filter`](../ref/log.md) decides that the diagnostic entry should be persisted, the [`table_ostream`](../ref/table_stream.md) formats it, and the [`log_line_ostream`](../ref/log.md) puts it into the underlying [`table_ostream`](../ref/table_stream.md).
Otherwise, the diagnostic entry is thrown away.

## Line
There are three classes that derive from [`log_line_ostream`](../ref/log.md).
Other overrides of [`log_line_ostream`](../ref/log.md) may be used as well.

### `debug_line_ostream`
[`debug_line_ostream`](../ref/log.md) includes all supported diagnostic fields formatted in fixed-width columns.
This line format is human-readable as is.
It is recommended as a primary choice of override.

### `diag_line_ostream`
[`diag_line_ostream`](../ref/log.md) also includes all supported diagnostic fields, but this time they are formatted without any space padding.
This line format optimizes the amount of bites persisted and transported at the expense of human readability.
An additional tool or script may be needed to make it human-readable.

### `test_line_ostream`
[`test_line_ostream`](../ref/log.md) includes a subset of the input diagnostic data formatted in a way intended for aesthetics rather than for real diagnostics.

## Filter
The purpose of the filter is to skip diagnostic entries based on _origin prefix_ and _severity_.

The provided [`str_log_filter`](../ref/log.md) implements the pure virtual [`log_filter`](../ref/log.md).

An instance of another class may be used as long as that class implements [`log_filter`](../ref/log.md).

## Diagnostic Data
There is a set of "known" fields, and an optional free-text tail.
Here are the known fields:

### Origin
Origin is a string that represents the "path" to the place where the event originates.
It looks like "namespace::class::method".

### Severity
Severity is an unsigned integer of type [`severity_t`](../ref/log.md).
The supported values are already defined in namespace `severity`.
The smaller the value, the higher the severity.

### Tag
Tag is an unsigned integer of type [tag_t](../ref/tag.md).
For more information, visit the [Tagging](tagging.md) concept.

## Further Reading
For guidance and examples on how to code diagnostic calls throughout your program, visit tutorial [How to Log Diagnostics](../tutorials/diagnostics.md).