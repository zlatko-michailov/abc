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


#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <ratio>
#include <thread>
#include <atomic>

#include "../../src/log.h"
#include "../../src/endpoint.h"
#include "../../src/gpio.h"


namespace abc { namespace samples {

	using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;
	using limits = abc::endpoint_limits;


	// --------------------------------------------------------------


	template <typename Log>
	class picar_4wd_hat : public abc::gpio_smbus_target<Log> {
		using base = abc::gpio_smbus_target<Log>;

	public:
		picar_4wd_hat(abc::gpio_chip<Log>* chip, gpio_smbus_address_t addr, gpio_smbus_clock_frequency_t clock_frequency, bool requires_byte_swap, Log* log);

	public:
		void reset();

	private:
		abc::gpio_chip<Log>*	_chip;
		Log*					_log;
	};


	// --------------------------------------------------------------


	template <typename Limits, typename Log>
	class car_endpoint : public endpoint<abc::tcp_server_socket<Log>, abc::tcp_client_socket<Log>, Limits, Log> {
		using base = endpoint<abc::tcp_server_socket<Log>, abc::tcp_client_socket<Log>, Limits, Log>;

	public:
		car_endpoint(endpoint_config* config, Log* log);

	protected:
		virtual abc::tcp_server_socket<Log>	create_server_socket() override;
		virtual void	process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) override;

	private:
		void			process_power(abc::http_server_stream<Log>& http, const char* method);
		void			process_turn(abc::http_server_stream<Log>& http, const char* method);
		void			process_autos(abc::http_server_stream<Log>& http, const char* method);
		void			process_servo(abc::http_server_stream<Log>& http, const char* method);
		void			process_shutdown(abc::http_server_stream<Log>& http, const char* method);

		template <typename T>
		bool			verify_range(abc::http_server_stream<Log>& http, T value, T lo_bound, T hi_bound, T step) noexcept;

		void			drive_verified() noexcept;
		void			get_side_powers(std::int32_t& left_power, std::int32_t& right_power) noexcept;
		std::int32_t	get_delta_power() noexcept;

		bool			verify_method_get(abc::http_server_stream<Log>& http, const char* method) noexcept;
		bool			verify_method_post(abc::http_server_stream<Log>& http, const char* method) noexcept;
		bool			verify_header_json(abc::http_server_stream<Log>& http) noexcept;

	private:
		static void		start_auto_loop(car_endpoint<Limits, Log>* this_ptr) noexcept;
		void			auto_loop() noexcept;
		void			auto_limit_power() noexcept;

	private:
		abc::gpio_chip<Log>										_chip;
		abc::gpio_smbus<Log>									_smbus;
		picar_4wd_hat<Log>										_hat;

		abc::gpio_smbus_motor<Log>								_motor_front_left;
		abc::gpio_smbus_motor<Log>								_motor_front_right;
		abc::gpio_smbus_motor<Log>								_motor_rear_left;
		abc::gpio_smbus_motor<Log>								_motor_rear_right;

		abc::gpio_ultrasonic<std::centi, Log>					_ultrasonic;
		abc::gpio_smbus_servo<std::chrono::milliseconds, Log>	_servo;
		abc::gpio_smbus_grayscale<Log>							_grayscale;

		abc::gpio_smbus_motion<Log>								_motion;
		abc::gpio_smbus_motion_tracker<std::centi, Log>			_motion_tracker;

		bool													_forward;
		std::int32_t											_power;
		std::int32_t											_turn;
		std::atomic<std::size_t>								_obstacle_cm;
		std::atomic<std::uint16_t>								_grayscale_left;
		std::atomic<std::uint16_t>								_grayscale_center;
		std::atomic<std::uint16_t>								_grayscale_right;

		std::thread												_auto_thread;
	};


	// --------------------------------------------------------------

}}

