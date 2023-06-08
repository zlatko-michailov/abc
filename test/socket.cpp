/*
MIT License

Copyright (c) 2018-2023 Zlatko Michailov 

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


#include <cctype>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "inc/http.h"
#include "inc/json.h"
#include "inc/socket.h"
#include "inc/heap.h"


namespace abc { namespace test { namespace socket {

	static constexpr std::size_t max_path_size = abc::size::k1;
	static constexpr const char cert_filename[] = "cert.pem";
	static constexpr const char pkey_filename[] = "pkey.pem";
	static constexpr const char pkey_password[] = "server";
	static constexpr bool verify_client = false;
	static constexpr bool verify_server = false;


	bool make_filepath(test_context<abc::test::log>& context, char* filepath, std::size_t filepath_size, const char* process_path, const char* filename) {
		context.log->put_any(abc::category::abc::base, abc::severity::optional, __TAG__, "process_path='%s'", process_path);

		std::size_t filename_len = std::strlen(filename); 
		const char* process_last_separator = std::strrchr(process_path, '/');
		std::size_t process_dir_len = 0;

		if (process_last_separator != nullptr) {
			process_dir_len = process_last_separator - process_path;
			std::size_t filepath_len = process_dir_len + 1 + filename_len;

			if (filepath_len >= filepath_size) {
				context.log->put_any(abc::category::abc::base, abc::severity::important, __TAG__, "filepath_len=%zu >= filepath_size=%zu", filepath_len, filepath_size);

				return false;
			}

			std::strncpy(filepath, process_path, process_dir_len + 1);
		}

		std::strcpy(filepath + process_dir_len + 1, filename);
		context.log->put_any(abc::category::abc::base, abc::severity::optional, __TAG__, "filepath='%s'", filepath);

		return true;
	}


	bool test_udp_socket(test_context<abc::test::log>& context) {
		const char server_port[] = "31234";
		const char request_content[] = "Some request content.";
		const char response_content[] = "The corresponding response content.";
		bool passed = true;

		abc::udp_socket<abc::test::log> server(abc::socket::family::ipv4, context.log);
		server.bind(server_port);

		std::thread client_thread([&passed, &context, server_port, request_content, response_content] () {
			try {
				abc::udp_socket<abc::test::log> client(abc::socket::family::ipv4, context.log);
				client.connect("localhost", server_port);

				std::uint16_t content_length = std::strlen(request_content);
				client.send(&content_length, sizeof(std::uint16_t));
				client.send(request_content, content_length);

				client.receive(&content_length, sizeof(std::uint16_t));

				char content[abc::size::k1];
				client.receive(content, content_length);
				content[content_length] = '\0';

				passed = context.are_equal(content, response_content, 0x10028) && passed;
			}
			catch (const std::exception& ex) {
				context.log->put_any(abc::category::abc::base, abc::severity::important, 0x10029, "client: EXCEPTION: %s", ex.what());
			}
		});

#if defined(__ABC__LINUX)
		abc::test::heap::counter_t closure_allocation_count = 1;
#elif defined(__ABC__MACOS)
		abc::test::heap::counter_t closure_allocation_count = 3;
#else
		abc::test::heap::counter_t closure_allocation_count = 1;
#endif

		passed = abc::test::heap::ignore_heap_allocations(abc::test::heap::instance_unaligned_throw_count, closure_allocation_count, context, 0x100e6) && passed; // Lambda closure

		abc::socket::address client_address;
		std::uint16_t content_length;
		server.receive(&content_length, sizeof(std::uint16_t), &client_address);

		char content[abc::size::k1];
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


	template <typename ServerSocket, typename ClientSocket>
	bool tcp_socket(test_context<abc::test::log>& context, ServerSocket& server, ClientSocket& client) {
		const char server_port[] = "31235";
		const char request_content[] = "Some request content.";
		const char response_content[] = "The corresponding response content.";
		bool passed = true;

		server.bind(server_port);
		server.listen(5);

		std::thread client_thread([&client, &passed, &context, server_port, request_content, response_content] () {
			try {
				client.connect("localhost", server_port);

				std::uint16_t content_length = std::strlen(request_content);
				client.send(&content_length, sizeof(std::uint16_t));
				client.send(request_content, content_length);

				client.receive(&content_length, sizeof(std::uint16_t));

				char content[abc::size::k1];
				client.receive(content, content_length);
				content[content_length] = '\0';

				passed = context.are_equal(content, response_content, 0x1002b) && passed;
			}
			catch (const std::exception& ex) {
				context.log->put_any(abc::category::abc::base, abc::severity::important, 0x1002c, "client: EXCEPTION: %s", ex.what());
			}
		});

#if defined(__ABC__LINUX)
		abc::test::heap::counter_t closure_allocation_count = 1;
#elif defined(__ABC__MACOS)
		abc::test::heap::counter_t closure_allocation_count = 3;
#else
		abc::test::heap::counter_t closure_allocation_count = 1;
#endif

		passed = abc::test::heap::ignore_heap_allocations(abc::test::heap::instance_unaligned_throw_count, closure_allocation_count, context, 0x100e7) && passed; // Lambda closure

		ClientSocket connection = server.accept();

		std::uint16_t content_length;
		connection.receive(&content_length, sizeof(std::uint16_t));

		char content[abc::size::k1];
		connection.receive(content, content_length);
		content[content_length] = '\0';

		passed = context.are_equal(content, request_content, 0x1002d) && passed;

		content_length = std::strlen(response_content);
		connection.send(&content_length, sizeof(std::uint16_t));

		connection.send(response_content, content_length);

		client_thread.join();
		return passed;
	}
	
	bool test_tcp_socket(test_context<abc::test::log>& context) {
		abc::tcp_server_socket<abc::test::log> server(abc::socket::family::ipv4, context.log);
		abc::tcp_client_socket<abc::test::log> client(abc::socket::family::ipv4, context.log);

		return tcp_socket(context, server, client);
	}


	bool test_openssl_tcp_socket(test_context<abc::test::log>& context) {
		bool passed = true;

#ifdef __ABC__OPENSSL
		char cert_path[max_path_size];
		passed = passed && make_filepath(context, cert_path, max_path_size, context.process_path, cert_filename);

		char pkey_path[max_path_size];
		passed = passed && make_filepath(context, pkey_path, max_path_size, context.process_path, pkey_filename);

		if (passed) {
			abc::openssl_tcp_server_socket<abc::test::log> server(cert_path, pkey_path, pkey_password, verify_client, abc::socket::family::ipv4, context.log);
			abc::openssl_tcp_client_socket<abc::test::log> client(verify_server, abc::socket::family::ipv4, context.log);

			passed = passed && tcp_socket(context, server, client);
		}
#endif
		return passed;
	}


	template <typename ServerSocket, typename ClientSocket>
	bool tcp_socket_stream_move(test_context<abc::test::log>& context, ServerSocket& server, ClientSocket& client1) {
		const char server_port[] = "31236";
		const char request_content[] = "Some request line.";
		const char response_content[] = "The corresponding response line.";
		bool passed = true;

		server.bind(server_port);
		server.listen(5);

		std::thread client_thread([&passed, &context, &client1, server_port, request_content, response_content] () {
			try {
				client1.connect("localhost", server_port);

				ClientSocket client2(std::move(client1));

				abc::socket_streambuf<ClientSocket, abc::test::log> sb1(&client2, context.log);
				std::ostream client_out(&sb1);

				client_out << request_content << "\n";
				client_out.flush();

				abc::socket_streambuf<ClientSocket, abc::test::log> sb2(std::move(sb1));
				std::istream client_in(&sb2);

				char content[abc::size::k1];
				client_in.getline(content, sizeof(content) - 1);
				passed = context.are_equal(content, response_content, 0x10037) && passed;
			}
			catch (const std::exception& ex) {
				context.log->put_any(abc::category::abc::base, abc::severity::important, 0x10038, "client: EXCEPTION: %s", ex.what());
			}
		});

#if defined(__ABC__LINUX)
		abc::test::heap::counter_t closure_allocation_count = 1;
#elif defined(__ABC__MACOS)
		abc::test::heap::counter_t closure_allocation_count = 3;
#else
		abc::test::heap::counter_t closure_allocation_count = 1;
#endif

		passed = abc::test::heap::ignore_heap_allocations(abc::test::heap::instance_unaligned_throw_count, closure_allocation_count, context, 0x100e8) && passed; // Lambda closure

		ClientSocket connection = server.accept();

		abc::socket_streambuf<ClientSocket, abc::test::log> sb(&connection, context.log);
		std::istream connection_in(&sb);
		std::ostream connection_out(&sb);

		char content[abc::size::k1];
		connection_in.getline(content, sizeof(content) - 1);
		passed = context.are_equal(content, request_content, 0x10039) && passed;

		connection_out << response_content << "\n";
		connection_out.flush();

		client_thread.join();
		return passed;
	}


	bool test_tcp_socket_stream_move(test_context<abc::test::log>& context) {
		abc::tcp_server_socket<abc::test::log> server(abc::socket::family::ipv4, context.log);
		abc::tcp_client_socket<abc::test::log> client(abc::socket::family::ipv4, context.log);

		return tcp_socket_stream_move(context, server, client);
	}


	bool test_openssl_tcp_socket_stream_move(test_context<abc::test::log>& context) {
		bool passed = true;

#ifdef __ABC__OPENSSL
		char cert_path[max_path_size];
		passed = passed && make_filepath(context, cert_path, max_path_size, context.process_path, cert_filename);

		char pkey_path[max_path_size];
		passed = passed && make_filepath(context, pkey_path, max_path_size, context.process_path, pkey_filename);

		if (passed) {
			abc::openssl_tcp_server_socket<abc::test::log> server(cert_path, pkey_path, pkey_password, verify_client, abc::socket::family::ipv4, context.log);
			abc::openssl_tcp_client_socket<abc::test::log> client(verify_server, abc::socket::family::ipv4, context.log);

			passed = passed && tcp_socket_stream_move(context, server, client);
		}
#endif
		return passed;
	}


	static constexpr const char server_port[] = "31237";
	static constexpr const char protocol[] = "HTTP/1.1";
	static constexpr const char request_method[] = "POST";
	static constexpr const char request_resource[] = "/scope/v1.0/api";
	static constexpr const char request_header_name[] = "Request-Header-Name";
	static constexpr const char request_header_value[] = "Request-Header-Value";
	static constexpr const char response_status_code[] = "200";
	static constexpr const char response_reason_phrase[] = "OK";
	static constexpr const char response_header_name[] = "Response-Header-Name";
	static constexpr const char response_header_value[] = "Response-Header-Value";

	template <typename ClientSocket>
	void http_json_stream_client(bool& passed, test_context<abc::test::log>& context, ClientSocket& client) {
		try {
			client.connect("localhost", server_port);

			abc::socket_streambuf<ClientSocket, abc::test::log> sb(&client, context.log);
			abc::http_client_stream<abc::test::log> http(&sb, context.log);

			// Send request
			{
				http.put_method(request_method);
				http.put_resource(request_resource);
				http.put_protocol(protocol);
				http.put_header_name(request_header_name);
				http.put_header_value(request_header_value);
				http.end_headers();

				// Body (json)
				abc::json_ostream<abc::size::_64, abc::test::log> json(&sb, context.log);
				json.put_begin_object();
					json.put_property("param");
					json.put_string("foo");
				json.put_end_object();
			}

			// Receive response
			{
				char buffer[abc::size::k1];

				http.get_protocol(buffer, sizeof(buffer));
				passed = context.are_equal(buffer, protocol, 0x100e9) && passed;

				http.get_status_code(buffer, sizeof(buffer));
				passed = context.are_equal(buffer, response_status_code, 0x100ea) && passed;

				http.get_reason_phrase(buffer, sizeof(buffer));
				passed = context.are_equal(buffer, response_reason_phrase, 0x100eb) && passed;

				http.get_header_name(buffer, sizeof(buffer));
				passed = context.are_equal(buffer, response_header_name, 0x100ec) && passed;

				http.get_header_value(buffer, sizeof(buffer));
				passed = context.are_equal(buffer, response_header_value, 0x100ed) && passed;

				http.get_header_name(buffer, sizeof(buffer));
				passed = context.are_equal(buffer, "", 0x100ee) && passed;

				// Body (json)
				abc::json_istream<abc::size::_64, abc::test::log> json(&sb, context.log);
				abc::json::token_t* token = (abc::json::token_t*)buffer;

				json.get_token(token, sizeof(buffer));
				passed = context.are_equal(token->item, abc::json::item::begin_object, 0x102a0, "%u") && passed;

					json.get_token(token, sizeof(buffer));
					passed = context.are_equal(token->item, abc::json::item::property, 0x102a1, "%u") && passed;
					passed = context.are_equal(token->value.property, "n", 0x102a2) && passed;

					json.get_token(token, sizeof(buffer));
					passed = context.are_equal(token->item, abc::json::item::number, 0x102a3, "%u") && passed;
					passed = context.are_equal(token->value.number, 42.0, 0x102a4, "%g") && passed;

					json.get_token(token, sizeof(buffer));
					passed = context.are_equal(token->item, abc::json::item::property, 0x102a5, "%u") && passed;
					passed = context.are_equal(token->value.property, "s", 0x102a6) && passed;

					json.get_token(token, sizeof(buffer));
					passed = context.are_equal(token->item, abc::json::item::string, 0x102a7, "%u") && passed;
					passed = context.are_equal(token->value.string, "bar", 0x102a8) && passed;

				json.get_token(token, sizeof(buffer));
				passed = context.are_equal(token->item, abc::json::item::end_object, 0x102a9, "%u") && passed;
			}
		}
		catch (const std::exception& ex) {
			context.log->put_any(abc::category::abc::base, abc::severity::important, 0x100f0, "client: EXCEPTION: %s", ex.what());
		}
	}


	void http_json_stream_server(bool& passed, test_context<abc::test::log>& context, abc::http_server_stream<abc::test::log>& http) {
		// Receive request
		{
			char buffer[abc::size::k1];

			http.get_header_name(buffer, sizeof(buffer));
			passed = context.are_equal(buffer, request_header_name, 0x100f5) && passed;

			http.get_header_value(buffer, sizeof(buffer));
			passed = context.are_equal(buffer, request_header_value, 0x100f6) && passed;

			http.get_header_name(buffer, sizeof(buffer));
			passed = context.are_equal(buffer, "", 0x100f7) && passed;

			// Body (json)
			abc::json_istream<abc::size::_64, abc::test::log> json(static_cast<abc::http_request_istream<abc::test::log>&>(http).rdbuf(), context.log);
			abc::json::token_t* token = (abc::json::token_t*)buffer;

			json.get_token(token, sizeof(buffer));
			passed = context.are_equal(token->item, abc::json::item::begin_object, 0x102aa, "%u") && passed;

				json.get_token(token, sizeof(buffer));
				passed = context.are_equal(token->item, abc::json::item::property, 0x102ab, "%u") && passed;
				passed = context.are_equal(token->value.property, "param", 0x102ac) && passed;

				json.get_token(token, sizeof(buffer));
				passed = context.are_equal(token->item, abc::json::item::string, 0x102ad, "%u") && passed;
				passed = context.are_equal(token->value.string, "foo", 0x102ae) && passed;

			json.get_token(token, sizeof(buffer));
			passed = context.are_equal(token->item, abc::json::item::end_object, 0x102af, "%u") && passed;
		}

		// Send response
		http.put_protocol(protocol);
		http.put_status_code(response_status_code);
		http.put_reason_phrase(response_reason_phrase);
		http.put_header_name(response_header_name);
		http.put_header_value(response_header_value);
		http.end_headers();

		// Body (json)
		abc::json_ostream<abc::size::_64, abc::test::log> json(static_cast<abc::http_response_ostream<abc::test::log>&>(http).rdbuf(), context.log);
		json.put_begin_object();
			json.put_property("n");
			json.put_number(42.0);
			json.put_property("s");
			json.put_string("bar");
		json.put_end_object();
	}


	template <typename ServerSocket, typename ClientSocket>
	bool tcp_socket_http_json_stream(test_context<abc::test::log>& context, ServerSocket& server, ClientSocket& client) {
		bool passed = true;

		server.bind(server_port);
		server.listen(5);

		std::thread client_thread([&passed, &context, &client] () {
			http_json_stream_client(passed, context, client);
		});

#if defined(__ABC__LINUX)
		abc::test::heap::counter_t closure_allocation_count = 1;
#elif defined(__ABC__MACOS)
		abc::test::heap::counter_t closure_allocation_count = 3;
#else
		abc::test::heap::counter_t closure_allocation_count = 1;
#endif

		passed = abc::test::heap::ignore_heap_allocations(abc::test::heap::instance_unaligned_throw_count, closure_allocation_count, context, 0x100f1) && passed; // Lambda closure

		ClientSocket connection = server.accept();

		abc::socket_streambuf<ClientSocket, abc::test::log> sb(&connection, context.log);
		abc::http_server_stream<abc::test::log> http(&sb, context.log);

		{
			char buffer[abc::size::k1];

			http.get_method(buffer, sizeof(buffer));
			passed = context.are_equal(buffer, request_method, 0x100f2) && passed;

			http.get_resource(buffer, sizeof(buffer));
			passed = context.are_equal(buffer, request_resource, 0x100f3) && passed;

			http.get_protocol(buffer, sizeof(buffer));
			passed = context.are_equal(buffer, protocol, 0x100f4) && passed;
		}

		http_json_stream_server(passed, context, http);

		client_thread.join();
		return passed;
	}


	bool test_tcp_socket_http_json_stream(test_context<abc::test::log>& context) {
		abc::tcp_server_socket<abc::test::log> server(abc::socket::family::ipv4, context.log);
		abc::tcp_client_socket<abc::test::log> client(abc::socket::family::ipv4, context.log);

		return tcp_socket_http_json_stream(context, server, client);
	}


	bool test_openssl_tcp_socket_http_json_stream(test_context<abc::test::log>& context) {
		bool passed = true;

#ifdef __ABC__OPENSSL
		char cert_path[max_path_size];
		passed = passed && make_filepath(context, cert_path, max_path_size, context.process_path, cert_filename);

		char pkey_path[max_path_size];
		passed = passed && make_filepath(context, pkey_path, max_path_size, context.process_path, pkey_filename);

		if (passed) {
			abc::openssl_tcp_server_socket<abc::test::log> server(cert_path, pkey_path, pkey_password, verify_client, abc::socket::family::ipv4, context.log);
			abc::openssl_tcp_client_socket<abc::test::log> client(verify_server, abc::socket::family::ipv4, context.log);

			passed = passed && tcp_socket_http_json_stream(context, server, client);
		}
#endif
		return passed;
	}

}}}

