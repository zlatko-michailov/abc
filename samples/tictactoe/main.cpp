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


#include <iostream> ////
#include "board.h"



int main(int argc, const char* const argv[]) {
	abc::samples::tictactoe::board board;
	board.print();

	abc::samples::tictactoe::player_t player = abc::samples::tictactoe::player::me;

	for (int i = 0; i < 9; i++) {
		abc::samples::tictactoe::rowcol_t row;
		std::cout << "Enter row: ";
		std::cin >> row;
		row -= '0';

		abc::samples::tictactoe::rowcol_t col;
		std::cout << "Enter col: ";
		std::cin >> col;
		col -= '0';

		board.make_move(row, col, player);
		board.print();

		player = abc::samples::tictactoe::player::other(player);
	}

	return 0;
}
