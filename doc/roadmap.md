# Roadmap

Up to [Documentation](README.md).

> ATTENTION!  
> The next release will be a complete rewrite.
> Classes will start using `std` facilities as necessary, which may lead to dynamic memory allocation.
> Methods will no longer return status codes.
> Exceptions will be thrown when an operation cannot be performed.

## 2.0.0
- Nested namespaces.
- Throw exceptions. Return results, not status codes.
- Use `std` facilities. Allocate heap memory if necessary.
- Improve logging and filtering.
- Improve usability for `http` and `json`.
- Implement basic SAL - at least `assert()`.

## 2.1.0
- Async continuations.

## 2.2.0
- Self-driving - shapes of obstacles

## Pick List
- Untabify
- `vmem_pool`:
  - `check`
  - `repair`
  - `rebuild`
- `WebSocket` client and server.
  - `base64` encoding and decoding. (Required for WebSocket.)
  - `SHA-1` hashing. (Required for WebSocket.)
- Internationalization, if needed.
