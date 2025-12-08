# Documentation

> If you find a bug on any page, please file an [issue](../../../issues).
Provide the title and the link to the page along with as much information as you consider relevant.

- [MIT License](../LICENSE)
- Fundamentals
  - [Features](fundamentals/features.md)
  - [Intent](fundamentals/intent.md)
  - [Principles](fundamentals/principles.md)
- Concepts
  - [Media and Streams](concepts/media_and_streams.md)
  - [Tagging](concepts/tagging.md)
  - [Diagnostics](concepts/diagnostics.md)
  - [HTTP Endpoint](concepts/endpoint.md)
  - [Virtual Memory](concepts/vmem.md)
  - [GPIO and SMBus (I2C)](concepts/gpio.md)
- Getting Started
  - [Dependencies](start/dependencies.md)
  - [Clone and Build the Repo](start/clone_and_build.md)
  - [Adopt the Library](start/adopt.md)
  - [Use the Headers](start/use.md)
- Tutorials
  - [How to Log Diagnostics](tutorials/diagnostics.md)
  - [How to Enable GUI and REST](tutorials/endpoint.md)
  - [How to Use Virtual Memory](tutorials/vmem.md)
  - [How to Interact with Peripherals over GPIO](tutorials/gpio.md)
- [Contribute](contribute.md)
- [Release Notes](releases.md)
- [Roadmap](roadmap.md)

## Class Reference
### Namespace `abc`
- [size](ref/root/size.md)
- [ascii](ref/root/ascii.md)
- [timestamp](ref/root/timestamp.md)
- [util](ref/root/util.md)

### Namespace `abc::stream`
- [buffer_streambuf](ref/stream/buffer_streambuf.md)
- [vector_streambuf](ref/stream/vector_streambuf.md)
- [multifile_streambuf](ref/stream/multifile_streambuf.md)
  - [duration_multifile_streambuf](ref/stream/multifile_streambuf.md)
  - [size_multifile_streambuf](ref/stream/multifile_streambuf.md)
-
- [stream](ref/stream/stream.md)
  - [istream](ref/stream/stream.md)
  - [ostream](ref/stream/stream.md)
    - [table_ostream](ref/stream/table_stream.md)
    - [line_ostream](ref/stream/table_stream.md)

### Namespace `abc::diag`
- [log_ostream](ref/diag/log.md)
- [log_line_ostream](ref/diag/log.md)
  - [debug_line_ostream](ref/diag/log.md)
  - [diag_line_ostream](ref/diag/log.md)
  - [test_line_ostream](ref/diag/log.md)
- [log_filter](ref/diag/log.md)
-
- [exception](ref/diag/exception.md)
-
- [diag_ready](ref/diag/diag_ready.md)

### Namespace `abc::net`
- [socket_streambuf](ref/net/socket.md)
-
- [udp_socket](ref/net/socket.md)
- [tcp_server_socket](ref/net/socket.md)
- [tcp_client_socket](ref/net/socket.md)
-
- [endpoint](ref/net/endpoint.md)

### Namespace `abc::net::openssl`
- [openssl_tcp_server_socket](ref/net/openssl_socket.md)
- [openssl_tcp_client_socket](ref/net/openssl_socket.md)

### Namespace `abc::net::http`
- [request_reader](ref/net/http.md)
- [request_writer](ref/net/http.md)
- [response_reader](ref/net/http.md)
- [response_writer](ref/net/http.md)
-
- [client](ref/net/http.md)
- [server](ref/net/http.md)
-
- [request_istream](ref/net/http.md)
- [request_ostream](ref/net/http.md)
- [response_istream](ref/net/http.md)
- [response_ostream](ref/net/http.md)

### Namespace `abc::net::json`
- [value](ref/net/json.md)
-
- [reader](ref/net/json.md)
- [writer](ref/net/json.md)
-
- [istream](ref/net/json.md)
- [ostream](ref/net/json.md)

### Namespace `abc::net`
- [socket_streambuf](ref/net/socket.md)
-
- [udp_socket](ref/net/socket.md)
- [tcp_server_socket](ref/net/socket.md)
- [tcp_client_socket](ref/net/socket.md)
- [openssl_tcp_server_socket](ref/net/openssl_socket.md)
- [openssl_tcp_client_socket](ref/net/openssl_socket.md)
-
- [endpoint](ref/net/endpoint.md)

xxx

- [gpio](ref/gpio.md)
  - [chip](ref/gpio.md)
  - [gpio_line](ref/gpio.md)
  - [gpio_pwm_emulator](ref/gpio.md)
  - [gpio_smbus](ref/gpio.md)
  - [gpio_smbus_target](ref/gpio.md)
  - [gpio_smbus_pwm](ref/gpio.md)
  - [gpio_smbus_motor](ref/gpio.md)
  - [gpio_smbus_servo](ref/gpio.md)
  - [gpio_smbus_ultrasonic](ref/gpio.md)
  - [gpio_smbus_grayscale](ref/gpio.md)
  - [gpio_smbus_motion](ref/gpio.md)
  - [gpio_smbus_motion_tracker](ref/gpio.md)
- [test](ref/test.md)
- [vmem](ref/vmem.md)
  - [vmem_pool](ref/vmem.md)
  - [vmem_page](ref/vmem.md)
  - [vmem_ptr](ref/vmem.md)
  - [vmem_list](ref/vmem.md)
