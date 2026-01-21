/*
MIT License

Copyright (c) 2018-2026 Zlatko Michailov 

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
#include <sstream>
#include <string>

#include "../../src/root/ascii.h"
#include "../../src/net/endpoint.h"
#include "../../src/net/http.h"
#include "../../src/net/json.h"

#include "game.i.h"


// --------------------------------------------------------------


constexpr std::size_t len_request_path_games = 6U;


// --------------------------------------------------------------


inline vmem_bundle::vmem_bundle(abc::vmem::pool_config&& pool_config, abc::diag::log_ostream* log)
    : pool(std::move(pool_config), log)
    , start_page(&pool, abc::vmem::page_pos_start, log)
    , state_scores_map(&static_cast<start_page_layout*>(start_page.ptr())->map_state, &pool, log)
    , log(log) {
}


// --------------------------------------------------------------


inline player_type_t player_type::from_text(const char* text) {
    if (abc::ascii::are_equal(text, "external")) {
        return player_type::external;
    }
    else if (abc::ascii::are_equal(text, "slow_engine")) {
        return player_type::slow_engine;
    }
    else if (abc::ascii::are_equal(text, "fast_engine")) {
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


inline board::board(abc::diag::log_ostream* log)
    : diag_base("board", log) {
}


inline void board::reset() {
    constexpr const char* suborigin = "reset()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107c1, "Begin:");

    _is_game_over      = false;
    _winner            = player_id::none;
    _current_player_id = player_id::x;
    _board_state       = { };
    _move_count        = 0;

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107c2, "End:");
}


inline void board::accept_move(const move& move) {
    constexpr const char* suborigin = "accept_move()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107c3, "Begin: move={%u,%u}", move.row, move.col);

    diag_base::expect(suborigin, move.is_valid(), 0x107c4, "move.is_valid()");
    diag_base::expect(suborigin, !is_game_over(), 0x107c5, "!is_game_over()");
    diag_base::expect(suborigin, get_move(move) == player_id::none, 0x107c6, "get_move(move) == player_id::none"); 
    diag_base::expect(suborigin, move.row == 0 || get_move({move.row - 1, move.col}) != player_id::none, 0x107c7, "move.row == 0 || get_move({move.row - 1, move.col}) != player_id::none"); 

    set_move(move);
    check_winner(move);

    if (!is_game_over()) {
        switch_current_player_id();
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107c8, "End:");
}


inline void board::undo_move(const move& move) {
    constexpr const char* suborigin = "undo_move()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107c9, "Begin: move={%u,%u}", move.row, move.col);

    diag_base::expect(suborigin, move.is_valid(), 0x107ca, "move.is_valid()");
    diag_base::expect(suborigin, get_move(move) != player_id::none, 0x107cb, "get_move(move) != player_id::none"); 
    diag_base::expect(suborigin, move.row == 0 || get_move({move.row - 1, move.col}) != player_id::none, 0x107cc, "move.row == 0 || get_move({move.row - 1, move.col}) != player_id::none"); 

    if (!is_game_over()) {
        switch_current_player_id();
    }
    clear_move(move);

    _winner = player_id::none;
    _is_game_over = false;

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107cd, "End:");
}


inline bool board::is_game_over() const {
    return _is_game_over;
}


inline player_id_t board::winner() const {
    return _winner;
}


inline player_id_t board::get_move(const move& move) const {
    constexpr const char* suborigin = "get_move()";
    count_t col_sz = col_size(move.col);
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107ce, "Begin: board_state=0x%16.16llx, col_sz=%u, move.row=%u", (unsigned long long)_board_state, col_sz, move.row);

    player_id_t ret = player_id::none;
    if (col_sz == 0 || move.row >= col_sz) {
        ret = player_id::none;
    }
    else {
        ret = get_move_bits(move);
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107cf, "End: player_id=%u", ret);

    return ret;
}


inline void board::set_move(const move& move) {
    inc_col_size(move.col);
    set_move_bits(move, _current_player_id);
    _move_count++;
}


inline void board::clear_move(const move& move) {
    dec_col_size(move.col);
    clear_move_bits(move);
    _move_count--;
}


inline bool board::has_move(player_id_t player_id, const move& move) const {
    return get_move(move) == player_id;
}


inline bool board::check_winner(const move& move) {
    count_t west_count = 0;
    for (count_t c = move.col - 1; 0 <= c && has_move(_current_player_id, { move.row, c }); c--) {
        west_count++;
    }

    count_t east_count = 0;
    for (count_t c = move.col + 1; c < col_count && has_move(_current_player_id, { move.row, c }); c++) {
        east_count++;
    }

    count_t south_count = 0;
    for (count_t r = move.row - 1; 0 <= r && has_move(_current_player_id, { r, move.col }); r--) {
        south_count++;
    }

    count_t southwest_count = 0;
    for (count_t i = 1; 0 <= move.row - i && 0 <= move.col - i && has_move(_current_player_id, { move.row - i, move.col - i }); i++) {
        southwest_count++;
    }

    count_t northeast_count = 0;
    for (count_t i = 1; move.row + i < row_count && move.col + i < col_count && has_move(_current_player_id, { move.row + i, move.col + i }); i++) {
        northeast_count++;
    }

    count_t southeast_count = 0;
    for (count_t i = 1; 0 <= move.row - i && move.col + i < col_count && has_move(_current_player_id, { move.row - i, move.col + i }); i++) {
        southeast_count++;
    }

    count_t northwest_count = 0;
    for (count_t i = 1; move.row + i < row_count && 0 <= move.col - i && has_move(_current_player_id, { move.row + i, move.col - i }); i++) {
        northwest_count++;
    }

    bool horizontal  = west_count + east_count >= 3;
    bool vertical    = south_count >= 3;
    bool diagonal1   = southwest_count + northeast_count >= 3;
    bool diagonal2   = southeast_count + northwest_count >= 3;

    bool win = (horizontal || vertical || diagonal1 || diagonal2);

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
    return player_id ^ player_id::mask;
}


inline count_t board::col_size(count_t col) const {
    count_t col_sz = ( (_board_state >> col_pos(col)) & col_size_mask );

    return col_sz;
}


inline void board::inc_col_size(count_t col) {
    constexpr const char* suborigin = "inc_col_size()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107d0, "Begin: board_state=0x%16.16llx, col=%u", (unsigned long long)_board_state, col);

    _board_state += (board_state_1 << col_pos(col));

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107d1, "End: board_state=0x%16.16llx", (unsigned long long)_board_state);
}


inline void board::dec_col_size(count_t col) {
    constexpr const char* suborigin = "dec_col_size()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107d2, "Begin: board_state=0x%16.16llx, col=%u", (unsigned long long)_board_state, col);

    _board_state -= (board_state_1 << col_pos(col));

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107d3, "End: board_state=0x%16.16llx", (unsigned long long)_board_state);
}


inline count_t board::col_pos(count_t col) const {
    return sizes_pos + col * col_size_bit_count;
}


inline player_id_t board::get_move_bits(const move& move) const {
    constexpr const char* suborigin = "get_move_bits()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107d4, "Begin: board_state=0x%16.16llx, move={%u,%u}", (unsigned long long)_board_state, move.row, move.col);

    count_t pos = move_pos(move);
    player_id_t move_bits = ( (_board_state >> pos) & move_mask );

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107d5, "End: bits=%u", move_bits);

    return move_bits;
}


inline void board::set_move_bits(const move& move, count_t bits) {
    constexpr const char* suborigin = "set_move_bits()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107d6, "Begin: board_state=0x%16.16llx, move={%u,%u}, bits=%u", (unsigned long long)_board_state, move.row, move.col, bits);

    count_t pos = move_pos(move);

    clear_move_bits(move);

    _board_state |= ((board_state_t)bits << pos);

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107d7, "End: board_state=0x%16.16llx", (unsigned long long)_board_state);
}


inline void board::clear_move_bits(const move& move) {
    constexpr const char* suborigin = "clear_move_bits()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107d8, "Begin: board_state=0x%16.16llx, move={%u,%u}", (unsigned long long)_board_state, move.row, move.col);

    count_t pos = move_pos(move);

    _board_state &= ( ~((board_state_t)move_mask << pos) );

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107d9, "End: board_state=0x%16.16llx", (unsigned long long)_board_state);
}


inline count_t board::move_pos(const move& move) const {
    return moves_pos + move.col * col_bit_count + move.row * move_bit_count;
}


// --------------------------------------------------------------


inline player_agent::player_agent(abc::diag::log_ostream* log)
    : diag_base("player_agent", log)
    , _temp_board(log) {
}


inline void player_agent::reset(::game* game, player_id_t player_id, player_type_t player_type) {
    constexpr const char* suborigin = "reset()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107da, "Begin: player_id=%u, player_type=%u", player_id, player_type);

    _game        = game;
    _player_id   = player_id;
    _player_type = player_type;

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107db, "End:");
}


inline void player_agent::make_move_async() {
    constexpr const char* suborigin = "make_move_async()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10619, "Begin:");

    std::thread(player_agent::make_move_proc, this).detach();

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107dc, "End:");
}


inline void player_agent::make_move_proc(player_agent* this_ptr) {
    this_ptr->make_move();
}


inline void player_agent::make_move() {
    constexpr const char* suborigin = "make_move()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x1061b, "Begin:");

    switch (_player_type) {
        case player_type::slow_engine:
            slow_make_move();
            break;

        case player_type::fast_engine:
            fast_make_move();
            break;
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107dd, "End:");
}


inline void player_agent::slow_make_move() {
    constexpr const char* suborigin = "slow_make_move()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x1061c, "Begin: player_id=%u, board_state=0x%16.16llx", _player_id, (unsigned long long)_game->board().state());

    _temp_board = _game->board();

    move best_move;
    if (_game->moves().size() < 4) {
        slow_make_first_move(best_move);
    }
    else {
        int max_depth = slow_choose_max_depth();
        slow_find_best_move(best_move, max_depth, max_depth);
    }

    _game->accept_move(_player_id, best_move);

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107de, "End: best_move={%u,%u}", best_move.row, best_move.col);
}


inline void player_agent::slow_make_first_move(move& best_move) {
    constexpr const char* suborigin = "slow_make_first_move()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107df, "Begin: player_id=%u", _player_id);

    unsigned move_count = _game->moves().size();
    diag_base::expect(suborigin, move_count < 4, 0x107e0, "move_count < 4");

    move mid_next{ _game->board().col_size(col_count / 2), col_count / 2 };
    move right{ 0, col_count / 2 + 1 };
    move left{ 0, col_count / 2 - 1 };

    player_id_t opo = board::opponent(_player_id);

    if (move_count < 2) {
        best_move = mid_next;
    }
    else if (_game->board().get_move(move{ 0, col_count / 2 }) == _player_id) {
        best_move = mid_next;
    }
    else if (_game->board().get_move(right) == opo) {
        best_move = left;
    }
    else {
        best_move = right;
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107e1, "End: best_move={%u,%u}", best_move.row, best_move.col);
}


inline int player_agent::slow_choose_max_depth() const {
    constexpr const char* suborigin = "slow_choose_max_depth()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107e2, "Begin: player_id=%u", _player_id);

    unsigned move_count = _game->moves().size();
    int max_depth = -1;

    if (move_count < 12) {
        max_depth = 6;
    }
    else if (move_count < 18) {
        max_depth = 8;
    }
    else if (move_count < 24) {
        max_depth = 10;
    }
    else if (move_count < 30) {
        max_depth = 16;
    }
    else {
        max_depth = 20;
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107e3, "End: max_depth=%d", max_depth);

    return max_depth;
}


inline int player_agent::slow_find_best_move(move& best_move, int max_depth, int depth) {
    constexpr const char* suborigin = "slow_find_best_move()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107e4, "Begin: player_id=%u, max_depth=%d, depth=%d", _player_id, max_depth, depth);

    int best_score = -(2 * max_depth);

    int sign = -1;
    int mid = col_count / 2;
    for (int i = 0; i < col_count; i++) {
        count_t c = (count_t)(mid + sign * (i + 1) / 2);
        sign = -sign;

        move mv{ _temp_board.col_size(c), c };

        if (mv.is_valid()) {
            _temp_board.accept_move(mv);

            {
                int score = -max_depth;
                if (_temp_board.is_game_over()) {
                    score = _temp_board.winner() != player_id::none ? depth + 2 : 0;
                }
                else if (depth > 0) {
                    move dummy_mv;
                    score = -slow_find_best_move(dummy_mv, max_depth, depth - 1);
                }

                if (score > best_score) {
                    best_move = mv;
                    best_score = score;
                }

                if (depth == max_depth) {
                    diag_base::put_any(suborigin, abc::diag::severity::optional, 0x1061d, "mv.col=%d, score=%d", mv.col, score);
                }
            }

            _temp_board.undo_move(mv);
        }
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107e5, "End: best_score=%d", best_score);

    return best_score;
}


inline void player_agent::fast_make_move() {
    constexpr const char* suborigin = "fast_make_move()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107e6, "Begin: player_id=%u, board_state=0x%8.8x", _player_id, (unsigned)_game->board().state());

    move best_move = fast_find_best_move();
    _game->accept_move(_player_id, best_move);

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107e7, "End: best_move={%u,%u}", best_move.row, best_move.col);
}


inline move player_agent::fast_find_best_move() {
    constexpr const char* suborigin = "fast_find_best_move()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107e8, "Begin: player_id=%u, board_state=0x%16.16llx", _player_id, (unsigned long long)_game->board().state());

    std::lock_guard<std::mutex> lock(_vmem->mutex);

    state_scores_map::iterator itr = ensure_board_state_in_map(_game->board().state());
    diag_base::expect(suborigin, itr.can_deref(), 0x107e9, "itr.can_deref()");

    move some_move;
    bool should_explore = true; //// TODO: Calculate exploration

    // Analyze what we have.
    score_calc_t max_count   = 0;
    score_calc_t min_count   = 0;
    score_calc_t none_count  = 0;
    score_calc_t score_count = 0;
    score_calc_t score_sum   = 0;

    for (count_t c = 0; c < col_count; c++) {
        count_t r = _game->board().col_size(c);
        if (r >= row_count) {
            continue;
        }

        move mv{ r, c };

        if (_game->board().get_move(mv) == player_id::none) {
            score_calc_t curr_score = itr->value[c];

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
                score_count++;
                score_sum += learning_weight(curr_score);
            }
        }
    }

    // If there is one or more max scores, pick one of them.
    if (max_count > 0) {
        score_calc_t rand_i = static_cast<score_calc_t>(1 + std::rand() % max_count);

        for (count_t c = 0; c < col_count; c++) {
            count_t r = _game->board().col_size(c);
            if (r >= row_count) {
                continue;
            }

            move mv{ r, c };

            if (_game->board().get_move(mv) == player_id::none && itr->value[c] == score::max) {
                if (--rand_i == 0) {
                    diag_base::ensure(suborigin, mv.is_valid(), 0x107ea, "mv.is_valid()");
                    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107eb, "End: (max) mv={%u,%u}", mv.row, mv.col);

                    return mv;
                }
            }
        }
    }

    // If all the scores are min, pick one of them.
    else if (min_count > 0 && none_count == 0 && score_count == 0) {
        score_calc_t rand_i = static_cast<score_calc_t>(1 + std::rand() % min_count);

        for (count_t c = 0; c < col_count; c++) {
            count_t r = _game->board().col_size(c);
            if (r >= row_count) {
                continue;
            }

            move mv{ r, c };

            if (_game->board().get_move(mv) == player_id::none && itr->value[c] == score::min) {
                if (--rand_i == 0) {
                    diag_base::ensure(suborigin, mv.is_valid(), 0x107ec, "mv.is_valid()");
                    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107ed, "End: (min) mv={%u,%u}", mv.row, mv.col);

                    return mv;
                }
            }
        }
    }

    // Make a weighted pick - the weight of each move is its score.
    else {
        if (should_explore) {
            score_sum += none_count * learning_weight(score::mid);
        }

        score_calc_t rand_sum = static_cast<score_calc_t>(1 + std::rand() % score_sum);

        for (count_t c = 0; c < col_count; c++) {
            count_t r = _game->board().col_size(c);
            if (r >= row_count) {
                continue;
            }

            move mv{ r, c };
            score_calc_t curr_score = itr->value[c];

            if (_game->board().get_move(mv) == player_id::none) {
                if (score::min < curr_score && curr_score < score::max) {
                    some_move = mv;
                    rand_sum -= learning_weight(curr_score);
                }
                else if (should_explore && curr_score == score::none) {
                    some_move = mv;
                    rand_sum -= learning_weight(score::mid);
                }

                if (rand_sum <= 0) {
                    diag_base::ensure(suborigin, mv.is_valid(), 0x107ee, "mv.is_valid()");
                    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107ef, "End: mv={%u,%u}, curr_score=%d", mv.row, mv.col, curr_score);

                    return mv;
                }
            }
        }
    }

    diag_base::assert(suborigin, false, 0x107f0, "Impossible!");

    return some_move;
}


inline void player_agent::learn() {
    constexpr const char* suborigin = "learn()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107f1, "Begin: player_id=%u", _player_id);

    // Learning is a process that takes place after a game is over.
    // If the game was won by the agent's player, a "reward" is added to the score of each move made by the learning player, but the final can't be higher `max`.
    // If the game was drawn by the agent's player, a reward is still added, but it is smaller.
    // If the game was lost by the agent's player, a "penalty" is subtracted, but the final can't be lower than 'min'.

    std::lock_guard<std::mutex> lock(_vmem->mutex);

    // The temp board is used as a key in the knowledge base.
    board learning_key_board(diag_base::log());

    // Replay each move of the game, and update the scores for the learning player.
    for (unsigned i = 0; i < _game->moves().size(); i++) {
        move mv(_game->moves()[i]);

        if (learning_key_board.current_player_id() == _player_id) {
            // Find the current state in the knowledge base. 
            state_scores_map::iterator itr = ensure_board_state_in_map(learning_key_board.state());

            score_t old_score = itr->value[mv.col] == score::none ? score::mid : itr->value[mv.col];

            if (_game->board().winner() == _player_id) {
                // Win
                score_t new_score = old_score + score::win;
                itr->value[mv.col] = std::min(score::max, new_score);

                diag_base::put_any(suborigin, abc::diag::severity::debug, 0x105b1, "(win) move:%u, state=%8.8x, row=%d, col=%d, old_score=%d, new_score=%d",
                        i, learning_key_board.state(), mv.row, mv.col, old_score, new_score);
            }
            else if (_game->board().winner() == player_id::none) {
                // Draw
                score_t new_score = old_score + score::draw;
                itr->value[mv.col] = std::min(score::max, new_score);

                diag_base::put_any(suborigin, abc::diag::severity::debug, 0x105b2, "(draw) move:%u, state=%8.8x, row=%d, col=%d, old_score=%d, new_score=%d",
                        i, learning_key_board.state(), mv.row, mv.col, old_score, new_score);
            }
            else {
                // Loss
                score_t new_score = old_score + score::loss;
                itr->value[mv.col] = std::max(score::min, new_score);

                diag_base::put_any(suborigin, abc::diag::severity::debug, 0x105b3, "(loss) move:%u, state=%8.8x, row=%d, col=%d, old_score=%d, new_score=%d",
                        i, learning_key_board.state(), mv.row, mv.col, old_score, new_score);
            }
        }

        learning_key_board.accept_move(mv);
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107f2, "End:");
}


inline player_type_t player_agent::player_type() const {
    return _player_type;
}


inline state_scores_map::iterator player_agent::ensure_board_state_in_map(board_state_t board_state) {
    constexpr const char* suborigin = "ensure_board_state_in_map";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107f3, "Begin:");

    state_scores_map::iterator itr = _vmem->state_scores_map.find(board_state);

    if (itr.can_deref()) {
        diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107f4, "End:");

        return itr;
    }

    // An item with this key was not found. Insert a 'none' one.

    // Init the item before inserting it.
    state_scores_map::value_type item;
    item.key = board_state;
    for (int c = 0; c < col_count; c++) {
        item.value[c] = score::none;
    }

    // Insert the item.
    state_scores_map::iterator_bool itr_b = _vmem->state_scores_map.insert(item);
    diag_base::expect(suborigin, itr_b.second, 0x107f5, "itr_b.second");
    diag_base::expect(suborigin, itr_b.first.can_deref(), 0x107f6, "itr_b.first.can_deref()");

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107f7, "End:");

    return itr_b.first;
}


inline score_calc_t player_agent::learning_weight(score_calc_t score) noexcept {
    return score * score;
}


// --------------------------------------------------------------


inline game::game(abc::diag::log_ostream* log) 
    : game("game", log) {

}


inline game::game(const char* origin, abc::diag::log_ostream* log)
    : diag_base(abc::copy(origin), log)
    , _board(log)
    , _agent_x(log)
    , _agent_o(log) {
}


inline void game::reset(const player_types& player_types) {
    constexpr const char* suborigin = "reset";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107f8, "Begin:");

    _agent_x.reset(this, player_id::x, player_types.player_x_type);
    _agent_o.reset(this, player_id::o, player_types.player_o_type);
    _board.reset();

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107f9, "End:");
}


inline void game::start() {
    constexpr const char* suborigin = "start";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10621, "Begin: current_player_id=%u", _board.current_player_id());

    if (_board.current_player_id() == player_id::x) {
        _agent_x.make_move_async();
    }
    else if (_board.current_player_id() == player_id::o) {
        _agent_o.make_move_async();
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107fa, "End:");
}


inline std::size_t game::accept_move(player_id_t player_id, const move& move) {
    constexpr const char* suborigin = "accept_move";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107fb, "Begin: player_id=%u, move={%u,%u}", player_id, move.row, move.col);

    diag_base::expect(suborigin, player_id == _board.current_player_id(), 0x107fc, "player_id == _board.current_player_id()");

    _board.accept_move(move);
    _moves.push_back(move);
    std::size_t move_i = _moves.size() - 1;

    if (_board.is_game_over()) {
        if (_board.winner() != player_id::none) {
            diag_base::put_any(suborigin, abc::diag::severity::important, 0x10623, "GAME OVER - player_id=%u wins", _board.winner());
        }
        else {
            diag_base::put_any(suborigin, abc::diag::severity::important, 0x10624, "GAME OVER - draw");
        }

        for (std::size_t i = 0; i < _moves.size(); i++) {
            diag_base::put_any(suborigin, abc::diag::severity::optional, 0x10625, "%zu (%c) - {%u,%u}", i, (i & 1) == 0 ? 'X' : 'O', _moves[i].row, _moves[i].col);
        }

        if (_agent_x.player_type() == player_type::fast_engine && _agent_o.player_type() == player_type::slow_engine) {
            _agent_x.learn();
        }
        else if (_agent_o.player_type() == player_type::fast_engine && _agent_x.player_type() == player_type::slow_engine) {
            _agent_o.learn();
        }
    }
    else {
        if (_board.current_player_id() == player_id::x) {
            _agent_x.make_move_async();
        }
        else if (_board.current_player_id() == player_id::o) {
            _agent_o.make_move_async();
        }
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107fd, "End: move_i=%zu", move_i);

    return move_i;
}


inline const board& game::board() const {
    return _board;
}


inline const std::vector<move>& game::moves() const {
    return _moves;
}


// --------------------------------------------------------------


inline endpoint_game::endpoint_game(abc::diag::log_ostream* log)
    : base("endpoint_game", log) {
}


inline void endpoint_game::reset(endpoint_game_id_t endpoint_game_id,
                                player_type_t player_x_type, endpoint_player_id_t endpoint_player_x_id,
                                player_type_t player_o_type, endpoint_player_id_t endpoint_player_o_id) {
    constexpr const char* suborigin = "reset";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107fe, "Begin: endpoint_game_id=%u", endpoint_game_id);

    player_types player_types;
    player_types.player_x_type = player_x_type;
    player_types.player_o_type = player_o_type;

    base::reset(player_types);

    _endpoint_game_id                        = endpoint_game_id;
    _endpoint_player_x.endpoint_player_id    = endpoint_player_x_id;
    _endpoint_player_x.is_claimed            = endpoint_player_x_id == 0;
    _endpoint_player_o.endpoint_player_id    = endpoint_player_o_id;
    _endpoint_player_o.is_claimed            = endpoint_player_o_id == 0;

    if (_endpoint_player_x.is_claimed && _endpoint_player_o.is_claimed) {
        start();
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x107ff, "End:");
}


inline endpoint_player_id_t endpoint_game::claim_player(unsigned player_i) {
    constexpr const char* suborigin = "claim_player";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10800, "Begin: player_i=%u", player_i);

    diag_base::expect(suborigin, player_i <= 1, 0x10801, "player_i <= 1");

    endpoint_player_id_t endpoint_player_id;

    if (player_i == 0) {
        diag_base::expect(suborigin, !_endpoint_player_x.is_claimed, 0x10802, "!_endpoint_player_x.is_claimed");

        endpoint_player_id = _endpoint_player_x.endpoint_player_id;
        _endpoint_player_x.is_claimed = true;
    }
    else {
        diag_base::expect(suborigin, !_endpoint_player_o.is_claimed, 0x10803, "!_endpoint_player_o.is_claimed");

        endpoint_player_id = _endpoint_player_o.endpoint_player_id;
        _endpoint_player_o.is_claimed = true;
    }

    if (_endpoint_player_x.is_claimed && _endpoint_player_o.is_claimed) {
        start();
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10804, "End: endpoint_player_id=%u", endpoint_player_id);

    return endpoint_player_id;
}


inline bool endpoint_game::is_player_claimed(unsigned player_i) {
    constexpr const char* suborigin = "is_player_claimed";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10805, "Begin: player_i=%u", player_i);

    diag_base::expect(suborigin, player_i <= 1, 0x10806, "player_i <= 1");

    bool is_claimed = player_i == 0 ? _endpoint_player_x.is_claimed : _endpoint_player_o.is_claimed;

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10807, "End: is_claimed=%d", is_claimed);

    return is_claimed;
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


inline game_endpoint::game_endpoint(abc::net::http::endpoint_config&& config, abc::diag::log_ostream* log)
    : base(std::move(config), log) {
}


inline std::unique_ptr<abc::net::tcp_server_socket> game_endpoint::create_server_socket() {
    return std::unique_ptr<abc::net::tcp_server_socket>(new abc::net::tcp_server_socket(abc::net::socket::family::ipv4, base::log()));
}


inline void game_endpoint::process_rest_request(abc::net::http::server& http, const abc::net::http::request& request) {
    constexpr const char* suborigin = "process_rest_request";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10626, "Begin: method=%s, path=%s", request.method.c_str(), request.resource.path.c_str());

    try {
        if (abc::ascii::are_equal_i_n(request.resource.path.c_str(), "/games", len_request_path_games)) {
            process_games(http, request);
        }
        else if (abc::ascii::are_equal_i(request.resource.path.c_str(), "/shutdown")) {
            process_shutdown(http, request);
        }
        else {
            // 404
            throw_exception(suborigin, 0x10627,
                abc::net::http::status_code::Not_Found, abc::net::http::reason_phrase::Not_Found, abc::net::http::content_type::text, "The requested resource was not found.");
        }
    }
    catch (const abc::net::http::endpoint_error& err) {
        base::send_simple_response(http, err.status_code, err.reason_phrase.c_str(), err.content_type.c_str(), err.body.c_str(), err.tag);
    }
    catch (const std::runtime_error& err) {
        base::send_simple_response(http, abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, err.what(), 0x10808);
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10628, "End:");
}


inline void game_endpoint::process_games(abc::net::http::server& http, const abc::net::http::request& request) {
    constexpr const char* suborigin = "process_games";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10809, "Begin: method=%s, path=%s", request.method.c_str(), request.resource.path.c_str());

    const char* request_path_games = request.resource.path.c_str() + len_request_path_games;
    if (abc::ascii::are_equal_i(request_path_games, "")) {
        create_game(http, request);
    }
    else {
        unsigned game_id = 0;
        unsigned player_id = 0;
        unsigned player_i = 0;
        unsigned since_move_i = 0;
        char moves[6 + 1 + 1];
        moves[0] = '\0';

        if (std::sscanf(request_path_games, "/%u/players/%u/%6s", &game_id, &player_id, moves) == 3) {
            accept_move(http, request, static_cast<endpoint_game_id_t>(game_id), static_cast<endpoint_player_id_t>(player_id), moves);
        }
        else if (std::sscanf(request_path_games, "/%u/players/%u", &game_id, &player_i) == 2) {
            claim_player(http, request, static_cast<endpoint_game_id_t>(game_id), player_i);
        }
        else {
            abc::net::http::query::const_iterator query_since = request.resource.query.find("since");

            if (std::sscanf(request_path_games, "/%u/moves", &game_id) == 1
                && query_since != request.resource.query.end()
                && std::sscanf(query_since->second.c_str(), "%u", &since_move_i) == 1) {

                get_moves(http, request, static_cast<endpoint_game_id_t>(game_id), since_move_i);
            }
        }
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x1080a, "End:");
}


inline void game_endpoint::create_game(abc::net::http::server& http, const abc::net::http::request& request) {
    constexpr const char* suborigin = "create_game";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10629, "Begin: method=%s", request.method.c_str());

    require_method_post(suborigin, 0x1080b, request);
    require_content_type_json(suborigin, 0x1080c, request);

    player_types player_types = get_player_types(http, request);
    require(suborigin, 0x1080d, player_types.player_x_type != player_type::none && player_types.player_o_type != player_type::none,
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "At least one of the player types provided was invalid.");

    // Create an endpoint_game in memory.
    std::size_t game_i;
    {
        {
            //// TODO: Take a lock;
            game_i = _games.size();
            _games.push_back(endpoint_game(diag_base::log()));
            diag_base::put_any(suborigin, abc::diag::severity::optional, 0x105bd, "game_i=%zu", game_i);
        }

        endpoint_game_id_t endpoint_game_id = ((std::rand() & 0xffff) << 16) | ((std::rand() & 0xffff));

        endpoint_player_id_t endpoint_player_x_id = 0;
        endpoint_player_id_t endpoint_player_o_id = 0;

        if (player_types.player_x_type == player_type::external) {
            endpoint_player_x_id = ((std::rand() & 0xffff) << 16) | ((std::rand() & 0xffff));
        }

        if (player_types.player_o_type == player_type::external) {
            endpoint_player_o_id = ((std::rand() & 0xffff) << 16) | ((std::rand() & 0xffff));
        }

        _games[game_i].reset(endpoint_game_id, player_types.player_x_type, endpoint_player_x_id, player_types.player_o_type, endpoint_player_o_id);
    }

    // 200
    std::stringbuf sb;
    abc::net::json::writer json(&sb, diag_base::log());

    abc::net::json::literal::object obj = {
        { "gameId", abc::net::json::literal::number(_games[game_i].id()) }
    };
    json.put_value(abc::net::json::value(std::move(obj)));

    base::send_simple_response(http, abc::net::http::status_code::OK, abc::net::http::reason_phrase::OK, abc::net::http::content_type::json, sb.str().c_str(), 0x1080e);

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x1080f, "End:");
}


inline player_types game_endpoint::get_player_types(abc::net::http::server& http, const abc::net::http::request& /*request*/) {
    constexpr const char* suborigin = "get_player_types";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10810, "Begin:");

    player_types player_types{ player_type::none, player_type::none };

    std::streambuf* sb = static_cast<abc::net::http::request_reader&>(http).rdbuf();
    abc::net::json::reader json(sb, diag_base::log());

    abc::net::json::value val = json.get_value();
    require(suborigin, 0x1062f, val.type() == abc::net::json::value_type::object,
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a JSON object.");

    abc::net::json::literal::object::const_iterator players_itr = val.object().find("players");
    require(suborigin, 0x10811, players_itr != val.object().cend(),
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a \"players\" property.");
    require(suborigin, 0x10812, players_itr->second.type() == abc::net::json::value_type::array,
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a \"players\" array.");

    const abc::net::json::literal::array& players_array = players_itr->second.array();
    require(suborigin, 0x10813, players_array.size() == 2,
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a \"players\" array of size 2.");

    for (std::size_t i = 0; i < 2; i++) {
        require(suborigin, 0x10814, players_array[i].type() == abc::net::json::value_type::string,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a string item in the \"players\" array.");

        player_type_t current_player_type = player_type::from_text(players_array[i].string().c_str());
        require(suborigin, 0x10815, current_player_type != player_type::none,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a valid player_type item in the \"players\" array.");

        // { player_x_type, player_o_type }
        if (player_types.player_x_type == player_type::none) {
            player_types.player_x_type = current_player_type;
        }
        else {
            player_types.player_o_type = current_player_type;
        }
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10816, "End:");
    return player_types;
}


