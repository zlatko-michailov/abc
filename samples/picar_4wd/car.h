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
		: base(config, log)
		, _power(0)
		, _turn(0) {
		reset_hat();
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "car_endpoint::process_rest_request: Start.");
		}

		if (ascii::are_equal_i(resource, "/power")) {
			process_power(http, method);
		}
		else if (ascii::are_equal_i(resource, "/turn")) {
			process_turn(http, method);
		}
		else if (ascii::are_equal_i(resource, "/shutdown")) {
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
	inline void car_endpoint<Limits, Log>::process_power(abc::http_server_stream<Log>& http, const char* method) {
		if (!verify_method_post(http, method)) {
			return;
		}

		if (!verify_header_json(http)) {
			return;
		}

		std::int32_t power;
		// Read power from JSON
		{
			std::streambuf* sb = static_cast<abc::http_request_istream<Log>&>(http).rdbuf();
			abc::json_istream<abc::size::_64, Log> json(sb, base::_log);
			char buffer[sizeof(abc::json::token_t) + abc::size::k1 + 1];
			abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(buffer);
			constexpr const char* invalid_json = "An invalid JSON payload was supplied. Must be: {\"power\": 50}.";

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::begin_object) {
				// Not {.
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Expected '{'.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, __TAG__);
				return;
			}

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::property || !ascii::are_equal(token->value.property, "power")) {
				// Not "power".
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Expected \"power\".");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, __TAG__);
				return;
			}

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::number || !(0 <= token->value.number && token->value.number <= 100)) {
				// Not a valid power.
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Expected 0 <= number <= 100.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, __TAG__);
				return;
			}

			power = token->value.number;
		}

		if (!verify_range(http, power, 0, 100, 25)) {
			return;
		}

		_power = power;
		if (power == 0) {
			_turn = 0;
		}
		drive_verified();

		// 200
		char body[abc::size::_256 + 1];
		std::snprintf(body, sizeof(body), "power: power=%d, turn=%d", _power, _turn);
		base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, body, __TAG__);
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::process_turn(abc::http_server_stream<Log>& http, const char* method) {
		if (!verify_method_post(http, method)) {
			return;
		}

		if (!verify_header_json(http)) {
			return;
		}

		std::int32_t turn;
		// Read turn from JSON
		{
			std::streambuf* sb = static_cast<abc::http_request_istream<Log>&>(http).rdbuf();
			abc::json_istream<abc::size::_64, Log> json(sb, base::_log);
			char buffer[sizeof(abc::json::token_t) + abc::size::k1 + 1];
			abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(buffer);
			constexpr const char* invalid_json = "An invalid JSON payload was supplied. Must be: {\"turn\": 50}.";

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::begin_object) {
				// Not {.
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Expected '{'.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, __TAG__);
				return;
			}

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::property || !ascii::are_equal(token->value.property, "turn")) {
				// Not "turn".
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Expected \"turn\".");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, __TAG__);
				return;
			}

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::number || !(-90 <= token->value.number && token->value.number <= 90)) {
				// Not a valid turn.
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "Content error: Expected -90 <= number <= 90.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, __TAG__);
				return;
			}

			turn = token->value.number;
		}

		if (!verify_range(http, turn, -90, 90, 30)) {
			return;
		}

		_turn = turn;
		drive_verified();

		// 200
		char body[abc::size::_256 + 1];
		std::snprintf(body, sizeof(body), "turn: power=%d, turn=%d", _power, _turn);
		base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, body, __TAG__);
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::process_shutdown(abc::http_server_stream<Log>& http, const char* method) {
		if (!verify_method_post(http, method)) {
			return;
		}

		base::set_shutdown_requested();
		reset_hat();

		// 200
		base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, "Server is shuting down...", __TAG__);
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::drive_verified() {
		constexpr abc::gpio_smbus_clock_frequency_t	smbus_hat_clock_frequency		= 72 * std::mega::num;
		constexpr abc::gpio_smbus_address_t			smbus_hat_addr					= 0x14;
		constexpr bool								smbus_hat_requires_byte_swap	= true;
		constexpr abc::gpio_smbus_register_t		smbus_hat_reg_base_pwm			= 0x20;
		constexpr abc::gpio_smbus_register_t		reg_base_autoreload				= 0x44;
		constexpr abc::gpio_smbus_register_t		reg_base_prescaler				= 0x40;

		constexpr abc::gpio_smbus_register_t		reg_wheel_front_left			= 0x0d;
		constexpr abc::gpio_smbus_register_t		reg_wheel_front_right			= 0x0c;
		constexpr abc::gpio_smbus_register_t		reg_wheel_rear_left				= 0x08;
		constexpr abc::gpio_smbus_register_t		reg_wheel_rear_right			= 0x09;
		constexpr abc::gpio_smbus_register_t		reg_timer_front_left			= reg_wheel_front_left / 4;
		constexpr abc::gpio_smbus_register_t		reg_timer_front_right			= reg_wheel_front_right / 4;
		constexpr abc::gpio_smbus_register_t		reg_timer_rear_left				= reg_wheel_rear_left / 4;
		constexpr abc::gpio_smbus_register_t		reg_timer_rear_right			= reg_wheel_rear_right / 4;
		constexpr abc::gpio_pwm_pulse_frequency_t	frequency						= 50; // 50 Hz

		abc::gpio_smbus<log_ostream> smbus(1, base::_log);
		abc::gpio_smbus_target<log_ostream> hat(smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap, base::_log);

		abc::gpio_smbus_pwm<log_ostream> pwm_wheel_front_left(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_front_left,
															reg_base_autoreload + reg_timer_front_left, reg_base_prescaler + reg_timer_front_left, base::_log);
		abc::gpio_smbus_pwm<log_ostream> pwm_wheel_front_right(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_front_right,
															reg_base_autoreload + reg_timer_front_right, reg_base_prescaler + reg_timer_front_right, base::_log);
		abc::gpio_smbus_pwm<log_ostream> pwm_wheel_rear_left(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_rear_left,
															reg_base_autoreload + reg_timer_rear_left, reg_base_prescaler + reg_timer_rear_left, base::_log);
		abc::gpio_smbus_pwm<log_ostream> pwm_wheel_rear_right(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_rear_right,
															reg_base_autoreload + reg_timer_rear_right, reg_base_prescaler + reg_timer_rear_right, base::_log);

		std::int32_t left_power;
		std::int32_t right_power;
		get_side_powers(left_power, right_power);

		pwm_wheel_front_left.set_duty_cycle(left_power);
		pwm_wheel_front_right.set_duty_cycle(right_power);
		pwm_wheel_rear_left.set_duty_cycle(left_power);
		pwm_wheel_rear_right.set_duty_cycle(right_power);
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::get_side_powers(std::int32_t& left_power, std::int32_t& right_power) {
		std::int32_t delta = get_delta_power();

		left_power  = _power + delta;
		right_power = _power - delta;

		constexpr std::int32_t min_power = static_cast<std::int32_t>(abc::gpio_pwm_duty_cycle::min);
		constexpr std::int32_t max_power = static_cast<std::int32_t>(abc::gpio_pwm_duty_cycle::max);

		std::int32_t adjust = 0;
		if (left_power > max_power) {
			adjust = max_power - left_power;
		}
		else if (right_power > max_power) {
			adjust = max_power - right_power;
		}
		else if (left_power < min_power) {
			adjust = min_power - left_power;
		}
		else if (right_power < min_power) {
			adjust = min_power - right_power;
		}

		left_power  += adjust;
		right_power += adjust;

		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "left_power = %3d, right_power = %3d", (int)left_power, (int)right_power);
		}
	}


	template <typename Limits, typename Log>
	inline std::int32_t car_endpoint<Limits, Log>::get_delta_power() {
		std::int32_t delta = 0;

		switch (_turn) {
		case -30:
		case +30:
			delta = 11;
			break;

		case -45:
		case +45:
			delta = 18;
			break;

		case -60:
		case +60:
			delta = 25;
			break;

		case -90:
		case +90:
			delta = 50;
			break;
		}

		if (_turn < 0) {
			delta = -delta;
		}

		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "power = %3d, turn = %3d, delta = %3d", (int)_power, (int)_turn, (int)delta);
		}

		return delta;
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::reset_hat() {
		abc::gpio_chip<Log> chip(0, "picar_4wd", base::_log);
		abc::gpio_output_line<Log> reset_line(&chip, 21, base::_log);

		reset_line.put_level(abc::gpio_level::low,  std::chrono::milliseconds(1));
		reset_line.put_level(abc::gpio_level::high, std::chrono::milliseconds(3));
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

}}

