## Done
----
## To Do
- vmem_map
  - rename `vmem_container_page_lead_flag` to `vmem_container_page_lead_operation`
  - remove `memmove`
  - clear
  - test clear
  - `vmem_map` docs
  - remove `vmem_map_key_header` and `vmem_map_value_header`

- Prepare release
  - tag
  - version up
---
- Samples
  - tictactoe
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
