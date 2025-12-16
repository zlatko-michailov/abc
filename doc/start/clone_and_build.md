# Clone and Build the Repo

Up to [Documentation](../README.md).

## Clone the Repo
The repo URL is: `https://github.com/zlatko-michailov/abc.git`.

```sh
mkdir abc
git clone https://github.com/zlatko-michailov/abc.git abc
cd abc
```

## Choose a Branch
### `main`
`main` is the "stable" branch.
This is the branch from where releases are cut.
Documentation reflects this branch.

> This is the only branch that is tagged.

```sh
git checkout main
```

### `dev`
`dev` is the "development" branch.
This is the branch where the next big feature is developed.
When that feature is ready, this branch gets merged into `main`.

```sh
git checkout dev
```

## Build
> Note: The library itself has nothing to build - it is a header-only library.
What is being built is tests and samples.

To build with default options:
```sh
make
```

The first thing the build prints is the values of configurable variables.
You can pass in custom values if needed.
For instance, to build with GCC:
```sh
make CPP=g++
```

That will build the tests, the samples, and the API documentation.
It will also create a release.

Finally, it will run the tests.
Each individual test as well as the `summary` at the end should show `PASS`.

All artifacts are generated in the `out` subfolder.
To see what is there:
```sh
ls -l out
```
