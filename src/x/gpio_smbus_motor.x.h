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


#pragma once

#include <thread>

#include "../exception.h"
#include "../log.h"
#include "../i/gpio.i.h"


namespace abc {

	template <typename Log>
	inline gpio_smbus_motor<Log>::gpio_smbus_motor(const gpio_chip<Log>* chip, gpio_line_pos_t direction_line_pos,
				gpio_smbus<Log>* smbus, const gpio_smbus_target<Log>& smbus_target,
				gpio_pwm_pulse_frequency_t frequency,
				gpio_smbus_register_t reg_pwm, gpio_smbus_register_t reg_autoreload, gpio_smbus_register_t reg_prescaler,
				Log* log)
		: _direction_line(chip, direction_line_pos, log)
		, _pwm(smbus, smbus_target, frequency, reg_pwm, reg_autoreload, reg_prescaler, log)
		, _forward(true)
		, _duty_cycle(gpio_pwm_duty_cycle::min) {
		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, 0x10704, "gpio_smbus_motor::gpio_smbus_motor() Done.");
		}
	}


	template <typename Log>
	inline void gpio_smbus_motor<Log>::set_forward(bool forward) noexcept {
		_forward = forward;
		_direction_line.put_level(_forward ? gpio_level::low : gpio_level::high);
	}


	template <typename Log>
	inline bool gpio_smbus_motor<Log>::is_forward() const noexcept {
		return _forward;
	}


	template <typename Log>
	inline void gpio_smbus_motor<Log>::set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle) noexcept {
		_pwm.set_duty_cycle(duty_cycle);
	}


	// --------------------------------------------------------------

}
