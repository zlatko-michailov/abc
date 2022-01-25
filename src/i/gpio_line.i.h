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
#include <linux/gpio.h>

#include "gpio_base.i.h"
#include "gpio_chip.i.h"


namespace abc {

	using gpio_line_pos_t	= std::uint32_t;


	// --------------------------------------------------------------



	using gpio_level_t	= std::uint32_t;

	namespace gpio_level {
		static constexpr gpio_level_t mask			= 0x1;

		static constexpr gpio_level_t low			= 0x0;
		static constexpr gpio_level_t high			= 0x1;
		static constexpr gpio_level_t invalid		= mask + 1;
	}


	// --------------------------------------------------------------


	template <typename Log>
	class gpio_chip;

	template <typename Log = null_log>
	class gpio_line {
	public:
		gpio_line(const gpio_chip<Log>& chip, gpio_line_pos_t pos, gpio_line_flags_t flags, Log* log = nullptr);
		gpio_line(gpio_line<Log>&& other) noexcept = default;
		gpio_line(const gpio_line<Log>& other) = delete;

		virtual ~gpio_line() noexcept;

	public:
		gpio_level_t	get_level() const noexcept;
		template <typename Duration>
		gpio_level_t	expect_level(gpio_level_t level, Duration timeout) const noexcept;

		gpio_level_t	put_level(gpio_level_t level) const noexcept;
		template <typename Duration>
		gpio_level_t	put_level(gpio_level_t level, Duration duration) const noexcept;

	private:
		gpio_fd_t		_fd		= -1;
		Log*			_log;
	};


	// --------------------------------------------------------------


	template <typename Log = null_log>
	class gpio_input_line : public gpio_line<Log> {
		using base = gpio_line<Log>;

	public:
		gpio_input_line(const gpio_chip<Log>& chip, gpio_line_pos_t pos, Log* log = nullptr);

	public:
		gpio_level_t	put_level(gpio_level_t level) const noexcept = delete;
		template <typename Duration>
		gpio_level_t	put_level(gpio_level_t level, Duration duration) const noexcept = delete;
	};


	// --------------------------------------------------------------


	template <typename Log = null_log>
	class gpio_output_line : public gpio_line<Log> {
		using base = gpio_line<Log>;

	public:
		gpio_output_line(const gpio_chip<Log>& chip, gpio_line_pos_t pos, Log* log = nullptr);

	public:
		gpio_level_t	get_level() const noexcept = delete;
		template <typename Duration>
		gpio_level_t	expect_level(gpio_level_t level, Duration timeout) const noexcept = delete;
	};


	// --------------------------------------------------------------

}
