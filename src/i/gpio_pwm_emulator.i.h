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
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <linux/gpio.h>

#include "gpio_chip.i.h"
#include "gpio_pwm_base.i.h"


namespace abc {

	template <typename Log = null_log>
	class gpio_pwm_emulator {
		// Break const level sleeps periodically to prevent notification misses.
		const std::chrono::milliseconds	const_level_period = std::chrono::milliseconds(200);

	public:
		template <typename PulseWidthDuration>
		gpio_pwm_emulator(gpio_output_line<Log>&& line, PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width, gpio_pwm_pulse_frequency_t frequency, Log* log = nullptr);
		gpio_pwm_emulator(gpio_output_line<Log>&& line, gpio_pwm_pulse_frequency_t frequency, Log* log = nullptr);
		gpio_pwm_emulator(const gpio_pwm_emulator<Log>& other) = default;
		gpio_pwm_emulator(gpio_pwm_emulator<Log>&& other) = default;
		virtual ~gpio_pwm_emulator() noexcept;

	public:
		void		set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle);

		template <typename PwmDuration>
		void		set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle, PwmDuration duration);

	private:
		static void	thread_func(gpio_pwm_emulator* this_ptr) noexcept;

	private:
		const gpio_output_line<Log>	_line;

		// Parameters
		const std::chrono::microseconds		_min_pulse_width;
		const std::chrono::microseconds		_max_pulse_width;
		const gpio_pwm_pulse_frequency_t	_frequency;
		const std::chrono::microseconds		_period;

		// Sync
		std::mutex							_control_mutex;
		std::condition_variable				_control_condition;

		// Controlables
		std::atomic<gpio_pwm_duty_cycle_t>	_duty_cycle;
		std::atomic<bool>					_quit;

		Log*								_log;

		std::thread							_thread;
	};


	// --------------------------------------------------------------

}
