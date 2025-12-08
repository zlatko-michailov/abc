# log

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [diag/log.h](../../../src/diag/log.h)
Interface        | [diag/i/log.i.h](../../../src/diag/i/log.i.h)
Tests / Examples | [test/main.cpp](../../../test/main.cpp)

`log_ostream` is a binder of `log_line_ostream` and a `log_filter`.
The former formats log properties while the latter filters out unwanted lines.

`log_line_ostream` is a `line_output` specialization for diagnostic purposes.

`debug_line_ostream` is a specialization of `log_line_ostream` that outputs the most fields, and it does it in a human readable format.
This class is recommended for investigating issues.

`diag_line_ostream` is a specialization of `log_line_ostream` that also has the most fields, but there is no extra space, which reduces volume at the expense of readability.
This class is recommended for automatic collection and shipment to a remote location.

`test_line_ostream` is a specialization of `log_line_ostream` that has the minimum number of fields.
This class is recommended for test suites.

`log_filter` is a base interface to allow or to reject line entries before they are formatted and appended to the underlying table stream.

`str_log_filter` is a specialization of `log_filter` that allows entries from a given origin path and an equal or a higher severity than a given one.
