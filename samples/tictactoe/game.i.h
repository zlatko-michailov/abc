/*
MIT License

Copyright (c) 2018-2024 Zlatko Michailov 

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
#include <thread>
#include <mutex>
#include <vector>

#include "../../src/diag/diag_ready.h"
#include "../../src/net/endpoint.h"
#include "../../src/vmem/map.h"


// --------------------------------------------------------------


using board_state_t = std::uint32_t;
using score_calc_t = std::int16_t;
using score_t = std::int8_t;
using move_t = std::int32_t;

constexpr move_t row_count = 3;
constexpr move_t col_count = 3;

namespace score { //// TODO: enum?
    constexpr score_t none = -1;

    constexpr score_t max  = 20;
    constexpr score_t mid  = 10;
    constexpr score_t min  =  1;

    constexpr score_t win  =  3;
    constexpr score_t draw =  1;
    constexpr score_t loss = -1;
}


// IMPORTANT: Ensure a predictable layout of the data on disk!
#pragma pack(push, 1)

using scores = score_t[row_count][col_count];

struct start_page_layout {
    abc::vmem::map_state map_state;
};

#pragma pack(pop)


// --------------------------------------------------------------


// Max 8 pages = 32KB in memory.
////using vmem_pool = abc::vmem::pool;
////using vmem_page = abc::vmem_page<vmem_pool, log_ostream>;
using state_scores_map = abc::vmem::map<board_state_t, scores>;


struct vmem_bundle { //// TODO: knowledge_base?
    vmem_bundle(abc::vmem::pool_config&& pool_config, abc::diag::log_ostream* log);

    std::mutex              mutex;
    abc::vmem::pool         pool;
    abc::vmem::page         start_page;
    ::state_scores_map      state_scores_map;
    abc::diag::log_ostream* log;
};


// --------------------------------------------------------------


using player_id_t    = std::uint8_t;

namespace player_id { //// TODO: enum?
    constexpr player_id_t none = 0x0;
    constexpr player_id_t x    = 0x2; // First player
    constexpr player_id_t o    = 0x3; // Second player
    constexpr player_id_t mask = 0x3;
}


// --------------------------------------------------------------


using player_type_t    = std::uint8_t;

namespace player_type { //// TODO: enum?
    constexpr player_type_t none        = 0;
    constexpr player_type_t external    = 1;
    constexpr player_type_t slow_engine = 2;
    constexpr player_type_t fast_engine = 3;

    player_type_t from_text(const char* text);
}


struct player_types {
    player_type_t player_x_type;
    player_type_t player_o_type;
};


// --------------------------------------------------------------


struct move {
    move_t row;
    move_t col;

    bool is_valid() const;
};


// --------------------------------------------------------------


class board
    : public abc::diag::diag_ready<const char*> {

    using diag_base = abc::diag::diag_ready<const char*>;

public:
    board(abc::diag::log_ostream* log);

public:
    void reset();

public:
    void accept_move(const move& move);
    void undo_move(const move& move);

public:
    bool                 is_game_over() const;
    player_id_t          winner() const;
    player_id_t          get_move(const move& move) const;
    unsigned             move_count() const;
    bool                 has_move(player_id_t player_id, const move& move) const;
    player_id_t          current_player_id() const;
    board_state_t        state() const;

    static player_id_t   opponent(player_id_t player_id);
    
private:
    static board_state_t shift_up(player_id_t player_id, const move& move);
    player_id_t          shift_down(const move& move) const;

    void                 set_move(const move& move);
    void                 clear_move(const move& move);
    bool                 check_winner();
    void                 switch_current_player_id();

private:
    bool                 _is_game_over      = false;
    player_id_t          _winner            = player_id::none;
    player_id_t          _current_player_id = player_id::x;
    board_state_t        _board_state       = { };
    move_t               _move_count        = 0;
};


// --------------------------------------------------------------


class game;


class player_agent
    : public abc::diag::diag_ready<const char*> {

    using diag_base = abc::diag::diag_ready<const char*>;

public:
    player_agent(abc::diag::log_ostream* log);

public:
    void          reset(::game* game, player_id_t player_id, player_type_t player_type);
    void          make_move_async();
    void          learn();
    player_type_t player_type() const;

private:
    static void   make_move_proc(player_agent* this_ptr);
    void          make_move();

// Thinking slow
private:
    void          slow_make_move();
    int           slow_find_best_move_for(player_id_t player_id, move& best_move);

// Thinking fast
private:
    void          fast_make_move();
    move          fast_find_best_move();
    state_scores_map::iterator ensure_board_state_in_map(board_state_t board_state);

private:
    game*         _game        = nullptr;
    player_id_t   _player_id   = player_id::none;
    player_type_t _player_type = player_type::none;
    ::board       _temp_board;

public:
    static vmem_bundle* _vmem;
};


// --------------------------------------------------------------


class game
    : public abc::diag::diag_ready<const char*> {

    using diag_base = abc::diag::diag_ready<const char*>;

public:
    game(abc::diag::log_ostream* log);

protected:
    game(const char* origin, abc::diag::log_ostream* log);

public:
    void reset(const player_types& player_types);
    void start();
    void accept_move(player_id_t player_id, const move& move);

public:
    const ::board&           board() const;
    const std::vector<move>& moves() const;

private:
    ::board           _board;
    player_agent      _agent_x;
    player_agent      _agent_o;

    std::vector<move> _moves;
};


// --------------------------------------------------------------


using endpoint_player_id_t = std::uint32_t;


struct endpoint_player {
    endpoint_player_id_t endpoint_player_id = 0;
    bool                 is_claimed         = true;
};


// --------------------------------------------------------------


using endpoint_game_id_t = std::uint32_t;


class endpoint_game
    : public game {

    using base = game;
    using diag_base = abc::diag::diag_ready<const char*>;

public:
    endpoint_game(abc::diag::log_ostream* log);

public:
    void reset(endpoint_game_id_t endpoint_game_id,
               player_type_t player_x_type, endpoint_player_id_t endpoint_player_x_id,
               player_type_t player_o_type, endpoint_player_id_t endpoint_player_o_id);
    endpoint_player_id_t claim_player(unsigned player_i);
    bool is_player_claimed(unsigned player_i);

    endpoint_game_id_t id() const;
    player_id_t        player_id(endpoint_player_id_t endpoint_player_id) const;

private:
    endpoint_game_id_t _endpoint_game_id   = 0;
    endpoint_player    _endpoint_player_x;
    endpoint_player    _endpoint_player_o;
};


// --------------------------------------------------------------


class game_endpoint
    : public abc::net::http::endpoint {

    using base = abc::net::http::endpoint;
    using diag_base = abc::diag::diag_ready<const char*>;

public:
    game_endpoint(abc::net::http::endpoint_config&& config, abc::diag::log_ostream* log);

protected:
    virtual std::unique_ptr<abc::net::tcp_server_socket> create_server_socket() override;
    virtual void process_rest_request(abc::net::http::server& http, const abc::net::http::request& request) override;

private:
    void process_games(abc::net::http::server& http, const abc::net::http::request& request);
    void process_shutdown(abc::net::http::server& http, const abc::net::http::request& request);

    void create_game(abc::net::http::server& http, const abc::net::http::request& request);
    player_types get_player_types(abc::net::http::server& http, const abc::net::http::request& request);
    void claim_player(abc::net::http::server& http, const abc::net::http::request& request, endpoint_game_id_t endpoint_game_id, unsigned player_i);
    void accept_move(abc::net::http::server& http, const abc::net::http::request& request, endpoint_game_id_t endpoint_game_id, endpoint_player_id_t endpoint_player_id, const char* moves);
    void get_moves(abc::net::http::server& http, const abc::net::http::request& request, endpoint_game_id_t endpoint_game_id, unsigned since_move_i);

    void require_method_get(const char* suborigin, abc::diag::tag_t tag, const abc::net::http::request& request);
    void require_method_post(const char* suborigin, abc::diag::tag_t tag, const abc::net::http::request& request);
    void require_content_type_json(const char* suborigin, abc::diag::tag_t tag, const abc::net::http::request& request);

private:
    std::vector<endpoint_game> _games;
};
