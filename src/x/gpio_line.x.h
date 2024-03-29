/*
MIT License

Copyright (c) 2018-2023 Zlatko Michailov

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
#include <chrono>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../exception.h"
#include "../log.h"
#include "../i/gpio.i.h"


namespace abc {

	using clock = std::chrono::steady_clock;


	template <typename Log>
	inline gpio_line<Log>::gpio_line(const gpio_chip<Log>* chip, gpio_line_pos_t pos, gpio_line_flags_t flags, Log* log)
		: _log(log) {
		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, 0x106c8, "gpio_line::gpio_line() Start.");
		}

		if (chip == nullptr) {
			throw exception<std::logic_error, Log>("gpio_line::gpio_line() chip == nullptr", 0x106c9);
		}

		gpio_fd_t fd = open(chip->path(), O_RDONLY);
		if (fd < 0) {
			throw exception<std::logic_error, Log>("gpio_line::gpio_line() open() < 0", 0x106ca);
		}

		gpio_line_request line_request{ };
#if ((__ABC__GPIO_VER) == 2)
		line_request.num_lines = 1;
		line_request.offsets[0] = pos;
		std::strncpy(line_request.consumer, chip->consumer(), gpio_max_consumer);
		line_request.config.flags = flags;
#else
		line_request.lines = 1;
		line_request.lineoffsets[0] = pos;
		std::strncpy(line_request.consumer_label, chip->consumer(), gpio_max_consumer);
		line_request.flags = flags;
#endif

		int ret = ioctl(fd, gpio_ioctl::get_line, &line_request);
		if (ret < 0) {
			throw exception<std::runtime_error, Log>("gpio_line::gpio_line() ioctl() < 0", 0x106cb);
		}

		if (close(fd) < 0) {
			throw exception<std::runtime_error, Log>("gpio_line::gpio_line() close(fd) < 0", 0x106cc);
		}

		_fd = line_request.fd;

		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, 0x106cd, "gpio_line::gpio_line() Done.");
		}
	}


	template <typename Log>
	inline gpio_line<Log>::~gpio_line() noexcept {
		if (_fd >= 0) {
			close(_fd);
		}
	}


	template <typename Log>
	inline gpio_level_t gpio_line<Log>::get_level() const noexcept {
		gpio_line_values values{ };
#if ((__ABC__GPIO_VER) == 2)
		values.mask = gpio_level::mask;
#endif		

		int ret = ioctl(_fd, gpio_ioctl::get_line_values, &values);
		if (ret < 0) {
			return gpio_level::invalid;
		}

#if ((__ABC__GPIO_VER) == 2)
		return (values.bits & gpio_level::mask);
#else
		return (values.values[0] & gpio_level::mask);
#endif
	}


	template <typename Log>
	template <typename Duration>
	gpio_level_t gpio_line<Log>::expect_level(gpio_level_t level, Duration timeout) const noexcept {
		clock::time_point start_tp = clock::now();
		clock::time_point current_tp = clock::now();
		gpio_level_t current_level = get_level();

		while (current_level != level) {
			if (std::chrono::duration_cast<Duration>(current_tp - start_tp) > timeout) {
				return gpio_level::invalid;
			}

			current_tp = clock::now();
			current_level = get_level();
		}

		return level;
	}


	template <typename Log>
	inline gpio_level_t gpio_line<Log>::put_level(gpio_level_t level) const noexcept {
		if ((level & ~gpio_level::mask) != 0) {
			return gpio_level::invalid;
		}

		gpio_line_values values{ };
#if ((__ABC__GPIO_VER) == 2)
		values.mask = gpio_level::mask;
		values.bits = (level & gpio_level::mask);
#else
		values.values[0] = level;
#endif

		int ret = ioctl(_fd, gpio_ioctl::set_line_values, &values);
		if (ret < 0) {
			return gpio_level::invalid;
		}

		return level;
	}


	template <typename Log>
	template <typename Duration>
	inline gpio_level_t gpio_line<Log>::put_level(gpio_level_t level, Duration duration) const noexcept {
		gpio_level_t ret = put_level(level);

		if (ret != gpio_level::invalid) {
			std::this_thread::sleep_for(duration);
		}

		return ret;
	}


	// --------------------------------------------------------------


	template <typename Log>
	inline gpio_input_line<Log>::gpio_input_line(const gpio_chip<Log>* chip, gpio_line_pos_t pos, Log* log)
		: base(chip, pos, gpio_line_flag::input, log) {
	}


	// --------------------------------------------------------------


	template <typename Log>
	inline gpio_output_line<Log>::gpio_output_line(const gpio_chip<Log>* chip, gpio_line_pos_t pos, Log* log)
		: base(chip, pos, gpio_line_flag::output, log) {
	}


	// --------------------------------------------------------------

}
