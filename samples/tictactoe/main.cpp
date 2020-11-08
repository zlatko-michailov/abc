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
#include "../../src/socket.h"
#include "../../src/http.h"
#include "player.h"
#include "board.h"



int main() {
	{
		abc::tcp_server_socket<> listener;
		listener.bind("30303");
		listener.listen(2);
		abc::tcp_client_socket<> client = std::move(listener.accept());
		abc::socket_streambuf<abc::tcp_client_socket<>> sb(&client);
		abc::http_server_stream<> http(&sb);

		char buffer[abc::size::k1 + 1];
		http.get_method(buffer, sizeof(buffer));
		std::cout << "Method  =" << buffer << std::endl;
		http.get_resource(buffer, sizeof(buffer));
		std::cout << "Resource=" << buffer << std::endl;
		http.get_protocol(buffer, sizeof(buffer));
		std::cout << "Protocol=" << buffer << std::endl;

		http.put_protocol("HTTP/1.1");
		http.put_status_code("200");
		http.put_reason_phrase("OK");
		http.put_header_name("Content-Length");
		http.put_header_value("10");
		http.end_headers();
		http.put_body("1234567890");
		http.flush();
	}

	abc::samples::tictactoe::board board;
	board.print();

	abc::samples::tictactoe::player_t player = abc::samples::tictactoe::player::me;

	while (!board.is_game_over()) {
		bool made_move = false;
		while (!made_move) {
			std::cout << "Player " << abc::samples::tictactoe::player::symbol[player] << ": " << std::endl;

			abc::samples::tictactoe::rowcol_t row;
			std::cout << "Enter row: ";
			std::cin >> row;
			row -= '0';

			abc::samples::tictactoe::rowcol_t col;
			std::cout << "Enter col: ";
			std::cin >> col;
			col -= '0';

			made_move = board.make_move(row, col, player);
		}

		board.print();

		player = abc::samples::tictactoe::player::other(player);
	}

	return 0;
}
