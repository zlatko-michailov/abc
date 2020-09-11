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

#include <iostream> ////
#include "base.h"


namespace abc { namespace samples { namespace tictactoe {

	class board {
	public:
		board() noexcept;

	public:
		bool is_cell_empty(rowcol_t row, rowcol_t col) const noexcept;
		player_t get_move(rowcol_t row, rowcol_t col) const noexcept;
		bool make_move(rowcol_t row, rowcol_t col, player_t player) noexcept;

		bool is_winner(player_t player) const noexcept;

	public:
		void print() const noexcept; ////

	private:
		player_t _cells[rowcol::max_rows][rowcol::max_cols];
	};


	// --------------------------------------------------------------


	inline board::board() noexcept {
		for (rowcol_t row = 0; row < rowcol::max_rows; row++) {
			for (rowcol_t col = 0; col < rowcol::max_cols; col++) {
				_cells[row][col] = player::empty;
			}
		}
	}


	inline bool board::is_cell_empty(rowcol_t row, rowcol_t col) const noexcept {
		return player::is_empty(get_move(row, col));
	}


	inline player_t board::get_move(rowcol_t row, rowcol_t col) const noexcept {
		return _cells[row][col];
	}


	inline bool board::make_move(rowcol_t row, rowcol_t col, player_t player) noexcept {
		if (!is_cell_empty(row, col)) {
			return false;
		}

		_cells[row][col] = player;
		return true;
	}


	inline void board::print() const noexcept {
		for (rowcol_t row = 0; row < rowcol::max_rows; row++) {
			std::cout << "| ";

			for (rowcol_t col = 0; col < rowcol::max_cols; col++) {
				std::cout << player::symbol[get_move(row, col)] << " | ";
			}

			std::cout << std::endl;
		}

		std::cout << std::endl;
	}

}}}