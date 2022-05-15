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


namespace abc {

	template <typename Log = null_log>
	class gpio_ultrasonic {
	public:
		gpio_ultrasonic(const gpio_chip<Log>* chip, gpio_line_pos_t trigger_line_pos, gpio_line_pos_t echo_line_pos, Log* log = nullptr);
		gpio_ultrasonic(gpio_ultrasonic<Log>&& other) noexcept = default;
		gpio_ultrasonic(const gpio_ultrasonic<Log>& other) = delete;

	public:
		template <typename DistanceScale>
		std::size_t			measure_distance(std::size_t max_distance) const noexcept;

	public: //private:
		template <typename DistanceScale, typename Duration>
		static std::size_t	sonic_distance(Duration duration) noexcept;
		template <typename DistanceScale, typename Duration>
		static Duration		sonic_duration(std::size_t distance) noexcept;

	private:
		gpio_output_line<Log>	_trigger_line;
		gpio_input_line<Log>	_echo_line;
		Log*					_log;
	};


	// --------------------------------------------------------------

}
