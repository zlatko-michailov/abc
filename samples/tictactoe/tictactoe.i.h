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

#include "../../src/log.h"
#include "../../src/vmem.h"


namespace abc { namespace samples {

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


	using player_id_t	= std::uint8_t;

	namespace player_id {
		constexpr player_id_t	none		= 0x0;
		constexpr player_id_t	x			= 0x2;
		constexpr player_id_t	o			= 0x3;
	}


	// --------------------------------------------------------------


	using player_type_t	= std::uint8_t;

	namespace player_type {
		constexpr player_type_t	none		= 0;
		constexpr player_type_t	external	= 1;
		constexpr player_type_t	slow_engine	= 2;
		constexpr player_type_t	fast_engine	= 3;
	}


	// --------------------------------------------------------------


	struct move {
		int		row;
		int		col;

		bool	is_valid() const;
	};


	// --------------------------------------------------------------


	class board {
	public:
		////board() = default;

	public:
		bool				is_game_over() const;
		player_id_t			winner() const;
		bool				accept_move(const move& move);

	public:
		player_id_t			get_move(const move& move) const;
		void				set_move(const move& move);
		void				clear_move(const move& move);
		bool				has_move(player_id_t player_id, const move& move) const;
		bool				check_winner();
		player_id_t			current_player_id() const;
		void				switch_current_player_id();

		static player_id_t	opponent(player_id_t player_id);

	private:
		player_id_t			_winner				= player_id::none;
		player_id_t			_current_player_id	= player_id::x;
		board_state			_board_state	= { 0 };
	};


	// --------------------------------------------------------------


	class game;


	class player_agent {
	public:
		player_agent(game* game, player_id_t player_id, player_type_t player_type);

		void	make_move_async();

	private:
		static void	make_move_proc(player_agent* this_ptr);
		void	make_move();

	// Thinking slow
	private:
		void	slow_make_move();
		bool	slow_make_necessary_move();
		bool	slow_make_winning_move();
		bool	slow_make_defending_move();
		bool	slow_complete(player_id_t player_id);
		bool	slow_complete_horizontal(player_id_t player_id, int i);
		bool	slow_complete_vertical(player_id_t player_id, int j);
		bool	slow_complete_main_diagonal(player_id_t player_id);
		bool	slow_complete_reverse_diagonal(player_id_t player_id);
		bool	slow_make_best_move();

	// Thinking fast
	private:
		void	fast_make_move();

	private:
		game*			_game;
		player_id_t		_player_id;
		player_type_t	_player_type;
	};


	// --------------------------------------------------------------


	class game {
	public:
		game();
		game(player_type_t player_x_type, player_type_t player_o_type, log_ostream* log);

	public:
		void					start();
		bool					accept_move(player_id_t player_id, const move& move);

	public:
		const samples::board&	board() const;

	private:
		samples::board			_board;
		player_agent			_agent_x;
		player_agent			_agent_o;
		log_ostream*			_log;
	};


	// --------------------------------------------------------------

}}

