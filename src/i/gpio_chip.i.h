/*
MIT License

Copyright (c) 2018-2021 Zlatko Michailov

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
#include <linux/gpio.h>

#include "../size.h"
#include "gpio_pin.i.h"


namespace abc {

	struct gpio_chip_info : public gpiochip_info {
		gpio_chip_info() noexcept;

		bool is_valid;
	};


	struct gpio_line_info : public gpio_v2_line_info {
		gpio_line_info() noexcept;

		bool is_valid;
	};


	// --------------------------------------------------------------


	template <typename Log>
	class gpio_chip {
	public:
		static constexpr std::size_t max_path = size::_64;

	public:
		gpio_chip(const char* path, Log* log = nullptr);

	public:
		const char* 			path() const noexcept;

	public:
		gpio_chip_info			chip_info() noexcept;
		gpio_line_info			line_info(gpio_line_pos_t pos) noexcept;

	public:
		gpio_input_line<Log>	open_input_line(gpio_line_pos_t pos) noexcept;
		gpio_output_line<Log>	open_output_line(gpio_line_pos_t pos) noexcept;

	private:
		char					_path[max_path + 1];
		Log*					_log;
	};


	// --------------------------------------------------------------

}
