/*
MIT License

Copyright (c) 2018-2024 Zlatko Michailov 

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
#include <ratio>

#include "../../src/root/ascii.h"
#include "../../src/net/endpoint.h"
#include "../../src/net/http.h"
#include "../../src/net/json.h"

#include "car.i.h"


namespace abc { namespace samples {


	// --------------------------------------------------------------


	template <typename Log>
	inline picar_4wd_hat<Log>::picar_4wd_hat(abc::gpio::chip<Log>* chip, gpio_smbus_address_t addr, gpio_smbus_clock_frequency_t clock_frequency, bool requires_byte_swap, Log* log)
		: base(addr, clock_frequency, requires_byte_swap, log)
		, _chip(chip)
		, _log(log) {
		reset();
	}


	template <typename Log>
	inline void picar_4wd_hat<Log>::reset() {
		abc::gpio_output_line<Log> reset_line(_chip, 21, _log);

		reset_line.put_level(abc::gpio::level::low,  std::chrono::milliseconds(1));
		reset_line.put_level(abc::gpio::level::high, std::chrono::milliseconds(3));
	}

	// --------------------------------------------------------------


	static constexpr abc::gpio_smbus_clock_frequency_t	smbus_hat_clock_frequency		= 72 * std::mega::num;
	static constexpr abc::gpio_smbus_address_t			smbus_hat_addr					= 0x14;
	static constexpr bool								smbus_hat_requires_byte_swap	= true;
	static constexpr abc::gpio_smbus_register_t			reg_pwm_base					= 0x20;
	static constexpr abc::gpio_smbus_register_t			reg_autoreload_base				= 0x44;
	static constexpr abc::gpio_smbus_register_t			reg_prescaler_base				= 0x40;

	static constexpr abc::gpio_smbus_register_t			reg_wheel_front_left			= 0x0d;
	static constexpr abc::gpio_smbus_register_t			reg_wheel_front_right			= 0x0c;
	static constexpr abc::gpio_smbus_register_t			reg_wheel_rear_left				= 0x08;
	static constexpr abc::gpio_smbus_register_t			reg_wheel_rear_right			= 0x09;
	static constexpr abc::gpio_smbus_register_t			reg_timer_front_left			= reg_wheel_front_left / 4;
	static constexpr abc::gpio_smbus_register_t			reg_timer_front_right			= reg_wheel_front_right / 4;
	static constexpr abc::gpio_smbus_register_t			reg_timer_rear_left				= reg_wheel_rear_left / 4;
	static constexpr abc::gpio_smbus_register_t			reg_timer_rear_right			= reg_wheel_rear_right / 4;

	static constexpr abc::gpio_smbus_register_t			reg_grayscale_left				= 0x12;
	static constexpr abc::gpio_smbus_register_t			reg_grayscale_center			= 0x11;
	static constexpr abc::gpio_smbus_register_t			reg_grayscale_right				= 0x10;

	static constexpr abc::gpio_pwm_pulse_frequency_t	frequency						= 50; // 50 Hz

	static constexpr abc::gpio::line_pos_t				pos_line_dir_front_left			= 23;
	static constexpr abc::gpio::line_pos_t				pos_line_dir_front_right		= 24;
	static constexpr abc::gpio::line_pos_t				pos_line_dir_rear_left			= 13;
	static constexpr abc::gpio::line_pos_t				pos_line_dir_rear_right			= 20;

	static constexpr abc::gpio::line_pos_t				pos_line_ultrasonic_trigger		= 5;
	static constexpr abc::gpio::line_pos_t				pos_line_ultrasonic_echo		= 6;
	static constexpr std::size_t						ultrasonic_max_cm				= 100;

	static constexpr abc::gpio_smbus_register_t			reg_servo						= 0x00;
	static constexpr abc::gpio_smbus_register_t			reg_timer_servo					= reg_servo / 4;

	static constexpr std::chrono::microseconds			servo_pulse_width_min			= std::chrono::microseconds(500);
	static constexpr std::chrono::microseconds			servo_pulse_width_max			= std::chrono::microseconds(2500);
	static constexpr std::chrono::milliseconds			servo_duty_duration				= std::chrono::milliseconds(250);


	// --------------------------------------------------------------


	template <typename Limits, typename Log>
	inline car_endpoint<Limits, Log>::car_endpoint(endpoint_config* config, Log* log)
		: base(config, log)

		, _chip(0, "picar_4wd", log)
		, _smbus(1, log)
		, _hat(&_chip, smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap, log)

		, _motor_front_left(&_chip, pos_line_dir_front_left,
							&_smbus, _hat, frequency, reg_pwm_base + reg_wheel_front_left,
							reg_autoreload_base + reg_timer_front_left, reg_prescaler_base + reg_timer_front_left, log)
		, _motor_front_right(&_chip, pos_line_dir_front_right,
							&_smbus, _hat, frequency, reg_pwm_base + reg_wheel_front_right,
							reg_autoreload_base + reg_timer_front_right, reg_prescaler_base + reg_timer_front_right, log)
		, _motor_rear_left(&_chip, pos_line_dir_rear_left,
							&_smbus, _hat, frequency, reg_pwm_base + reg_wheel_rear_left,
							reg_autoreload_base + reg_timer_rear_left, reg_prescaler_base + reg_timer_rear_left, log)
		, _motor_rear_right(&_chip, pos_line_dir_rear_right,
							&_smbus, _hat, frequency, reg_pwm_base + reg_wheel_rear_right,
							reg_autoreload_base + reg_timer_rear_right, reg_prescaler_base + reg_timer_rear_right, log)

		, _ultrasonic(&_chip, pos_line_ultrasonic_trigger, pos_line_ultrasonic_echo, log)
		, _servo(&_smbus, _hat, servo_pulse_width_min, servo_pulse_width_max,
							servo_duty_duration, frequency,
							reg_pwm_base + reg_servo, reg_autoreload_base + reg_timer_servo, reg_prescaler_base + reg_timer_servo, log)
		, _grayscale(&_smbus, _hat, reg_grayscale_left, reg_grayscale_center, reg_grayscale_right, log)

		, _motion(&_smbus, log)
		, _motion_tracker(&_motion, log)

		, _forward(true)
		, _power(0)
		, _turn(0)
		, _obstacle_cm(ultrasonic_max_cm)

		, _auto_thread(start_auto_loop, this) {

		_motion.calibrate(abc::gpio_smbus_motion_channel::all);
	}


	template <typename Limits, typename Log>
	inline abc::tcp_server_socket<Log> car_endpoint<Limits, Log>::create_server_socket() {
		return abc::tcp_server_socket<Log>(socket::family::ipv4, base::_log);
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x10678, "car_endpoint::process_rest_request: Start.");
		}

		if (ascii::are_equal_i(resource, "/power")) {
			process_power(http, method);
		}
		else if (ascii::are_equal_i(resource, "/turn")) {
			process_turn(http, method);
		}
		else if (ascii::are_equal_i(resource, "/autos")) {
			process_autos(http, method);
		}
		else if (ascii::are_equal_i(resource, "/servo")) {
			process_servo(http, method);
		}
		else if (ascii::are_equal_i(resource, "/shutdown")) {
			process_shutdown(http, method);
		}
		else {
			// 404
			base::send_simple_response(http, status_code::Not_Found, reason_phrase::Not_Found, content_type::text, "The requested resource was not found.", 0x10679);
		}

		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x1067a, "car_endpoint::process_rest_request: Done.");
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
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x1067b, "Content error: Expected '{'.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x1067c);
				return;
			}

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::property || !ascii::are_equal(token->value.property, "power")) {
				// Not "power".
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x1067d, "Content error: Expected \"power\".");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x1067e);
				return;
			}

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::number || !(-100 <= token->value.number && token->value.number <= 100)) {
				// Not a valid power.
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x1067f, "Content error: Expected -100 <= number <= 100.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x10680);
				return;
			}

			power = token->value.number;
		}

		if (!verify_range(http, power, -100, 100, 25)) {
			return;
		}

		// If changing direction, stop the vehicle and reset the motion tracker.
		if (power * _power < 0) {
			std::int32_t turn = _turn;

			_power = 0;
			_turn = 0;
			drive_verified();

			_motion_tracker.stop();

			_turn = turn;
		}

		if (power != 0 && !_motion_tracker.is_running()) {
			_motion_tracker.start();
		}

		_forward = true;
		if (power == 0) {
			_turn = 0;
		}

		if (power < 0) {
			_forward = false;
			power = -power;
		}

		_power = power;
		drive_verified();

		// If vehicle stopped, reset the motion tracker.
		if (_power == 0) {
			_motion_tracker.stop();
		}

		// 200
		char body[abc::size::_256 + 1];
		std::snprintf(body, sizeof(body), "power: forward=%d, power=%d, turn=%d", (int)_forward, (int)_power, (int)_turn);
		base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, body, 0x10681);
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
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x10682, "Content error: Expected '{'.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x10683);
				return;
			}

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::property || !ascii::are_equal(token->value.property, "turn")) {
				// Not "turn".
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x10684, "Content error: Expected \"turn\".");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x10685);
				return;
			}

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::number || !(-90 <= token->value.number && token->value.number <= 90)) {
				// Not a valid turn.
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x10686, "Content error: Expected -90 <= number <= 90.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x10687);
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
		std::snprintf(body, sizeof(body), "turn: power=%d, turn=%d", (int)_power, (int)_turn);
		base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, body, 0x10688);
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::process_autos(abc::http_server_stream<Log>& http, const char* method) {
		if (!verify_method_get(http, method)) {
			return;
		}

		// Write the JSON to a char buffer, so we can calculate the Content-Length before we start sending the body.
		char body[abc::size::_512 + 1];
		abc::buffer_streambuf sb(nullptr, 0, 0, body, 0, sizeof(body));
		abc::json_ostream<abc::size::_16, Log> json(&sb, base::_log);
		json.put_begin_object();
			json.put_property("obstacle");
			json.put_begin_object();
				json.put_property("distance");
				json.put_number(_obstacle_cm.load());
				json.put_property("units");
				json.put_string("cm");
			json.put_end_object();
			json.put_property("grayscale");
			json.put_begin_object();
				json.put_property("left");
				json.put_number(_grayscale_left.load());
				json.put_property("center");
				json.put_number(_grayscale_center.load());
				json.put_property("right");
				json.put_number(_grayscale_right.load());
			json.put_end_object();
			json.put_property("depth");
			json.put_begin_object();
				json.put_property("distance");
				json.put_number(_motion_tracker.depth());
				json.put_property("units");
				json.put_string("cm");
			json.put_end_object();
			json.put_property("width");
			json.put_begin_object();
				json.put_property("distance");
				json.put_number(_motion_tracker.width());
				json.put_property("units");
				json.put_string("cm");
			json.put_end_object();
		json.put_end_object();
		json.put_char('\0');
		json.flush();

		char content_length[abc::size::_32 + 1];
		std::snprintf(content_length, sizeof(content_length), "%zu", std::strlen(body));

		// Send the http response
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x10689, "Sending response 200");
		}

		http.put_protocol(protocol::HTTP_11);
		http.put_status_code(status_code::OK);
		http.put_reason_phrase(reason_phrase::OK);

		http.put_header_name(header::Connection);
		http.put_header_value(connection::close);
		http.put_header_name(header::Content_Type);
		http.put_header_value(content_type::json);
		http.put_header_name(header::Content_Length);
		http.put_header_value(content_length);
		http.end_headers();

		http.put_body(body);

		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x1068a, "car::process_autos: Done.");
		}
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::process_servo(abc::http_server_stream<Log>& http, const char* method) {
		if (!verify_method_post(http, method)) {
			return;
		}

		if (!verify_header_json(http)) {
			return;
		}

		std::int32_t angle;
		// Read turn from JSON
		{
			std::streambuf* sb = static_cast<abc::http_request_istream<Log>&>(http).rdbuf();
			abc::json_istream<abc::size::_64, Log> json(sb, base::_log);
			char buffer[sizeof(abc::json::token_t) + abc::size::k1 + 1];
			abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(buffer);
			constexpr const char* invalid_json = "An invalid JSON payload was supplied. Must be: {\"servo\": 50}.";

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::begin_object) {
				// Not {.
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x1068b, "Content error: Expected '{'.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x1068c);
				return;
			}

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::property || !ascii::are_equal(token->value.property, "angle")) {
				// Not "servo".
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x1068d, "Content error: Expected \"angle\".");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x1068e);
				return;
			}

			json.get_token(token, sizeof(buffer));
			if (token->item != abc::json::item::number || !(-90 <= token->value.number && token->value.number <= 90)) {
				// Not a valid angle.
				if (base::_log != nullptr) {
					base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x1068f, "Content error: Expected -90 <= number <= 90.");
				}

				// 400
				base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x10690);
				return;
			}

			angle = token->value.number;
		}

		if (!verify_range(http, angle, -90, 90, 30)) {
			return;
		}

		abc::gpio_pwm_duty_cycle_t duty_cycle = 100 - static_cast<abc::gpio_pwm_duty_cycle_t>(100 * (angle + 92) / 184);
		_servo.set_duty_cycle(duty_cycle);

		// 200
		char body[abc::size::_256 + 1];
		std::snprintf(body, sizeof(body), "servo: angle=%d", (int)angle);
		base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, body, 0x10691);
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::process_shutdown(abc::http_server_stream<Log>& http, const char* method) {
		if (!verify_method_post(http, method)) {
			return;
		}

		base::set_shutdown_requested();
		_hat.reset();
		_auto_thread.join();

		// 200
		base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, "Server is shuting down...", 0x10692);
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::drive_verified() noexcept {
		std::int32_t left_power;
		std::int32_t right_power;
		get_side_powers(left_power, right_power);

		gpio_pwm_duty_cycle_t left_duty_cycle = static_cast<gpio_pwm_duty_cycle_t>(left_power);
		gpio_pwm_duty_cycle_t right_duty_cycle = static_cast<gpio_pwm_duty_cycle_t>(right_power);

		_motor_front_left.set_forward(_forward);
		_motor_front_left.set_duty_cycle(left_duty_cycle);

		_motor_front_right.set_forward(_forward);
		_motor_front_right.set_duty_cycle(right_duty_cycle);

		_motor_rear_left.set_forward(_forward);
		_motor_rear_left.set_duty_cycle(left_duty_cycle);

		_motor_rear_right.set_forward(_forward);
		_motor_rear_right.set_duty_cycle(right_duty_cycle);
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::get_side_powers(std::int32_t& left_power, std::int32_t& right_power) noexcept {
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
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x10693, "left_power = %3d, right_power = %3d", (int)left_power, (int)right_power);
		}
	}


	template <typename Limits, typename Log>
	inline std::int32_t car_endpoint<Limits, Log>::get_delta_power() noexcept {
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
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x10694, "power = %3d, turn = %3d, delta = %3d", (int)_power, (int)_turn, (int)delta);
		}

		return delta;
	}


	template <typename Limits, typename Log>
	inline bool car_endpoint<Limits, Log>::verify_method_get(abc::http_server_stream<Log>& http, const char* method) noexcept {
		if (!ascii::are_equal_i(method, method::GET)) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x10695, "Method error: Expected 'GET'.");
			}

			// 405
			base::send_simple_response(http, status_code::Method_Not_Allowed, reason_phrase::Method_Not_Allowed, content_type::text, "Expected method GET for this request.", 0x10696);
			return false;
		}

		return true;
	}


	template <typename Limits, typename Log>
	inline bool car_endpoint<Limits, Log>::verify_method_post(abc::http_server_stream<Log>& http, const char* method) noexcept {
		if (!ascii::are_equal_i(method, method::POST)) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x10697, "Method error: Expected 'POST'.");
			}

			// 405
			base::send_simple_response(http, status_code::Method_Not_Allowed, reason_phrase::Method_Not_Allowed, content_type::text, "Expected method POST for this request.", 0x10698);
			return false;
		}

		return true;
	}


	template <typename Limits, typename Log>
	inline bool car_endpoint<Limits, Log>::verify_header_json(abc::http_server_stream<Log>& http) noexcept {
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
						base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x10699, "Header error: Already received 'Content-Type'.");
					}

					// 400
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "The Content-Type header was supplied more than once.", 0x1069a);
					return false;
				}

				http.get_header_value(header, sizeof(header));

				static const std::size_t content_type_json_len = std::strlen(content_type::json);
				if (!ascii::are_equal_i_n(header, content_type::json, content_type_json_len)) {
					// The Content-Type is not json.
					if (base::_log != nullptr) {
						base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x1069b, "Header error: Expected `application/json` as 'Content-Type'.");
					}

					// 400
					base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "'application/json' is the only supported Content-Type.", 0x1069c);
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
	inline bool car_endpoint<Limits, Log>::verify_range(abc::http_server_stream<Log>& http, T value, T lo_bound, T hi_bound, T step) noexcept {
		if (value < lo_bound || hi_bound < value || value % step != 0) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x1069d, "Range error: value = %d.", (int)value);
			}

			// 400
			base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, "Value not in range.", 0x1069e);
			return false;
		}

		return true;
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::start_auto_loop(car_endpoint<Limits, Log>* this_ptr) noexcept {
		this_ptr->auto_loop();
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::auto_loop() noexcept {
		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x1069f, "car_endpoint::auto_loop: Start.");
		}

		while (!base::is_shutdown_requested()) {
			// Refresh obstacle
			std::size_t distance_cm = _ultrasonic.measure_distance(ultrasonic_max_cm);
			_obstacle_cm.store(distance_cm);

			// Refresh grayscales
			std::uint16_t grayscale_left = 0;
			std::uint16_t grayscale_center = 0;
			std::uint16_t grayscale_right = 0;
			_grayscale.get_values(grayscale_left, grayscale_center, grayscale_right);
			_grayscale_left.store(grayscale_left);
			_grayscale_center.store(grayscale_center);
			_grayscale_right.store(grayscale_right);

			auto_limit_power();

			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}

		if (base::_log != nullptr) {
			base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x106a0, "car_endpoint::auto_loop: Done.");
		}
	}


	template <typename Limits, typename Log>
	inline void car_endpoint<Limits, Log>::auto_limit_power() noexcept {
		std::size_t distance_cm = _obstacle_cm.load();
		std::int32_t power = _power;

		if (_forward) {
			if (_power > 75 && distance_cm < 30) {
				power = 75;
			}
			else if (_power > 50 && distance_cm < 20) {
				power = 50;
			}
			else if (_power > 25 && distance_cm < 10) {
				power = 25;
			}
			else if (_power > 0 && distance_cm < 5) {
				power = 0;
			}
		}

		if (power < _power) {
			if (base::_log != nullptr) {
				base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x106a1, "car_endpoint::auto_limit: old_power=%d, new_power=%d.", (int)_power, (int)power);
			}

			_power = power;
			drive_verified();
		}
	}

}}