inline void game_endpoint::claim_player(abc::net::http::server& http, const abc::net::http::request& request, endpoint_game_id_t endpoint_game_id, unsigned player_i) {
    constexpr const char* suborigin = "claim_player";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10641, "Begin: method=%s, game_id=%u, player_i=%u", request.method.c_str(), (unsigned)endpoint_game_id, (unsigned)player_i);

    require_method_post(suborigin, 0x10817, request);

    require(suborigin, 0x10818, endpoint_game_id > 0 && player_i <= 1,
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Resource error: An invalid game ID or player ID was supplied.");

    for (std::size_t game_i = 0; game_i < _games.size(); game_i++) {
        if (_games[game_i].id() == endpoint_game_id) {
            require(suborigin, 0x10819, !_games[game_i].is_player_claimed(player_i),
                abc::net::http::status_code::Conflict, abc::net::http::reason_phrase::Conflict, abc::net::http::content_type::text, "State error: The player with the given index has already been claimed.");

            endpoint_player_id_t endpoint_player_id = _games[game_i].claim_player(player_i);

            // 200
            std::stringbuf sb;
            abc::net::json::writer json(&sb, diag_base::log());

            abc::net::json::literal::object obj = {
                { "playerId", abc::net::json::literal::number(endpoint_player_id) }
            };
            json.put_value(abc::net::json::value(std::move(obj)));

            base::send_simple_response(http, abc::net::http::status_code::OK, abc::net::http::reason_phrase::OK, abc::net::http::content_type::json, sb.str().c_str(), 0x1081a);

            diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10647, "End:");
            return;
        }
    }

    // 404
    throw_exception(suborigin, 0x10649,
        abc::net::http::status_code::Not_Found, abc::net::http::reason_phrase::Not_Found, abc::net::http::content_type::text, "A game with the supplied ID was not found.");
}


