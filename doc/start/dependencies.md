# Dependencies

Up to [Documentation](../README.md).

## POSIX OS
The following `abc` features use POSIX API:
- The `socket` classes use the POSIX socket API.
- `endpoint` uses the POSIX `stat` function.

### Linux
The following `abc` features use Linux API:
- The `gpio` and `smbus` classes depend on the Linux `gpio` and `i2c` API.

`abc` has been tested on:
- Linux on PC
  - openSUSE x64
  - Ubuntu x64
- Linux on Raspberry Pi 4
  - openSUSE aarch64
  - Raspberry Pi OS aarch32
- Windows 11 x64 and Windows 10 x64 on PC with [WSL 2](https://docs.microsoft.com/en-us/windows/wsl/install-win10) enabled
  - openSUSE x64
  - Ubuntu x64

## Packages
The library itself doesn't need anything beyond the `std` C++ 11 headers and the POSIX-specific and Linux-specific headers mentioned above.
`abc` has been compiled with Clang 10 through 20, as well as with GCC 7 through 14.

Until version 2.0 of `abc`, the default compiler was GCC although Clang was also supported.
That was due to a bug in Clang 7, which was the default Clang version on Raspberry Pi 4.

> Starting with version 2.0, the default compiler is Clang.

A compiler of choice may be used by passing in a `CPP` variable to `make`.

To build the tests and the samples, some packages are needed, which your system may already have.

### Refresh Distribution Repos
Refresh your distribution repos, before trying to install any packages.

openSUSE
```sh
sudo zypper refresh
```

Ubuntu
```sh
sudo apt update
```

### Install Packages
openSUSE
```sh
sudo zypper install clang gcc-c++ make git zip unzip doxygen graphviz
```

Ubuntu
```sh
sudo apt install clang g++ make git zip unzip doxygen graphviz
```

## GPIO Setup
There should be nothing to do on Raspberry Pi OS.
The packages that enable access to GPIO and SMBus should be preinstalled.
The user that is enabled on the box is the `root` user, and has the necessary access.

On other Linux distributions, a little extra work is needed.

### Install Packages
openSUSE
```sh
sudo zypper install libgpiod2 libgpiod-utils libi2c0 i2c-tools
```

Here are the original pages on openSUSE:
- [GPIO](https://en.opensuse.org/openSUSE:GPIO)
- [I2C](https://en.opensuse.org/openSUSE:I2C)
 
### Grant Access to Non-root Users
The Linux kernel creates device files at every boot with access only to the `root` user.
If other users should be able to use GPIO and/or SMBus, access to those device files should be granted to those users using `chmod`.

A script is provided:
```sh
bin/chmod_dev.sh
```

That could further be automated by executing a script upon boot.
Notice that this step is distribution-specific.

#### openSUSE
Copy the `chmod` commands from `bin/chmod_dev.sh` and paste them in `/etc/init.d/after.local`

Make sure that file is executable:
```sh
sudo chmod 777 /etc/init.d/after.local
```

Reboot, and check that `chmod` has been applied:
```sh
ls -l /dev/gpio*
ls -l /dev/i2c*
```

If it did not work out, check the status of the `after-local` service:
```sh
sudo systemctl status after-local
```
