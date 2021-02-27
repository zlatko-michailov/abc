## Done
- BREAKING CHANGE
  - vmem_list_state
  - vmem_root_page
- vmem_list::erase() bug
- Refactor
  - vmem_pool
- vmem_linked

## To Do
- Refactor
  - vmem_list
    + insert()
    + erase()
    - clear()
- vmem_container
- vmem_list
---
- vmem_map
---
- Roadmap
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
  - compact
---
- GPIO
---
- Samples
  - tictactoe
  - connect four
---
- tls_streambuf
---

## Postponed
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
when to unload pages?
-----------------------------------------

page_file<Record, MaxPages, FreePercent = 25> {
  page<Record> create_page()
  page<Record> get_page(i)
}

page<Record> {
  ~page()
  Record* get()
}

meta | free pages list + index b-tree + heap

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
