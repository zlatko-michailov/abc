# How to Enable GUI and REST

Up to [Documentation](../README.md).

> A prerequisite to this tutorial is becoming familiar with these concepts:
>- [Endpoint](../concepts/Endpoint.md)
>- [Media and Streams](../concepts/media_and_streams.md)
>- [Diagnostics](../concepts/diagnostics.md)

Programs that use `abc` can show GUI by processing HTTP GET requests and sending down HTML.
That HTML can contain JavaScript that can make HTTP REST requests back to the program, and can update the HTML DOM based on the responses.

Using this mechanism, you can skip the HTML, and implement a command line interface (CLI).

Since both GUI and REST require processing of HTTP requests, they are coupled together into the `abc::net::http::endpoint` class.
Thus, enabling GUI and REST boils down to standing up an HTTP endpoint.

## Creating an `abc::diag::log_ostream`
It is strongly recommended to pass in an `abc::diag::log_ostream` to the `abc::net::http::endpoint`.
Visit the [How to Log Diagnostics](diagnostics.md) tutorial if needed.

## Creating an `abc::net::http::endpoint_config`
An `abc::net::http::endpoint` could be configured by quite a few parameters, and more may be added in future.
All of those parameters are bundled in a class - `abc::net::http::endpoint_config`.

The `abc::net::http::endpoint_config` constructor may require some rationalization:
```c++
endpoint_config(
    const char* port,
    std::size_t listen_queue_size,
    const char* root_dir,
    const char* files_prefix,
    const char* cert_file_path = "",
    const char* pkey_file_path = "",
    const char* pkey_file_password = "");
```
- `port` - The port on which the endpoint will listen.
While the port is a number, it is accepted as a `const char*`.
The reason for that is that the underlying POSIX socket API, specifically the `getaddrinfo()` expects this as a `const char*`.
- `listen_queue_size` - This parameter is also required by the underlying POSIX socket API.
It is the length of the queue where incoming client connection requests are kept.
- `root_dir` - This parameter is endpoint-specific.
It represents the __local__ path to the folder that is considered the endpoint's root.
To keep your program resilient to the current working directory, calculate an absolute path eventually based on the folder where your program resides.
- `files_prefix` - This parameter is also endpoint-specific.
It is a __relative__ subpath of one or more subfolders under `root_dir` where your static files have been deployed.
- The rest of the properties are needed if you want to support HTTPS.
The names should be self explanatory.

With that, creating an `abc::net::http::endpoint_config` instance may look like this:
```c++
abc::net::http::endpoint_config config(
    "30301",       // port
    5,             // listen_queue_size
    path,          // root_dir (Note: No trailing slash!)
    "/resources/"  // files_prefix
);
```

## Deriving Your Own Class from `abc::net::http::endpoint`
`abc::net::http::endpoint` is flexible - it allows for quite a few methods to be overridden.
However, you may be able to get away with overriding just `process_rest_request()`.

See [equations.h](../../samples/basic/equations.h) from Basic Sample for how to override `abc::net::http::endpoint`.

## Creating an Endpoint
Now that you have everything, constructing your endpoint is simple:
```c++
// Create a log.
abc::stream::table_ostream table(std::cout.rdbuf());
abc::diag::debug_line_ostream<> line(&table);
abc::diag::str_log_filter<const char *> filter("", abc::diag::severity::important);
abc::diag::log_ostream log(&line, &filter);

// Create an endpoint.
your_endpoint endpoint(std::move(config), &log);
```
