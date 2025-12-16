# Adopt the Library

Up to [Documentation](../README.md).

## Get a Release
> The recommended way to get `abc` is to download the `abc_xx.xx.xx.zip` file from the latest [release](../../../../releases).

Releases are cut from the `main` (stable) branch, and the latest of them should be the most suitable one.

In case you want to adopt features that haven't been merged into `main` yet, e.g. features from the `dev` branch, you will have to build a release yourself, which is straightforward - simply follow the instructions from [Clone and Build the Repo](./clone_and_build.md).
You will find an `abc_xx.xx.xx.zip` file in the `out` folder.

## Integrate with Your Project
Designate a subfolder in your project for dependencies.
For the purpose of this tutorial, we assume that subfolder is `deps`.

Unzip the release under `deps`:
```sh
unzip out/abc_xx.xx.xx.zip -d deps
```

That will create the following folder structure under `deps`:
```
abc
|- include (sym link to xx.xx.xx/include)
|- bin (sym link to xx.xx.xx/bin)
|- xx.xx.xx
   |- include
   |  |- *.h
   |- bin
   |  |- tag.*
   |- samples
```

## Update the Release
When there is a new release available, you may want to evaluate it without removing the old one.
In that case, you can unzip the new release next to the old one. You can point the sym links to the desired version.

## Include the Headers
Use the sym links from the release to avoid disturbance in your program's codebase when you upgrade to a newer `abc` release:
```c++
#include "../deps/abc/include/root/timestamp.h"
```
