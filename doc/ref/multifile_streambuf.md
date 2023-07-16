# multifile_streambuf

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [multifile_streambuf.h](../../src/multifile_streambuf.h)
Interface        | [multifile_streambuf.i.h](../../src/i/multifile_streambuf.i.h)
Tests / Examples | n/a

`multifile_streambuf` is a specialization of `std::filebuf`.
It is the base class that implements writing to a sequence of files.
Each file's name matches the UTC time down to one second.
Therefore, a `multifile_streambuf` must not be _reopened_ more than once within one second.

`duration_multifile_streambuf` is a specialization of `multifile_streambuf` where the instance is automatically _reopened_ when a given duration has passed.
_Reopening_ is done only on `sync()`, which happens when `flush()` is called on overlying stream.

`size_multifile_streambuf` is a specialization of `multifile_streambuf` where the instance is automatically _reopened_ when a given size has been exceeded.
_Reopening_ is done only on `sync()`, which happens when `flush()` is called on overlying stream.
