# HTTP Endpoint

Up to [Documentation](../README.md).

The HTTP endpoint, or simply [`abc::net::http::endpoint`](../ref/net/endpoint.md), is a very primitive web server that could be embedded in programs.
An endpoint can serve both static file requests and REST requests.

The provided [`abc::net::http::endpoint`](../ref/net/endpoint.md) base class decides whether the request is for a static file or not, based on the config settings it was initialized with -
if the request method is `GET`, and the resource path starts with a given prefix, the [`abc::net::http::endpoint`](../ref/net/endpoint.md) class tries to find the file and to send it back to the client.
This part of the endpoint is suitable for enabling GUI by sending down HTML, CSS, JavaScript, images, etc. to the client.
In most cases, programs do not need to override the `process_file_request()` method.

If the resource path does not start with the configured prefix, the request has to be handled by the `process_rest_request()` method.
For that purpose, the program has to derive a class from [`abc::net::http::endpoint`](../ref/net/endpoint.md) and to override the `process_rest_request()` method.

For a complete end-to-end guide on how to stand up an endpoint to enable GUI and/or REST, visit tutorial [How to Enable GUI and REST](../tutorials/endpoint.md).
