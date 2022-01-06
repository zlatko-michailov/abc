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
#include "gpio_line.i.h"


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
		static constexpr std::size_t max_path 		= GPIO_MAX_NAME_SIZE;
		static constexpr std::size_t max_consumer	= GPIO_MAX_NAME_SIZE;

	public:
		gpio_chip(const char* path, const char* consumer = "abc", Log* log = nullptr);

	public:
		const char* 			path() const noexcept;
		const char* 			consumer() const noexcept;

	public:
		gpio_chip_info			chip_info() const noexcept;
		gpio_line_info			line_info(gpio_line_pos_t pos) const noexcept;

	private:
		char					_path[max_path];
		char					_consumer[max_consumer];
		Log*					_log;
	};


	// --------------------------------------------------------------

}
