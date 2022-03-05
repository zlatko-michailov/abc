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
#include <chrono>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "../size.h"
#include "gpio_base.i.h"
#include "gpio_pwm_base.i.h"
#include "gpio_smbus.i.h"


namespace abc {

	template <typename Log = null_log>
	class gpio_smbus_pwm {
	public:
		template <typename PulseWidthDuration>
		gpio_smbus_pwm(gpio_smbus<Log>* smbus, PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width, gpio_pwm_pulse_frequency_t frequency,
					gpio_smbus_address_t addr, gpio_smbus_register_t reg_pwm, gpio_smbus_register_t reg_autoreload, gpio_smbus_register_t reg_prescaler, Log* log = nullptr);
		gpio_smbus_pwm(gpio_smbus<Log>* smbus, gpio_pwm_pulse_frequency_t frequency,
					gpio_smbus_address_t addr, gpio_smbus_register_t reg_pwm, gpio_smbus_register_t reg_autoreload, gpio_smbus_register_t reg_prescaler, Log* log = nullptr);
		gpio_smbus_pwm(gpio_smbus_pwm<Log>&& other) noexcept = default;
		gpio_smbus_pwm(const gpio_smbus_pwm<Log>& other) = default;

	public:
		void	set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle);

		template <typename PwmDuration>
		void	set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle, PwmDuration duration);

	public:
		gpio_smbus<Log>*				_smbus;
		gpio_pwm_duration_t				_min_pulse_width;
		gpio_pwm_duration_t				_max_pulse_width;
		gpio_pwm_pulse_frequency_t		_frequency;
		gpio_smbus_clock_frequency_t	_period;
		gpio_smbus_clock_frequency_t	_autoreload;
		gpio_smbus_clock_frequency_t	_prescaler;
		gpio_smbus_address_t			_addr;
		gpio_smbus_register_t			_reg_pwm;
		gpio_smbus_register_t			_reg_autoreload;
		gpio_smbus_register_t			_reg_prescaler;
		gpio_pwm_duty_cycle_t			_duty_cycle;
		Log*							_log;
	};


	// --------------------------------------------------------------

}
