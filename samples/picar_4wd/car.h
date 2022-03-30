/*
MIT License

Copyright (c) 2018-2022 Zlatko Michailov 

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


#include <cstdlib>
#include <cstdio>

#include "../../src/ascii.h"
#include "../../src/endpoint.h"
#include "../../src/http.h"
#include "../../src/json.h"

#include "car.i.h"


namespace abc { namespace samples {


	// --------------------------------------------------------------


	template <typename Limits, typename Log>
	inline car_endpoint<Limits, Log>::car_endpoint(endpoint_config* config, Log* log)
		: base(config, log) {
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "car_endpoint::process_rest_request: Start.");
		}

		if (ascii::are_equal_i(resource, "/shutdown")) {
			process_shutdown(http, method);
		}
		else {
			// 404
			base::send_simple_response(http, status_code::Not_Found, reason_phrase::Not_Found, content_type::text, "The requested resource was not found.", __TAG__);
		}

		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "car_endpoint::process_rest_request: Done.");
		}
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::process_shutdown(abc::http_server_stream<Log>& http, const char* method) {
		if (!verify_method_post(http, method)) {
			return;
		}

		base::set_shutdown_requested();

		// 200
		base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, "Server is shuting down...", __TAG__);
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::set_power(abc::http_server_stream<Log>& http, const char* method, std::int32_t power) {
		if (!verify_method_post(http, method)) {
			return;
		}

		if (!verify_range(http, power, 0, 100, 25)) {
			return;
		}

		drive(http, power, _turn);
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::set_turn(abc::http_server_stream<Log>& http, const char* method, std::int32_t turn) {
		if (!verify_method_post(http, method)) {
			return;
		}

		if (!verify_range(http, turn, -90, 90, 30)) {
			return;
		}

		drive(http, _power, turn);
	}


	template <typename Limits, typename Log>
	inline bool car_endpoint<Limits, Log>::verify_method_get(abc::http_server_stream<Log>& http, const char* method) {
		if (!ascii::are_equal_i(method, method::GET)) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Method error: Expected 'GET'.");
			}

			// 405
			base::send_simple_response(http, status_code::Method_Not_Allowed, reason_phrase::Method_Not_Allowed, content_type::text, "Expected method GET for this request.", __TAG__);
			return false;
		}

		return true;
	}


	template <typename Limits, typename Log>
	inline bool car_endpoint<Limits, Log>::verify_method_post(abc::http_server_stream<Log>& http, const char* method) {
		if (!ascii::are_equal_i(method, method::POST)) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Method error: Expected 'POST'.");
			}

			// 405
			base::send_simple_response(http, status_code::Method_Not_Allowed, reason_phrase::Method_Not_Allowed, content_type::text, "Expected method POST for this request.", __TAG__);
			return false;
		}

		return true;
	}


	template <typename Limits, typename Log>
	inline bool car_endpoint<Limits, Log>::verify_header_json(abc::http_server_stream<Log>& http) {
		bool has_content_type_json = false;

		// Read all headers
		while (true) {
			char header[abc::size::k1 + 1];

			// No more headers
			http.get_header_name(header, sizeof(header));
			if (http.gcount() == 0) {
				break;
			}

			if (ascii::are_equal_i(header, header::Content_Type)) {
				if (has_content_type_json) {
					// We've already received a Content-Type header.
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Header error: Already received 'Content-Type'.");
					}

					// 400
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "The Content-Type header was supplied more than once.", __TAG__);
					return false;
				}

				http.get_header_value(header, sizeof(header));

				static const std::size_t content_type_json_len = std::strlen(content_type::json);
				if (!ascii::are_equal_i_n(header, content_type::json, content_type_json_len)) {
					// The Content-Type is not json.
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Header error: Expected `application/json` as 'Content-Type'.");
					}

					// 400
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "'application/json' is the only supported Content-Type.", __TAG__);
					return false;
				}

				has_content_type_json = true;
			}
			else {
				// Future-proof: Ignore unknown headers.
				http.get_header_value(header, sizeof(header));
			}
		}

		return has_content_type_json;
	}


	template <typename Limits, typename Log>
	template <typename T>
	inline bool car_endpoint<Limits, Log>::verify_range(abc::http_server_stream<Log>& http, T value, T lo_bound, T hi_bound, T step) {
		if (value < lo_bound || hi_bound < value || value % step != 0) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Range error: value = %d.", (int)value);
			}

			// 400
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "Value not in range.", __TAG__);
			return false;
		}

		return true;
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::drive(abc::http_server_stream<Log>& http, std::int32_t power, std::int32_t turn) {
		//// TODO: drive

		// 200
		base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, "TODO: drive", __TAG__);
	}

}}

