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


#include "../../src/http.h"
#include "../../src/json.h"

#include "webserver.h"


namespace abc { namespace samples {

	template <typename Log>
	class equations_webserver : public webserver<Log> {
		using base = webserver<Log>;

	public:
		equations_webserver(webserver_config* config, Log* log);

	protected:
		virtual void	process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) override;

	private:
		bool			parse_array_2(abc::http_server_stream<Log>& http, abc::json_istream<abc::size::_64, Log>& json, abc::json::token_t* token, std::size_t buffer_size, const char* invalid_json, double arr[]);
	};


	// --------------------------------------------------------------


	template <typename Log>
	inline equations_webserver<Log>::equations_webserver(webserver_config* config, Log* log)
		: base(config, log) {
	}


	template <typename Log>
	inline void equations_webserver<Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Start REST processing");

		// Support a graceful shutdown.
		if (std::strcmp(method, "POST") == 0 && std::strcmp(resource, "/shutdown") == 0) {
			base::set_shutdown_requested();

			base::send_simple_response(http, "200", content_type::text, "Server is shuting down...", __TAG__);
			return;
		}

		// If the resource is not /problem, return 404.
		if (std::strcmp(resource, "/problem") != 0) {
			base::send_simple_response(http, "404", content_type::text, "The requested resource was not found.", __TAG__);
			return;
		}

		// If the method is not POST, return 405.
		if (std::strcmp(method, "POST") != 0) {
			base::send_simple_response(http, "405", content_type::text, "POST is the only supported method for resource '/problem'.", __TAG__);
			return;
		}

		// Read all headers
		bool has_valid_content_type = false;
		while (true) {
			char header[abc::size::k1 + 1];

			http.get_header_name(header, sizeof(header));
			if (http.gcount() == 0) {
				break;
			}

			if (std::strcmp(header, "Content-Type") == 0) {
				http.get_header_value(header, sizeof(header));

				// If the Content-Type is not json, return 400.
				static const std::size_t content_type_json_len = std::strlen(content_type::json);
				if (std::strncmp(header, content_type::json, content_type_json_len) != 0) {
					base::send_simple_response(http, "400", content_type::text, "'application/json' is the only supported Content-Type.", __TAG__);
					return;
				}

				has_valid_content_type = true;
			}
			else if (has_valid_content_type) {
				// We've already received a Content-Type header, return 400.
				base::send_simple_response(http, "400", content_type::text, "The Content-Type header was supplied more than once.", __TAG__);
				return;
			}
			else {
				// Future-proof: Ignore unknown headers.
				http.get_header_value(header, sizeof(header));
			}
		}

		std::streambuf* sb = static_cast<abc::http_request_istream<Log>&>(http).rdbuf();
		abc::json_istream<abc::size::_64, Log> json(sb, base::_log);
		char buffer[sizeof(abc::json::token_t) + abc::size::k1 + 1];
		abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(buffer);
		const char* const invalid_json = "An invalid JSON payload was supplied. Must be {\"a\": [ [1, 2], [3, 4] ], \"b\": [5, 6] }.";

		// If body is not a JSON object, return 400.
		json.get_token(token, sizeof(buffer));
		if (token->item != abc::json::item::begin_object) {
			base::send_simple_response(http, "400", content_type::text, invalid_json, __TAG__);
			return;
		}

		// Here's where we store the parsed JSON input.
		bool has_a = false;
		double a[2][2];
		bool has_b = false;
		double b[2];

		// Read all properties.
		while (true) {
			// The tokens at this level must be properties or a }.
			json.get_token(token, sizeof(buffer));

			// If we reached }, then we are done parsing.
			if (token->item == abc::json::item::end_object) {
				break;
			}

			// If we got anything but a property, error out.
			if (token->item != abc::json::item::property) {
				base::send_simple_response(http, "400", content_type::text, invalid_json, __TAG__);
				return;
			}

			// We expect 2 properties - "a" and "b".
			if (std::strcmp(token->value.property, "a")) {
				// Parse array [2][2].
				json.get_token(token, sizeof(buffer));

				if (token->item != abc::json::item::begin_array) {
					base::send_simple_response(http, "400", content_type::text, invalid_json, __TAG__);
					return;
				}

				for (std::size_t i = 0; i < 2; i++) {
					if (!parse_array_2(http, json, token, sizeof(buffer), invalid_json, a[i])) {
						return;
					}
				}

				if (token->item != abc::json::item::end_array) {
					base::send_simple_response(http, "400", content_type::text, invalid_json, __TAG__);
					return;
				}

				has_a = true;
			}
			else if (std::strcmp(token->value.property, "b")) {
				// Parse array [2].
				if (!parse_array_2(http, json, token, sizeof(buffer), invalid_json, b)) {
					return;
				}

				has_b = true;
			}
			else {
				// Future-proof: Ignore unknown properties.
				json.skip_value();
			}
		}

		if (!has_a || !has_b) {
			base::send_simple_response(http, "400", content_type::text, invalid_json, __TAG__);
			return;
		}

		base::_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Sending response 200");
		http.put_protocol("HTTP/1.1");
		http.put_status_code("200");
		http.put_reason_phrase("OK");
		http.put_header_name("Content-Length");
		http.put_header_value("24");
		http.end_headers();
		http.put_body("Yae: I have an override!");

		base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Finish REST processing");
	}


	template <typename Log>
	inline bool equations_webserver<Log>::parse_array_2(abc::http_server_stream<Log>& http, abc::json_istream<abc::size::_64, Log>& json, abc::json::token_t* token, std::size_t buffer_size, const char* invalid_json, double arr[]) {
		json.get_token(token, buffer_size);

		if (token->item != abc::json::item::begin_array) {
			base::send_simple_response(http, "400", content_type::text, invalid_json, __TAG__);
			return false;
		}

		for (std::size_t i = 0; i < 2; i++) {
			json.get_token(token, buffer_size);
			if (token->item != abc::json::item::number) {
				base::send_simple_response(http, "400", content_type::text, invalid_json, __TAG__);
				return false;
			}

			arr[i] = token->value.number;
		}

		if (token->item != abc::json::item::end_array) {
			base::send_simple_response(http, "400", content_type::text, invalid_json, __TAG__);
			return false;
		}

		return true;
	}


	// --------------------------------------------------------------

}}


