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
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <linux/gpio.h>

#include "gpio_chip.i.h"
#include "gpio_pwm_base.i.h"


namespace abc {

	/**
	 * @brief								PWM emulator over a regular GPIO output line.
	 * @details								The emulation uses cycles on the main CPU, which may affect accuracy of the PWM as well as the overall responsiveness of the program.
	 * 										PWM emulation should only be used when no HAT that supports PWM is available.
	 * @tparam Log							Logging facility.
	 */
	template <typename Log = null_log>
	class gpio_pwm_emulator {
		/**
		 * @brief							Break const level sleeps periodically to prevent notification misses.
		 */
		const std::chrono::milliseconds	const_level_period = std::chrono::milliseconds(200);

	public:
		/**
		 * @brief							Constructor for servos or other peripherals where the pulse width must be within a given range.
		 * @tparam PulseWidthDuration		`std::duration` type.
		 * @param chip						Pointer to the `gpio_chip` instance that owns the GPIO line.
		 * @param line_pos					Chip-specific line position.
		 * @param min_pulse_width			Minimum pulse width.
		 * @param max_pulse_width			Maximum pulse width.
		 * @param frequency					Signal frequency.
		 * @param log						Pointer to a `Log` instance. May be `nullptr`.
		 */
		template <typename PulseWidthDuration>
		gpio_pwm_emulator(const gpio_chip<Log>* chip, gpio_line_pos_t line_pos, PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width, gpio_pwm_pulse_frequency_t frequency, Log* log = nullptr);

		/**
		 * @brief							Constructor for motors or other peripherals where the pulse width is not restricted.
		 * @param chip						Pointer to the `gpio_chip` instance that owns the GPIO line.
		 * @param line_pos					Chip-specific line position.
		 * @param frequency					Signal frequency.
		 * @param log						Pointer to a `Log` instance. May be `nullptr`.
		 */
		gpio_pwm_emulator(const gpio_chip<Log>* chip, gpio_line_pos_t line_pos, gpio_pwm_pulse_frequency_t frequency, Log* log = nullptr);

		/**
		 * @brief							Move constructor.
		 */
		gpio_pwm_emulator(gpio_pwm_emulator<Log>&& other) noexcept = default;

		/**
		 * @brief							Deleted.
		 */
		gpio_pwm_emulator(const gpio_pwm_emulator<Log>& other) = delete;

		/**
		 * @brief							Destructor.
		 */
		virtual ~gpio_pwm_emulator() noexcept;

	public:
		/**
		 * @brief							Sets the duty cycle using a separate thread. Returns immediately.
		 * @param duty_cycle				Duty cycle. Must be between 0 and 100.
		 */
		void set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle) noexcept;

		/**
		 * @brief							Sets the duty cycle and keeps it for the given duration. Then, sets it to 0.
		 * @tparam PwmDuration				`std::duration` type.
		 * @param duty_cycle				Duty cycle. Must be between 0 and 100.
		 * @param duration					Duration for which to keep the given duty cycle.
		 */
		template <typename PwmDuration>
		void set_duty_cycle(gpio_pwm_duty_cycle_t duty_cycle, PwmDuration duration) noexcept;

	private:
		/**
		 * @brief							Thread function that does the PWM emulation.
		 * @param this_ptr					Pointer to the owning instance.
		 */
		static void thread_func(gpio_pwm_emulator<Log>* this_ptr) noexcept;

	private:
		/**
		 * @brief							GPIO output line over which PWM is emulated.
		 */
		gpio_output_line<Log> _line;

		// Parameters
		/**
		 * @brief							Minimum pulse width.
		 */
		gpio_pwm_duration _min_pulse_width;

		/**
		 * @brief							Maximum pulse width.
		 */
		gpio_pwm_duration _max_pulse_width;

		/**
		 * @brief							Signal frequency.
		 */
		gpio_pwm_pulse_frequency_t _frequency;

		/**
		 * @brief							Calculated period.
		 */
		gpio_pwm_duration _period;

		// Sync
		/**
		 * @brief							Mutex needed for `_control_condition`.
		 */
		std::mutex _control_mutex;

		/**
		 * @brief							Condition variable used to save CPU cycles when the duty cycle is min or max.
		 */
		std::condition_variable _control_condition;

		// Controlables
		/**
		 * @brief							Duty cycle.
		 */
		std::atomic<gpio_pwm_duty_cycle_t> _duty_cycle;

		/**
		 * @brief							"Quit requested" flag.
		 */
		std::atomic<bool> _quit;

		/**
		 * @brief							The log passed in to the constructor.
		 */
		Log* _log;

		/**
		 * @brief							The thread on which PWM is emulated.
		 */
		std::thread _thread;
	};


	// --------------------------------------------------------------

}
