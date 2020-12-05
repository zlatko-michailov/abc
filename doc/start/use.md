# Use the Headers

Up to [Documentation](../README.md).

## `.h`
These are the "primary" headers.
They contain the actual implementations.
Wherever there is a `.i.h` header,its corresponding `.h` header includes it.

> If you need a particular entity, include its `.h` file.

## `.i.h`
The `.i.h` files' primary purpose is to avoid circular dependencies within `abc`.
They contain class declarations without method definitions, i.e. they are something like "interfaces".
Hence the `.i`.

You should never need to include a `.i.h` header.

> `.i.h` files are very good as API references.

If you want to examine the methods a class exposes in a condensed format, the `.i.h` file is the please to look.
Since class references don't currently include descriptions of individual methods, it is recommended to use the `.i.h` files for that purpose.