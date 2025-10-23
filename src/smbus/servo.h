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


#pragma once

#include <thread>

#include "../exception.h"
#include "../log.h"
#include "../i/gpio.i.h"


namespace abc {

	template <typename PwmDuration, typename Log>
	template <typename PulseWidthDuration>
	inline gpio_smbus_servo<PwmDuration, Log>::gpio_smbus_servo(gpio_smbus<Log>* smbus, const gpio_smbus_target<Log>& smbus_target,
				PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width,
				PwmDuration pwm_duration,
				gpio_pwm_pulse_frequency_t frequency,
				gpio_smbus_register_t reg_pwm, gpio_smbus_register_t reg_autoreload, gpio_smbus_register_t reg_prescaler,
				Log* log)
		: _pwm(smbus, smbus_target, min_pulse_width, max_pulse_width, frequency, reg_pwm, reg_autoreload, reg_prescaler, log)
		, _pwm_duration(pwm_duration) {
		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, 0x1070e, "gpio_smbus_servo::gpio_smbus_servo() Done.");
		}
	}


	template <typename PwmDuration, typename Log>
	inline void gpio_smbus_servo<PwmDuration, Log>::set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle) noexcept {
		_pwm.set_duty_cycle(duty_cycle, _pwm_duration);
	}


	// --------------------------------------------------------------

}
