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

#include "gpio_chip.i.h"


namespace abc {

	using gpio_fd_t			= int;

	using gpio_line_pos_t	= std::uint32_t;


	// --------------------------------------------------------------


	using gpio_line_flags_t	= std::uint64_t;

	namespace gpio_line_flag {
		constexpr gpio_line_flags_t	none					= static_cast<gpio_line_flags_t>(0);
		constexpr gpio_line_flags_t	used					= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_USED);
		constexpr gpio_line_flags_t	active_low				= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_ACTIVE_LOW);
		constexpr gpio_line_flags_t	input					= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_INPUT);
		constexpr gpio_line_flags_t	output					= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_OUTPUT);
		constexpr gpio_line_flags_t	edge_rising				= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_EDGE_RISING);
		constexpr gpio_line_flags_t	edge_falling			= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_EDGE_FALLING);
		constexpr gpio_line_flags_t	open_drain				= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_OPEN_DRAIN);
		constexpr gpio_line_flags_t	open_source				= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_OPEN_SOURCE);
		constexpr gpio_line_flags_t	bias_pull_up			= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_BIAS_PULL_UP);
		constexpr gpio_line_flags_t	bias_pull_down			= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN);
		constexpr gpio_line_flags_t	bias_disabled			= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_BIAS_DISABLED);
		// constexpr gpio_line_flags_t	event_clock_realtime	= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_EVENT_CLOCK_REALTIME);
	}


	// --------------------------------------------------------------


	using gpio_line_value_t	= std::uint32_t;

	class gpio_line_value {
	public:
		static constexpr gpio_line_value_t low			= 0x0;
		static constexpr gpio_line_value_t high			= 0x1;

		static constexpr gpio_line_value_t valid_mask	= 0x1;
		static constexpr gpio_line_value_t invalid_mask	= ~valid_mask;

	public:
		gpio_line_value(gpio_line_value_t value) noexcept;

	public:
		bool				is_valid() const noexcept;
		gpio_line_value_t	value() const noexcept;
		operator gpio_line_value_t() const noexcept;

	private:
		gpio_line_value_t	_value;
	};


	// --------------------------------------------------------------


	template <typename Log>
	class gpio_chip;

	template <typename Log>
	class gpio_line {
	protected:
		gpio_line(gpio_fd_t fd, gpio_line_flags_t flags, Log* log);

	protected:
		gpio_line_value		get_value() noexcept;
		bool				set_value(const gpio_line_value& value) noexcept;

	public:
		bool				is_valid() const noexcept;

	private:
		gpio_fd_t			_fd;
		Log*				_log;
	};


	// --------------------------------------------------------------


	template <typename Log>
	class gpio_input_line : public gpio_line<Log> {
		using base = gpio_line<Log>;

	protected:
		friend class gpio_chip<Log>;
		gpio_input_line(gpio_fd_t fd, Log* log);

	public:
		gpio_line_value	get_value() noexcept;
	};


	// --------------------------------------------------------------


	template <typename Log>
	class gpio_output_line : public gpio_line<Log> {
		using base = gpio_line<Log>;

	protected:
		friend class gpio_chip<Log>;
		gpio_output_line(gpio_fd_t fd, Log* log);

	public:
		bool	set_value(const gpio_line_value& value) noexcept;
	};


	// --------------------------------------------------------------

}
