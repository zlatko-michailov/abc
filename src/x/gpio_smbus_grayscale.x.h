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

#include <thread>

#include "../exception.h"
#include "../log.h"
#include "../i/gpio.i.h"


namespace abc {

	template <typename Log>
	inline gpio_smbus_grayscale<Log>::gpio_smbus_grayscale(gpio_smbus<Log>* smbus, const gpio_smbus_target<Log>& smbus_target,
					gpio_smbus_register_t reg_left, gpio_smbus_register_t reg_center, gpio_smbus_register_t reg_right,
					Log* log)
		: _smbus(smbus)
		, _smbus_target(smbus_target)
		, _reg_left(reg_left)
		, _reg_center(reg_center)
		, _reg_right(reg_right) {
		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_grayscale::gpio_smbus_grayscale() Start.");
		}

		if (smbus == nullptr) {
			throw exception<std::logic_error, Log>("gpio_smbus_grayscale::gpio_smbus_grayscale() smbus == nullptr", __TAG__);
		}

		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, 0x10703, "gpio_smbus_grayscale::gpio_smbus_grayscale() Done.");
		}
	}


	template <typename Log>
	inline void gpio_smbus_grayscale<Log>::get_values(std::uint16_t& left, std::uint16_t& center, std::uint16_t& right) noexcept {
		static constexpr std::uint16_t zero = 0x0000;

		_smbus->put_word(_smbus_target, _reg_left, zero);
		_smbus->get_noreg_2(_smbus_target, left);

		_smbus->put_word(_smbus_target, _reg_center, zero);
		_smbus->get_noreg_2(_smbus_target, center);

		_smbus->put_word(_smbus_target, _reg_right, zero);
		_smbus->get_noreg_2(_smbus_target, right);
	}


	// --------------------------------------------------------------

}
