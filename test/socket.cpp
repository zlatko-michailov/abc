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

	bool test_udp_sync_socket(test_context<abc::test_log>& context) {
		const char server_port[] = "31234";
		const char request_content[] = "Some request content.";
		const char response_content[] = "The corresponding response content.";
		bool passed = true;

			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x1a, "server: begin");
		abc::udp_socket server;
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x1b, "server: created");
		server.bind(server_port);
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x1c, "server: bound");

		std::thread client_thread([&passed, &context, server_port, request_content, response_content] () {
			try {
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x1d, "client: begin");
				abc::udp_socket client;
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x1e, "client: created");
				client.connect("localhost", server_port);
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x1f, "client: connected");

				std::uint16_t content_length = std::strlen(request_content);
				client.send(&content_length, sizeof(std::uint16_t));
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x20, "client: sent 2 bytes: %d", content_length);
				client.send(request_content, content_length);
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x21, "client: sent %d bytes: %s", content_length, request_content);

					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x22, "client: will receive 2 bytes");
				client.receive(&content_length, sizeof(std::uint16_t));
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x23, "client: received 2 bytes: %d", content_length);

				char content[1024];
				client.receive(content, content_length);
				content[content_length] = '\0';
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x24, "client: received %d bytes: %s", content_length, content);

				passed = context.are_equal(content, response_content,0x25) && passed;
			}
			catch (const std::exception& ex) {
				context.log.push_back(abc::category::abc::base, abc::severity::important, 0x26, "client: EXCEPTION: %s", ex.what());
			}

			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x27, "client: end");
		});

		abc::socket::address client_address;
		std::uint16_t content_length;
		server.receive(&content_length, sizeof(std::uint16_t), &client_address);
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x28, "server: received 2 bytes: %d", content_length);
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x29, "server: client address: %d.%d.%d.%d port: %d",
				client_address.value.sa_data[2], client_address.value.sa_data[3], client_address.value.sa_data[4], client_address.value.sa_data[5],
				::ntohs(*(std::uint16_t*)client_address.value.sa_data));

		char content[1024];
		server.receive(content, content_length);
		content[content_length] = '\0';
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x2a, "server: received %d bytes: %s", content_length, content);

		passed = context.are_equal(content, request_content, 0x2b) && passed;

		server.connect(client_address);
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x2c, "server: connected");

		content_length = std::strlen(response_content);
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x2d, "server: will send 2 bytes: %d", content_length);
		server.send(&content_length, sizeof(std::uint16_t));
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x2e, "server: sent 2 bytes: %d", content_length);

		server.send(response_content, content_length);
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x2f, "server: sent %d bytes", content_length);

			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x30, "server: wait for client");
		client_thread.join();
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x31, "server: end");
		return passed;
	}


	bool test_tcp_sync_socket(test_context<abc::test_log>& context) {
		const char server_port[] = "31234";
		const char request_content[] = "Some request content.";
		const char response_content[] = "The corresponding response content.";
		bool passed = true;

			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x32, "server: begin");
		abc::tcp_server_socket server;
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x33, "server: created");
		server.bind(server_port);
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x34, "server: bound");
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x35, "server: will listen");
		server.listen(5);
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x36, "server: listening");

		std::thread client_thread([&passed, &context, server_port, request_content, response_content] () {
			try {
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x37, "client: begin");
				abc::tcp_client_socket client;
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x38, "client: created");
				client.connect("localhost", server_port);
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x39, "client: connected");

				std::uint16_t content_length = std::strlen(request_content);
				client.send(&content_length, sizeof(std::uint16_t));
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x3a, "client: sent 2 bytes: %d", content_length);
				client.send(request_content, content_length);
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x3b, "client: sent %d bytes: %s", content_length, request_content);

					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x3c, "client: will receive 2 bytes");
				client.receive(&content_length, sizeof(std::uint16_t));
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x3d, "client: received 2 bytes: %d", content_length);

				char content[1024];
				client.receive(content, content_length);
				content[content_length] = '\0';
					context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x3e, "client: received %d bytes: %s", content_length, content);

				passed = context.are_equal(content, response_content, 0x3f) && passed;
			}
			catch (const std::exception& ex) {
				context.log.push_back(abc::category::abc::base, abc::severity::important, 0x40, "client: EXCEPTION: %s", ex.what());
			}

			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x41, "client: end");
		});

			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x42, "server: waiting for connection");
		abc::tcp_client_socket client = std::move(server.accept());
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x43, "server: accepted");

		std::uint16_t content_length;
		client.receive(&content_length, sizeof(std::uint16_t));
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x44, "server: received 2 bytes: %d", content_length);

		char content[1024];
		client.receive(content, content_length);
		content[content_length] = '\0';
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x45, "server: received %d bytes: %s", content_length, content);

		passed = context.are_equal(content, request_content, 0x46) && passed;

		content_length = std::strlen(response_content);
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x47, "server: will send 2 bytes: %d", content_length);
		client.send(&content_length, sizeof(std::uint16_t));
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x48, "server: sent 2 bytes: %d", content_length);

		client.send(response_content, content_length);
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x49, "server: sent %d bytes", content_length);

			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x4a, "server: wait for client");
		client_thread.join();
			context.log.push_back(abc::category::abc::base, abc::severity::debug, 0x4b, "server: end");
		return passed;
	}

}}}

