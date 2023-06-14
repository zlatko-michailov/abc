# endpoint

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [endpoint.h](../../src/endpoint.h)
Interface        | [endpoint.i.h](../../src/i/endpoint.i.h)
Tests / Examples | [samples/basic/*](../../samples/basic/main.cpp)

This class implements a simple web server using [`socket`](socket.md), [`openssl_socket`](openssl_socket.md), or a custom TLS socket implementation, [`http`](http.md), and [`json`](json.md).
It can handle both static file requests as well as REST requests.
This way, every `abc` app can be interacted with using a web browser or curl.
