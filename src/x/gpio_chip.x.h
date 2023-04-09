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
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../exception.h"
#include "../log.h"
#include "../i/gpio.i.h"



namespace abc {

	inline gpio_chip_info::gpio_chip_info() noexcept
		: gpio_chip_info_base{ }
		, is_valid(false) {
	}


	inline gpio_line_info::gpio_line_info() noexcept
		: gpio_line_info_base{ }
		, is_valid(false) {
	}


	// --------------------------------------------------------------


	template <typename Log>
	inline gpio_chip<Log>::gpio_chip(int dev_gpiochip_pos, const char* consumer, Log* log)
		: _log(log) {
		char path[gpio_max_path];
		std::snprintf(path, gpio_max_path, "/dev/gpiochip%d", dev_gpiochip_pos);

		init(path, consumer);
	}


	template <typename Log>
	inline gpio_chip<Log>::gpio_chip(const char* path, const char* consumer, Log* log)
		: _log(log) {
		init(path, consumer);
	}


	template <typename Log>
	inline void gpio_chip<Log>::init(const char* path, const char* consumer) {
		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, 0x106b9, "gpio_chip::init() Start.");
		}

		if (path == nullptr) {
			throw exception<std::logic_error, Log>("gpio_chip::init() path == nullptr", 0x106ba);
		}

		if (std::strlen(path) >= gpio_max_path) {
			throw exception<std::logic_error, Log>("gpio_chip::int() path >= gpo_max_path", 0x106bb);
		}

		if (consumer == nullptr) {
			throw exception<std::logic_error, Log>("gpio_chip::init() consumer == nullptr", 0x106bc);
		}

		if (std::strlen(consumer) >= gpio_max_consumer) {
			throw exception<std::logic_error, Log>("gpio_chip::init() consumer >= gpio_max_consumer", 0x106bd);
		}

		gpio_fd_t fd = open(path, O_RDONLY);
		if (fd < 0) {
			throw exception<std::logic_error, Log>("gpio_chip::init() open() < 0", 0x106be);
		}
		close(fd);

		std::strncpy(_path, path, gpio_max_path);
		std::strncpy(_consumer, consumer, gpio_max_consumer);

		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, 0x106bf, "gpio_chip::init() Done.");
		}
	}


	template <typename Log>
	inline const char* gpio_chip<Log>::path() const noexcept {
		return _path;
	}


	template <typename Log>
	inline const char* gpio_chip<Log>::consumer() const noexcept {
		return _consumer;
	}


	template <typename Log>
	inline gpio_chip_info gpio_chip<Log>::chip_info() const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, 0x106c0, "gpio_chip::chip_info() Start.");
		}

		gpio_chip_info info;
		info.is_valid = false;

		gpio_fd_t fd = open(_path, O_RDONLY);
		if (fd < 0) {
			if (_log != nullptr) {
				_log->put_any(category::abc::gpio, severity::abc::important, 0x106c1, "gpio_chip::chip_info() Could not open()");
			}

			return info;
		}

		int stat = ioctl(fd, gpio_ioctl::get_chip_info, &info);
		close(fd);

		if (stat < 0)
		{
			if (_log != nullptr) {
				_log->put_any(category::abc::gpio, severity::abc::important, 0x106c2, "gpio_chip::chip_info() Could not ioctl()");
			}

			return info;
		}

		info.is_valid = true;

		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, 0x106c3, "gpio_chip::chip_info() Done.");
		}

		return info;
	}


	template <typename Log>
	inline gpio_line_info gpio_chip<Log>::line_info(gpio_line_pos_t pos) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, 0x106c4, "gpio_chip::line_info() Start.");
		}

		gpio_line_info info;
		info.is_valid = false;
#if ((__ABC__GPIO_VER) == 2)
		info.offset = pos;
#else
		info.line_offset = pos;
#endif

		gpio_fd_t fd = open(_path, O_RDONLY);
		if (fd < 0) {
			if (_log != nullptr) {
				_log->put_any(category::abc::gpio, severity::abc::important, 0x106c5, "gpio_chip::line_info() Could not open()");
			}

			return info;
		}

		int stat = ioctl(fd, gpio_ioctl::get_line_info, &info);
		close(fd);

		if (stat < 0)
		{
			if (_log != nullptr) {
				_log->put_any(category::abc::gpio, severity::abc::important, 0x106c6, "gpio_chip::line_info() Could not ioctl()");
			}

			return info;
		}

		info.is_valid = true;

		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, 0x106c7, "gpio_chip::line_info() Done.");
		}

		return info;
	}


	// --------------------------------------------------------------

}
