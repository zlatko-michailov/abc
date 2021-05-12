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


#include <cstdint>

#include "../../src/log.h"
#include "../../src/vmem.h"


// --------------------------------------------------------------


using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;
using results_ostream = abc::log_ostream<abc::test_line_ostream<>, abc::log_filter>;

// Max 8 pages = 32KB in memory.
using vmem_pool = abc::vmem_pool<8, log_ostream>;
using vmem_page = abc::vmem_page<vmem_pool, log_ostream>;


constexpr int size = 3;

// IMPORTANT: Ensure a predictable layout of the data on disk!
#pragma pack(push, 1)

using board_state = std::uint32_t;
using board_move_stats = std::int8_t[size][size];

#pragma pack(pop)


using vmem_kb = abc::vmem_map<board_state, board_move_stats, vmem_pool, log_ostream>;


// --------------------------------------------------------------


using player	= int;

constexpr player	player_none	= 0x0;
constexpr player	player_x	= 0x2;
constexpr player	player_o	= 0x3;


// --------------------------------------------------------------


struct move {
	int		row;
	int		col;

	bool	is_valid() const;
};


inline bool move::is_valid() const {
	return (0 <= row && row < size && 0 <= col && col < size);
}


// --------------------------------------------------------------


class board {
public:
	////board();

public:
	bool			is_game_over() const;
	player			winner() const;
	bool			accept_move(const move& mv);

public:
	player			get_move(const move& mv) const;
	void			set_move(const move& mv);
	void			clear_move(const move& mv);
	bool			has_move(player plr, const move& mv) const;
	bool			check_winner();
	void			switch_current_player();

private:
	static player	opponent(player plr);

private:
	player			_winner			= player_none;
	player			_current_player	= player_x;
	board_state		_board_state	= { 0 };
};


inline bool board::is_game_over() const {
	return _winner != player_none;
}


inline player board::winner() const {
	return _winner;
}


inline bool board::accept_move(const move& mv) {
	if (is_game_over()) {
		return false;
	}

	if (get_move(mv) != player_none) {
		return false;
	}

	set_move(mv);
	if (!check_winner()) {
		switch_current_player();
	}

	return true;
}


inline player board::get_move(const move& mv) const {
	int cell = mv.row * size + mv.col;
	return (_board_state >> (cell * 2)) & 0x3;
}


inline void board::set_move(const move& mv) {
	int cell = mv.row * size + mv.col;
	_board_state |= (_current_player << (cell * 2));
}


inline void board::clear_move(const move& mv) {
	int cell = mv.row * size + mv.col;
	_board_state &= ~(0x3 << (cell * 2));
}


inline bool board::has_move(player plr, const move& mv) const {
	int cell = mv.row * size + mv.col;
	int bits = (plr << (cell * 2));
	return (_board_state & bits) == bits;
}


inline bool board::check_winner() {
	bool horizontal =
		(has_move(_current_player, { 0, 0 }) && has_move(_current_player, { 0, 1 }) && has_move(_current_player, { 0, 2 })) ||
		(has_move(_current_player, { 1, 0 }) && has_move(_current_player, { 1, 1 }) && has_move(_current_player, { 1, 2 })) ||
		(has_move(_current_player, { 2, 0 }) && has_move(_current_player, { 2, 1 }) && has_move(_current_player, { 2, 2 }));

	bool vertical =
		(has_move(_current_player, { 0, 0 }) && has_move(_current_player, { 1, 0 }) && has_move(_current_player, { 2, 0 })) ||
		(has_move(_current_player, { 0, 1 }) && has_move(_current_player, { 1, 1 }) && has_move(_current_player, { 2, 1 })) ||
		(has_move(_current_player, { 0, 2 }) && has_move(_current_player, { 1, 2 }) && has_move(_current_player, { 2, 2 }));

	bool diagonal =
		(has_move(_current_player, { 0, 0 }) && has_move(_current_player, { 1, 1 }) && has_move(_current_player, { 2, 2 })) ||
		(has_move(_current_player, { 0, 2 }) && has_move(_current_player, { 1, 1 }) && has_move(_current_player, { 2, 0 }));

	return horizontal || vertical || diagonal;
}


inline void board::switch_current_player() {
	_current_player = opponent(_current_player);
}


inline player board::opponent(player plr) {
	return plr ^ 0x1;
}


// --------------------------------------------------------------


template <typename Engine1, typename Engine2>
class game {
public:
	game(board* brd, Engine1* eng1, Engine2* eng2, log_ostream* log);

public:
	void start();

private:
	board*			_board;
	Engine1*		_engine1;
	Engine2*		_engine2;
	log_ostream*	_log;
};


template <typename Engine1, typename Engine2>
inline game<Engine1, Engine2>::game(board* brd, Engine1* eng1, Engine2* eng2, log_ostream* log)
	: _board(brd)
	, _engine1(eng1)
	, _engine2(eng2)
	, _log(log) {
}


template <typename Engine1, typename Engine2>
inline void game<Engine1, Engine2>::start() {
	for (;;) {
		move mv = _engine1->make_move(static_cast<const board*>(_board));
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "x: row=%d, col=%d", mv.row, mv.col);

		if (!_board->accept_move(mv)) {
			_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Move (x) not accepted");
			break;
		}

		if (_board->is_game_over()) {
			_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Winner! (x)");
			break;
		}

		mv = _engine2->make_move(static_cast<const board*>(_board));
		_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "o: row=%d, col=%d", mv.row, mv.col);

		if (!_board->accept_move(mv)) {
			_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Move (o) not accepted");
			break;
		}

		if (_board->is_game_over()) {
			_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Winner! (o)");
			break;
		}
	}
}


// --------------------------------------------------------------


class thinking_slow {
public:
	move	make_move(const board* brd);

private:
	move	make_necessary_move(const board* brd);
	move	find_best_move(board* brd);
};


inline move thinking_slow::make_move(const board* brd) {
	move mv = make_necessary_move(brd);
	if (mv.is_valid()) {
		return mv;
	}

	board board_copy = *brd;
	return find_best_move(&board_copy);

}


inline move thinking_slow::find_best_move(board* brd) {
	//// Try the center
	//// Try the corners
	//// Try the sides
}


// --------------------------------------------------------------

