/*
MIT License

Copyright (c) 2018-2020 Zlatko Michailov 

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


#pragma once

#include <cstdint>


namespace abc { namespace samples { namespace tictactoe {

	using player_t = std::uint8_t;

	namespace player {
		static constexpr player_t	empty		= 0;
		static constexpr player_t	me			= 1;
		static constexpr player_t	opponent	= 2;

		inline player_t is_empty(player_t player) noexcept {
			return player == empty;
		}

		inline player_t other(player_t player) noexcept {
			return (me + opponent) - player;
		}

		static constexpr char		symbol[] = { ' ', 'X', 'O' };
	}


	// --------------------------------------------------------------


	using rowcol_t = std::uint8_t;

	namespace rowcol {
		static constexpr rowcol_t	max_rows		= 3;
		static constexpr rowcol_t	max_cols		= 3;
	}

}}}
