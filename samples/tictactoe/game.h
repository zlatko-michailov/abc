/*
MIT License

Copyright (c) 2018-2023 Zlatko Michailov 

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


#include <cstdlib>
#include <cstdio>
#include <algorithm>

#include "../../src/ascii.h"
#include "../../src/endpoint.h"
#include "../../src/http.h"
#include "../../src/json.h"

#include "game.i.h"


namespace abc { namespace samples {


	// --------------------------------------------------------------


	inline vmem_bundle::vmem_bundle(const char* path, log_ostream* log)
		: pool(path, log)
		, start_page(&pool, vmem_page_pos_start, log)
		, state_scores_map(&static_cast<start_page_layout*>(start_page.ptr())->map_state, &pool, log)
		, log(log) {
	}


	// --------------------------------------------------------------


	inline player_type_t player_type::from_text(const char* text) {
		if (ascii::are_equal(text, "external")) {
			return player_type::external;
		}
		else if (ascii::are_equal(text, "slow_engine")) {
			return player_type::slow_engine;
		}
		else if (ascii::are_equal(text, "fast_engine")) {
			return player_type::fast_engine;
		}
		else {
			return player_type::none;
		}
	}


	// --------------------------------------------------------------


	inline bool move::is_valid() const {
		return (0 <= row && row < row_count && 0 <= col && col < col_count);
	}


	// --------------------------------------------------------------


	inline void board::reset() {
		_is_game_over		= false;
		_winner				= player_id::none;
		_current_player_id	= player_id::x;
		_board_state		= { };
		_move_count			= 0;
	}


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

		_winner = player_id::none;
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
		board_state_t bits = shift_up(player_id, move);
		board_state_t mask = shift_up(player_id::mask, move);
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

		bool draw = (_move_count == (row_count * col_count));

		if (win) {
			_is_game_over = true;
			_winner = _current_player_id;
		}
		else if (draw) {
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


	inline board_state_t board::state() const {
		return _board_state;
	}


	inline player_id_t board::opponent(player_id_t player_id) {
		return player_id ^ 0x1;
	}


	inline board_state_t board::shift_up(player_id_t player_id, const move& move) {
		int cell = move.row * col_count + move.col;
		return static_cast<board_state_t>(player_id) << (cell * 2);
	}


	inline player_id_t board::shift_down(const move& move) const {
		int cell = move.row * col_count + move.col;
		return static_cast<player_id_t>((_board_state >> (cell * 2)) & player_id::mask);
	}


	// --------------------------------------------------------------


	inline void player_agent::reset(game* game, player_id_t player_id, player_type_t player_type, log_ostream* log) {
		_game = game;
		_player_id = player_id;
		_player_type = player_type;
		_log = log;
	}


	inline void player_agent::make_move_async() {
		if (_log != nullptr) {
			_log->put_any(category::abc::samples, severity::debug, 0x105aa, "player_agent::make_move_async()");
		}

		std::thread(player_agent::make_move_proc, this).detach();
	}


	inline void player_agent::make_move_proc(player_agent* this_ptr) {
		if (this_ptr->_log != nullptr) {
			this_ptr->_log->put_any(category::abc::samples, severity::debug, 0x105ab, "player_agent::make_move_proc()");
		}

		this_ptr->make_move();
	}


	inline void player_agent::make_move() {
		if (_log != nullptr) {
			_log->put_any(category::abc::samples, severity::debug, 0x105ac, "player_agent::make_move()");
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
			_log->put_any(category::abc::samples, severity::debug, 0x105ad, "player_agent::slow_make_move(): player_id=%u, board_state=0x%8.8x, temp_board_state=0x%8.8x",
				_player_id, _game->board().state(), _temp_board.state());
		}

		move best_move;
		slow_find_best_move_for(_player_id, best_move);

		_game->accept_move(_player_id, best_move);
	}


	inline int player_agent::slow_find_best_move_for(player_id_t player_id, move& best_move) {
		int best_score = -1;

		// For simplicity, try cells in order.
		for (move_t r = 0; r < row_count; r++) {
			for (move_t c = 0; c < col_count; c++) {
				move mv{ r, c };

				if (best_score < 1 && _temp_board.get_move(mv) == player_id::none) {
					if (_temp_board.accept_move(mv)) {
						int score = -1;
						if (_temp_board.is_game_over()) {
							score = _temp_board.winner() == player_id ? 1 : 0;
						}
						else {
							move dummy_mv;
							score = -slow_find_best_move_for(board::opponent(player_id), dummy_mv);
						}

						if (score > best_score) {
							best_move = mv;
							best_score = score;
						}

						_temp_board.undo_move(mv);
					}
					else{
						if (_log != nullptr) {
							_log->put_any(category::abc::samples, severity::important, 0x105ae, "player_agent::slow_find_best_move(): IMPOSSIBLE. move_count=%u, current_player_id=%u, best_score=%d, is_game_over=%d, get_move({%d, %d})=%d",
								_temp_board.move_count(), _temp_board.current_player_id(), best_score, _temp_board.is_game_over(), mv.row, mv.col, _temp_board.get_move(mv));
						}
					}
				}
			}
		}

		return best_score;
	}


	inline void player_agent::fast_make_move() {
		move best_move = fast_find_best_move();
		_game->accept_move(_player_id, best_move);
	}


	inline move player_agent::fast_find_best_move() {
		std::lock_guard<std::mutex> lock(_vmem->mutex);

		vmem_map::iterator itr = ensure_board_state_in_map(_game->board().state());

		move some_move;
		if (itr.can_deref()) {
			bool should_explore = true; // TODO: Calculate exploration

			// Analyze what we have.
			score_calc_t max_count = 0;
			score_calc_t min_count = 0;
			score_calc_t none_count = 0;
			score_calc_t score_sum = 0;

			for (move_t r = 0; r < row_count; r++) {
				for (move_t c = 0; c < col_count; c++) {
					if (_game->board().get_move(move{ r, c }) == abc::samples::player_id::none) {
						score_calc_t curr_score = itr->value[r][c];

						if (curr_score == score::max) {
							max_count++;
						}
						else if (curr_score == score::min) {
							min_count++;
						}
						else if (curr_score == score::none) {
							none_count++;
						}
						else {
							score_sum += curr_score;
						}
					}
				}
			}

			// If there is one or more max scores, pick one of them.
			if (max_count > 0) {
				score_calc_t rand_i = static_cast<score_calc_t>(1 + std::rand() % max_count);

				for (move_t r = 0; r < row_count; r++) {
					for (move_t c = 0; c < col_count; c++) {
						if (_game->board().get_move(move{ r, c }) == abc::samples::player_id::none && itr->value[r][c] == score::max) {
							if (--rand_i == 0) {
								return move{ r, c };
							}
						}
					}
				}
			}

			// If all the scores are min, pick one of them.
			else if (min_count == row_count * col_count) {
				score_calc_t rand_i = static_cast<score_calc_t>(1 + std::rand() % min_count);

				return move{ rand_i / (score_calc_t)col_count, rand_i % (score_calc_t)col_count };
			}

			// Make a weighted pick.
			else {
				if (should_explore) {
					score_sum += none_count * score::mid;
				}

				score_calc_t rand_sum = static_cast<score_calc_t>(1 + std::rand() % score_sum);

				for (move_t r = 0; r < row_count; r++) {
					for (move_t c = 0; c < col_count; c++) {
						score_calc_t curr_score = itr->value[r][c];

						if (_game->board().get_move(move{ r, c }) == abc::samples::player_id::none) {
							if (score::min <= curr_score && curr_score <= score::max) {
								some_move = move{ r, c };
								rand_sum -= curr_score;
							}
							else if (should_explore && curr_score == score::none) {
								some_move = move{ r, c };
								rand_sum -= score::mid;
							}

							if (rand_sum <= 0) {
								if (_log != nullptr) {
									_log->put_any(category::abc::samples, severity::debug, 0x105af, "player_agent::fast_find_best_move(): row=%d, col=%d, score=%d", r, c, curr_score);
								}

								return move{ r, c };
							}
						}
					}
				}
			}
		}

		// We should never end up here.
		if (_log != nullptr) {
			_log->put_any(category::abc::samples, severity::important, 0x105b0, "player_agent::fast_find_best_move(): Impossible!");
		}

		return some_move;
	}


	inline void player_agent::learn() {
		std::lock_guard<std::mutex> lock(_vmem->mutex);

		board temp_board;
		for (unsigned i = 0; i < _game->board().move_count(); i++) {
			move mv(_game->moves()[i]);

			if (temp_board.current_player_id() == _player_id) {
				vmem_map::iterator itr = ensure_board_state_in_map(temp_board.state());

				score_t old_score = itr->value[mv.row][mv.col] == score::none ? score::mid : itr->value[mv.row][mv.col];

				if (_game->board().winner() == _player_id) {
					// Win
					score_t new_score = old_score + score::win;
					itr->value[mv.row][mv.col] = std::min(score::max, new_score);

					if (_log != nullptr) {
						_log->put_any(category::abc::samples, severity::debug, 0x105b1, "player_agent::learn: (win) move:%u, state=%8.8x, row=%d, col=%d, old_score=%d, new_score=%d",
							i, temp_board.state(), mv.row, mv.col, old_score, new_score);
					}
				}
				else if (_game->board().winner() == player_id::none) {
					// Draw
					score_t new_score = old_score + score::draw;
					itr->value[mv.row][mv.col] = std::min(score::max, new_score);

					if (_log != nullptr) {
						_log->put_any(category::abc::samples, severity::debug, 0x105b2, "player_agent::learn: (draw) move:%u, state=%8.8x, row=%d, col=%d, old_score=%d, new_score=%d",
							i, temp_board.state(), mv.row, mv.col, old_score, new_score);
					}
				}
				else {
					// Loss
					score_t new_score = old_score + score::loss;
					itr->value[mv.row][mv.col] = std::max(score::min, new_score);

					if (_log != nullptr) {
						_log->put_any(category::abc::samples, severity::debug, 0x105b3, "player_agent::learn: (loss) move:%u, state=%8.8x, row=%d, col=%d, old_score=%d, new_score=%d",
							i, temp_board.state(), mv.row, mv.col, old_score, new_score);
					}
				}
			}

			temp_board.accept_move(mv);
		}
	}


	inline player_type_t player_agent::player_type() const {
		return _player_type;
	}


	inline vmem_map::iterator player_agent::ensure_board_state_in_map(board_state_t board_state) {
		vmem_map::iterator itr = _vmem->state_scores_map.find(board_state);

		if (itr.can_deref()) {
			return itr;
		}

		// An item with this key was not found. We'll insert it.

		// Init the item before inserting it.
		vmem_map::value_type item;
		item.key = board_state;
		for (int r = 0; r < row_count; r++) {
			for (int c = 0; c < col_count; c++) {
				item.value[r][c] = score::none;
			}
		}

		// Insert the item.
		vmem_map::iterator_bool itr_b = _vmem->state_scores_map.insert(item);
		return itr_b.first;
	}


	// --------------------------------------------------------------


	inline void game::reset(player_type_t player_x_type, player_type_t player_o_type, log_ostream* log) {
		_agent_x.reset(this, player_id::x, player_x_type, log);
		_agent_o.reset(this, player_id::o, player_o_type, log);
		_log = log;
		_board.reset();
	}


	inline void game::start() {
		if (_log != nullptr) {
			_log->put_any(category::abc::samples, severity::optional, 0x105b4, "game::start(): player_id=%u", _board.current_player_id());
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
			_log->put_any(category::abc::samples, severity::optional, 0x105b5, "game::accept_move(): accepted=%d, move_count=%u, player_id=%u, best_move={%d, %d}",
				accepted, _board.move_count(), player_id, move.row, move.col);
		}

		if (accepted) {
			_moves[_board.move_count() - 1] = move;
		}

		if (_board.is_game_over()) {
			if (_log != nullptr) {
				if (_board.winner() != player_id::none) {
					_log->put_any(category::abc::samples, severity::important, 0x105b6, "game::accept_move(): GAME OVER - player_id=%u wins", _board.winner());
				}
				else {
					_log->put_any(category::abc::samples, severity::important, 0x105b7, "game::accept_move(): GAME OVER - draw");
				}

				for (std::size_t i = 0; i < _board.move_count(); i++) {
					_log->put_any(category::abc::samples, severity::optional, 0x105b8, "game::accept_move(): %zu (%c) - { %d, %d }", i, (i & 1) == 0 ? 'X' : 'O', _moves[i].row, _moves[i].col);
				}
			}

			if (_agent_x.player_type() == player_type::fast_engine && _agent_o.player_type() == player_type::slow_engine) {
				_agent_x.learn();
			}
			else if (_agent_o.player_type() == player_type::fast_engine && _agent_x.player_type() == player_type::slow_engine) {
				_agent_o.learn();
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


	inline const move* game::moves() const {
		return _moves;
	}


	// --------------------------------------------------------------


	inline void endpoint_game::reset(endpoint_game_id_t endpoint_game_id,
									player_type_t player_x_type, endpoint_player_id_t endpoint_player_x_id,
									player_type_t player_o_type, endpoint_player_id_t endpoint_player_o_id,
									log_ostream* log) {
		base::reset(player_x_type, player_o_type, log);

		_endpoint_game_id						= endpoint_game_id;
		_endpoint_player_x.endpoint_player_id	= endpoint_player_x_id;
		_endpoint_player_x.is_claimed			= endpoint_player_x_id == 0;
		_endpoint_player_o.endpoint_player_id	= endpoint_player_o_id;
		_endpoint_player_o.is_claimed			= endpoint_player_o_id == 0;

		if (_endpoint_player_x.is_claimed && _endpoint_player_o.is_claimed) {
			start();
		}
	}


	inline bool endpoint_game::claim_player(unsigned player_i, endpoint_player_id_t& endpoint_player_id) {
		if (player_i == 0) {
			if (_endpoint_player_x.is_claimed) {
				return false;
			}

			endpoint_player_id = _endpoint_player_x.endpoint_player_id;
			_endpoint_player_x.is_claimed = true;
		}
		else {
			if (_endpoint_player_o.is_claimed) {
				return false;
			}

			endpoint_player_id = _endpoint_player_o.endpoint_player_id;
			_endpoint_player_o.is_claimed = true;
		}

		if (_endpoint_player_x.is_claimed && _endpoint_player_o.is_claimed) {
			start();
		}

		return true;
	}


	inline endpoint_game_id_t endpoint_game::id() const {
		return _endpoint_game_id;
	}


	inline player_id_t endpoint_game::player_id(endpoint_player_id_t endpoint_player_id) const {
		if (endpoint_player_id == _endpoint_player_x.endpoint_player_id) {
			return player_id::x;
		}
		else if (endpoint_player_id == _endpoint_player_o.endpoint_player_id) {
			return player_id::o;
		}

		return player_id::none;
	}


	// --------------------------------------------------------------


	template <typename Limits, typename Log>
	inline game_endpoint<Limits, Log>::game_endpoint(endpoint_config* config, Log* log)
		: base(config, log) {
	}


	template <typename Limits, typename Log>
	inline abc::tcp_server_socket<Log> game_endpoint<Limits, Log>::create_server_socket() {
		return abc::tcp_server_socket<Log>(socket::family::ipv4, base::_log);
	}


	template <typename Limits, typename Log>
	inline void game_endpoint<Limits, Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x105b9, "game_endpoint::process_rest_request: Start.");
		}

		if (ascii::are_equal_i_n(resource, "/games", 6)) {
			process_games(http, method, resource);
		}
		else if (ascii::are_equal_i(resource, "/shutdown")) {
			process_shutdown(http, method);
		}
		else {
			// 404
			base::send_simple_response(http, status_code::Not_Found, reason_phrase::Not_Found, content_type::text, "The requested resource was not found.", 0x105ba);
		}

		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x105bb, "game_endpoint::process_rest_request: Done.");
		}
	}


	template <typename Limits, typename Log>
	inline void game_endpoint<Limits, Log>::process_games(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		const char* resource_games = resource + 6;

		if (ascii::are_equal_i(resource_games, "")) {
			create_game(http, method);
		}
		else {
			unsigned game_id = 0;
			unsigned player_id = 0;
			unsigned player_i = 0;
			unsigned since_move_i = 0;
			char moves[6 + 1 + 1];
			moves[0] = '\0';

			if (std::sscanf(resource_games, "/%u/players/%u/%6s", &game_id, &player_id, moves) == 3) {
				accept_move(http, method, static_cast<endpoint_game_id_t>(game_id), static_cast<endpoint_player_id_t>(player_id), moves);
			}
			else if (std::sscanf(resource_games, "/%u/players/%u", &game_id, &player_i) == 2) {
				claim_player(http, method, static_cast<endpoint_game_id_t>(game_id), player_i);
			}
			else if (std::sscanf(resource_games, "/%u/moves?since=%u", &game_id, &since_move_i) == 2) {
				get_moves(http, method, static_cast<endpoint_game_id_t>(game_id), since_move_i);
			}
		}
	}


	template <typename Limits, typename Log>
	inline bool game_endpoint<Limits, Log>::create_game(abc::http_server_stream<Log>& http, const char* method) {
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x105bc, "game_endpoint::create_game: Start.");
		}

		if (!verify_method_post(http, method)) {
			return false;
		}

		if (!verify_header_json(http)) {
			return false;
		}

		player_type_t player_x_type;
		player_type_t player_o_type;

		if (!get_player_types(http, method, player_x_type, player_o_type)) {
			return false;
		}

		// Find a slot
		std::size_t game_i = max_game_count;
		if (_game_count < max_game_count) {
			game_i = _game_count++;
		}
		else {
			for (std::size_t i = 0; i < max_game_count; i++) {
				if (_games[i].board().is_game_over()) {
					game_i = i;

					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x105bd, "game_endpoint::create_game: game_i=%zu", game_i);
					}
					break;
				}
			}
		}

		if (game_i >= max_game_count) {
			constexpr const char* no_game_capacity = "The service has a temporary game capacity shortage.";

			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105be, "Service error: Out of game capacity.");
			}

			// 503
			base::send_simple_response(http, status_code::Service_Unavailable, reason_phrase::Service_Unavailable, content_type::text, no_game_capacity, 0x105bf);
			return false;
		}

		endpoint_game_id_t endpoint_game_id = ((std::rand() & 0xffff) << 16) | ((std::rand() & 0xffff));

		endpoint_player_id_t endpoint_player_x_id = 0;
		endpoint_player_id_t endpoint_player_o_id = 0;

		if (player_x_type == player_type::external) {
			endpoint_player_x_id = ((std::rand() & 0xffff) << 16) | ((std::rand() & 0xffff));
		}

		if (player_o_type == player_type::external) {
			endpoint_player_o_id = ((std::rand() & 0xffff) << 16) | ((std::rand() & 0xffff));
		}

		_games[game_i].reset(endpoint_game_id, player_x_type, endpoint_player_x_id, player_o_type, endpoint_player_o_id, base::_log);

		// Write the JSON to a char buffer, so we can calculate the Content-Length before we start sending the body.
		char body[abc::size::k1 + 1];
		abc::buffer_streambuf sb(nullptr, 0, 0, body, 0, sizeof(body));
		abc::json_ostream<abc::size::_16, Log> json(&sb, base::_log);
		json.put_begin_object();
			json.put_property("gameId");
			json.put_number(endpoint_game_id);
		json.put_end_object();
		json.put_char('\0');
		json.flush();

		char content_length[abc::size::_32 + 1];
		std::snprintf(content_length, sizeof(content_length), "%zu", std::strlen(body));

		// Send the http response
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x105c0, "Sending response 200");
		}

		http.put_protocol(protocol::HTTP_11);
		http.put_status_code(status_code::OK);
		http.put_reason_phrase(reason_phrase::OK);

		http.put_header_name(header::Connection);
		http.put_header_value(connection::close);
		http.put_header_name(header::Content_Type);
		http.put_header_value(content_type::json);
		http.put_header_name(header::Content_Length);
		http.put_header_value(content_length);
		http.end_headers();

		http.put_body(body);

		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x105c1, "game_endpoint::create_game: Done.");
		}

		return true;
	}


	template <typename Limits, typename Log>
	inline bool game_endpoint<Limits, Log>::get_player_types(abc::http_server_stream<Log>& http, const char* /*method*/, player_type_t& player_x_type, player_type_t& player_o_type) {
		char players[2][abc::size::_64 + 1];
		bool has_players = false;

		std::streambuf* sb = static_cast<abc::http_request_istream<Log>&>(http).rdbuf();
		abc::json_istream<abc::size::_64, Log> json(sb, base::_log);
		char buffer[sizeof(abc::json::token_t) + abc::size::k1 + 1];
		abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(buffer);
		constexpr const char* invalid_json = "An invalid JSON payload was supplied. Must be: {\"players\": [ \"external\", \"slow_engine\" ]}.";

		json.get_token(token, sizeof(buffer));
		if (token->item != abc::json::item::begin_object) {
			// The body is not a JSON object.
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105c2, "Content error: Expected '{'.");
			}

			// 400
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x105c3);
			return false;
		}

		// Read all properties.
		while (true) {
			// The tokens at this level must be properties or }.
			json.get_token(token, sizeof(buffer));

			// If we reached }, then we are done parsing.
			if (token->item == abc::json::item::end_object) {
				break;
			}

			if (token->item != abc::json::item::property) {
				// Not a property.
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105c4, "Content error: Expected a property.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x105c5);
				return false;
			}

			// We expect 1 property - "players".
			if (ascii::are_equal(token->value.property, "players")) {
				// Parse array [2].
				json.get_token(token, sizeof(buffer));

				if (token->item != abc::json::item::begin_array) {
					// Not [.
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105c6, "Content error: Expected '['.");
					}

					// 400
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x105c7);
					return false;
				}

				for (std::size_t i = 0; i < 2; i++) {
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x105c8, "Parsing players[%zu]", i);
					}

					json.get_token(token, sizeof(buffer));
					if (token->item != abc::json::item::string) {
						// Not a string.
						if (base::_log != nullptr) {
							base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105c9, "Content error: Expected a string.");
						}

						// 400
						base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x105ca);
						return false;
					}

					std::strcpy(players[i], token->value.string);
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x105cb, "players[%zu]='%s'", i, players[i]);
					}
				}

				json.get_token(token, sizeof(buffer));
				if (token->item != abc::json::item::end_array) {
					// Not ].
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105cc, "Content error: Expected ']'.");
					}

					// 400
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x105cd);
					return false;
				}

				has_players = true;
			}
			else {
				// Future-proof: Ignore unknown properties.
				json.skip_value();
			}
		}

		if (!has_players) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105ce, "Content error: Players not received.");
			}

			// 400
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x105cf);
			return false;
		}

		// Player types
		constexpr const char* invalid_player_type = "An invalid player type was received.";

		player_x_type = player_type::from_text(players[0]);
		if (player_x_type == player_type::none) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105d0, "Content error: Invalid value of players[0]='%s'.", players[0]);
			}

			// 400
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_player_type, 0x105d1);
			return false;
		}

		player_o_type = player_type::from_text(players[1]);
		if (player_o_type == player_type::none) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105d2, "Content error: Invalid value of players[1]='%s'.", players[1]);
			}

			// 400
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_player_type, 0x105d3);
			return false;
		}

		return true;
	}


	template <typename Limits, typename Log>
	inline bool game_endpoint<Limits, Log>::claim_player(abc::http_server_stream<Log>& http, const char* method, endpoint_game_id_t endpoint_game_id, unsigned player_i) {
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x105d4, "game_endpoint::claim_player: Start.");
		}

		if (!verify_method_post(http, method)) {
			return false;
		}

		if (endpoint_game_id == 0 || player_i > 1) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105d5, "Resource error: game_id=%u, player_i=%u", (unsigned)endpoint_game_id, player_i);
			}

			// 400
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "An invalid resource was supplied.", 0x105d6);
			return false;
		}

		for (std::size_t game_i = 0; game_i < _game_count; game_i++) {
			if (_games[game_i].id() == endpoint_game_id) {
				endpoint_player_id_t endpoint_player_id;
				if (!_games[game_i].claim_player(player_i, endpoint_player_id)) {
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105d7, "Security error: Player already claimed. game_id=%u, player_i=%u", (unsigned)endpoint_game_id, player_i);
					}

					// 403
					base::send_simple_response(http, status_code::Forbidden, reason_phrase::Forbidden, content_type::text, "This play has already been claimed.", 0x105d8);
					return false;
				}

				// Write the JSON to a char buffer, so we can calculate the Content-Length before we start sending the body.
				char body[abc::size::k1 + 1];
				abc::buffer_streambuf sb(nullptr, 0, 0, body, 0, sizeof(body));
				abc::json_ostream<abc::size::_16, Log> json(&sb, base::_log);
				json.put_begin_object();
					json.put_property("playerId");
					json.put_number(endpoint_player_id);
				json.put_end_object();
				json.put_char('\0');
				json.flush();

				char content_length[abc::size::_32 + 1];
				std::snprintf(content_length, sizeof(content_length), "%zu", std::strlen(body));

				// Send the http response
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x105d9, "Sending response 200");
				}

				http.put_protocol(protocol::HTTP_11);
				http.put_status_code(status_code::OK);
				http.put_reason_phrase(reason_phrase::OK);

				http.put_header_name(header::Connection);
				http.put_header_value(connection::close);
				http.put_header_name(header::Content_Type);
				http.put_header_value(content_type::json);
				http.put_header_name(header::Content_Length);
				http.put_header_value(content_length);
				http.end_headers();

				http.put_body(body);

				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x105da, "game_endpoint::claim_player: Done.");
				}

				return true;
			}
		}

		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105db, "Resource error: Game not found. game_id=%u, player_i=%u", (unsigned)endpoint_game_id, player_i);
		}

		// 404
		base::send_simple_response(http, status_code::Not_Found, reason_phrase::Not_Found, content_type::text, "A game with the supplied ID was not found.", 0x105dc);
		return false;
	}


	template <typename Limits, typename Log>
	inline bool game_endpoint<Limits, Log>::accept_move(abc::http_server_stream<Log>& http, const char* method, endpoint_game_id_t endpoint_game_id, endpoint_player_id_t endpoint_player_id, const char* moves) {
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x105dd, "game_endpoint::accept_move: Start.");
		}

		if (!ascii::are_equal_i(moves, "moves")) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105de, "Resource error: '%s' must be 'moves'.", moves);
			}

			// 400
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "An invalid resource was supplied.", 0x105df);
			return false;
		}

		if (!verify_method_post(http, method)) {
			return false;
		}

		if (!verify_header_json(http)) {
			return false;
		}

		if (endpoint_game_id == 0 || endpoint_player_id == 0) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105e0, "Resource error: game_id=%u, player_id=%u",
					(unsigned)endpoint_game_id, (unsigned)endpoint_player_id);
			}

			// 400
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "An invalid resource was supplied.", 0x105e1);
			return false;
		}

		move mv;
		// Read move from JSON
		{
			std::streambuf* sb = static_cast<abc::http_request_istream<Log>&>(http).rdbuf();
			abc::json_istream<abc::size::_64, Log> json(sb, base::_log);
			char buffer[sizeof(abc::json::token_t) + abc::size::k1 + 1];
			abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(buffer);
			constexpr const char* invalid_json = "An invalid JSON payload was supplied. Must be: {\"row\": 0, \"col\": 1}.";

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::begin_object) {
				// Not {.
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105e2, "Content error: Expected '{'.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x105e3);
				return false;
			}

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::property || !ascii::are_equal(token->value.property, "row")) {
				// Not "row".
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105e4, "Content error: Expected \"row\".");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x105e5);
				return false;
			}

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::number || !(0 <= token->value.number && token->value.number <= 2)) {
				// Not a valid row.
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105e6, "Content error: Expected 0 <= number <= 2.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x105e7);
				return false;
			}

			mv.row = token->value.number;

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::property || !ascii::are_equal(token->value.property, "col")) {
				// Not "col".
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105e8, "Content error: Expected \"col\".");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x105e9);
				return false;
			}

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::number || !(0 <= token->value.number && token->value.number <= 2)) {
				// Not a valid row.
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105ea, "Content error: Expected 0 <= number <= 2.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x105eb);
				return false;
			}

			mv.col = token->value.number;
		}

		for (std::size_t game_i = 0; game_i < _game_count; game_i++) {
			if (_games[game_i].id() == endpoint_game_id) {
				player_id_t player_id = _games[game_i].player_id(endpoint_player_id);

				if (player_id == player_id::none) {
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105ec, "Resource error: Player not found. player_id=%u", (unsigned)endpoint_player_id);
					}

					// 404
					base::send_simple_response(http, status_code::Not_Found, reason_phrase::Not_Found, content_type::text, "A player with the supplied ID was not found.", 0x105ed);
					return false;
				}

				if (!_games[game_i].accept_move(player_id, mv)) {
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105ee, "Resource error: Move not accepted. move={ %u, %u }", mv.row, mv.col);
					}

					// 403
					base::send_simple_response(http, status_code::Forbidden, reason_phrase::Forbidden, content_type::text, "The move was not accepted.", 0x105ef);
					return false;
				}

				// Return 200.
				// Write the JSON to a char buffer, so we can calculate the Content-Length before we start sending the body.
				char body[abc::size::k1 + 1];
				abc::buffer_streambuf sb(nullptr, 0, 0, body, 0, sizeof(body));
				abc::json_ostream<abc::size::_16, Log> json(&sb, base::_log);

				json.put_begin_object();
					json.put_property("i");
					json.put_number(_games[game_i].board().move_count() - 1);

					if (_games[game_i].board().is_game_over()) {
						json.put_property("winner");
						json.put_number(_games[game_i].board().winner());
					}

				json.put_end_object();
				json.put_char('\0');
				json.flush();

				char content_length[abc::size::_32 + 1];
				std::snprintf(content_length, sizeof(content_length), "%zu", std::strlen(body));

				// Send the http response
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x105f0, "Sending response 200");
				}

				http.put_protocol(protocol::HTTP_11);
				http.put_status_code(status_code::OK);
				http.put_reason_phrase(reason_phrase::OK);

				http.put_header_name(header::Connection);
				http.put_header_value(connection::close);
				http.put_header_name(header::Content_Type);
				http.put_header_value(content_type::json);
				http.put_header_name(header::Content_Length);
				http.put_header_value(content_length);
				http.end_headers();

				http.put_body(body);

				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x105f1, "game_endpoint::get_moves: Done.");
				}

				return true;
			}
		}

		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105f2, "Resource error: Game not found. game_id=%u", (unsigned)endpoint_game_id);
		}

		// 404
		base::send_simple_response(http, status_code::Not_Found, reason_phrase::Not_Found, content_type::text, "A game with the supplied ID was not found.", 0x105f3);
		return false;
	}


	template <typename Limits, typename Log>
	inline bool game_endpoint<Limits, Log>::get_moves(abc::http_server_stream<Log>& http, const char* method, endpoint_game_id_t endpoint_game_id, unsigned since_move_i) {
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x105f4, "game_endpoint::get_moves: Start.");
		}

		if (!verify_method_get(http, method)) {
			return false;
		}

		if (endpoint_game_id == 0) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105f5, "Resource error: game_id=%u", (unsigned)endpoint_game_id);
			}

			// 400
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "An invalid resource was supplied.", 0x105f6);
			return false;
		}

		for (std::size_t game_i = 0; game_i < _game_count; game_i++) {
			if (_games[game_i].id() == endpoint_game_id) {
				// Write the JSON to a char buffer, so we can calculate the Content-Length before we start sending the body.
				char body[abc::size::k1 + 1];
				abc::buffer_streambuf sb(nullptr, 0, 0, body, 0, sizeof(body));
				abc::json_ostream<abc::size::_16, Log> json(&sb, base::_log);

				json.put_begin_object();
					json.put_property("moves");
						json.put_begin_array();

						for (std::size_t move_i = since_move_i; move_i < _games[game_i].board().move_count(); move_i++) {
							json.put_begin_object();
								json.put_property("i");
								json.put_number(move_i);
								json.put_property("move");
								json.put_begin_object();
									json.put_property("row");
									json.put_number(_games[game_i].moves()[move_i].row);
									json.put_property("col");
									json.put_number(_games[game_i].moves()[move_i].col);
								json.put_end_object();
							json.put_end_object();
						}

						json.put_end_array();

					if (_games[game_i].board().is_game_over()) {
						json.put_property("winner");
						json.put_number(_games[game_i].board().winner());
					}

				json.put_end_object();
				json.put_char('\0');
				json.flush();

				char content_length[abc::size::_32 + 1];
				std::snprintf(content_length, sizeof(content_length), "%zu", std::strlen(body));

				// Send the http response
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x105f7, "Sending response 200");
				}

				http.put_protocol(protocol::HTTP_11);
				http.put_status_code(status_code::OK);
				http.put_reason_phrase(reason_phrase::OK);

				http.put_header_name(header::Connection);
				http.put_header_value(connection::close);
				http.put_header_name(header::Content_Type);
				http.put_header_value(content_type::json);
				http.put_header_name(header::Content_Length);
				http.put_header_value(content_length);
				http.end_headers();

				http.put_body(body);

				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x105f8, "game_endpoint::get_moves: Done.");
				}

				return true;
			}
		}

		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x105f9, "Resource error: Game not found. game_id=%u, since_move_i=%u", (unsigned)endpoint_game_id, since_move_i);
		}

		// 404
		base::send_simple_response(http, status_code::Not_Found, reason_phrase::Not_Found, content_type::text, "A game with the supplied ID was not found.", 0x105fa);
		return false;
	}


	template <typename Limits, typename Log>
	inline void game_endpoint<Limits, Log>::process_shutdown(abc::http_server_stream<Log>& http, const char* method) {
		if (!verify_method_post(http, method)) {
			return;
		}

		base::set_shutdown_requested();

		// 200
		base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, "Server is shuting down...", 0x105fb);
	}


	template <typename Limits, typename Log>
	inline bool game_endpoint<Limits, Log>::verify_method_get(abc::http_server_stream<Log>& http, const char* method) {
		if (!ascii::are_equal_i(method, method::GET)) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x105fc, "Method error: Expected 'GET'.");
			}

			// 405
			base::send_simple_response(http, status_code::Method_Not_Allowed, reason_phrase::Method_Not_Allowed, content_type::text, "Expected method GET for this request.", 0x105fd);
			return false;
		}

		return true;
	}


	template <typename Limits, typename Log>
	inline bool game_endpoint<Limits, Log>::verify_method_post(abc::http_server_stream<Log>& http, const char* method) {
		if (!ascii::are_equal_i(method, method::POST)) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x105fe, "Method error: Expected 'POST'.");
			}

			// 405
			base::send_simple_response(http, status_code::Method_Not_Allowed, reason_phrase::Method_Not_Allowed, content_type::text, "Expected method POST for this request.", 0x105ff);
			return false;
		}

		return true;
	}


	template <typename Limits, typename Log>
	inline bool game_endpoint<Limits, Log>::verify_header_json(abc::http_server_stream<Log>& http) {
		bool has_content_type_json = false;

		// Read all headers
		while (true) {
			char header[abc::size::k1 + 1];

			// No more headers
			http.get_header_name(header, sizeof(header));
			if (http.gcount() == 0) {
				break;
			}

			if (ascii::are_equal_i(header, header::Content_Type)) {
				if (has_content_type_json) {
					// We've already received a Content-Type header.
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x10600, "Header error: Already received 'Content-Type'.");
					}

					// 400
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "The Content-Type header was supplied more than once.", 0x10601);
					return false;
				}

				http.get_header_value(header, sizeof(header));

				static const std::size_t content_type_json_len = std::strlen(content_type::json);
				if (!ascii::are_equal_i_n(header, content_type::json, content_type_json_len)) {
					// The Content-Type is not json.
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x10602, "Header error: Expected `application/json` as 'Content-Type'.");
					}

					// 400
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "'application/json' is the only supported Content-Type.", 0x10603);
					return false;
				}

				has_content_type_json = true;
			}
			else {
				// Future-proof: Ignore unknown headers.
				http.get_header_value(header, sizeof(header));
			}
		}

		return has_content_type_json;
	}

}}

