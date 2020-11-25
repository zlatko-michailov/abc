# Dependencies

Up to [Documentation](../README.md).

## POSIX OS
The following `abc` features use POSIX API:
- The `socket` classes use the BSD socket API.
- `endpoint` uses the POSIX `stat` function.

`abc` has been tested on:
- Linux on PC
  - openSUSE x64
  - Ubuntu x64
- Linux on Raspberry Pi 4
  - openSUSE arm64
  - Raspbian (Debian) arm32
- Windows 10 x64 on PC, with [WSL 2](https://docs.microsoft.com/en-us/windows/wsl/install-win10) enabled.

## Package `gcc-c++`
`abc` has only been compiled with GCC C++.
It may be possible to compile it with other compilers, but that has not been tested, nor is it planned.

`abc` only uses C++ 11 features.
According to the [GCC documentation](https://gcc.gnu.org/projects/cxx-status.html#cxx11), GCC C++ 4.8.1 or later should suffice.
`abc` has been compiled with GCC C++ 9 and GCC C++ 10.

## Package `zip`
`zip` is an optional package.
It is not used by the library itself.
It is only used by the `makefile` for packaging a release.