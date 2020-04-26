# abc
Minimalistic library of essential utilities for C++ development.


## Summary
`abc` contains a few (mostly unrelated) classes that are missing in the `std` library. The key entities are:
- timestamp
- log
- socket (TCP and UDP)

All classes are provided as headers, and must be compiled in client programs.
There is no precompiled flavor of the library.
That is because `abc` targets devices with pequliar architectures and small memory capacities.

### Why Minimalistic?
Platforms like [node.js](https://nodejs.org/en/) and [python](https://www.python.org/) have healthy ecosysems where independent developers provide individual components on top of the standard frameowrk.

Unfortunately, the `std` library is optimized for microbenchmarks, and not so much for implementing real-life use cases.
There are some key gaps that make it difficult to implement a component without the need to patch some of those gaps. For instance, futures are not `then`-able.
Thus, one can hardly implement a meaningful asynchronous pattern without implementing a new `future` and a new `promise`.
In today's connected world, there is no socket implementtion. Period.
Believe it or not, there is no thread-safe facility to convert a `time_point` to a timestamp. 

The alternative is to use a big library like [asio](https://think-async.com/Asio/) or [boost](https://www.boost.org/), but then you run into massive overlpas with the `std` library.


## Dependencies
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

### Tagging
Tagging is the assignment of unique numbers to places in the code, typically to arguments passed to exceptions and log entries.
That way, each log entry or exception could be pinpointed to the place in the code where it originated.


