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


#pragma once

#include <cstdint>
#include <ratio>
#include <chrono>
#include <linux/gpio.h>

#include "gpio_base.i.h"
#include "gpio_chip.i.h"
#include "gpio_line.i.h"
#include "gpio_smbus.i.h"
#include "gpio_smbus_pwm.i.h"


namespace abc {

	template <typename Log = null_log>
	class gpio_smbus_motor {
	public:
		gpio_smbus_motor(const gpio_chip<Log>* chip, gpio_line_pos_t direction_line_pos,
					gpio_smbus<Log>* smbus, const gpio_smbus_target<Log>& smbus_target,
					gpio_pwm_pulse_frequency_t frequency,
					gpio_smbus_register_t reg_pwm, gpio_smbus_register_t reg_autoreload, gpio_smbus_register_t reg_prescaler,
					Log* log = nullptr);
		gpio_smbus_motor(gpio_smbus_motor<Log>&& other) noexcept = default;
		gpio_smbus_motor(const gpio_smbus_motor<Log>& other) = delete;

	public:
		void					set_forward(bool forward) noexcept;
		bool					is_forward() const noexcept;

		void					set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle) noexcept;
		gpio_pwm_duty_cycle_t	get_duty_cycle() const noexcept;

	private:
		gpio_output_line<Log>	_direction_line;
		gpio_smbus_pwm<Log>		_pwm;
		bool					_forward;
		gpio_pwm_duty_cycle_t	_duty_cycle;
		Log*					_log;
	};


	// --------------------------------------------------------------

}
