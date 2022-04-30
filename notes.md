## Done
- CRC: Moved to boneyard.
- GPIO:
  - Chip - get chip and line info.
  - Line - get and put low/high signal level.
  - PWM Emulator - put variable signal level through a duty cycle.
- Sample picar_4wd:
  - Get chip and line info.
  - Get distance through the ultrasound sensor.
  - Turn the ultrasound sensor through its servo.
  
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

## To Do
Drive:
  - Get power and turn on page reload.
  - Enable negative power for reverse.

GPIO:
  - measure distance


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
