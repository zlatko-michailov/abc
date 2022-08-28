## Done
- Bugfixes:
  - timestamp - time parts
  - buffer_streambuf move constructor
  - stream, istream, ostream move constructor
  - _table_ostream move constructor
  - _line_ostream move constructor
  - log_ostream, debug_line_ostream, diag_line_ostream, test_line_ostream move constructor
  - http_request_istream, http_request_ostream, http_response_istream, http_response_ostream, http_client_stream, http_server_stream move constructor
  - json_istream, json_ostream move constructor
  - vmem_pool move constructor, destructor
- Copy constructor - explicit
Breaking change:
  - `vmem_ptr`:
    - Rename `item_pos` to `byte_pos`.
  - Uninternalize name:
    - _stream
    - _istream
    - _ostream
    - _http_istream

## To Do
- Breaking change:
  - Uninternalize name:
    - _http_ostream
    - _vmem_iterator*
    - _vmem_mapped_page*
    - Re-examine others
  - Untabify
  
## Pick List
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


- conf
- flight (killbit)
- uuid (request/correlation)
- usage
- ring
- then


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
