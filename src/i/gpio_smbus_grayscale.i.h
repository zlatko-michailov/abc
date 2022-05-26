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
#include <ratio>
#include <chrono>
#include <linux/gpio.h>

#include "gpio_base.i.h"
#include "gpio_chip.i.h"
#include "gpio_line.i.h"
#include "gpio_smbus.i.h"
#include "gpio_smbus_pwm.i.h"


namespace abc {

	template <typename Log = null_log>
	class gpio_smbus_grayscale {
	public:
		gpio_smbus_grayscale(gpio_smbus<Log>* smbus, const gpio_smbus_target<Log>& smbus_target,
					gpio_smbus_register_t reg_left, gpio_smbus_register_t reg_center, gpio_smbus_register_t reg_right,
					Log* log = nullptr);
		gpio_smbus_grayscale(gpio_smbus_grayscale<Log>&& other) noexcept = default;
		gpio_smbus_grayscale(const gpio_smbus_grayscale<Log>& other) = delete;

	public:
		void					get_values(std::uint16_t& left, std::uint16_t& center, std::uint16_t& right) noexcept;

	private:
		gpio_smbus<Log>*		_smbus;
		gpio_smbus_target<Log>	_smbus_target;
		gpio_smbus_register_t	_reg_left;
		gpio_smbus_register_t	_reg_center;
		gpio_smbus_register_t	_reg_right;
	};


	// --------------------------------------------------------------

}
