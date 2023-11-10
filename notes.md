## Done
- internal doc pages. UNLINKED!
- `diag_ready`
- `http`
    - `request_reader`
    - `request_writer`
    - `response_reader`
    - `response_writer`

## To Do
- __UNTABIFY each file before making changes!__
- `json`
    `value`
        - Test.
    - Restore `istream` and `ostream`.
    - Create `reader` and `writer`.
    - docs

- kvp.h
    - `key_value_pair<Key, Value>`- derives from `std::pair<Key, Value>`
        - key_type = Key
        - value_type = Value
        - `key()`
        - `value()`
    - `key_value_list<Key, Value>` - derives from `std::deque<key_value_pair<Key, Value>>`
        - key_type = Key
        - value_type = Value
        - `find(key)`
        - `at(key)`
        - `operator [key]`
    - `http::headers` to become a `key_value_list<std::string, std::string>`.
    - `json::literal::object`to become a `key_value_list<std::string, value>`.
    - docs

- Update doc/ref/*.md for all classes that have been done.
- Link the internal docs.
- log:
  - Rename `category` to `origin`.
  - Rename `severity` to `level`.

## To Do Refactoring
- internal docs
  - log
    - exceptions
    - severities
    - filters
- sub-namespaces
- spaces
- doxygen comment alignment
- copy src and samples to boneyard
- log severities
- exceptions!
- assert (sal)
- enum class
- std classes
- LogPtr template parameters
- clang

## Pick List
- drive
- Untabify
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