inline void game_endpoint::accept_move(abc::net::http::server& http, const abc::net::http::request& request, endpoint_game_id_t endpoint_game_id, endpoint_player_id_t endpoint_player_id, const char* moves) {
    constexpr const char* suborigin = "accept_move";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x1064a, "Begin: method=%s, game_id=%u, player_i=%u", request.method.c_str(), (unsigned)endpoint_game_id, (unsigned)endpoint_player_id);

    require_method_post(suborigin, 0x1081b, request);
    require_content_type_json(suborigin, 0x1081c, request);

    require(suborigin, 0x1064b, abc::ascii::are_equal_i(moves, "moves"),
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Resource error: The segment after the player ID must be 'moves'.");

    require(suborigin, 0x1064d, endpoint_game_id > 0 && endpoint_player_id > 0,
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Resource error: An invalid game ID or player ID was supplied.");

    move mv;
    // Read move from JSON
    {
        std::streambuf* sb = static_cast<abc::net::http::request_reader&>(http).rdbuf();
        abc::net::json::reader json(sb, diag_base::log());

        abc::net::json::value val = json.get_value();
        require(suborigin, 0x1064f, val.type() == abc::net::json::value_type::object,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a JSON object.");

        abc::net::json::literal::object::const_iterator row_itr = val.object().find("row");
        require(suborigin, 0x10651, val.type() == abc::net::json::value_type::object,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Missing property \"row\".");
        require(suborigin, 0x10653, row_itr->second.type() == abc::net::json::value_type::number && static_cast<int>(row_itr->second.number()) == row_itr->second.number() && 0 <= row_itr->second.number() && row_itr->second.number() < row_count,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected 0 <= row <= 2.");

        mv.row = static_cast<count_t>(row_itr->second.number());

        abc::net::json::literal::object::const_iterator col_itr = val.object().find("col");
        require(suborigin, 0x10655, val.type() == abc::net::json::value_type::object,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Missing property \"col\".");
        require(suborigin, 0x10657, col_itr->second.type() == abc::net::json::value_type::number && static_cast<int>(col_itr->second.number()) == col_itr->second.number() && 0 <= col_itr->second.number() && col_itr->second.number() < col_count,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected 0 <= col <= 2.");

        mv.col = static_cast<count_t>(col_itr->second.number());
    }

    for (std::size_t game_i = 0; game_i < _games.size(); game_i++) {
        if (_games[game_i].id() == endpoint_game_id) {
            require(suborigin, 0x1081d, !_games[game_i].board().is_game_over(),
                abc::net::http::status_code::Conflict, abc::net::http::reason_phrase::Conflict, abc::net::http::content_type::text, "State error: The game with the supplied ID is over.");

            player_id_t player_id = _games[game_i].player_id(endpoint_player_id);
            require(suborigin, 0x10659, player_id != player_id::none,
               abc::net::http::status_code::Not_Found, abc::net::http::reason_phrase::Not_Found, abc::net::http::content_type::text, "A player with the supplied ID was not found.");

            require(suborigin, 0x1081e, _games[game_i].board().get_move(mv) == player_id::none,
                abc::net::http::status_code::Conflict, abc::net::http::reason_phrase::Conflict, abc::net::http::content_type::text, "State error: The square of the supplied move is occupied.");

            std::size_t move_i = _games[game_i].accept_move(player_id, mv);

            // 200
            std::stringbuf sb;
            abc::net::json::writer json(&sb, diag_base::log());

            abc::net::json::literal::object obj = {
                { "i", abc::net::json::literal::number(move_i) }
            };
            if (_games[game_i].board().is_game_over()) {
                obj["winner"] = abc::net::json::literal::number(_games[game_i].board().winner());
            }
            json.put_value(abc::net::json::value(std::move(obj)));

            base::send_simple_response(http, abc::net::http::status_code::OK, abc::net::http::reason_phrase::OK, abc::net::http::content_type::json, sb.str().c_str(), 0x1081f);

            diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10820, "End:");
            return;
        }
    }

    // 404
    throw_exception(suborigin, 0x1065f,
        abc::net::http::status_code::Not_Found, abc::net::http::reason_phrase::Not_Found, abc::net::http::content_type::text, "A game with the supplied ID was not found.");
}


inline void game_endpoint::get_moves(abc::net::http::server& http, const abc::net::http::request& request, endpoint_game_id_t endpoint_game_id, unsigned since_move_i) {
    constexpr const char* suborigin = "get_moves";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10661, "Begin: method=%s, game_id=%u, move_i=%u", request.method.c_str(), (unsigned)endpoint_game_id, since_move_i);

    require_method_get(suborigin, 0x10821, request);

    require(suborigin, 0x10662, endpoint_game_id > 0,
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Resource error: An invalid game ID was supplied.");

    for (std::size_t game_i = 0; game_i < _games.size(); game_i++) {
        if (_games[game_i].id() == endpoint_game_id) {
            // 200
            std::stringbuf sb;
            abc::net::json::writer json(&sb, diag_base::log());

            abc::net::json::literal::array moves;
            for (std::size_t move_i = since_move_i; move_i < _games[game_i].moves().size(); move_i++) {
                moves.push_back(abc::net::json::literal::object({
                    { "i",    abc::net::json::literal::number(move_i) },
                    { "move", abc::net::json::literal::object( {
                        { "row", abc::net::json::literal::number(_games[game_i].moves()[move_i].row) },
                        { "col", abc::net::json::literal::number(_games[game_i].moves()[move_i].col) },
                    }) },
                }));
            }

            abc::net::json::literal::object obj = {
                { "moves", std::move(moves) }
            };
            if (_games[game_i].board().is_game_over()) {
                obj["winner"] = abc::net::json::literal::number(_games[game_i].board().winner());
            }
            json.put_value(abc::net::json::value(std::move(obj)));

            base::send_simple_response(http, abc::net::http::status_code::OK, abc::net::http::reason_phrase::OK, abc::net::http::content_type::json, sb.str().c_str(), 0x10822);

            diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10823, "End:");
            return;
        }
    }

    // 404
    throw_exception(suborigin, 0x10666,
        abc::net::http::status_code::Not_Found, abc::net::http::reason_phrase::Not_Found, abc::net::http::content_type::text, "A game with the supplied ID was not found.");
}


inline void game_endpoint::process_shutdown(abc::net::http::server& http, const abc::net::http::request& request) {
    constexpr const char* suborigin = "process_shutdown";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10824, "Begin: method=%s", request.method.c_str());

    require_method_post(suborigin, 0x10825, request);

    base::set_shutdown_requested();

    // 200
    base::send_simple_response(http, abc::net::http::status_code::OK, abc::net::http::reason_phrase::OK, abc::net::http::content_type::text, "Server is shuting down...", 0x10668);
}


inline void game_endpoint::require_method_get(const char* suborigin, abc::diag::tag_t tag, const abc::net::http::request& request) {
    require(suborigin, tag, abc::ascii::are_equal_i(request.method.c_str(), abc::net::http::method::GET),
        abc::net::http::status_code::Method_Not_Allowed, abc::net::http::reason_phrase::Method_Not_Allowed, abc::net::http::content_type::text, "Method error: Expected 'GET'.");
}


inline void game_endpoint::require_method_post(const char* suborigin, abc::diag::tag_t tag, const abc::net::http::request& request) {
    require(suborigin, tag, abc::ascii::are_equal_i(request.method.c_str(), abc::net::http::method::POST),
        abc::net::http::status_code::Method_Not_Allowed, abc::net::http::reason_phrase::Method_Not_Allowed, abc::net::http::content_type::text, "Method error: Expected 'POST'.");
}


inline void game_endpoint::require_content_type_json(const char* suborigin, abc::diag::tag_t tag, const abc::net::http::request& request) {
    abc::net::http::headers::const_iterator content_type_itr = request.headers.find(abc::net::http::header::Content_Type);

    require(suborigin, tag, content_type_itr != request.headers.cend(),
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "The 'Content-Type' header was not supplied.");
    require(suborigin, tag, abc::ascii::are_equal_i(content_type_itr->second.c_str(), abc::net::http::content_type::json),
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "The value of header 'Content-Type' must be `application/json`.");
}
