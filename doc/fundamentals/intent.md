# Intent

Up to [Documentation](../README.md).

The primary intent of `abc` is to make programs running on IoT devices, where resources are limited, manageable and diagnosable.
Even if the device is capable of running specialized software, like a web server or a database server, that server may consume all the resources on the device, and may starve the program(s).

That said, `abc` is not limited to IoT devices.
It can be used on any architecture that supports C++ 11, especially where efficiency and diagnostics are required.