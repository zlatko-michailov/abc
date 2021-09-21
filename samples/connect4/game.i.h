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
#include <thread>
#include <mutex>

#include "../../src/log.h"
#include "../../src/vmem.h"
#include "../../src/endpoint.h"


namespace abc { namespace samples {

	using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;
	using results_ostream = abc::log_ostream<abc::test_line_ostream<>, abc::log_filter>;
	using limits = abc::endpoint_limits;


	// --------------------------------------------------------------

	using count_t = int;

	constexpr count_t	row_count			= 6;
	constexpr count_t	col_count			= 7;

	constexpr count_t	col_size_bit_count	= 3;
	constexpr count_t	col_size_mask		= 0x7;
	constexpr count_t	sizes_bit_count		= col_count * col_size_bit_count;		// 21
	constexpr count_t	move_bit_count		= 1;
	constexpr count_t	move_mask			= 0x1;
	constexpr count_t	col_bit_count		= row_count * move_bit_count;
	constexpr count_t	moves_bit_count		= col_count * col_bit_count;			// 42
	constexpr count_t	board_bit_count		= sizes_bit_count + moves_bit_count;	// 63
	constexpr count_t	sizes_pos			= 0;
	constexpr count_t	moves_pos			= sizes_bit_count;


	using board_state_t = std::uint64_t;

	constexpr board_state_t	board_state_0	= 0;
	constexpr board_state_t	board_state_1	= 1;


	using score_calc_t = std::int16_t;

	using score_t = std::int8_t;

	namespace score {
		constexpr score_t none	= -1;

		constexpr score_t max	= 20;
		constexpr score_t mid	= 10;
		constexpr score_t min	=  1;

		constexpr score_t win	=  3;
		constexpr score_t draw	=  1;
		constexpr score_t loss	= -1;
	}


	// IMPORTANT: Ensure a predictable layout of the data on disk!
	#pragma pack(push, 1)

	using scores = score_t[col_count];

	struct start_page_layout {
		vmem_map_state	map_state;
	};

	#pragma pack(pop)


	// --------------------------------------------------------------


	// Max 8 pages = 32KB in memory.
	using vmem_pool = abc::vmem_pool<8, log_ostream>;
	using vmem_page = abc::vmem_page<vmem_pool, log_ostream>;
	using vmem_map = abc::vmem_map<board_state_t, scores, vmem_pool, log_ostream>;


	struct vmem_bundle {
		vmem_bundle(const char* path, log_ostream* log);

		std::mutex		mutex;
		vmem_pool		pool;
		vmem_page		start_page;
		vmem_map		state_scores_map;
		log_ostream*	log;
	};


	// --------------------------------------------------------------


	using player_id_t	= std::uint8_t;

	namespace player_id {
		constexpr player_id_t	x			= 0x0;
		constexpr player_id_t	o			= 0x1;
		constexpr player_id_t	mask		= 0x1;
		constexpr player_id_t	none		= 0x2;
	}


	// --------------------------------------------------------------


	using player_type_t	= std::uint8_t;

	namespace player_type {
		constexpr player_type_t	none		= 0;
		constexpr player_type_t	external	= 1;
		constexpr player_type_t	slow_engine	= 2;
		constexpr player_type_t	fast_engine	= 3;

		player_type_t	from_text(const char* text);
	}


	// --------------------------------------------------------------


	struct move {
		count_t	row;
		count_t	col;

		bool	is_valid() const;
	};


	// --------------------------------------------------------------


	class board {
	public:
		void				reset(log_ostream* log);

	public:
		bool				accept_move(const move& move);
		bool				undo_move(const move& move);

	public:
		bool				is_game_over() const;
		player_id_t			winner() const;
		player_id_t			get_move(const move& move) const;
		unsigned			move_count() const;
		bool				has_move(player_id_t player_id, const move& move) const;
		player_id_t			current_player_id() const;
		board_state_t		state() const;
		count_t				col_size(count_t col) const;

		static player_id_t	opponent(player_id_t player_id);
		
	private:
		void				set_move(const move& move);
		void				clear_move(const move& move);
		bool				check_winner(const move& move);
		void				switch_current_player_id();

		count_t				inc_col_size(count_t col);
		count_t				dec_col_size(count_t col);
		count_t				col_pos(count_t col) const;
		player_id_t			get_move_bits(const move& move) const;
		void				set_move_bits(const move& move, count_t bits);
		void				clear_move_bits(const move& move);
		count_t				move_pos(const move& move) const;

