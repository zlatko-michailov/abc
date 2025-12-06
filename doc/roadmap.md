# Roadmap

Up to [Documentation](README.md).

What is currently on the radar is __deep learning__.
More specifically - __inference on nano devices__.

That is likely to require additions to `vmem` and `concurrent`.

## 2.1.0
- `vmem`:
  - `zip_iterator`
  - `array`
  - `matrix`

## 2.2.0
- `concurrent`
  - `async()`
  - `future.then()`

## Pick List
- `concurrent`
  - Async continuations.
- Self-driving
  - Reliable speed and location tracking.
  - Shapes of obstacles.
- `vmem::pool`:
  - `check`
  - `repair`
  - `rebuild`
- `WebSocket` client and server.
  - `base64` encoding and decoding. (Required for WebSocket.)
  - `SHA-1` hashing. (Required for WebSocket.)
- Internationalization, if needed.
