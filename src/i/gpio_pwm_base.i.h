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

#include <cstdint>
#include <chrono>


namespace abc {

	using gpio_pwm_pulse_frequency_t	= std::uint16_t;
	using gpio_pwm_duration				= std::chrono::microseconds;


	// --------------------------------------------------------------


	using gpio_pwm_duty_cycle_t = std::uint16_t;

	namespace gpio_pwm_duty_cycle {
		constexpr gpio_pwm_duty_cycle_t min =   0;
		constexpr gpio_pwm_duty_cycle_t max = 100;
	}


	// --------------------------------------------------------------


	/**
	 * @brief			Returns the duration of the PWM period in `gpio_pwm_duration` units based on the given frequency.
	 * @param frequency	Frequency in Hz (ticks per second).
	 */
	constexpr gpio_pwm_duration gpio_pwm_period(gpio_pwm_pulse_frequency_t frequency) noexcept {
		return gpio_pwm_duration(gpio_pwm_duration::period::den / frequency);
	}


	// --------------------------------------------------------------

}
