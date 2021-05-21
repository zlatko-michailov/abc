/*
MIT License

Copyright (c) 2018-2021 Zlatko Michailov 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "tictactoe.i.h"


namespace abc { namespace samples {


	inline bool move::is_valid() const {
		return (0 <= row && row < size && 0 <= col && col < size);
	}


	// --------------------------------------------------------------


	inline bool board::accept_move(const move& move) {
		if (!move.is_valid()) {
			return false;
		}

		if (is_game_over()) {
			return false;
		}

		if (get_move(move) != player_id::none) {
			return false;
		}

		set_move(move);
		check_winner();

		if (!is_game_over()) {
			switch_current_player_id();
		}

		return true;
	}


	inline bool board::undo_move(const move& move) {
		if (!move.is_valid()) {
			return false;
		}

		if (!is_game_over()) {
			switch_current_player_id();
		}
		clear_move(move);

		_winner == player_id::none;
		_is_game_over = false;

		return true;
	}


	inline bool board::is_game_over() const {
		return _is_game_over;
	}


	inline player_id_t board::winner() const {
		return _winner;
	}


	inline player_id_t board::get_move(const move& move) const {
		return shift_down(move);
	}


	inline void board::set_move(const move& move) {
		_board_state |= shift_up(_current_player_id, move);
		_move_count++;
	}


	inline void board::clear_move(const move& move) {
		_board_state &= ~shift_up(player_id::mask, move);
		_move_count--;
	}


	inline unsigned board::move_count() const {
		return _move_count;
	}


	inline bool board::has_move(player_id_t player_id, const move& move) const {
		board_state bits = shift_up(player_id, move);
		board_state mask = shift_up(player_id::mask, move);
		return (_board_state & mask) == bits;
	}


	inline bool board::check_winner() {
		bool horizontal =
			(has_move(_current_player_id, { 0, 0 }) && has_move(_current_player_id, { 0, 1 }) && has_move(_current_player_id, { 0, 2 })) ||
			(has_move(_current_player_id, { 1, 0 }) && has_move(_current_player_id, { 1, 1 }) && has_move(_current_player_id, { 1, 2 })) ||
			(has_move(_current_player_id, { 2, 0 }) && has_move(_current_player_id, { 2, 1 }) && has_move(_current_player_id, { 2, 2 }));

		bool vertical =
			(has_move(_current_player_id, { 0, 0 }) && has_move(_current_player_id, { 1, 0 }) && has_move(_current_player_id, { 2, 0 })) ||
			(has_move(_current_player_id, { 0, 1 }) && has_move(_current_player_id, { 1, 1 }) && has_move(_current_player_id, { 2, 1 })) ||
			(has_move(_current_player_id, { 0, 2 }) && has_move(_current_player_id, { 1, 2 }) && has_move(_current_player_id, { 2, 2 }));

		bool diagonal =
			(has_move(_current_player_id, { 0, 0 }) && has_move(_current_player_id, { 1, 1 }) && has_move(_current_player_id, { 2, 2 })) ||
			(has_move(_current_player_id, { 0, 2 }) && has_move(_current_player_id, { 1, 1 }) && has_move(_current_player_id, { 2, 0 }));


		bool win = (horizontal || vertical || diagonal);

		bool draw = (_move_count == (size * size));

		if (win) {
			_is_game_over = true;
			_winner = _current_player_id;
		}

		if (draw) {
			_is_game_over = true;
			_winner = player_id::none;
		}

		return _is_game_over;
	}


	inline player_id_t board::current_player_id() const {
		return _current_player_id;
	}


	inline void board::switch_current_player_id() {
		_current_player_id = opponent(_current_player_id);
	}


	inline board_state board::state() const {
		return _board_state;
	}


	inline player_id_t board::opponent(player_id_t player_id) {
		return player_id ^ 0x1;
	}


	inline board_state board::shift_up(player_id_t player_id, const move& move) {
		int cell = move.row * size + move.col;
		return static_cast<board_state>(player_id) << (cell * 2);
	}


	inline player_id_t board::shift_down(const move& move) const {
		int cell = move.row * size + move.col;
		return static_cast<player_id_t>((_board_state >> (cell * 2)) & player_id::mask);
	}


	// --------------------------------------------------------------


	inline player_agent::player_agent(game* game, player_id_t player_id, player_type_t player_type, log_ostream* log)
		: _game(game)
		, _player_id(player_id)
		, _player_type(player_type)
		, _log(log) {
	}


	inline void player_agent::make_move_async() {
		if (_log != nullptr) {
			_log->put_any(category::abc::samples, severity::debug, __TAG__, "player_agent::make_move_async()");
		}

		std::thread(player_agent::make_move_proc, this).detach();
	}


	inline void player_agent::make_move_proc(player_agent* this_ptr) {
		if (this_ptr->_log != nullptr) {
			this_ptr->_log->put_any(category::abc::samples, severity::debug, __TAG__, "player_agent::make_move_proc()");
		}

		this_ptr->make_move();
	}


	inline void player_agent::make_move() {
		if (_log != nullptr) {
			_log->put_any(category::abc::samples, severity::debug, __TAG__, "player_agent::make_move()");
		}

		switch (_player_type) {
			case player_type::slow_engine:
				slow_make_move();
				break;

			case player_type::fast_engine:
				fast_make_move();
				break;
		}
	}


	inline void player_agent::slow_make_move() {
		_temp_board = _game->board();

		if (_log != nullptr) {
			_log->put_any(category::abc::samples, severity::debug, __TAG__, "player_agent::slow_make_move(): player_id=%u, board_state=0x%8.8x, temp_board_state=0x%8.8x",
				_player_id, _game->board().state(), _temp_board.state());
		}

		move best_move;
		slow_find_best_move_for(_player_id, &best_move);

		_game->accept_move(_player_id, best_move);
	}


	inline int player_agent::slow_find_best_move_for(player_id_t player_id, move* best_move) {
		int best_score = -1;

		// For simplicity, try cells in order.
		for (int r = 0; r < size; r++) {
			for (int c = 0; c < size; c++) {
				move mv{ r, c };

				if (best_score < 1 && _temp_board.get_move(mv) == player_id::none) {
					if (_temp_board.accept_move(mv)) {
						int score = -1;
						if (_temp_board.is_game_over()) {
							score = _temp_board.winner() == player_id ? 1 : 0;
						}
						else {
							move dummy_mv;
							score = -slow_find_best_move_for(board::opponent(player_id), &dummy_mv);
						}

						if (score > best_score) {
							*best_move = mv;
							best_score = score;
						}

						_temp_board.undo_move(mv);
					}
					else{
						if (_log != nullptr) {
							_log->put_any(category::abc::samples, severity::important, __TAG__, "player_agent::slow_find_best_move(): IMPOSSIBLE. move_count=%u, current_player_id=%u, best_score=%d, is_game_over=%d, get_move({%d, %d})=%d",
								_temp_board.move_count(), _temp_board.current_player_id(), best_score, _temp_board.is_game_over(), mv.row, mv.col, _temp_board.get_move(mv));
						}
					}
				}
			}
		}

		return best_score;
	}


	inline void player_agent::fast_make_move() {
		//// TODO:
	}


	// --------------------------------------------------------------


	inline game::game()
		: _agent_x(this, player_id::none, player_type::none, nullptr)
		, _agent_o(this, player_id::none, player_type::none, nullptr)
		, _log(nullptr) {
	}


	inline game::game(player_type_t player_x_type, player_type_t player_o_type, log_ostream* log)
		: _agent_x(this, player_id::x, player_x_type, log)
		, _agent_o(this, player_id::o, player_o_type, log)
		, _log(log) {
	}


	inline void game::start() {
		if (_log != nullptr) {
			_log->put_any(category::abc::samples, severity::optional, __TAG__, "game::start(): player_id=%u", _board.current_player_id());
		}

		if (_board.current_player_id() == player_id::x) {
			_agent_x.make_move_async();
		}
		else if (_board.current_player_id() == player_id::o) {
			_agent_o.make_move_async();
		}
	}


	inline bool game::accept_move(player_id_t player_id, const move& move) {
		if (player_id != _board.current_player_id()) {
			return false;
		}

		bool accepted = _board.accept_move(move);

		if (_log != nullptr) {
			_log->put_any(category::abc::samples, severity::optional, __TAG__, "game::accept_move(): accepted=%d, move_count=%u, player_id=%u, best_move={%d, %d}",
				accepted, _board.move_count(), player_id, move.row, move.col);
		}

		if (_board.is_game_over()) {
			if (_log != nullptr) {
				if (_board.winner() != player_id::none) {
					_log->put_any(category::abc::samples, severity::important, __TAG__, "game::accept_move(): GAME OVER - player_id=%u wins", _board.winner());
				}
				else {
					_log->put_any(category::abc::samples, severity::important, __TAG__, "game::accept_move(): GAME OVER - draw");
				}
			}
		}
		else if (accepted) {
			if (_board.current_player_id() == player_id::x) {
				_agent_x.make_move_async();
			}
			else if (_board.current_player_id() == player_id::o) {
				_agent_o.make_move_async();
			}
		}

		return accepted;
	}


	inline const samples::board& game::board() const {
		return _board;
	}


	// --------------------------------------------------------------


	template <typename Limits, typename Log>
	inline tictactoe_endpoint<Limits, Log>::tictactoe_endpoint(endpoint_config* config, Log* log)
		: base(config, log) {
	}


	template <typename Limits, typename Log>
	inline void tictactoe_endpoint<Limits, Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Start REST processing");
		}

		// Support a graceful shutdown.
		if (ascii::are_equal_i(method, method::POST) && ascii::are_equal_i(resource, "/shutdown")) {
			base::set_shutdown_requested();

			base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, "Server is shuting down...", __TAG__);
			return;
		}
	}

}}

