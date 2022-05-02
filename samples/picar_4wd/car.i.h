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


#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <ratio>
#include <thread>

#include "../../src/log.h"
#include "../../src/endpoint.h"
#include "../../src/gpio.h"


namespace abc { namespace samples {

	using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;
	using limits = abc::endpoint_limits;


	// --------------------------------------------------------------


	template <typename Log>
	class gpio_smbus_hat : public abc::gpio_smbus_target<Log> {
		using base = abc::gpio_smbus_target<Log>;

	public:
		gpio_smbus_hat(abc::gpio_chip<Log>* chip, gpio_smbus_address_t addr, gpio_smbus_clock_frequency_t clock_frequency, bool requires_byte_swap, Log* log);

	public:
		void reset();

	private:
		abc::gpio_chip<Log>*	_chip;
		Log*					_log;
	};


	// --------------------------------------------------------------


	template <typename Limits, typename Log>
	class car_endpoint : public endpoint<Limits, Log> {
		using base = endpoint<Limits, Log>;

	public:
		car_endpoint(endpoint_config* config, Log* log);

	protected:
		virtual void	process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) override;

	private:
		void			process_power(abc::http_server_stream<Log>& http, const char* method);
		void			process_turn(abc::http_server_stream<Log>& http, const char* method);
		void			process_shutdown(abc::http_server_stream<Log>& http, const char* method);

		template <typename T>
		bool			verify_range(abc::http_server_stream<Log>& http, T value, T lo_bound, T hi_bound, T step);

		void			drive_verified();
		void			get_side_powers(std::int32_t& left_power, std::int32_t& right_power);
		std::int32_t	get_delta_power();

		bool			verify_method_get(abc::http_server_stream<Log>& http, const char* method);
		bool			verify_method_post(abc::http_server_stream<Log>& http, const char* method);
		bool			verify_header_json(abc::http_server_stream<Log>& http);

	private:
		static constexpr abc::gpio_line_pos_t				pos_line_dir_front_left			= 23;
		static constexpr abc::gpio_line_pos_t				pos_line_dir_front_right		= 24;
		static constexpr abc::gpio_line_pos_t				pos_line_dir_rear_left			= 13;
		static constexpr abc::gpio_line_pos_t				pos_line_dir_rear_right			= 20;

		static constexpr abc::gpio_smbus_clock_frequency_t	smbus_hat_clock_frequency		= 72 * std::mega::num;
		static constexpr abc::gpio_smbus_address_t			smbus_hat_addr					= 0x14;
		static constexpr bool								smbus_hat_requires_byte_swap	= true;
		static constexpr abc::gpio_smbus_register_t			smbus_hat_reg_base_pwm			= 0x20;
		static constexpr abc::gpio_smbus_register_t			reg_base_autoreload				= 0x44;
		static constexpr abc::gpio_smbus_register_t			reg_base_prescaler				= 0x40;

		static constexpr abc::gpio_smbus_register_t			reg_wheel_front_left			= 0x0d;
		static constexpr abc::gpio_smbus_register_t			reg_wheel_front_right			= 0x0c;
		static constexpr abc::gpio_smbus_register_t			reg_wheel_rear_left				= 0x08;
		static constexpr abc::gpio_smbus_register_t			reg_wheel_rear_right			= 0x09;
		static constexpr abc::gpio_smbus_register_t			reg_timer_front_left			= reg_wheel_front_left / 4;
		static constexpr abc::gpio_smbus_register_t			reg_timer_front_right			= reg_wheel_front_right / 4;
		static constexpr abc::gpio_smbus_register_t			reg_timer_rear_left				= reg_wheel_rear_left / 4;
		static constexpr abc::gpio_smbus_register_t			reg_timer_rear_right			= reg_wheel_rear_right / 4;
		static constexpr abc::gpio_pwm_pulse_frequency_t	frequency						= 50; // 50 Hz

		abc::gpio_chip<Log>				_chip;
		abc::gpio_output_line<Log>		_line_dir_front_left;
		abc::gpio_output_line<Log>		_line_dir_front_right;
		abc::gpio_output_line<Log>		_line_dir_rear_left;
		abc::gpio_output_line<Log>		_line_dir_rear_right;

		abc::gpio_smbus<Log>			_smbus;
		gpio_smbus_hat<Log>				_hat;
		abc::gpio_smbus_pwm<Log>		_pwm_wheel_front_left;
		abc::gpio_smbus_pwm<Log>		_pwm_wheel_front_right;
		abc::gpio_smbus_pwm<Log>		_pwm_wheel_rear_left;
		abc::gpio_smbus_pwm<Log>		_pwm_wheel_rear_right;

		abc::gpio_level_t				_direction;
		std::int32_t					_power;
		std::int32_t					_turn;
	};


	// --------------------------------------------------------------

}}

