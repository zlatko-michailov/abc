# Release Notes

Up to [Documentation](README.md).

## 2.0.0
### IMPORTANT
> Version 2.0 is a full rewrite of `abc` to improve usability and integration with the `std` library.

While all classes and their public methods have remained semantically the same, migration from previous versions should be done carefully.
Newly added classes should be considered.

### Retrospection
Originally, `abc` had a strong premise of not throwing exceptions.
That led to not allocating heap memory, which could potentially throw.
Because of that, methods that needed memory required buffers from the caller.
The caller had to estimate the size of each buffer, had to allocate it, and had to dispose of it, which was far from a smooth experience.
This premise prevented integration with the `std` library where most classes allocate heap memory and eventually throw exceptions.

Another usability spoiler was the `Log` template parameter.
This was done to automate the lifecycle management of the `log_ostream` instance - the caller could choose whether to use raw pointers or smart pointers.
Unfortunately, this led to overly complicated class and method signatures.
Since each class had its own `Log` template parameter, when class `A` referenced class `B`, class `B` had to become a template parameter of class `A`.

### Exceptions
The first step was to embrace exceptions.
That opened the door for using `std` containers internally instead of asking the user to provide a buffer.

### `log_ostream`
As most `std` APIs do, `abc` classes take a raw pointer whenever a reference is needed, including `log_ostream` references.
It is up to the caller to manage the lifecycle of those instances.

### SAL via `diag_ready`
Another benefit of embracing exceptions is that simple SAL checks could be used instead without complicating the codebase.
Also, `abc` implemented a consistent pattern of __callstack__ logging. 

### Subnamespaces
`std` is not very liberal with namespaces, and `abc` tried to follow that path.
However, the header and class names had unnecessary prefixes.
This rewrite was taken as an opportunity to move the components into subfolders and subnamespaces respectively.

### Stream Readers and Writes
Working with streams directly requires the caller to know the protocol/format of the stream, which takes effort and is prone to errors. 
Therefore, the concepts of Stream Readers and Writers were implemented for `http` and `json`.
That came with tangible structs like `http::request`, `http::response`, and `json::value`.

## 1.20.0
- Theme
  - `vmem_string`.
  - Streaming in and out of `vmem_string`.
- Breaking Changes
  - `basic_iterator_state` - operators `++`, `--`, and `*` moved to `basic_iterator`.
- Bugs
  - `itr++` - postfix was always executing as prefix.
  - `*itr++` - could cause crashes when used at the end of a container.
- Additions
  - `vmem_string`
  - `vmem_string_streambuf`
- Refactoring
  - `basic_iterator`
  - `basic_iterator_state`

## 1.19.0
- Theme
  - TLS - proof of feasibility.
  - OpenSSL-based implementation.
- Breaking Changes
  - `*_socket` - Rename `handle()` to `fd()`. 
  - `*_socket` - Remove constructors without `family` parameter.
  - `endpoint` - Add template parameters for server and client sockets.
  - `log_ostream` - `put_blank_line()` requires category and severity for filtering.
- Bugs
  - `endpoint` - Exit the main loop when no more requests.
- Additions
  - `openssl_tcp_server_socket`
  - `openssl_tcp_client_socket`
  - Test - `endpoint`
- Refactoring
  - None.

## 1.18.0
- Breaking Changes
  - none.
- Bugs
  - Include `log.i.h` from gpio headers.
  - Check the `smbus` pointer in the `gpio_smbus_grayscale` constructor.
  - Fix deleted `gpio_smbus_motion` constructor.
  - Synchronize calls to `ioctl()` in `gpio_smbus`.
- Additions
  - `gpio_smbus_motion`
  - `gpio_smbus_motion_tracker`
  - Sample picar_4wd - track location.
- Refactoring
  - None.

## 1.17.0
- Breaking Changes
  - `vmem_ptr` - rename `item_pos` to `byte_pos`.
  - Uninternalize type names
    - `_stream` -> `stream`
    - `_istream` -> `istream`
    - `_ostream` -> `ostream`
    - `_http_istream` -> `http_istream`
    - `_http_ostream` -> `http_ostream`
    - `_client_socket` -> `client_socket`
    - `_basic_socket` -> `basic_socket`
    - `_vmem_iterator` -> `vmem_basic_iterator`
    - `_vmem_iterator_state` -> `vmem_basic_iterator_state`
    - `_vmem_mapped_page` -> `vmem_mapped_page`
    - `_vmem_mapped_page_totals` -> `vmem_mapped_page_totals`
    - `_http_state` -> `http_state`
    - `_json_state` -> `json_state`
- Bugs
  - Move constructors
    - `buffer_streambuf`
    - `stream`, `istream`, `ostream`
    - `table_ostream`, `line_ostream`
    - `log_ostream`, `..._line_ostream`
    - `http_..._stream`
    - `json_..._stream`
    - `..._multifile_streambuf`
    - `..._socket`
    - `endpoint`
    - `vmem_pool`
  - `vmem_pool` - destructor
  - `timestamp` - time parts
  - `color::yellow`
- Additions
  - none
- Refactoring
  - Add doxygen annotations for all classes and most of their methods.
  - Tests - simplify buffer initializations.

## 1.16.0
- Breaking Changes
  - Template parameter `Predicate` in `json_istream` and `http_istream` has become strictly `CharPredicate`.
- Bugs
  - Zero initialization.
  - Conditions in game samples.
  - Accurate `#include`s.
