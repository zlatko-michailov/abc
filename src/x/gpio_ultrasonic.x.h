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

#include <thread>

#include "../exception.h"
#include "../log.h"
#include "../i/gpio.i.h"


namespace abc {

	using clock = std::chrono::steady_clock;
	using microseconds = std::chrono::microseconds;
	using meters = std::ratio<1, 1>;
	constexpr std::size_t sonic_speed = 340; // meters per second


	template <typename Log>
	inline gpio_ultrasonic<Log>::gpio_ultrasonic(const gpio_chip<Log>* chip, gpio_line_pos_t trigger_line_pos, gpio_line_pos_t echo_line_pos, Log* log)
		: _trigger_line(chip, trigger_line_pos, log)
		, _echo_line(chip, echo_line_pos, log)
		, _log(log) {
		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_ultrasonic::gpio_ultrasonic() Done.");
		}
	}


	template <typename Log>
	template <typename DistanceScale>
	inline std::size_t gpio_ultrasonic<Log>::measure_distance(std::size_t max_distance) const noexcept {
		static const microseconds timeout = sonic_duration<meters, microseconds>(2); // 1 meter (back and forth)

		// Clear and send a pulse.
		_trigger_line.put_level(gpio_level::low, microseconds(10));
		_trigger_line.put_level(gpio_level::high, microseconds(10));
		_trigger_line.put_level(gpio_level::low);

		// Start the clock.
		microseconds time_left(timeout);
		clock::time_point echo_not_ready_tp = clock::now();

		// Make sure there is no echo in progress.
		gpio_level_t level = _echo_line.expect_level(gpio_level::low, time_left);
		clock::time_point echo_ready_tp = clock::now();

		// Wait until the echo starts.
		if (level != gpio_level::invalid) {
			time_left -= std::chrono::duration_cast<microseconds>(echo_ready_tp - echo_not_ready_tp);
			level = _echo_line.expect_level(gpio_level::high, time_left);
		}
		clock::time_point echo_start_tp = clock::now();

		// Wait until the echo ends.
		if (level != gpio_level::invalid) {
			time_left -= std::chrono::duration_cast<microseconds>(echo_start_tp - echo_ready_tp);
			level = _echo_line.expect_level(gpio_level::low, time_left);
		}
		clock::time_point echo_end_tp = clock::now();

		if (level == gpio_level::invalid) {
			if (_log != nullptr) {
				_log->put_any(category::abc::gpio, severity::abc::important, __TAG__, "gpio_ultrasonic::measure_distance() level = gpio_level::invalid, time_left = %llu us", (long long)time_left.count());
			}

			return 0;
		}

		std::size_t distance_x2 = sonic_distance<DistanceScale>(std::chrono::duration_cast<microseconds>(echo_end_tp - echo_start_tp));
		std::size_t distance = distance_x2 / 2;

		return distance;
	}


	template <typename Log>
	template <typename DistanceScale, typename Duration>
	inline std::size_t gpio_ultrasonic<Log>::sonic_distance(Duration duration) noexcept {
		return sonic_speed * duration.count() * (Duration::period::num * DistanceScale::den) / (Duration::period::den * DistanceScale::num);
	}


	template <typename Log>
	template <typename DistanceScale, typename Duration>
	inline Duration gpio_ultrasonic<Log>::sonic_duration(std::size_t distance) noexcept {
		return Duration(distance * (DistanceScale::num * Duration::period::den) / (sonic_speed * DistanceScale::den * Duration::period::num));
	}


	// --------------------------------------------------------------

}
