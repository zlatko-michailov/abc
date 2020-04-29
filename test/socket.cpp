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


#include <thread>
#include <cctype>

#include "socket.h"


namespace abc { namespace test { namespace socket {

	bool test_udp_sync_socket(test_context<abc::test_log_ptr>& context) {
		const char server_port[] = "31234";
		const char request_content[] = "Some request content.";
		const char response_content[] = "The corresponding response content.";
		bool passed = true;

		abc::udp_socket server(context.log_ptr);
		server.bind(server_port);

		std::thread client_thread([&passed, &context, server_port, request_content, response_content] () {
			try {
				abc::udp_socket client(context.log_ptr);
				client.connect("localhost", server_port);

				std::uint16_t content_length = std::strlen(request_content);
				client.send(&content_length, sizeof(std::uint16_t));
				client.send(request_content, content_length);

				client.receive(&content_length, sizeof(std::uint16_t));

				char content[1024];
				client.receive(content, content_length);
				content[content_length] = '\0';

				passed = context.are_equal(content, response_content, 0x10028) && passed;
			}
			catch (const std::exception& ex) {
				context.log_ptr->push_back(abc::category::abc::base, abc::severity::important, 0x10029, "client: EXCEPTION: %s", ex.what());
			}
		});

		abc::socket::address client_address;
		std::uint16_t content_length;
		server.receive(&content_length, sizeof(std::uint16_t), &client_address);

		char content[1024];
		server.receive(content, content_length);
		content[content_length] = '\0';

		passed = context.are_equal(content, request_content, 0x1002a) && passed;

		server.connect(client_address);

		content_length = std::strlen(response_content);
		server.send(&content_length, sizeof(std::uint16_t));

		server.send(response_content, content_length);

		client_thread.join();
		return passed;
	}


	bool test_tcp_sync_socket(test_context<abc::test_log_ptr>& context) {
		const char server_port[] = "31234";
		const char request_content[] = "Some request content.";
		const char response_content[] = "The corresponding response content.";
		bool passed = true;

		abc::tcp_server_socket server(context.log_ptr);
		server.bind(server_port);
		server.listen(5);

		std::thread client_thread([&passed, &context, server_port, request_content, response_content] () {
			try {
				abc::tcp_client_socket client(context.log_ptr);
				client.connect("localhost", server_port);

				std::uint16_t content_length = std::strlen(request_content);
				client.send(&content_length, sizeof(std::uint16_t));
				client.send(request_content, content_length);

				client.receive(&content_length, sizeof(std::uint16_t));

				char content[1024];
				client.receive(content, content_length);
				content[content_length] = '\0';

				passed = context.are_equal(content, response_content, 0x1002b) && passed;
			}
			catch (const std::exception& ex) {
				context.log_ptr->push_back(abc::category::abc::base, abc::severity::important, 0x1002c, "client: EXCEPTION: %s", ex.what());
			}
		});

		abc::tcp_client_socket client = std::move(server.accept());

		std::uint16_t content_length;
		client.receive(&content_length, sizeof(std::uint16_t));

		char content[1024];
		client.receive(content, content_length);
		content[content_length] = '\0';

		passed = context.are_equal(content, request_content, 0x1002d) && passed;

		content_length = std::strlen(response_content);
		client.send(&content_length, sizeof(std::uint16_t));

		client.send(response_content, content_length);

		client_thread.join();
		return passed;
	}

}}}

