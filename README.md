# abc
Header-only library of essential utilities for C++ development.


## Summary
`abc` is a header-only library that contains a few classes that are missing in the `std` library. The key entities are:
- timestamp
- log
- test
- socket (TCP and UDP)

All classes are provided as headers, and must be compiled in client programs.
There is no precompiled flavor of the library.
That is because `abc` targets devices with pequliar architectures and small memory capacities.


## Brief Class Description
### `timestamp`
A thread-safe, lock-free, facility to calculate the date and time by a `std::chrono::time_point`.

### `log`
A flexible output facility. The driving entity is `log`. It takes three other sub-entities:
- `LogContainer`- adapter to the output medium:
	- `log_container::ostream` - for any existing `std::ostream` instance.  
	- `log_container::file` - for a disk file.  
- `LogView` - line formatter:
	- `log_view::debug` - nicely organized columns suitable for a human eye.
	- `log_view::diagg` - condensed columns suitable for shipping of the network.
	- `log_view::test` - balanced information suitable test frameworks.
	- `log_view::blank` - prints no predefined field, only custom content.
- `LogFilter` - decides whether a line should be printed or not:
	- `log_filter::none` - no filtering, i.e. print everything.
	- `log_filter::none` - don't print anything.
	- `log_filter::seveity` - print lines with higher or equal to the given seveirty.
More implementations of each sub-entity could be done in user-space.

### `test`
A simple test framework. It is used to test `abc` itself. It could be used for projects that use `abc`.

### `socket`
__Note__: This entity is only available on POSIX systems where the BSD socket C API is available.

This is a C++ wrapper around the BSD socket C API. The following self-explanatory classes are provided:
- `udp_socket`
- `tcp_server_socket`
- `tcp_client_socket`


## Tool and Platform Dependencies
### GCC
The project is compiled using GCC 9.
It may be possible to compile with an earlier version of GCC or even with a differnt compiler, but that hasn't been tried.

### POSIX
The socket classes use the BSD socket C library.
This project has been tested on Windows 10 with the `Windows Subsystem for Linux` feature enabled.


## Try `abc`
Clone the repo loclaly.

From the root repo folder, run `make`.
This will compile and run the tests. The product will be located in the `out/abc/<version>/inc` subfolder. 

To see examples of how to use these classes, open the test files.


## Use `abc`
At least for now, the process is manual. From a built repo, copy the `out/abc` subfolder to the location where your project includes its dependencies, e.g. something like `deps/abc`.

Keep an eye on the `abc` [repo](https://github.com/zlatko-michailov/abc) for updates. You can follow the above procedure to install multiple versions of `abc` side by side, so you can easily evaluate new versions.



