## Done
----
tictactoe
BUGFIX: Socket bind(REUSEADDR)
BUGFIX: const_iterator to support ++ and --
## To Do
---
---
- Samples
  - connect four
---
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
- GPIO
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
