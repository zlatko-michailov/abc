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

	gpio_chip_info::gpio_chip_info() noexcept
		: gpiochip_info{0}
		, is_valid(false) {
	}


	gpio_line_info::gpio_line_info() noexcept
		: gpio_v2_line_info{0}
		, is_valid(false) {
	}


	// --------------------------------------------------------------


	template <typename Log>
	inline gpio_chip<Log>::gpio_chip(const char* path, Log* log)
		: _log(log) {
		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_chip::gpio_chip() Start.");
		}

		if (path == nullptr) {
			throw exception<std::logic_error, Log>("gpio_chip::gpio_chip() path == nullptr", __TAG__);
		}

		if (std::strlen(path) > max_path) {
			throw exception<std::logic_error, Log>("gpio_chip::gpio_chip() path > max_path", __TAG__);
		}

		gpio_fd_t fd = open(path, O_RDONLY);
		if (fd < 0) {
			throw exception<std::logic_error, Log>("gpio_chip::gpio_chip() open() < 0", __TAG__);
		}
		close(fd);

		std::strncpy(_path, path, sizeof(_path)/sizeof(_path[0]));

		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_chip::gpio_chip() Done.");
		}
	}


	template <typename Log>
	inline const char* gpio_chip<Log>::path() const noexcept {
		return _path;
	}


	template <typename Log>
	inline gpio_chip_info gpio_chip<Log>::chip_info() noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_chip::chip_info() Start.");
		}

		gpio_chip_info info;
		info.is_valid = false;

		gpio_fd_t fd = open(_path, O_RDONLY);
		if (fd < 0) {
			if (_log != nullptr) {
				_log->put_any(category::abc::gpio, severity::abc::important, __TAG__, "gpio_chip::chip_info() Could not open()");
			}

			return info;
		}

		int stat = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info);
		close(fd);

		if (stat < 0)
		{
			if (_log != nullptr) {
				_log->put_any(category::abc::gpio, severity::abc::important, __TAG__, "gpio_chip::chip_info() Could not ioctl(GPIO_GET_CHIPINFO_IOCTL)");
			}

			return info;
		}

		info.is_valid = true;

		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_chip::chip_info() Done.");
		}

		return info;
	}


	template <typename Log>
	inline gpio_line_info gpio_chip<Log>::line_info(gpio_line_pos_t pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_chip::line_info() Start.");
		}

		gpio_line_info info;
		info.is_valid = false;
		info.offset = pos;

		gpio_fd_t fd = open(_path, O_RDONLY);
		if (fd < 0) {
			if (_log != nullptr) {
				_log->put_any(category::abc::gpio, severity::abc::important, __TAG__, "gpio_chip::line_info() Could not open()");
			}

			return info;
		}

		int stat = ioctl(fd, GPIO_V2_GET_LINEINFO_IOCTL, &info);
		close(fd);

		if (stat < 0)
		{
			if (_log != nullptr) {
				_log->put_any(category::abc::gpio, severity::abc::important, __TAG__, "gpio_chip::line_info() Could not ioctl(GPIO_V2_GET_LINEINFO_IOCTL)");
			}

			return info;
		}

		info.is_valid = true;

		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_chip::line_info() Done.");
		}

		return info;
	}


	////	gpio_input_line<Log>		open_input_line(gpio_line_pos_t pos) noexcept;
	////	gpio_output_line<Log>	open_output_line(gpio_line_pos_t pos) noexcept;


	// --------------------------------------------------------------

}
