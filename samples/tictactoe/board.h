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

		bool is_game_over() const noexcept;

	public:
		void print() const noexcept; ////

	private:
		player_t	_cells[rowcol::max_rows][rowcol::max_cols];

		rowcol_t	_last_row;
		rowcol_t	_last_col;
		player_t	_last_player;
	};


	// --------------------------------------------------------------


	inline board::board() noexcept
		: _last_row(0)
		, _last_col(0)
		, _last_player(player::empty) {
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

		_last_row = row;
		_last_col = col;
		_last_player = player;

		return true;
	}


	inline bool board::is_game_over() const noexcept {
		if (player::is_empty(_last_player)) {
			return false;
		}

		// Check the row
		{
			rowcol_t count = 0;
			for (rowcol_t col = 0; col < rowcol::max_cols; col++) {
				if (get_move(_last_row, col) != _last_player) {
					break;
				}

				count++;
			}

			if (count == rowcol::max_cols) {
				return true;
			}
		}

		// Check the col
		{
			rowcol_t count = 0;
			for (rowcol_t row = 0; row < rowcol::max_rows; row++) {
				if (get_move(row, _last_col) != _last_player) {
					break;
				}

				count++;
			}

			if (count == rowcol::max_rows) {
				return true;
			}
		}

		// Check the main diagonal if applicable
		if (_last_row == _last_col) {
			rowcol_t count = 0;
			for (rowcol_t row = 0; row < rowcol::max_rows; row++) {
				if (get_move(row, row) != _last_player) {
					break;
				}

				count++;
			}

			if (count == rowcol::max_rows) {
				return true;
			}
		}

		// Check the second diagonal if applicable
		if (_last_row + _last_col == rowcol::max_rows - 1) {
			rowcol_t count = 0;
			for (rowcol_t row = 0; row < rowcol::max_rows; row++) {
				if (get_move(row, rowcol::max_rows - 1 - row) != _last_player) {
					break;
				}

				count++;
			}

			if (count == rowcol::max_rows) {
				return true;
			}
		}

		return false;
	}


	inline void board::print() const noexcept {
		for (rowcol_t row = 0; row < rowcol::max_rows; row++) {
			std::cout << "| ";

			for (rowcol_t col = 0; col < rowcol::max_cols; col++) {
				std::cout << player::symbol[get_move(row, col)] << " | ";
			}

			std::cout << std::endl;
		}

		if (is_game_over()) {
			std::cout << "Player " << player::symbol[_last_player] << " wins!" << std::endl;
		}

		std::cout << std::endl;
	}

}}}