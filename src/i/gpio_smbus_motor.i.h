/*
MIT License

Copyright (c) 2018-2024 Zlatko Michailov

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

	/**
	 * @brief							Wrapper around `gpio_smbus_pwm` representing a motor connected over SMBus.
	 * @tparam Log						Logging facility.
	 */
	template <typename Log = null_log>
	class gpio_smbus_motor {
	public:
		/**
		 * @brief						Constructor.
		 * @param chip					Pointer to a `gpio_chip` instance where the direction line is.
		 * @param direction_line_pos	Chip-specific position of the direction line.
		 * @param smbus					Pointer to a `gpio_smbus` instance.
		 * @param smbus_target			SMBus target representing the HAT to which the motor is connected.
		 * @param frequency				Signal frequency.
		 * @param reg_pwm				Duty cycle register on the HAT for the motor connection.
		 * @param reg_autoreload		ARR register on the HAT for the motor connection.
		 * @param reg_prescaler			Prescaler register on the HAT for the motor connection.
		 * @param log					Pointer to a `Log` instance. May be `nullptr`.
		 */
		gpio_smbus_motor(const gpio_chip<Log>* chip, gpio_line_pos_t direction_line_pos,
					gpio_smbus<Log>* smbus, const gpio_smbus_target<Log>& smbus_target,
					gpio_pwm_pulse_frequency_t frequency,
					gpio_smbus_register_t reg_pwm, gpio_smbus_register_t reg_autoreload, gpio_smbus_register_t reg_prescaler,
					Log* log = nullptr);

		/**
		 * @brief						Move constructor.
		 */
		gpio_smbus_motor(gpio_smbus_motor<Log>&& other) noexcept = default;

		/**
		 * @brief						Deleted.
		 */
		gpio_smbus_motor(const gpio_smbus_motor<Log>& other) = delete;

	public:
		/**
		 * @brief						Set the direction on the motor.
		 * @param forward				true = forward. false = backward.
		 */
		void set_forward(bool forward) noexcept;

		/**
		 * @brief						Returns whether the motor is set to turn forward.
		 */
		bool is_forward() const noexcept;

		/**
		 * @brief						Sets the duty cycle on the motor.
		 * @param duty_cycle			Duty cycle value. Must be between 0 and 100.
		 */
		void set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle) noexcept;

		/**
		 * @brief						Returns the duty cycle on the motor.
		 */
		gpio_pwm_duty_cycle_t get_duty_cycle() const noexcept;

	private:
		/**
		 * @brief						`gpio_output_line` instance representing the direction line.
		 */
		gpio_output_line<Log> _direction_line;

		/**
		 * @brief						`gpio_smbus_pwm` instance representing the PWM peripheral.
		 */
		gpio_smbus_pwm<Log> _pwm;

		/**
		 * @brief						Current direction of the motor.
		 */
		bool _forward;

		/**
		 * @brief						Current duty cycle on the motor.
		 */
		gpio_pwm_duty_cycle_t _duty_cycle;

		/**
		 * @brief						The log passed in to the constructor.
		 */
		Log* _log;
	};


	// --------------------------------------------------------------

}
