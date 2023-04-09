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
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "../size.h"
#include "gpio_base.i.h"
#include "gpio_pwm_base.i.h"
#include "gpio_smbus.i.h"


namespace abc {

	/**
	 * @brief							PWM peripheral connected over SMBus. For motors and servo, it is better to use `gpio_smbus_motor` and `gpio_smbus_servo`.
	 * @tparam Log 						Logging facility.
	 */
	template <typename Log = null_log>
	class gpio_smbus_pwm {
	public:
		/**
		 * @brief						Constructor for servo-like PWM peripherals.
		 * @tparam PulseWidthDuration	`std::duration` type of the pulse width duration.
		 * @param smbus					Pointer to a `gpio_smbus` instance.
		 * @param smbus_target			SMBus target representing the HAT to which the peripheral is connected.
		 * @param min_pulse_width		Minimum pulse width duration.
		 * @param max_pulse_width		Maximum pulse width duration.
		 * @param frequency				Duty cycle frequency.
		 * @param reg_pwm				Duty cycle register on the HAT.
		 * @param reg_autoreload		ARR register on the HAT.
		 * @param reg_prescaler			Prescaler register on the HAT.
		 * @param log					Pointer to a `Log` instance. May be `nullptr`.
		 */
		template <typename PulseWidthDuration>
		gpio_smbus_pwm(gpio_smbus<Log>* smbus, const gpio_smbus_target<Log>& smbus_target,
					PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width,
					gpio_pwm_pulse_frequency_t frequency,
					gpio_smbus_register_t reg_pwm, gpio_smbus_register_t reg_autoreload, gpio_smbus_register_t reg_prescaler,
					Log* log = nullptr);

		/**
		 * @brief						Constructor for motor-like PWM peripherals.
		 * @param smbus					Pointer to a `gpio_smbus` instance.
		 * @param smbus_target			SMBus target representing the HAT to which the peripheral is connected.
		 * @param frequency				Duty cycle frequency.
		 * @param reg_pwm				Duty cycle register on the HAT.
		 * @param reg_autoreload		ARR register on the HAT.
		 * @param reg_prescaler			Prescaler register on the HAT.
		 * @param log					Pointer to a `Log` instance. May be `nullptr`.
		 */
		gpio_smbus_pwm(gpio_smbus<Log>* smbus, const gpio_smbus_target<Log>& smbus_target,
					gpio_pwm_pulse_frequency_t frequency,
					gpio_smbus_register_t reg_pwm, gpio_smbus_register_t reg_autoreload, gpio_smbus_register_t reg_prescaler,
					Log* log = nullptr);

		/**
		 * @brief						Move constructor.
		 */
		gpio_smbus_pwm(gpio_smbus_pwm<Log>&& other) noexcept = default;

		/**
		 * @brief						Deleted.
		 */
		gpio_smbus_pwm(const gpio_smbus_pwm<Log>& other) = delete;

	public:
		/**
		 * @brief						Sets the duty cycle and returns immediately.
		 * @param duty_cycle			Duty cycle. Must be between 0 and 100.
		 */
		void set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle) noexcept;

		/**
		 * @brief						Sets the duty cycle and waits for the given duration.
		 * @tparam PwmDuration			`std::duration` type.
		 * @param duty_cycle			Duty cycle. Must be between 0 and 100.
		 * @param duration				Sleep duration.
		 */
		template <typename PwmDuration>
		void set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle, PwmDuration duration) noexcept;

	private:
		/**
		 * @brief						Calculates ARR from a PWM period.
		 * @details						Returns a number that is close to sqrt(period) and is round to 1000.
		 * @param						PWM period.
		 */
		static gpio_smbus_clock_frequency_t get_autoreload_from_period(gpio_smbus_clock_frequency_t period) noexcept;

	private:
		/**
		 * @brief						Pointer to the `gpio_smbus` instance passed in to the constructor.
		 */
		gpio_smbus<Log>* _smbus;

		/**
		 * @brief						Copy of the `gpio_smbus_target` passed in to the constructor.
		 */
		gpio_smbus_target<Log> _smbus_target;

		/**
		 * @brief						Minimum pulse width if passed in to the constructor.
		 */
		gpio_smbus_clock_frequency_t _min_pulse_width;

		/**
		 * @brief						Maximum pulse width if passed in to the constructor.
		 */
		gpio_smbus_clock_frequency_t _max_pulse_width;

		/**
		 * @brief						PWM frequency passed in to the constructor.
		 */
		gpio_pwm_pulse_frequency_t _frequency;

		/**
		 * @brief						Calculated PWM period.
		 */
		gpio_smbus_clock_frequency_t _period;

		/**
		 * @brief						Calculated ARR.
		 */
		gpio_smbus_clock_frequency_t _autoreload;

		/**
		 * @brief						Calculated prescaler.
		 */
		gpio_smbus_clock_frequency_t _prescaler;

		/**
		 * @brief						PWM register on the HAT.
		 */
		gpio_smbus_register_t _reg_pwm;

		/**
		 * @brief						ARR register on the HAT.
		 */
		gpio_smbus_register_t _reg_autoreload;

		/**
		 * @brief						Prescaler register on the HAT.
		 */
		gpio_smbus_register_t _reg_prescaler;

		/**
		 * @brief						The log passed in to the constructor.
		 */
		Log* _log;
	};


	// --------------------------------------------------------------

}
