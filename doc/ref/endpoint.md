# `endpoint`

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [endpoint.h](../../src/endpoint.h)
Interface        | [endpoint.i.h](../../src/endpoint.i.h)
Tests / Examples | [samples/basic/*](../../samples/basic/main.cpp)

## `endpoint`
This class implements a simple web server using [`socket`](socket.md), [`http`](http.md), and [`json`](json.md).
It can handle both GET requests for file resources as well as REST requests.
This way, every `abc` app can be interacted with using a web browser or curl.
