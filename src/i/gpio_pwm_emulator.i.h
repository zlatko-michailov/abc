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
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <linux/gpio.h>

#include "gpio_chip.i.h"


namespace abc {

	using gpio_pwm_frequency_t  = std::uint16_t;


	// --------------------------------------------------------------


	using gpio_pwm_duty_cycle_t = std::uint16_t;

	namespace gpio_pwm_duty_cycle {
		constexpr gpio_pwm_duty_cycle_t min =   0;
		constexpr gpio_pwm_duty_cycle_t max = 100;
	}


	// --------------------------------------------------------------


	template <typename PulseWidthDuration, typename Log = null_log>
	class gpio_pwm_emulator {
		using gpio_pwm_duration_t  = std::uint32_t;

		// Break const level sleeps periodically to prevent notification misses.
		const std::chrono::milliseconds	const_level_period = std::chrono::milliseconds(200);

	public:
		gpio_pwm_emulator(gpio_output_line<Log>&& line, PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width, gpio_pwm_frequency_t frequency, Log* log = nullptr);
		virtual ~gpio_pwm_emulator() noexcept;

	public:
		void				set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle);

		template <typename PwmDuration>
		void				set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle, PwmDuration duration);

	private:
		static void			thread_func(gpio_pwm_emulator* this_ptr) noexcept;

	private:
		const gpio_output_line<Log>	_line;

		// Parameters
		const PulseWidthDuration			_min_pulse_width;
		const PulseWidthDuration			_max_pulse_width;
		const gpio_pwm_frequency_t			_frequency;
		const PulseWidthDuration			_period;

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
