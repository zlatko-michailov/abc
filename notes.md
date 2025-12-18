## To Do
- Namespace `net::http`:
  - Server-Sent Events (SSE)
    - `endpoint`
      - method - `PATCH`
      - content_type - `event-stream`
      - ---
      - `send_event_part()`
      - `send_event()`
    - `event_part`
      - `event_comment(const char*) : event_part`
        - `event_field(const char*, const char* = nullptr) : event_part`
          - `event_type(const char*) : event_field`
          - `event_data(const char*) : event_field`
          - `event_id(const char*) : event_field`
          - `event_retry(const char*) : event_field`
    - `event` = `std::vector<event_part>`
  - ---
  - `json_rpc`

- Namespace `ai`:
  - `mcp`
    - `server`
    - `endpoint`
    - `client`

- Namespace `vmem`:
  - `zip_iterator`
  - `array`
  - `matrix` (or `mutidim_array`)

- Namespace `vmem`:
  - `blob`

- Namespace `math` (or `algorithm`):
  - `mmul(matrix, vector)`

- Namespace `ai`:
  - `deep_learning` (or `neural_net`)

## Pick List
- drive
- vmem:
  - check
    - No data loss
      - State total count mismatch
      - State front page mismatch
      - State back page mismatch
      - Forward iteration broken
      - Reverse iteration broken
    - Possible data loss
      - Chain broken
  - repair
---
- tls_streambuf
---

## Postponed
- Internationalization
- Improve code style
- `socket_streambuf` and `multifile_streambuf` should take a <Size> template parameter to buffer I/O.
- `socket_streambuf` and `multifile_streambuf` should take a <Size> template parameter to buffer I/O.

---
        context.log->filter()->min_severity(abc::severity::critical); ////
        context.log()->put_any(origin, "TEMP", abc::diag::severity::important, __TAG__, "Backward");
---

  export LDFLAGS="-L/usr/local/opt/openssl/lib"
  export CPPFLAGS="-I/usr/local/opt/openssl/include"


- conf
- flight (killbit)
- uuid (request/correlation)
- usage
- ring
- then


## TLS
1. Create a private key:
`openssl genpkey -out samples/tls/pkey.pem -algorithm RSA -pkeyopt rsa_keygen_bits:2048 -aes-256-cbc`
2. Create a self-signed certificate:
`openssl req -new -x509 -days 365 -key samples/tls/pkey.pem -out samples/tls/cert.pem -subj "/C=US/ST=Washington/L=Seattle/O=abc samples/CN=localhost"`
3. Client connect:
`openssl s_client --connect :31241`


PID PPID PS
ps -el | sed -E -e 's/^. +. +([[:digit:]]+) +([[:digit:]]+) +.+ +([[:alpha:]]+)$/\1 \2 \3/'


Speed (all wheels):
  
duty cycle |   25 |   50 |   75 |  100
---------- |   -- |   -- |   -- |  ---
 c / 200ms |    3 |    7 |   11 |   14
 c / sec   |   15 |   35 |   55 |   70
 r / sec   | 0.75 | 1.75 | 2.75 | 3.50
 cm / sec  | 15.6 | 36.3 | 57.0 | 72.6

Turn:

deg        |   30 |   45 |   60
---        |   -- |   -- |   --
rad        | 0.52 | 0.79 | 1.05
delta / 2  |  7.4 | 11.2 | 14.9
duty cycle |   12 |   16 |   25
experi. dc |   11 |   18 |   25


-----------------------------------------
POST /games
-----------------------------------------
Request
{
  "players": string[]
}

Response
{
  "gameId": number
}

-----------------------------------------
POST /games/{gameId}/players/{i}
-----------------------------------------
Response
{
  "playerId": number
}

-----------------------------------------
POST /games/{gameId}/players/{playerId}/moves
-----------------------------------------
Request
any (move)

Response
{
  "i": number,
  "winner": number | undefined
}

-----------------------------------------
GET /games/{gameId}/moves?since={i}
-----------------------------------------
Response
{
  "moves": Array<{
    "i": number,
    "move": any (move),
  }> | undefined,
  "winner": number | undefined
}
