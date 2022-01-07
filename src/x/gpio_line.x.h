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

#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../exception.h"
#include "../log.h"
#include "../i/gpio.i.h"


namespace abc {

	template <typename Log>
	inline gpio_line<Log>::gpio_line(const gpio_chip<Log>& chip, gpio_line_pos_t pos, gpio_line_flags_t flags, Log* log)
		: _log(log) {
		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_line::gpio_line() Start.");
		}

		gpio_fd_t fd = open(chip.path(), O_RDONLY);
		if (fd < 0) {
			throw exception<std::logic_error, Log>("gpio_line::gpio_line() open() < 0", __TAG__);
		}

		gpio_v2_line_request line_request{ 0 };
		line_request.num_lines = 1;
		line_request.offsets[0] = pos;
		std::strncpy(line_request.consumer, chip.consumer(), GPIO_MAX_NAME_SIZE);
		line_request.config.flags = flags;


		int ret = ioctl(fd, GPIO_V2_GET_LINE_IOCTL, &line_request);
		if (ret < 0) {
			throw exception<std::runtime_error, Log>("gpio_line::gpio_line() ioctl(GPIO_V2_GET_LINE_IOCTL) < 0", __TAG__);
		}

		if (close(fd) < 0) {
			throw exception<std::runtime_error, Log>("gpio_line::gpio_line() close(fd) < 0", __TAG__);
		}

		_fd = line_request.fd;

		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_line::gpio_line() Done.");
		}
	}


	template <typename Log>
	inline gpio_bit_value_t gpio_line<Log>::get_value() const noexcept {
		gpio_v2_line_values values{ 0 };
		values.mask = gpio_bit_value::mask;
		
		int ret = ioctl(_fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &values);
		if (ret < 0) {
			return gpio_bit_value::invalid;
		}

		return (values.bits & gpio_bit_value::mask);
	}


	template <typename Log>
	inline gpio_bit_value_t gpio_line<Log>::put_value(gpio_bit_value_t value) const noexcept {
		if ((value & ~gpio_bit_value::mask) != 0) {
			return gpio_bit_value::invalid;
		}

		gpio_v2_line_values values{ 0 };
		values.mask = gpio_bit_value::mask;
		values.bits = (value & gpio_bit_value::mask);

		int ret = ioctl(_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &values);
		if (ret < 0) {
			return gpio_bit_value::invalid;
		}

		return value;
	}


	// --------------------------------------------------------------


	template <typename Log>
	inline gpio_input_line<Log>::gpio_input_line(const gpio_chip<Log>& chip, gpio_line_pos_t pos, Log* log)
		: base(chip, pos, gpio_line_flag::input, log) {
	}


	// --------------------------------------------------------------


	template <typename Log>
	inline gpio_output_line<Log>::gpio_output_line(const gpio_chip<Log>& chip, gpio_line_pos_t pos, Log* log)
		: base(chip, pos, gpio_line_flag::output, log) {
	}


	// --------------------------------------------------------------

}
