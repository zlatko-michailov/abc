# Diagnostics

Up to [Documentation](README.md).


## Log
The log is an ostream that most other classes accept to log diagnostic messages.

Exceptions are also logged.

__Streambuf classes do not log.__
That is because the log may be using that streambuf, and there is a possibility for an infinite loop and/or a process crash.


## Level
In order to keep the reading of logs manageable, a guideline has to be defined and followed across all classes.
Here it is:
- `critical` - The program is terminating.
- `warning` - An exception was thrown and caught. The program can continue.
- `important` - Messages that should be available when method tracing messages filtered out.
- `callstack` - Denote entering/leaving a method. These messages may be used by contract assertion methods like `expect` and `ensure`.
- `optional` - Trace the code path or log key variables.
- `debug` - Log primitive variable values.
- `verbose` - Dump buffers or other data structures.


## Origin
An origin identifies a class or namespace.
It is expected that these identifications are hierarchical.
That means all classes and sub-namespaces from a given namespace share the same prefix. 
That is important for filtering.


## Filtering
Filtering of irrelevant messages is necessary when diagnosing issues.

__Filtering must be cheap and quick.__

Filtering is supported on 2 dimensions:

### By Level
Messages with levels below a given one are not logged.

### By Origin
Given a bit mask and an origin, messages whose mask-matching bits don't match the origin are not logged.
 