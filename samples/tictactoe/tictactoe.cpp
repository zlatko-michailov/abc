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


	inline bool board::is_game_over() const {
		return _winner != player_id::none;
	}


	inline player_id_t board::winner() const {
		return _winner;
	}


	inline bool board::accept_move(const move& move) {
		if (is_game_over()) {
			return false;
		}

		if (get_move(move) != player_id::none) {
			return false;
		}

		set_move(move);
		if (!check_winner()) {
			switch_current_player_id();
		}

		return true;
	}


	inline player_id_t board::get_move(const move& move) const {
		int cell = move.row * size + move.col;
		return (_board_state >> (cell * 2)) & 0x3;
	}


	inline void board::set_move(const move& move) {
		int cell = move.row * size + move.col;
		_board_state |= (_current_player_id << (cell * 2));
	}


	inline void board::clear_move(const move& move) {
		int cell = move.row * size + move.col;
		_board_state &= ~(0x3 << (cell * 2));
	}


	inline bool board::has_move(player_id_t player_id, const move& move) const {
		int cell = move.row * size + move.col;
		int bits = (player_id << (cell * 2));
		return (_board_state & bits) == bits;
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

		return horizontal || vertical || diagonal;
	}


	inline player_id_t board::current_player_id() const {
		return _current_player_id;
	}


	inline void board::switch_current_player_id() {
		_current_player_id = opponent(_current_player_id);
	}


	inline player_id_t board::opponent(player_id_t player_id) {
		return player_id ^ 0x1;
	}


	// --------------------------------------------------------------


	inline player_agent::player_agent(game* game, player_id_t player_id, player_type_t player_type)
		: _game(game)
		, _player_id(player_id)
		, _player_type(player_type) {
	}


	inline void player_agent::make_move_async() {
		std::thread(player_agent::make_move_proc, this).detach();
	}


	inline void player_agent::make_move_proc(player_agent* this_ptr) {
		this_ptr->make_move();
	}


	inline void player_agent::make_move() {
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
		if (slow_make_necessary_move()) {
			return;
		}

		slow_make_best_move();
	}


	inline bool player_agent::slow_make_necessary_move() {
		if (slow_make_winning_move()) {
			return true;
		}

		if (slow_make_defending_move()) {
			return true;
		}

		return false;
	}


	inline bool player_agent::slow_make_winning_move() {
		return slow_complete(_player_id);
	}


	inline bool player_agent::slow_make_defending_move() {
		return slow_complete(board::opponent(_player_id));
	}


	inline bool player_agent::slow_complete(player_id_t player_id) {
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


	inline bool player_agent::slow_complete_horizontal(player_id_t player_id, int i) {
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


	inline bool player_agent::slow_complete_vertical(player_id_t player_id, int j) {
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


	inline bool player_agent::slow_complete_main_diagonal(player_id_t player_id) {
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


	inline bool player_agent::slow_complete_reverse_diagonal(player_id_t player_id) {
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


	inline bool player_agent::slow_make_best_move() {
		//// TODO:
	}


	//// TODO: Continue

	// --------------------------------------------------------------


	inline game::game()
		: _agent_x(this, player_id::none, player_type::none)
		, _agent_o(this, player_id::none, player_type::none)
		, _log(nullptr) {
	}


	inline game::game(player_type_t player_x_type, player_type_t player_o_type, log_ostream* log)
		: _agent_x(this, player_id::x, player_x_type)
		, _agent_o(this, player_id::o, player_o_type)
		, _log(log) {
	}


	inline void game::start() {
		_agent_x.make_move_async();
	}


	inline bool game::accept_move(player_id_t player_id, const move& move) {
		if (player_id != _board.current_player_id()) {
			return false;
		}

		return _board.accept_move(move);
	}


	inline const samples::board& game::board() const {
		return _board;
	}


	// --------------------------------------------------------------

}}

