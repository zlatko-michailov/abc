# Principles

Up to [Documentation](../README.md).

## `std`
`abc` complements `std`.

`abc` only provides features that are missing in `std`.
`abc` doesn't compete with `std`.

While certain `std` concepts may not be perfect, it is better to wait for the C++ community to evolve that concept than to release some competing functionality that is likely to become obsolete.
An example of this is `std::future` - the only async token that is not continuable.

## Exceptions
`abc` is exception-neutral.

In general, an `abc` method that calls into a potentially throwing `std` method doesn't catch exceptions, and is also potentially throwing.
An exception to this principle are the `abc::line_ostream:put_*` methods - they catch any exception that may come from the underlying `std` calls, because a program developer should not worry about exceptions coming from diagnostic logging.

`abc` methods may throw exceptions on their own.
Most of those exception represent program logic errors that the program developer must address.