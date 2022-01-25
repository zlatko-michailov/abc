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
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "../size.h"
#include "gpio_base.i.h"
#include "gpio_line.i.h"


namespace abc {

	using gpio_smbus_functionality_t	= unsigned long;
	using gpio_smbus_address_t			= std::uint8_t;
	using gpio_smbus_register_t			= std::uint8_t;


	// --------------------------------------------------------------


	template <typename Log = null_log>
	class gpio_smbus {
	public:
		gpio_smbus(const char* path, Log* log = nullptr);
		gpio_smbus(gpio_smbus<Log>&& other) noexcept = default;
		gpio_smbus(const gpio_smbus<Log>& other) = default;
		~gpio_smbus() noexcept;

	public:
		const char* 				path() const noexcept;
		gpio_smbus_functionality_t	functionality() const noexcept;

	public:
		bool						put_nodata(gpio_smbus_address_t addr, gpio_smbus_register_t reg) noexcept;
		bool						put_byte(gpio_smbus_address_t addr, gpio_smbus_register_t reg, std::uint8_t byte) noexcept;
		bool						put_word(gpio_smbus_address_t addr, gpio_smbus_register_t reg, std::uint16_t word) noexcept;
		bool						put_block(gpio_smbus_address_t addr, gpio_smbus_register_t reg, const void* block, std::size_t size) noexcept;

		bool						get_noreg(gpio_smbus_address_t addr, std::uint8_t& byte) noexcept;
		bool						get_byte(gpio_smbus_address_t addr, gpio_smbus_register_t reg, std::uint8_t& byte) noexcept;
		bool						get_word(gpio_smbus_address_t addr, gpio_smbus_register_t reg, std::uint16_t& word) noexcept;
		bool						get_block(gpio_smbus_address_t addr, gpio_smbus_register_t reg, void* block, std::size_t& size) noexcept;

	private:
		bool						ensure_address(gpio_smbus_address_t addr) noexcept;

	private:
		char						_path[gpio_max_path];
		gpio_fd_t					_fd;
		gpio_smbus_functionality_t	_functionality;
		gpio_smbus_address_t		_addr;
		Log*						_log;
	};


	// --------------------------------------------------------------

}
