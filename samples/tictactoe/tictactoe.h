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


#include <cstdlib>

#include "../../src/ascii.h"
#include "../../src/endpoint.h"
#include "../../src/http.h"
#include "../../src/json.h"

#include "tictactoe.i.h"


namespace abc { namespace samples {


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


	inline void player_agent::reset(game* game, player_id_t player_id, player_type_t player_type, log_ostream* log) {
		_game = game;
		_player_id = player_id;
		_player_type = player_type;
		_log = log;
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


	inline void game::reset(player_type_t player_x_type, player_type_t player_o_type, log_ostream* log) {
		_agent_x.reset(this, player_id::x, player_x_type, log);
		_agent_o.reset(this, player_id::o, player_o_type, log);
		_log = log;
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


	inline bool endpoint_game::is_done() const {
		return _endpoint_game_id == 0;
	}


	// --------------------------------------------------------------


	template <typename Limits, typename Log>
	inline tictactoe_endpoint<Limits, Log>::tictactoe_endpoint(endpoint_config* config, Log* log)
		: base(config, log) {
	}


	template <typename Limits, typename Log>
	inline void tictactoe_endpoint<Limits, Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "tictactoe_endpoint::process_rest_request: Start.");
		}

		if (ascii::are_equal_i_n(resource, "/games", 6)) {
			process_games(http, method, resource);
		}
		else if (ascii::are_equal_i(resource, "/shutdown")) {
			process_shutdown(http, method);
		}
		else {
			// 404
			base::send_simple_response(http, status_code::Not_Found, reason_phrase::Not_Found, content_type::text, "The requested resource was not found.", __TAG__);
		}

		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "tictactoe_endpoint::process_rest_request: Done.");
		}
	}


	template <typename Limits, typename Log>
	inline void tictactoe_endpoint<Limits, Log>::process_games(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		const char* resource_games = resource + 6;

		if (ascii::are_equal_i(resource_games, "")) {
			create_game(http, method);
		}
	}


	template <typename Limits, typename Log>
	inline void tictactoe_endpoint<Limits, Log>::create_game(abc::http_server_stream<Log>& http, const char* method) {
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "tictactoe_endpoint::create_game: Start.");
		}

		if (!verify_method_post(http, method)) {
			return;
		}

		if (!verify_header_json(http)) {
			return;
		}

		char players[2][abc::size::_64 + 1];
		bool has_players = false;

		// Use a block to release the buffers when done parsing
		{
			std::streambuf* sb = static_cast<abc::http_request_istream<Log>&>(http).rdbuf();
			abc::json_istream<abc::size::_64, Log> json(sb, base::_log);
			char buffer[sizeof(abc::json::token_t) + abc::size::k1 + 1];
			abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(buffer);
			constexpr const char* invalid_json = "An invalid JSON payload was supplied. Must be: {\"players\": [ \"external\", \"slow_engine\" ]}.";

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::begin_object) {
				// The body is not a JSON object.
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Expected '{'.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, __TAG__);
				return;
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
						base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Expected a property.");
					}

					// 400
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, __TAG__);
					return;
				}

				// We expect 1 property - "players".
				if (ascii::are_equal(token->value.property, "players")) {
					// Parse array [2].
					json.get_token(token, sizeof(buffer));

					if (token->item != abc::json::item::begin_array) {
						// Not [.
						if (base::_log != nullptr) {
							base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Expected '['.");
						}

						// 400
						base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, __TAG__);
						return;
					}

					for (std::size_t i = 0; i < 2; i++) {
						if (base::_log != nullptr) {
							base::_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Parsing players[%zu]", i);
						}

						json.get_token(token, sizeof(buffer));
						if (token->item != abc::json::item::string) {
							// Not a string.
							if (base::_log != nullptr) {
								base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Expected a string.");
							}

							// 400
							base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, __TAG__);
							return;
						}

						std::strcpy(players[i], token->value.string);
						if (base::_log != nullptr) {
							base::_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "players[%zu]='%s'", i, players[i]);
						}
					}

					json.get_token(token, sizeof(buffer));
					if (token->item != abc::json::item::end_array) {
						// Not ].
						if (base::_log != nullptr) {
							base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Expected ']'.");
						}

						// 400
						base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, __TAG__);
						return;
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
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Players not received.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, __TAG__);
				return;
			}
		}

		// Player types
		constexpr const char* invalid_player_type = "An invalid player type was received.";

		player_type_t player_x_type = player_type::from_text(players[0]);
		if (player_x_type == player_type::none) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Invalid value of players[0]='%s'.", players[0]);
			}

			// 400
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_player_type, __TAG__);
			return;
		}

		player_type_t player_o_type = player_type::from_text(players[1]);
		if (player_o_type == player_type::none) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Invalid value of players[1]='%s'.", players[1]);
			}

			// 400
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_player_type, __TAG__);
			return;
		}

		endpoint_game_id_t endpoint_game_id = ((std::rand() & 0xffff) << 16) | ((std::rand() & 0xffff));

		// Find a slot
		std::size_t game_i = max_game_count;
		if (_game_count < max_game_count) {
			game_i = _game_count++;
		}
		else {
			for (std::size_t i = 0; i < max_game_count; i++) {
				if (_games[i].is_done()) {
					game_i = i;

					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "tictactoe_endpoint::create_game: game_i=%zu", game_i);
					}
					break;
				}
			}
		}

		if (game_i >= max_game_count) {
			constexpr const char* no_game_capacity = "The service has a temporary game capacity shortage.";

			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Service error: Out of game capacity.");
			}

			// 503
			base::send_simple_response(http, status_code::Service_Unavailable, reason_phrase::Service_Unavailable, content_type::text, no_game_capacity, __TAG__);
			return;
		}

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
			base::_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Sending response 200");
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
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "tictactoe_endpoint::create_game: Done.");
		}
	}


	template <typename Limits, typename Log>
	inline void tictactoe_endpoint<Limits, Log>::process_shutdown(abc::http_server_stream<Log>& http, const char* method) {
		if (!verify_method_post(http, method)) {
			return;
		}

		base::set_shutdown_requested();

		// 200
		base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, "Server is shuting down...", __TAG__);
	}


	template <typename Limits, typename Log>
	inline bool tictactoe_endpoint<Limits, Log>::verify_method_post(abc::http_server_stream<Log>& http, const char* method) {
		if (!ascii::are_equal_i(method, method::POST)) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Method error: Expected 'POST'.");
			}

			// 405
			base::send_simple_response(http, status_code::Method_Not_Allowed, reason_phrase::Method_Not_Allowed, content_type::text, "POST is the only supported method for resource '/problem'.", __TAG__);
			return false;
		}

		return true;
	}


	template <typename Limits, typename Log>
	inline bool tictactoe_endpoint<Limits, Log>::verify_header_json(abc::http_server_stream<Log>& http) {
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
						base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Header error: Already received 'Content-Type'.");
					}

					// 400
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "The Content-Type header was supplied more than once.", __TAG__);
					return false;
				}

				http.get_header_value(header, sizeof(header));

				static const std::size_t content_type_json_len = std::strlen(content_type::json);
				if (!ascii::are_equal_i_n(header, content_type::json, content_type_json_len)) {
					// The Content-Type is not json.
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Header error: Expected `application/json` as 'Content-Type'.");
					}

					// 400
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "'application/json' is the only supported Content-Type.", __TAG__);
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

