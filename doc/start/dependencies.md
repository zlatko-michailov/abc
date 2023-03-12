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
  - openSUSE aarch64
  - Raspberry Pi OS aarch32
- Windows 11 x64 and Windows 10 x64 on PC, with [WSL 2](https://docs.microsoft.com/en-us/windows/wsl/install-win10) enabled.
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
sudo zypper install clang gcc-c++ make git zip unzip doxygen graphviz
```

Ubuntu
```
sudo apt install clang g++ make git zip unzip doxygen graphviz
```

## GPIO Setup
There should be nothing to do on Raspberry Pi OS.
The packages that enable access to GPIO and SMBus should be preinstalled.
The user that is enabled on the box is the root user, and has the necessary access.

On other Linux distributions, a little extra work is needed.

### Install Packages
openSUSE
```
sudo zypper install libgpiod2 libgpiod-utils libi2c0 i2c-tools
```

Here are the original pages on openSUSE:
- [GPIO](https://en.opensuse.org/openSUSE:GPIO)
- [I2C](https://en.opensuse.org/openSUSE:I2C)
 
### Grant Access to Non-root Users
The Linux kernel creates device files at every boot with access only to the `root` user.
If other users should be able to use GPIO and/or SMBus, access to those device files should be granted to those users using `chmod`.

That could further be automated by executing a script upon boot.

openSUSE

Create `/etc/init.d/after.local` with the following content, or append these commands to it, if it already exists: 
```
#!/bin/sh
sudo chmod go+rw /dev/gpio*
sudo chmod go+rw /dev/i2c*

```
Make sure that file is executable:
```
sudo chmod 777 /etc/init.d/after.local
```

Reboot, and check that `chmod` has been applied:
```
ls -l /dev/gpio*
ls -l /dev/i2c*
```

If it didn't work out, check the status of the `after-local` service:
```
sudo systemctl status after-local
```
