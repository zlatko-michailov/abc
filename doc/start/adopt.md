# Adopt the Library

Up to [Documentation](../README.md).

## Get a Release
> The recommended way to get `abc` is to download the `abc_xx.xx.xx.zip` file from the latest [release](../../../../releases).

Releases are cut from the `master` (stable) branch, and the latest of them should be the most suitable one.

In case you want to adopt features that haven't been merged into `master` yet, e.g. features from the `dev` (beta) branch, you'll have to build a release yourself, which is straightforward - simply follow the instructions from [Clone and Build the Repo](./clone_and_build.md).
You will find an `abc_xx.xx.xx.zip` file in the `out` folder.

## Integrate with Your Project
Designate a subfolder in your project for dependencies.
For the purpose of this tutorial, let's assume that subfolder is `deps`.

Unzip the release under `deps`:
```
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
In that case, you can unzip the new release next to the old one. You can point the sym links to the desired version, if you use them.

## Include the Headers
There are multiple ways to include the `abc` headers.
Following are some ideas:

Leverage the sym links from the release:
``` c++
#include "../deps/abc/include/log.h"
```

If you create your own sym link to `abc/xx.xx.xx/include` directly under `deps`, you'll be able to simplify the statement to:
``` c++
#include "../deps/abc/log.h"
```

Then, if you leverage the `-I../deps` option of the `g++` compiler, you should be able to simplify the statement even further to:
``` c++
#include <abc/log.h>
```
