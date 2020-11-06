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


#include "../../src/ascii.h"
#include "../../src/endpoint.h"
#include "../../src/http.h"
#include "../../src/json.h"


namespace abc { namespace samples {

	template <typename Limits, typename Log>
	class equations_endpoint : public endpoint<Limits, Log> {
		using base = endpoint<Limits, Log>;

	public:
		equations_endpoint(endpoint_config* config, Log* log);

	protected:
		virtual void	process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) override;

	private:
		bool			parse_array_2(abc::http_server_stream<Log>& http, abc::json_istream<abc::size::_64, Log>& json, abc::json::token_t* token, std::size_t buffer_size, const char* invalid_json, double arr[]);
	};


	// --------------------------------------------------------------


	template <typename Limits, typename Log>
	inline equations_endpoint<Limits, Log>::equations_endpoint(endpoint_config* config, Log* log)
		: base(config, log) {
	}


	template <typename Limits, typename Log>
	inline void equations_endpoint<Limits, Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x102cd, "Start REST processing");
		}

		// Support a graceful shutdown.
		if (ascii::are_equal_i(method, method::POST) && ascii::are_equal_i(resource, "/shutdown")) {
			base::set_shutdown_requested();

			base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, "Server is shuting down...", 0x102ce);
			return;
		}

		// If the resource is not /problem, return 404.
		if (!ascii::are_equal_i(resource, "/problem")) {
			base::send_simple_response(http, status_code::Not_Found, reason_phrase::Not_Found, content_type::text, "The requested resource was not found.", 0x102cf);
			return;
		}

		// If the method is not POST, return 405.
		if (!ascii::are_equal_i(method, method::POST)) {
			base::send_simple_response(http, status_code::Method_Not_Allowed, reason_phrase::Method_Not_Allowed, content_type::text, "POST is the only supported method for resource '/problem'.", 0x102d0);
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

			if (ascii::are_equal_i(header, header::Content_Type)) {
				if (has_valid_content_type) {
					// We've already received a Content-Type header, return 400.
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "The Content-Type header was supplied more than once.", 0x102d2);
					return;
				}

				http.get_header_value(header, sizeof(header));

				// If the Content-Type is not json, return 400.
				static const std::size_t content_type_json_len = std::strlen(content_type::json);
				if (!ascii::are_equal_i_n(header, content_type::json, content_type_json_len)) {
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "'application/json' is the only supported Content-Type.", 0x102d1);
					return;
				}

				has_valid_content_type = true;
			}
			else {
				// Future-proof: Ignore unknown headers.
				http.get_header_value(header, sizeof(header));
			}
		}

		// Here's where we store the parsed JSON input.
		bool has_a = false;
		double a[2][2];
		bool has_b = false;
		double b[2];

		// Use a block to release the buffers when done parsing
		{
			std::streambuf* sb = static_cast<abc::http_request_istream<Log>&>(http).rdbuf();
			abc::json_istream<abc::size::_64, Log> json(sb, base::_log);
			char buffer[sizeof(abc::json::token_t) + abc::size::k1 + 1];
			abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(buffer);
			const char* const invalid_json = "An invalid JSON payload was supplied. Must be {\"a\": [ [1, 2], [3, 4] ], \"b\": [5, 6] }.";

			// If body is not a JSON object, return 400.
			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::begin_object) {
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x102d3);
				return;
			}

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
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x102d4);
					return;
				}

				// We expect 2 properties - "a" and "b".
				if (ascii::are_equal(token->value.property, "a")) {
					// Parse array [2][2].
					json.get_token(token, sizeof(buffer));

					if (token->item != abc::json::item::begin_array) {
						base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x102d5);
						return;
					}

					for (std::size_t i = 0; i < 2; i++) {
						if (base::_log != nullptr) {
							base::_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x102ee, "Parsing a[%lu]", i);
						}

						if (!parse_array_2(http, json, token, sizeof(buffer), invalid_json, a[i])) {
							return;
						}
					}

					json.get_token(token, sizeof(buffer));
					if (token->item != abc::json::item::end_array) {
						base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x102d6);
						return;
					}

					has_a = true;
				}
				else if (ascii::are_equal(token->value.property, "b")) {
					// Parse array [2].
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x102f0, "Parsing b");
					}

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
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x102d7);
				return;
			}
		}

		// Now, let's solve the system.
		double det = (a[0][0] * a[1][1]) - (a[0][1] * a[1][0]);
		double det_x = (b[0] * a[1][1]) - (a[0][1] * b[1]);
		double det_y = (a[0][0] * b[1]) - (b[0] * a[1][0]);

		std::int32_t status = -1;
		double x = 0;
		double y = 0;

		if (det != 0) {
			// 1 solution
			status = 1;
			x = det_x / det;
			y = det_y / det;
		}
		else if (det_x != 0 || det_y != 0) {
			// 0 solutions
			status = 0;
		}
		else {
			// inf. solutions
			status = 2;
		}

		// Write the JSON to a char buffer, so we can calculate the Content-Length before we start sending the body.
		char body[abc::size::k1 + 1];
		abc::buffer_streambuf sb(nullptr, 0, 0, body, 0, sizeof(body));
		abc::json_ostream<abc::size::_16, Log> json(&sb, base::_log);
		json.put_begin_object();
			json.put_property("status");
			json.put_number(status);
			json.put_property("x");
			json.put_number(x);
			json.put_property("y");
			json.put_number(y);
		json.put_end_object();
		json.put_char('\0');
		json.flush();

		char content_length[abc::size::_32 + 1];
		std::snprintf(content_length, sizeof(content_length), "%lu", std::strlen(body));

		// Send the http response
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x102d8, "Sending response 200");
		}

		http.put_protocol(protocol::HTTP_11);
		http.put_status_code(status_code::OK);
		http.put_reason_phrase(reason_phrase::OK);
		http.put_header_name(header::Content_Type);
		http.put_header_value(content_type::json);
		http.put_header_name(header::Content_Length);
		http.put_header_value(content_length);
		http.end_headers();
		http.put_body(body);

		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x102d9, "Finish REST processing");
		}
	}


	template <typename Limits, typename Log>
	inline bool equations_endpoint<Limits, Log>::parse_array_2(abc::http_server_stream<Log>& http, abc::json_istream<abc::size::_64, Log>& json, abc::json::token_t* token, std::size_t buffer_size, const char* invalid_json, double arr[]) {
		json.get_token(token, buffer_size);

		if (token->item != abc::json::item::begin_array) {
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x102da);
			return false;
		}

		for (std::size_t i = 0; i < 2; i++) {
			json.get_token(token, buffer_size);
			if (token->item != abc::json::item::number) {
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x102db);
				return false;
			}

			arr[i] = token->value.number;
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x102ef, "array[%lu]=%g", i, arr[i]);
			}
		}

		json.get_token(token, buffer_size);
		if (token->item != abc::json::item::end_array) {
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x102dc);
			return false;
		}

		return true;
	}


	// --------------------------------------------------------------

}}


