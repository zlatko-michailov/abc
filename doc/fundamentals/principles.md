# Principles

Up to [Documentation](../README.md).

## `std`
`abc` complements `std`.

`abc` only provides features that are missing in `std`.
`abc` doesn't compete with `std`.

While certain `std` concepts may not be perfect, it is better to wait for the C++ community to evolve that concept than to release some competing functionality that is likely to become obsolete.
An example of this is `std::future` - the only async token in the industry that is not continuable.

## Exceptions
`abc` methods throw exceptions.

That is because `std` methods in general throw exceptions, and therefore a C++ program should be ready to deal with exceptions.

`abc` uses a small exception class hierarchy, which makes it easy for programs to catch `abc` exceptions separately from other exceptions.