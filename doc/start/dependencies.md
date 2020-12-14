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
  - openSUSE x64
  - Ubuntu x64

## Packages
The library itself doesn't need anything beyond the `std` C++ 11 headers and the POSIX-specific headers mentioned above.
According to the [GCC documentation](https://gcc.gnu.org/projects/cxx-status.html#cxx11), GCC C++ 4.8.1 or later should suffice.
`abc` has been compiled with GCC C++ 9 and GCC C++ 10.

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
sudo zypper install gcc-c++ make git zip unzip
```

Ubuntu
```
sudo apt install g++ make git zip unzip
```
