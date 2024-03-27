# log

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [diag/log.h](../../src/diag/log.h)
Interface        | [diag/i/log.i.h](../../src/diag/i/log.i.h)
Tests / Examples | [test/main.cpp](../../test/main.cpp)

`log_ostream` is a specialization of `table_ostream`.
It takes a filter to skip lines with a lower than a specified severity.

`log_line_ostream` is a line output stream interface for diagnostic purposes.

`debug_line_ostream` is a specialization of `log_line_ostream` that outputs the most fields and it does it in a human readable format.
This class is recommended for investigating issues.

`diag_line_ostream` is a specialization of `log_line_ostream` that also has the most fields, but there is no extra space, which reduces volume at the expense of readability.
This class is recommended for automatic collection and shipment to a remote location.

`test_line_ostream` is a specialization of `log_line_ostream` that has the minimum number of fields.
This class is recommended for test suites.
