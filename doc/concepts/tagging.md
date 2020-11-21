# Tagging

Up to [Documentation](../README.md).

Tags are unique 64-bit integers that are used to correlate a log entry with the place in the code where it was created.

During development, pass in the `__TAG__` sentinel wherever an API needs a `tag_t` value.
Commit your changes with that sentinel.

At some later point, a designated contributor runs `bin/tag.sh --conf bin/tag.conf`, which assigns a unique number to each `__TAG__` instance.

While that second step can be done at any time, it should be done in a designated branch by designated contributor.
The reason for that is to avoid collisions in `bin/tag.conf`.
A collision would lead to duplicate tag values.

To run `bin\tag.sh` there must be no uncommitted changes.
