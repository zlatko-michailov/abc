# v0.10
## Done
- resource
  - split
  - parameter

## To Do
- Samples
  - tictactoe


## Postponed
- ttl_thread
- Internationalization
- Improve code style
- `socket_streambuf` and `multifile_streambuf` should take a <Size> template parameter to buffer I/O.
- `socket_streambuf` and `multifile_streambuf` should take a <Size> template parameter to buffer I/O.



# v1.12
## Done
## To Do
- WebSocket (SHA-1, base64)



# v1.13
## Done
## To Do
- ring - single writer, multiple readers
- file-backed buffer

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
POST /game
-----------------------------------------
Request

human vs human
{
  "play": false
}

human vs this (computer)
{
  "play": true,
  "start": false
}

remote (computer) vs this (computer)
{
  "play": true,
  "start": false
  "notify": "http://..."
}

-----------------------------------------
POST /move
-----------------------------------------
Response

{
  "delay": 0 | n (seconds)
}

-----------------------------------------
GET /moves?since=stamp
-----------------------------------------
Response

{
  "stamp": "...",
  "moves": [ ... ]
}
