## Done
----
## To Do
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
POST /games/{gameId}/player/{playerId}/move
-----------------------------------------
Request
any

Response
{
  "i": number
}

-----------------------------------------
GET /games/{gameId}/moves?since={i}
-----------------------------------------
Response
{
  "moves": Array<{
    "i": number,
    "move": any,
  }> | undefined,
  "winner": number | undefined
}


game_endpoint<MaxGames, Game, MaxMoves, Move> : endpoint {
  virtual void  process_rest_request()

  void      create_game()
  void      claim_player()
  void      accept_move()
  void      get_moves()
}

game<MaxMoves, Move> {
  game()

  virtual bool      init(const char* players[])
  virtual bool      start()
  virtual bool      accept_move(player_i, const Move& mv)

  uint8_t   status() const
  uint8_t   current_player_i() const
  uint16_t  move_count() const
  const Move* moves(since_i) const
}

game_player<Move> {
  virtual void      init(game* gm, player_i)
  virtual void      make_move()
}

Move {
  Move()

  void      from_json(json_stream)
  void      to_json(json_stream) const
}

