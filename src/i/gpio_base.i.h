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

#include "log.i.h"


namespace abc {

	using gpio_fd_t			= int;


	// --------------------------------------------------------------


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


#ifdef GPIO_V2_GET_LINE_IOCTL
	// GPIO v2

	#define __ABC__GPIO_VER 								2


	constexpr std::size_t gpio_max_path 					= GPIO_MAX_NAME_SIZE;
	constexpr std::size_t gpio_max_consumer					= GPIO_MAX_NAME_SIZE;


	using gpio_chip_info_base 								= gpiochip_info;
	using gpio_line_info_base 								= gpio_v2_line_info;
	using gpio_line_request									= gpio_v2_line_request;
	using gpio_line_values									= gpio_v2_line_values;
	

	using gpio_ioctl_t										= unsigned long;

	namespace gpio_ioctl {
		constexpr gpio_ioctl_t get_chip_info				= GPIO_GET_CHIPINFO_IOCTL;
		constexpr gpio_ioctl_t get_line_info				= GPIO_V2_GET_LINEINFO_IOCTL;
		constexpr gpio_ioctl_t get_line						= GPIO_V2_GET_LINE_IOCTL;
		constexpr gpio_ioctl_t get_line_values				= GPIO_V2_LINE_GET_VALUES_IOCTL;
		constexpr gpio_ioctl_t set_line_values				= GPIO_V2_LINE_SET_VALUES_IOCTL;
	}


	using gpio_line_flags_t									= std::uint64_t;

	namespace gpio_line_flag {
		constexpr gpio_line_flags_t	none					= static_cast<gpio_line_flags_t>(0);
		constexpr gpio_line_flags_t	used					= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_USED);
		constexpr gpio_line_flags_t	active_low				= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_ACTIVE_LOW);
		constexpr gpio_line_flags_t	input					= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_INPUT);
		constexpr gpio_line_flags_t	output					= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_OUTPUT);
		//constexpr gpio_line_flags_t	edge_rising				= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_EDGE_RISING);
		//constexpr gpio_line_flags_t	edge_falling			= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_EDGE_FALLING);
		constexpr gpio_line_flags_t	open_drain				= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_OPEN_DRAIN);
		constexpr gpio_line_flags_t	open_source				= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_OPEN_SOURCE);
		//constexpr gpio_line_flags_t	bias_pull_up			= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_BIAS_PULL_UP);
		//constexpr gpio_line_flags_t	bias_pull_down			= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN);
		//constexpr gpio_line_flags_t	bias_disabled			= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_BIAS_DISABLED);
		// constexpr gpio_line_flags_t	event_clock_realtime	= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_EVENT_CLOCK_REALTIME);
	}


#else

	// --------------------------------------------------------------
	// GPIO v1

	#define __ABC__GPIO_VER 								1


	constexpr std::size_t gpio_max_path 					= 32;
	constexpr std::size_t gpio_max_consumer					= 32;


	using gpio_chip_info_base								= gpiochip_info;
	using gpio_line_info_base								= gpioline_info;
	using gpio_line_request									= gpiohandle_request;
	using gpio_line_values									= gpiohandle_data;


	using gpio_ioctl_t										= unsigned long;

	namespace gpio_ioctl {
		constexpr gpio_ioctl_t get_chip_info				= GPIO_GET_CHIPINFO_IOCTL;
		constexpr gpio_ioctl_t get_line_info				= GPIO_GET_LINEINFO_IOCTL;
		constexpr gpio_ioctl_t get_line						= GPIO_GET_LINEHANDLE_IOCTL;
		constexpr gpio_ioctl_t get_line_values				= GPIOHANDLE_GET_LINE_VALUES_IOCTL;
		constexpr gpio_ioctl_t set_line_values				= GPIOHANDLE_SET_LINE_VALUES_IOCTL;
	}


	using gpio_line_flags_t									= std::uint32_t;

	namespace gpio_line_flag {
		constexpr gpio_line_flags_t	none					= static_cast<gpio_line_flags_t>(0);
		constexpr gpio_line_flags_t	used					= static_cast<gpio_line_flags_t>(GPIOLINE_FLAG_KERNEL);
		constexpr gpio_line_flags_t	active_low				= static_cast<gpio_line_flags_t>(GPIOLINE_FLAG_ACTIVE_LOW);
		constexpr gpio_line_flags_t	input					= static_cast<gpio_line_flags_t>(0);
		constexpr gpio_line_flags_t	output					= static_cast<gpio_line_flags_t>(GPIOLINE_FLAG_IS_OUT);
		//constexpr gpio_line_flags_t	edge_rising				= static_cast<gpio_line_flags_t>(GPIOLINE_FLAG_EDGE_RISING);
		//constexpr gpio_line_flags_t	edge_falling			= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_EDGE_FALLING);
		constexpr gpio_line_flags_t	open_drain				= static_cast<gpio_line_flags_t>(GPIOLINE_FLAG_OPEN_DRAIN);
		constexpr gpio_line_flags_t	open_source				= static_cast<gpio_line_flags_t>(GPIOLINE_FLAG_OPEN_SOURCE);
		//constexpr gpio_line_flags_t	bias_pull_up			= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_BIAS_PULL_UP);
		//constexpr gpio_line_flags_t	bias_pull_down			= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN);
		//constexpr gpio_line_flags_t	bias_disabled			= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_BIAS_DISABLED);
		// constexpr gpio_line_flags_t	event_clock_realtime	= static_cast<gpio_line_flags_t>(GPIO_V2_LINE_FLAG_EVENT_CLOCK_REALTIME);
	}


#endif


	// --------------------------------------------------------------

}