	private:
		bool				_is_game_over		= false;
		player_id_t			_winner				= player_id::none;
		player_id_t			_current_player_id	= player_id::x;
		board_state_t		_board_state		= { 0 };
		unsigned			_move_count			= 0;
		log_ostream*		_log				= nullptr;
	};


	// --------------------------------------------------------------


	class game;


	class player_agent {
	public:
		void				reset(game* game, player_id_t player_id, player_type_t player_type, log_ostream* log);
		void				make_move_async();
		void				learn();
		player_type_t		player_type() const;

	private:
		static void			make_move_proc(player_agent* this_ptr);
		void				make_move();

	// Thinking slow
	private:
		void				slow_make_move();
		bool				slow_make_first_move(move& best_move);
		int					slow_choose_max_depth() const;
		int					slow_find_best_move_for(player_id_t player_id, move& best_move, int max_depth, int depth);

	// Thinking fast
	private:
		void				fast_make_move();
		move				fast_find_best_move();
		vmem_map::iterator	ensure_board_state_in_map(board_state_t board_state);

	private:
		game*				_game			= nullptr;
		player_id_t			_player_id		= player_id::none;
		player_type_t		_player_type	= player_type::none;
		board				_temp_board;
		log_ostream*		_log			= nullptr;

	public:
		static vmem_bundle*	_vmem;
	};


	// --------------------------------------------------------------


	class game {
	public:
		static constexpr std::size_t max_move_count = row_count * col_count;

	public:
		void					reset(player_type_t player_x_type, player_type_t player_o_type, log_ostream* log);
		void					start();
		bool					accept_move(player_id_t player_id, const move& move);

	public:
		const samples::board&	board() const;
		const move*				moves() const;

	private:
		samples::board			_board;
		player_agent			_agent_x;
		player_agent			_agent_o;
		log_ostream*			_log			= nullptr;

		move					_moves[max_move_count];
	};


	// --------------------------------------------------------------


	using endpoint_player_id_t = std::uint32_t;


	struct endpoint_player {
		endpoint_player_id_t	endpoint_player_id		= 0;
		bool					is_claimed				= true;
	};


	// --------------------------------------------------------------


	using endpoint_game_id_t = std::uint32_t;


	class endpoint_game: public game {
		using base = game;

	public:
		void					reset(endpoint_game_id_t endpoint_game_id,
									 player_type_t player_x_type, endpoint_player_id_t endpoint_player_x_id,
									 player_type_t player_o_type, endpoint_player_id_t endpoint_player_o_id,
									 log_ostream* log);
		bool					claim_player(unsigned player_i, endpoint_player_id_t& endpoint_player_id);

		endpoint_game_id_t		id() const;
		player_id_t				player_id(endpoint_player_id_t endpoint_player_id) const;

	private:
		endpoint_game_id_t		_endpoint_game_id		= 0;
		endpoint_player			_endpoint_player_x;
		endpoint_player			_endpoint_player_o;
	};


	// --------------------------------------------------------------


	template <typename Limits, typename Log>
	class game_endpoint : public endpoint<Limits, Log> {
		using base = endpoint<Limits, Log>;

		static constexpr std::size_t max_game_count = 1;

	public:
		game_endpoint(endpoint_config* config, Log* log);

	protected:
		virtual void	process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) override;

	private:
		void			process_games(abc::http_server_stream<Log>& http, const char* method, const char* resource);
		void			process_shutdown(abc::http_server_stream<Log>& http, const char* method);

		bool			create_game(abc::http_server_stream<Log>& http, const char* method);
		bool			get_player_types(abc::http_server_stream<Log>& http, const char* method, player_type_t& player_x_type, player_type_t& player_o_type);
		bool			claim_player(abc::http_server_stream<Log>& http, const char* method, endpoint_game_id_t endpoint_game_id, unsigned player_i);
		bool			accept_move(abc::http_server_stream<Log>& http, const char* method, endpoint_game_id_t endpoint_game_id, endpoint_player_id_t endpoint_player_id, const char* moves);
		bool			get_moves(abc::http_server_stream<Log>& http, const char* method, endpoint_game_id_t endpoint_game_id, unsigned since_move_i);

		bool			verify_method_get(abc::http_server_stream<Log>& http, const char* method);
		bool			verify_method_post(abc::http_server_stream<Log>& http, const char* method);
		bool			verify_header_json(abc::http_server_stream<Log>& http);

	private:
		std::size_t				_game_count 		= 0;
		endpoint_game			_games[max_game_count];
	};


	// --------------------------------------------------------------

}}

