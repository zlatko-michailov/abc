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

#include "../size.h"
#include "gpio_base.i.h"
#include "gpio_line.i.h"


namespace abc {

	struct gpio_chip_info : public gpio_chip_info_base {
		gpio_chip_info() noexcept;

		bool is_valid;
	};


	struct gpio_line_info : public gpio_line_info_base {
		gpio_line_info() noexcept;

		bool is_valid;
	};


	// --------------------------------------------------------------


	template <typename Log = null_log>
	class gpio_chip {
	public:
		gpio_chip(int dev_pos, const char* consumer = "abc", Log* log = nullptr);
		gpio_chip(const char* path, const char* consumer = "abc", Log* log = nullptr);
		gpio_chip(gpio_chip<Log>&& other) noexcept = default;
		gpio_chip(const gpio_chip<Log>& other) = default;

	private:
		void init(const char* path, const char* consumer);

	public:
		const char* 			path() const noexcept;
		const char* 			consumer() const noexcept;

	public:
		gpio_chip_info			chip_info() const noexcept;
		gpio_line_info			line_info(gpio_line_pos_t pos) const noexcept;

	private:
		char					_path[gpio_max_path];
		char					_consumer[gpio_max_consumer];
		Log*					_log;
	};


	// --------------------------------------------------------------

}
