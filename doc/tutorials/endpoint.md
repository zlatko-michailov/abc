# How to Enable GUI and REST

Up to [Documentation](../README.md).

> A prerequisite to this tutorial is becoming familiar with these concepts:
>- [Endpoint](../concepts/Endpoint.md)
>- [Media and Streams](../concepts/media_and_streams.md)
>- [Diagnostics](../concepts/diagnostics.md)

`abc` programs can show GUI by processing HTTP GET requests and sending down HTML.
That HTML can contain JavaScript that can make HTTP REST requests back to the program, and can update the HTML DOM based on the responses.

Using this mechanism, you can skip the HTML, and implement a CLI.

Since both GUI and REST require processing HTTP requests, they are coupled together into the `endpoint` class.
Thus, enabling GUI and REST boils down to standing up an HTTP endpoint.

## Creating a `log_filter` and a `log_ostream`
It is strongly recommended to pass in a `log_ostream` to the `endpoint`.
Visit the [How to Log Diagnostics](diagnostics.md) tutorial if needed.

## Creating an `endpoint_config`
An `endpoint` takes quite a few parameters and constants, and more may be added in future.
All of those parameters are bundled in a class - `endpoint_config`, and constants are bundled in a template parameter, which the `endpoint` keeps referencing.

> It is important that the `endpoint_config` outlives the `endpoint`. 

The `endpoint_config` constructor may require some rationalization:
``` c++
endpoint_config(
    const char* port,
    std::size_t listen_queue_size,
    const char* root_dir,
    const char* files_prefix);
```
- `port` - The port on which the endpoint will listen.
While the port is a number, it is accepted as a `const char*`.
The reason for that is that the underlying POSIX socket API, specifically the `getaddrinfo()` expects this as a `const char*`.
- `listen_queue_size` - This parameter is also required by the underlying POSIX socket API.
It is the length of the queue where incoming client connection requests are kept.
- `root_dir` - This parameter is endpoint-specific.
It represents the path to the folder that is considered the endpoint's root.
To keep your program resilient to the current working directory, calculate an absolute path eventually based on the folder where your program resides.
- `files_prefix` - This parameter is also endpoint-specific.
It is a relative path of one or more subfolders under `root_dir` where your static files have been deployed.

With that, creating an `endpoint_config` instance may look like this:
``` c++
abc::endpoint_config config(
    "30301",       // port
    5,             // listen_queue_size
    path,          // root_dir (Note: No trailing slash!)
    "/resources/"  // files_prefix
);
```

## Choosing a `Limits` Template Parameter
The `Limits` template parameter contains a set of `static constexpr` definitions.
If the provided `endpoint_limits` doesn't work for you, feel free to define your own struct or namespace with those named constants.

## Deriving Your Own Class from `endpoint`
`endpoint` is the only class from the `abc` library that needs to be overridden.
It is flexible - it allows for quite a few methods to be overridden.
However, you may be able to get away with overriding just `process_rest_request()`.

See [equations.h](../../samples/basic/equations.h) from Basic Sample for how to override `endpoint`.

## Creating an `endpoint`
Now that you have everything, constructing the endpoint is simple:
``` c++
using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;
using limits = abc::endpoint_limits;

equations_endpoint<limits, log_ostream> endpoint(&config, &log);
```