- Additions
  - Configurable compiler and options
    - `make CPP_OPT_STD=--std=c++17`
  - Support for `clang`
    - `make CPP=clang++`
  - Add warnings.
  - Builds on Mac OS.
  - Conditional compilation based on OS.
- Refactoring
  - Unused variables removed.

## 1.15.0
- Breaking Changes
  - crc - moved to boneyard
- Bugs
  - none
- Additions
  - GPIO
    - Plain GPIO
      - `chip`, `gpio_input_line`, `gpio_output_line`, `gpio_pwm_emulator`
    - SMBus
      - `gpio_smbus`, `gpio_smbus_pwm`
    - Specializations
      - `gpio_ultrasonic`, `gpio_smbus_motor`, `gpio_smbus_servo`, `gpio_smbus_grayscale`
  - picar_4wd - SunFounder PiCar 4WD
- Refactoring
  - socket - reduce logging verbosity

## 1.14.0
- Breaking Changes
  - none
- Bugs
  - Tic-Tac-Toe visualization issues.
- Additions
  - Connect 4
    - "Thinking Slow" engine
    - "Thinking Fast" engine
- Refactoring
  - none

## 1.13.0
- Breaking Changes
  - none
- Bugs
  - `_basic_socket::bind(SO_REUSEADDR)` to prevent failure on quick restart
  - `const_iterator`
    - To support `++` and `--`
    - To support binding to `iterator`
- Additions
  - Tic-Tac-Toe
    - "Thinking Slow" engine
    - "Thinking Fast" engine
- Refactoring
  - Iterators

## 1.12.3
- Breaking Changes
  - `vmem_container_result2`
- Bugs
  - none
- Additions
  - `vmem_map` - a B-tree of key-value pairs
  - `vmem_stack` - a container with no balancing
- Refactoring
  - none

## 1.12.2
- Breaking Changes
  - `vmem_list_state` - version up
  - `vmem_root_page` - free_pages becomes a `vmem_linked`
- Bugs
  - `vmem_list::erase()` - was not balancing with previous
  - `vmem_ptr(vmem_page_pos_nil)` - was allocating a new page.
- Additions
  - `vmem_linked` - a sequence of linked pages
  - `vmem_container` - a sequence of items
- Refactoring
  - `vmem_pool`
  - `vmem_list`

## 1.12.1
- No breaking changes.
- Virtual memory:
  - `vmem_pool` bug fix
  - `vmem_iterator`
  - `vmem_list` balance items on `insert()` and on `erase()`

## 1.12.0
- No breaking changes.
- Virtual memory:
  - `vmem_pool`
  - `vmem_list`
  - Sample
- Move non-includable files to subfolders. 

## 1.11.0
- No breaking changes.
- Public verification.

## 0.11.0
- No breaking changes.
- Improved documentation.

## 0.10.0
- No breaking changes.
- Reduce the C++ requirement from 17 to 11.
- Support 32-bit.
- Fix build on Ubuntu.
- `http`
  - Enable ignore-case.
- `socket`
  - Bind to _any_ host.
  - Shutdown before closing.
- Basic sample
  - Improve usability.

## 0.9.0
- No breaking changes.
- `endpoint`
  - A simple web server.
- Basic sample
  - A simple app that solves systems of two linear equations of two variables.
  - The app is interacted with using a web browser.

## 0.8.0
- Breaking changes:
  - Refactored `log` and its related entities. Construction and usage are similar, but not the same.
  - The `test_log` shortcut was moved from the product to the test project.
  - Json streams take `Log` as the last template parameter.
  - `log` can only be passed as a raw pointer.
- `log_ostream` is now based on any `std::streambuf`, and thus can send content to any medium for which there is a `std::streambuf`.
`log_ostream` is now a specialization of `table_ostream`.
- `table_ostream` is a generic stream of lines.
- `line_ostream` is a stream over a fixed char buffer that can flush to a `table_ostream`.
`log_ostream` is one specialization of `table_ostream` and `line_ostream`.
Other tabular streams can be created in user space.
- `multifile_streambuf` a derivate of `std::filebuf` that streams out to a sequence of files.
  - `duration_multifilie_streambuf` automatically starts a new file after a given duration.
  - `size_multifilie_streambuf` automatically starts a new file after a given size.

## 0.7.0
- Breaking changes:
  - Removed method `gcount()` from `http_request_ostream` and `http_response_ostream`.
- `json`
  - JSON streams.

## 0.6.0
- No breaking changes.
- `http`
  - _Syntactic_ http streams.
- `ascii`
  - Basic character predicates.

## 0.5.0
- Breaking changes:
  - `socket.h` is no longer in the `posix` subfolder.
  It is in the same folder with all other headers.
  The `posix` subfolder has been removed.
  - `streambuf.h` has been renamed to `buffer_streambuf.h` and class `streambuf_adapter` has been renamed to `buffer_streambuf`.
- Add `socket_streambuf`class to enable creation of streams over sockets.

## 0.4.0
- No breaking changes.
- `socket`, `exception`
  - Enable all `socket` and `exception` classes to take a `log`. This way `abc` classes can log diagnostic info in the app's log.
- `log`
  - Add a `format_binary()` method that formats a byte buffer.
  - Enable all `log_view` class to format byte buffers. This enables sockets to log sent and received bytes.

## 0.3.0
- First promising release. 
