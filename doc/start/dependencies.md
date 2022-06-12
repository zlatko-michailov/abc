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
  - Raspberry Pi OS arm32
- Windows 10 x64 on PC, with [WSL 2](https://docs.microsoft.com/en-us/windows/wsl/install-win10) enabled.
  - openSUSE x64
  - Ubuntu x64

## Packages
The library itself doesn't need anything beyond the `std` C++ 11 headers and the POSIX-specific headers mentioned above.
According to the [GCC documentation](https://gcc.gnu.org/projects/cxx-status.html#cxx11), GCC C++ 4.8.1 or later should suffice.
`abc` has been compiled with Clang 10 and 11, as well as with GCC 7 through 9.

Although Clang is supported, the default compiler remains GCC.
That is due to a bug in Clang 7, which is the default Clang version on Raspberry Pi 4.

A compiler of choice may be used by passing in a `CPP` variable to `make`.

To build the tests and the samples, some essential packages are needed, which your system may already have.

### Refresh Repos
Refresh your repos, before trying to install any packages.

openSUSE
```
sudo zypper refresh
```

Ubuntu
```
sudo apt update
```

### Install Packages
openSUSE
```
sudo zypper install clang gcc-c++ make git zip unzip doxygen
```

Ubuntu
```
sudo apt install clang g++ make git zip unzip doxygen
```
