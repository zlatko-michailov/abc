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


	bool move::is_valid() const {
		return (0 <= row && row < size && 0 <= col && col < size);
	}


	// --------------------------------------------------------------


	bool board::accept_move(const move& move) {
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


	bool board::undo_move(const move& move) {
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


	bool board::is_game_over() const {
		return _is_game_over;
	}


	player_id_t board::winner() const {
		return _winner;
	}


	player_id_t board::get_move(const move& move) const {
		int cell = move.row * size + move.col;
		return (_board_state >> (cell * 2)) & 0x3;
	}


	void board::set_move(const move& move) {
		int cell = move.row * size + move.col;
		_board_state |= (_current_player_id << (cell * 2));
		_move_count++;
	}


	void board::clear_move(const move& move) {
		int cell = move.row * size + move.col;
		_board_state &= ~(0x3 << (cell * 2));
		_move_count--;
	}


	unsigned board::move_count() const {
		return _move_count;
	}


	bool board::has_move(player_id_t player_id, const move& move) const {
		int cell = move.row * size + move.col;
		int bits = (player_id << (cell * 2));
		int mask = (0x3 << (cell * 2));
		return (_board_state & mask) == bits;
	}


	bool board::check_winner() {
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


	player_id_t board::current_player_id() const {
		return _current_player_id;
	}


	void board::switch_current_player_id() {
		_current_player_id = opponent(_current_player_id);
	}


	board_state board::state() const {
		return _board_state;
	}


	player_id_t board::opponent(player_id_t player_id) {
		return player_id ^ 0x1;
	}


	// --------------------------------------------------------------


	player_agent::player_agent(game* game, player_id_t player_id, player_type_t player_type, log_ostream* log)
		: _game(game)
		, _player_id(player_id)
		, _player_type(player_type)
		, _log(log) {
	}


	void player_agent::make_move_async() {
		if (_log != nullptr) {
			_log->put_any(category::abc::samples, severity::debug, __TAG__, "player_agent::make_move_async()");
		}

		std::thread(player_agent::make_move_proc, this).detach();
	}


	void player_agent::make_move_proc(player_agent* this_ptr) {
		if (this_ptr->_log != nullptr) {
			this_ptr->_log->put_any(category::abc::samples, severity::debug, __TAG__, "player_agent::make_move_proc()");
		}

		this_ptr->make_move();
	}


	void player_agent::make_move() {
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


	void player_agent::slow_make_move() {
		_temp_board = _game->board();

		if (_log != nullptr) {
			_log->put_any(category::abc::samples, severity::debug, __TAG__, "player_agent::slow_make_move(): player_id=%u, board_state=0x%8.8x, temp_board_state=0x%8.8x",
				_player_id, _game->board().state(), _temp_board.state());
		}

		move best_move;
		slow_find_best_move_for(_player_id, &best_move);

		_game->accept_move(_player_id, best_move);
	}


	int player_agent::slow_find_best_move_for(player_id_t player_id, move* best_move) {
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

#ifdef REMOVE ////
	bool player_agent::slow_make_necessary_move() {
		if (slow_make_winning_move()) {
			return true;
		}

		if (slow_make_defending_move()) {
			return true;
		}

		return false;
	}


	bool player_agent::slow_make_winning_move() {
		return slow_complete(_player_id);
	}


	bool player_agent::slow_make_defending_move() {
		return slow_complete(board::opponent(_player_id));
	}


	bool player_agent::slow_complete(player_id_t player_id) {
		for (int i = 0; i < size; i++) {
			if (slow_complete_horizontal(player_id, i)) {
				return true;
			}

			if (slow_complete_vertical(player_id, i)) {
				return true;
			}
		}

		if (slow_complete_main_diagonal(player_id)) {
			return true;
		}

		if (slow_complete_reverse_diagonal(player_id)) {
			return true;
		}
	}


	bool player_agent::slow_complete_horizontal(player_id_t player_id, int i) {
		int player_count = 0;
		int empty_count = 0;
		int empty_j = -1;

		for (int j = 0; j < size; j++) {
			player_id_t plr = _game->board().get_move({ i, j });

			if (plr == player_id) {
				player_count++;
			}
			else if (plr == player_id::none) {
				empty_count++;
				empty_j = j;
			}
			else {
				return false;
			}
		}

		if (player_count == 2 && empty_count == 1) {
			_game->accept_move(_player_id, { i, empty_j });
			return true;
		}

		return false;
	}


	bool player_agent::slow_complete_vertical(player_id_t player_id, int j) {
		int player_count = 0;
		int empty_count = 0;
		int empty_i = -1;

		for (int i = 0; i < size; i++) {
			player_id_t plr = _game->board().get_move({ i, j });

			if (plr == player_id) {
				player_count++;
			}
			else if (plr == player_id::none) {
				empty_count++;
				empty_i = i;
			}
			else {
				return false;
			}
		}

		if (player_count == 2 && empty_count == 1) {
			_game->accept_move(_player_id, { empty_i, j });
			return true;
		}

		return false;
	}


	bool player_agent::slow_complete_main_diagonal(player_id_t player_id) {
		int player_count = 0;
		int empty_count = 0;
		int empty_i = -1;

		for (int i = 0; i < size; i++) {
			player_id_t plr = _game->board().get_move({ i, i });

			if (plr == player_id) {
				player_count++;
			}
			else if (plr == player_id::none) {
				empty_count++;
				empty_i = i;
			}
			else {
				return false;
			}
		}

		if (player_count == 2 && empty_count == 1) {
			_game->accept_move(_player_id, { empty_i, empty_i });
			return true;
		}

		return false;
	}


	bool player_agent::slow_complete_reverse_diagonal(player_id_t player_id) {
		int player_count = 0;
		int empty_count = 0;
		int empty_i = -1;

		for (int i = 0; i < size; i++) {
			player_id_t plr = _game->board().get_move({ i, 2 - i });

			if (plr == player_id) {
				player_count++;
			}
			else if (plr == player_id::none) {
				empty_count++;
				empty_i = i;
			}
			else {
				return false;
			}
		}

		if (player_count == 2 && empty_count == 1) {
			_game->accept_move(_player_id, { empty_i, 2 - empty_i });
			return true;
		}

		return false;
	}
#endif


	void player_agent::fast_make_move() {
		//// TODO:
	}


	// --------------------------------------------------------------


	game::game()
		: _agent_x(this, player_id::none, player_type::none, nullptr)
		, _agent_o(this, player_id::none, player_type::none, nullptr)
		, _log(nullptr) {
	}


	game::game(player_type_t player_x_type, player_type_t player_o_type, log_ostream* log)
		: _agent_x(this, player_id::x, player_x_type, log)
		, _agent_o(this, player_id::o, player_o_type, log)
		, _log(log) {
	}


	void game::start() {
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


	bool game::accept_move(player_id_t player_id, const move& move) {
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


	const samples::board& game::board() const {
		return _board;
	}


	// --------------------------------------------------------------

}}

