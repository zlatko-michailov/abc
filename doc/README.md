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
  - [GPIO](concepts/gpio.md)
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

- [size](ref/size.md)
- [ascii](ref/ascii.md)
- [timestamp](ref/timestamp.md)
- [util](ref/util.md)
-
- [buffer_streambuf](ref/buffer_streambuf.md)
- [multifile_streambuf](ref/multifile_streambuf.md)
- [duration_multifile_streambuf](ref/multifile_streambuf.md)
- [size_multifile_streambuf](ref/multifile_streambuf.md)
-
- [stream](ref/stream.md)
- [istream](ref/stream.md)
- [ostream](ref/stream.md)
- [table_ostream](ref/table_stream.md)
- [line_ostream](ref/table_stream.md)

### Namespace `abc::diag`

- [log_ostream](ref/log.md)
- [debug_line_ostream](ref/log.md)
- [diag_line_ostream](ref/log.md)
- [test_line_ostream](ref/log.md)
-
- [exception](ref/exception.md)
-
- [diag_ready](ref/diag_ready.md) __TODO:__

### Namespace `abc::net`

- [endpoint](ref/endpoint.md)
- [gpio](ref/gpio.md)
  - [gpio_chip](ref/gpio.md)
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
- [http](ref/http.md)
  - [http_request_istream](ref/http.md)
  - [http_request_ostream](ref/http.md)
  - [http_response_istream](ref/http.md)
  - [http_response_ostream](ref/http.md)
  - [http_client_stream](ref/http.md)
  - [http_server_stream](ref/http.md)
- [json](ref/json.md)
  - [json_istream](ref/json.md)
  - [json_ostream](ref/json.md)
- [sockets](ref/socket.md)
  - [socket_streambuf](ref/socket.md)
  - [udp_socket](ref/socket.md)
  - [tcp_server_socket](ref/socket.md)
  - [tcp_client_socket](ref/socket.md)
  - [openssl_tcp_server_socket](ref/openssl_socket.md)
  - [openssl_tcp_client_socket](ref/openssl_socket.md)
- [test](ref/test.md)
- [vmem](ref/vmem.md)
  - [vmem_pool](ref/vmem.md)
  - [vmem_page](ref/vmem.md)
  - [vmem_ptr](ref/vmem.md)
  - [vmem_list](ref/vmem.md)
