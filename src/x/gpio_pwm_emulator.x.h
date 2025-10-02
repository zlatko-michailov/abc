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

#include <cstdint>
#include <ratio>
#include <chrono>
#include <linux/gpio.h>

#include "../log.h"
#include "../i/gpio.i.h"


namespace abc {

	template <typename Log>
	template <typename PulseWidthDuration>
	inline gpio_pwm_emulator<Log>::gpio_pwm_emulator(const chip<Log>* chip, line_pos_t line_pos, PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width, gpio_pwm_pulse_frequency_t frequency, Log* log)
		: _line(chip, line_pos, log)
		, _min_pulse_width(std::chrono::duration_cast<gpio_pwm_duration>(min_pulse_width))
		, _max_pulse_width(std::chrono::duration_cast<gpio_pwm_duration>(max_pulse_width))
		, _frequency(frequency)
		, _period(gpio_pwm_period(frequency))
		, _duty_cycle(0)
		, _quit(false)
		, _log(log)
		, _thread(thread_func, this) {
		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, 0x106ce, "gpio_pwm_emulator::gpio_pwm_emulator() Start.");
		}

		if (min_pulse_width > max_pulse_width) {
			throw exception<std::logic_error, Log>("gpio_pwm_emulator::gpio_pwm_emulator() min_pulse_width", 0x106cf);
		}

		if (max_pulse_width > _period) {
			throw exception<std::logic_error, Log>("gpio_pwm_emulator::gpio_pwm_emulator() max_pulse_width", 0x106d0);
		}

		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, 0x106d1, "gpio_pwm_emulator::gpio_pwm_emulator() Done.");
		}
	}


	template <typename Log>
	inline gpio_pwm_emulator<Log>::gpio_pwm_emulator(const chip<Log>* chip, line_pos_t line_pos, gpio_pwm_pulse_frequency_t frequency, Log* log)
		: gpio_pwm_emulator<Log>(chip, line_pos, gpio_pwm_duration(0), gpio_pwm_period(frequency), frequency, log) {
	}


	template <typename Log>
	inline gpio_pwm_emulator<Log>::~gpio_pwm_emulator() noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, 0x106d2, "gpio_pwm_emulator::~gpio_pwm_emulator() Start.");
		}

		{
			_quit = true;

			_control_condition.notify_all();
		}

		_thread.join();

		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, 0x106d3, "gpio_pwm_emulator::~gpio_pwm_emulator() Done.");
		}
	}

	template <typename Log>
	inline void gpio_pwm_emulator<Log>::set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle) noexcept {
		if (duty_cycle == _duty_cycle) {
			return;
		}

		if (duty_cycle < gpio_pwm_duty_cycle::min) {
			if (_log != nullptr) {
				_log->put_any(category::abc::gpio, severity::abc::important, 0x106d4, "gpio_pwm_emulator::set_duty_cycle() Out of range: duty_cycle=%u, min=%u, max=%u. Assuming min.",
					(unsigned)duty_cycle, (unsigned)gpio_pwm_duty_cycle::min, (unsigned)gpio_pwm_duty_cycle::max);
			}

			duty_cycle = gpio_pwm_duty_cycle::min;
		}
		else if (duty_cycle > gpio_pwm_duty_cycle::max) {
			if (_log != nullptr) {
				_log->put_any(category::abc::gpio, severity::abc::important, 0x106d5, "gpio_pwm_emulator::set_duty_cycle() Out of range: duty_cycle=%u, min=%u, max=%u. Assuming max.",
					(unsigned)duty_cycle, (unsigned)gpio_pwm_duty_cycle::min, (unsigned)gpio_pwm_duty_cycle::max);
			}

			duty_cycle = gpio_pwm_duty_cycle::max;
		}

		bool should_notify = (_duty_cycle == gpio_pwm_duty_cycle::min || _duty_cycle == gpio_pwm_duty_cycle::max);

		{
			_duty_cycle = duty_cycle;

			if (should_notify) {
				_control_condition.notify_all();
			}
		}
	}

	template <typename Log>
	template <typename PwmDuration>
	inline void gpio_pwm_emulator<Log>::set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle, PwmDuration duration) noexcept {
		set_duty_cycle(duty_cycle);
		std::this_thread::sleep_for(duration);
		set_duty_cycle(gpio_pwm_duty_cycle::min);
	}


	template <typename Log>
	inline void gpio_pwm_emulator<Log>::thread_func(gpio_pwm_emulator* this_ptr) noexcept {
		using clock = std::chrono::steady_clock;

		if (this_ptr->_log != nullptr) {
			this_ptr->_log->put_any(category::abc::gpio, severity::abc::optional, 0x106d6, "gpio_pwm_emulator::thread_func() Start.");
		}

		bool quit = this_ptr->_quit;
		gpio_pwm_duty_cycle_t duty_cycle = this_ptr->_duty_cycle;

		for (;;) {
			if (quit) {
				if (this_ptr->_log != nullptr) {
					this_ptr->_log->put_any(category::abc::gpio, severity::abc::optional, 0x106d7, "gpio_pwm_emulator::thread_func() Quitting.");
				}

				this_ptr->_line.put_level(level::low);
				break;
			}

			if (duty_cycle == gpio_pwm_duty_cycle::min || duty_cycle == gpio_pwm_duty_cycle::max) {
				// Constant level:
				// Set the level, and block until the duty_cycle changes.
				level_t level = duty_cycle != gpio_pwm_duty_cycle::min ? level::high : level::low;
				this_ptr->_line.put_level(level);
				{
					std::unique_lock<std::mutex> lock(this_ptr->_control_mutex);
					this_ptr->_control_condition.wait_for(lock, this_ptr->const_level_period);

					quit = this_ptr->_quit;
					duty_cycle = this_ptr->_duty_cycle;
				}
			}
			else {
				// Alternating level:
				// Calculate the time points when the level should change, and use the longer interval to refresh the control variables. 
				gpio_pwm_duration high_duration = this_ptr->_min_pulse_width + duty_cycle * (this_ptr->_max_pulse_width - this_ptr->_min_pulse_width) / gpio_pwm_duty_cycle::max;
				gpio_pwm_duration low_duration  = this_ptr->_period - high_duration;

				typename clock::time_point start_time_point = clock::now();
				typename clock::time_point high_end_time_point = start_time_point + high_duration;
				typename clock::time_point low_end_time_point = high_end_time_point + low_duration;

				// High level.
				this_ptr->_line.put_level(level::high);
				if (high_duration >= low_duration) {
					quit = this_ptr->_quit;
					duty_cycle = this_ptr->_duty_cycle;
				}
				std::this_thread::sleep_until(high_end_time_point);

				// Low level.
				this_ptr->_line.put_level(level::low);
				if (high_duration < low_duration) {
					quit = this_ptr->_quit;
					duty_cycle = this_ptr->_duty_cycle;
				}
				std::this_thread::sleep_until(low_end_time_point);
			}
		}

		if (this_ptr->_log != nullptr) {
			this_ptr->_log->put_any(category::abc::gpio, severity::abc::optional, 0x106d8, "gpio_pwm_emulator::thread_func() Done.");
		}
	}


	// --------------------------------------------------------------

}
