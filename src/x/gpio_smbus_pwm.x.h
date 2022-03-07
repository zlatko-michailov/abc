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
#include <cmath>
#include <linux/gpio.h>

#include "../log.h"
#include "../i/gpio.i.h"


namespace abc {

	template <typename Log>
	template <typename PulseWidthDuration>
	inline gpio_smbus_pwm<Log>::gpio_smbus_pwm(gpio_smbus<Log>* smbus, const gpio_smbus_target<Log>& smbus_target,
										PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width,
										gpio_pwm_pulse_frequency_t frequency,
										gpio_smbus_register_t reg_pwm, gpio_smbus_register_t reg_autoreload, gpio_smbus_register_t reg_prescaler,
										Log* log)
		: _smbus(smbus)
		, _smbus_target(smbus_target)
		, _min_pulse_width(std::chrono::duration_cast<gpio_pwm_duration_t>(min_pulse_width))
		, _max_pulse_width(std::chrono::duration_cast<gpio_pwm_duration_t>(max_pulse_width))
		, _frequency(frequency)
		, _period(0)
		, _autoreload(0)
		, _prescaler(0)
		, _reg_pwm(reg_pwm)
		, _reg_autoreload(reg_autoreload)
		, _reg_prescaler(reg_prescaler)
		, _duty_cycle(0)
		, _log(log) {
		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_pwm::gpio_smbus_pwm() Start.");
		}

		if (smbus == nullptr) {
			throw exception<std::logic_error, Log>("gpio_smbus_pwm::gpio_smbus_pwm() smbus == nullptr", __TAG__);
		}

		// Calculate auto_reload and prescaler
		_period = _smbus_target.clock_frequency() / _frequency;
		gpio_smbus_clock_frequency_t sqrt_period = static_cast<gpio_smbus_clock_frequency_t>(std::lround(std::sqrt(_period)));
		_autoreload = (sqrt_period / 100) * 100;
		_prescaler = _period / _autoreload;

		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::debug, __TAG__, "gpio_smbus_pwm::gpio_smbus_pwm() period = %u | autoreload = %u | prescaler = %u",
				(unsigned)_period, (unsigned)_autoreload, (unsigned)_prescaler);
		}

		_smbus->put_word(_smbus_target, _reg_autoreload, _autoreload);
		_smbus->put_word(_smbus_target, _reg_prescaler, _prescaler);

		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_pwm::gpio_smbus_pwm() Done.");
		}
	}


	template <typename Log>
	inline gpio_smbus_pwm<Log>::gpio_smbus_pwm(gpio_smbus<Log>* smbus, const gpio_smbus_target<Log>& smbus_target,
										gpio_pwm_pulse_frequency_t frequency,
										gpio_smbus_register_t reg_pwm, gpio_smbus_register_t reg_autoreload, gpio_smbus_register_t reg_prescaler,
										Log* log)
		: gpio_smbus_pwm<Log>(smbus, smbus_target, gpio_pwm_duration_t(0), gpio_pwm_period(frequency), frequency, reg_pwm, reg_autoreload, reg_prescaler, log) {
	}


	template <typename Log>
	inline void gpio_smbus_pwm<Log>::set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle) {
		if (duty_cycle < gpio_pwm_duty_cycle::min || gpio_pwm_duty_cycle::max < duty_cycle) {
			throw exception<std::logic_error, Log>("gpio_smbus_pwm::set_duty_cycle() Out of range", __TAG__);
		}

		if (duty_cycle == _duty_cycle) {
			return;
		}

		_duty_cycle = duty_cycle;
		gpio_smbus_clock_frequency_t capture_compare = (_duty_cycle * _autoreload) / gpio_pwm_duty_cycle::max;

		_smbus->put_word(_smbus_target, _reg_pwm, capture_compare);
	}

	template <typename Log>
	template <typename PwmDuration>
	inline void gpio_smbus_pwm<Log>::set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle, PwmDuration duration) {
		set_duty_cycle(duty_cycle);
		std::this_thread::sleep_for(duration);
		set_duty_cycle(0);
	}


	// --------------------------------------------------------------

}
