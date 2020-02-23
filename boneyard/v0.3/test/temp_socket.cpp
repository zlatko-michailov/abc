class _basic_socket {
	void close();
};

class _connected_socket : public _basic_socket {
	void read();
	void write();

	void read_async();
	void write_async();
};

class _client_socket : public _connected_socket {
	void connect();

	void connect_async();
};

class udp_client_socket : public _client_socket {
};

class tcp_client_socket : public _client_socket {
};

class _server_socket : public _basic_socket {
	void bind();
};

class udp_server_socket : public _server_socket {
};

class tcp_server_socket : public _server_socket {
	void listen();
	void accept();
};


void dgram_send() {
	const char* target_address = "12.34.56.78";
	const char* target_port = "2345";

	dgram_socket dgram(target_address, target_port);

	std::uint8_t bytes[] = {0x01, 0x02, 0x03, 0x04};
	dgram.send(sizeof(bytes), bytes);
}


void dgram_receive() {
	const char* local_port = "2345";

	dgram_socket dgram(local_port);

	constexpr std::size_t header_size = 8;
	std::uint8_t header[header_size];
	dgram.receive(header_size, header);

	std::uint8_t payload[max_payload_size];
	std::size_t payload_size = get_payload_size_from_header(header);
	dgram.receive(payload_size, payload);
}


void dgram_send_async() {
	const char* target_address = "12.34.56.78";
	const char* target_port = "2345";

	dgram_socket dgram(target_address, target_port);

	std::uint8_t bytes[] = {0x01, 0x02, 0x03, 0x04};

	dgram.send_async(sizeof(bytes), bytes)
		.then([] (abc::future&& sent, &dgram) {
			dgram.close();
		})
		.wait();

	std::future sent = dgram.send_async(sizeof(bytes), bytes);
	std::future closed = std::async([] (std::move(sent)) {
		sent.wait();
		dgram.close();
	});
	closed.wait();

	std::future done = std::async([]() {
		dgram.send(sizeof(bytes), bytes);
		dgram.close();
	});
	done.wait();
}


void dgram_receive_async() {
	const char* local_port = "2345";

	dgram_socket dgram(local_port);

	constexpr std::size_t header_size = 8;
	std::uint8_t header[header_size];

	dgram.receive(header_size, header)
		.then([&dgram, &header](abc::future&& received) {
			std::size_t payload_size = get_payload_size_from_header(header);
			std::uint8_t payload[max_payload_size];

			return dgram.receive(payload_size, payload);
		})
		.then([&dgram, &payload](abc::future&& received) {
			dgram.close();
		})
		.wait();
}


