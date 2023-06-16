# Roadmap

Up to [Documentation](README.md).

## 1.20.0
- vmem_string.
- vmem_paged_string.

## 2.0.0
- Throw exception, return results.
- Take `vmem_string` or `std::string_view` instead of buffer and size.
- Lazily create a temp `vmem_pool`.
- Override `operator new` to use the temp `vmem_pool`?

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
