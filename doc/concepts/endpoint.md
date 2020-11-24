# HTTP Endpoint

Up to [Documentation](../README.md).

The HTTP endpoint, or simply [`endpoint`](../ref/endpoint.md), is a very primitive web server that could be embedded in programs.
An endpoint can serve both static file requests and REST requests.

The provided [`endpoint`](../ref/endpoint.md) base class decides whether the request is for a static file or not based on the config settings it was initialized with -
if the request method is `GET`, and the resource path starts with a given prefix, the [`endpoint`](../ref/endpoint.md) class tries to find the file and to send it back to the client.
This part of the endpoint is suitable for enabling GUI by sending down HTML, CSS, JavaScript, images, etc. to the client.
In most cases, programs do not need to override the `process_file_request()` method.

If the resource path does not start with the configured prefix, the request has to be handled by the `process_request_request()` method, i.e. the program has to derive a class from [`endpoint`](../ref/endpoint.md) and to override the `process_request_request()` method.

For a complete end-to-end guide on how to stand up an endpoint to enable GUI and/or REST, visit tutorial [How to Enable GUI and REST Endpoint](../tutorials/endpoint.md).