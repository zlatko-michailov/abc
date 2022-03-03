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
	inline gpio_smbus_pwm<Log>::gpio_smbus_pwm(gpio_smbus<Log>* smbus, PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width, gpio_pwm_pulse_frequency_t frequency,
										gpio_smbus_address_t addr, gpio_smbus_register_t reg_pwm, gpio_smbus_register_t reg_autoreload, gpio_smbus_register_t reg_prescaler, Log* log)
		: _smbus(smbus)
		, _min_pulse_width(std::chrono::duration_cast<gpio_pwm_duration_t>(min_pulse_width))
		, _max_pulse_width(std::chrono::duration_cast<gpio_pwm_duration_t>(max_pulse_width))
		, _frequency(frequency)
		, _period(gpio_pwm_period(frequency))
		, _autoreload(0)
		, _prescaler(0)
		, _addr(addr)
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
		gpio_smbus_clock_frequency_t period_ticks = static_cast<gpio_smbus_clock_frequency_t>(_period.count());
		gpio_smbus_clock_frequency_t sqrt_period_ticks = static_cast<gpio_smbus_clock_frequency_t>(std::lround(std::sqrt(period_ticks)));
		_autoreload = (sqrt_period_ticks / 100) * 100;
		_prescaler = period_ticks / _autoreload;

		if (log != nullptr) {
			//// TODO: Reduce severity
			log->put_any(category::abc::gpio, severity::abc::important, __TAG__, "gpio_smbus_pwm::gpio_smbus_pwm() period = %lu | autoreload = %u | prescaler = %u",
				(long)_period.count(), _autoreload, _prescaler);
		}

		_smbus->put_word(_addr, _reg_autoreload, _autoreload);
		_smbus->put_word(_addr, _reg_prescaler, _prescaler);

		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_pwm::gpio_smbus_pwm() Done.");
		}
	}


	template <typename Log>
	inline gpio_smbus_pwm<Log>::gpio_smbus_pwm(gpio_smbus<Log>* smbus, gpio_pwm_pulse_frequency_t frequency,
										gpio_smbus_address_t addr, gpio_smbus_register_t reg_pwm, gpio_smbus_register_t reg_autoreload, gpio_smbus_register_t reg_prescaler, Log* log)
		: gpio_smbus_pwm<Log>(smbus, gpio_pwm_duration_t(0), gpio_pwm_period(frequency), frequency, addr, reg_pwm, reg_autoreload, reg_prescaler, log) {
	}


	template <typename Log>
	inline void gpio_smbus_pwm<Log>::set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle) {
		if (duty_cycle < gpio_pwm_duty_cycle::min || gpio_pwm_duty_cycle::max < duty_cycle) {
			throw exception<std::logic_error, Log>("gpio_smbus_pwm::set_duty_cycle() Out of range", __TAG__);
		}

		if (duty_cycle == _duty_cycle) {
			return;
		}

		//// TODO: Set CCR
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
