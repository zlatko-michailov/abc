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
#include "../../src/endpoint.h"


namespace abc { namespace samples {

	using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;
	using results_ostream = abc::log_ostream<abc::test_line_ostream<>, abc::log_filter>;
	using limits = abc::endpoint_limits;

	// Max 8 pages = 32KB in memory.
	using vmem_pool = abc::vmem_pool<8, log_ostream>;
	using vmem_page = abc::vmem_page<vmem_pool, log_ostream>;


	constexpr std::size_t size = 3;

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
		constexpr player_id_t	mask		= 0x3;
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
		int		row;
		int		col;

		bool	is_valid() const;
	};


	// --------------------------------------------------------------


	class board {
	public:
		////board() = default;

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
		board_state			state() const;

		static player_id_t	opponent(player_id_t player_id);
		
	private:
		static board_state	shift_up(player_id_t player_id, const move& move);
		player_id_t			shift_down(const move& move) const;

		void				set_move(const move& move);
		void				clear_move(const move& move);
		bool				check_winner();
		void				switch_current_player_id();

	private:
		bool				_is_game_over		= false;
		player_id_t			_winner				= player_id::none;
		player_id_t			_current_player_id	= player_id::x;
		board_state			_board_state		= { 0 };
		unsigned			_move_count			= 0;
	};


	// --------------------------------------------------------------


	class game;


	class player_agent {
	public:
		////player_agent() = default;

	public:
		void	reset(game* game, player_id_t player_id, player_type_t player_type, log_ostream* log);
		void	make_move_async();

	private:
		static void	make_move_proc(player_agent* this_ptr);
		void	make_move();

	// Thinking slow
	private:
		void	slow_make_move();
		int		slow_find_best_move_for(player_id_t player_id, move& best_move);

	// Thinking fast
	private:
		void	fast_make_move();

	private:
		game*				_game			= nullptr;
		player_id_t			_player_id		= player_id::none;
		player_type_t		_player_type	= player_type::none;
		board				_temp_board;
		log_ostream*		_log			= nullptr;
	};


	// --------------------------------------------------------------


	class game {
	public:
		////game() = default;

	public:
		void					reset(player_type_t player_x_type, player_type_t player_o_type, log_ostream* log);
		void					start();
		bool					accept_move(player_id_t player_id, const move& move);

	public:
		const samples::board&	board() const;

	private:
		samples::board			_board;
		player_agent			_agent_x;
		player_agent			_agent_o;
		log_ostream*			_log		= nullptr;
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

		static constexpr std::size_t max_move_count = size * size;

	public:
		void					reset(endpoint_game_id_t endpoint_game_id,
									 player_type_t player_x_type, endpoint_player_id_t endpoint_player_x_id,
									 player_type_t player_o_type, endpoint_player_id_t endpoint_player_o_id,
									 log_ostream* log);
		bool					claim_player(unsigned player_i, endpoint_player_id_t& endpoint_player_id);

		endpoint_game_id_t		id() const;
		bool					is_done() const;

	private:
		endpoint_game_id_t		_endpoint_game_id		= 0;
		endpoint_player			_endpoint_player_x;
		endpoint_player			_endpoint_player_o;
		std::size_t				_move_count				= 0;
		move					_moves[max_move_count];

	};


	// --------------------------------------------------------------


	template <typename Limits, typename Log>
	class tictactoe_endpoint : public endpoint<Limits, Log> {
		using base = endpoint<Limits, Log>;

		static constexpr std::size_t max_game_count = 1;

	public:
		tictactoe_endpoint(endpoint_config* config, Log* log);

	protected:
		virtual void	process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) override;

	private:
		void			process_games(abc::http_server_stream<Log>& http, const char* method, const char* resource);
		void			process_shutdown(abc::http_server_stream<Log>& http, const char* method);

		bool			create_game(abc::http_server_stream<Log>& http, const char* method);
		bool			get_player_types(abc::http_server_stream<Log>& http, const char* method, player_type_t& player_x_type, player_type_t& player_o_type);
		bool			claim_player(abc::http_server_stream<Log>& http, const char* method, endpoint_game_id_t endpoint_game_id, unsigned player_i);

		bool			verify_method_post(abc::http_server_stream<Log>& http, const char* method);
		bool			verify_header_json(abc::http_server_stream<Log>& http);

	private:
		std::size_t				_game_count 		= 0;
		endpoint_game			_games[max_game_count];
	};


	// --------------------------------------------------------------

}}

