# abc
> ATTENTION!
>
> The next release will be a complete rewrite.  
> While the entities will remain the same, there will be many breaking changes.
> Plan your adoption accordingly.

___

> For a complete guide on `abc`, please visit the project's [Documentation](doc/README.md).

`abc` is a C++ header-only library that complements the `std` library. The most notable `abc` entities are:
- HTTP endpoint
- TCP and UDP sockets
- HTTP streams
- JSON streams
- Virtual memory
- GPIO and SMBus

Those entities enable a C++ daemon running on remote device to have a GUI as well as a REST API surface.

Additionally, `abc` provides:
- generic table output stream
- diagnostic log
- test framework
- timestamp

All classes are provided as headers.
The needed ones must be included and compiled in client programs.
There is no precompiled flavor of the library that needs to be linked.

> To get started, please visit the project's [Documentation](doc).
