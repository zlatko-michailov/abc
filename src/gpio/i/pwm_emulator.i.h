/*
MIT License

Copyright (c) 2018-2025 Zlatko Michailov

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

#include "../../diag/i/diag_ready.i.h"
#include "base.i.h"
#include "chip.i.h"
#include "line.i.h"
#include "pwm_base.i.h"


namespace abc { namespace gpio {

    /**
     * @brief   PWM emulator over a regular GPIO output line.
     * @details The emulation uses cycles on the main CPU, which may affect accuracy of the PWM as well as the overall responsiveness of the program.
     *          PWM emulation should only be used when no HAT that supports PWM is available.
     */
    class pwm_emulator
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief                     Constructor for servos or other peripherals where the pulse width must be within a given range.
         * @tparam PulseWidthDuration `std::duration` type.
         * @param chip                Pointer to the `chip` instance that owns the GPIO line.
         * @param line_pos            Chip-specific line position.
         * @param min_pulse_width     Minimum pulse width.
         * @param max_pulse_width     Maximum pulse width.
         * @param frequency           Signal frequency.
         * @param log                 `diag::log_ostream` pointer. May be `nullptr`.
         */
        template <typename PulseWidthDuration>
        pwm_emulator(const chip* chip, line_pos_t line_pos, PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width, pwm_pulse_frequency_t frequency, diag::log_ostream* log = nullptr);

        /**
         * @brief           Constructor for motors or other peripherals where the pulse width is not restricted.
         * @param chip      Pointer to the `chip` instance that owns the GPIO line.
         * @param line_pos  Chip-specific line position.
         * @param frequency Signal frequency.
         * @param log       `diag::log_ostream` pointer. May be `nullptr`.
         */
        pwm_emulator(const chip* chip, line_pos_t line_pos, pwm_pulse_frequency_t frequency, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        pwm_emulator(pwm_emulator&& other) noexcept;

        /**
         * @brief Deleted.
         */
        pwm_emulator(const pwm_emulator& other) = delete;

        /**
         * @brief Destructor.
         */
        virtual ~pwm_emulator() noexcept;

    public:
        /**
         * @brief            Sets the duty cycle using a separate thread. Returns immediately.
         * @param duty_cycle Duty cycle. Must be between 0 and 100.
         */
        void set_duty_cycle(pwm_duty_cycle_t duty_cycle);

        /**
         * @brief              Sets the duty cycle and keeps it for the given duration. Then, sets it to 0.
         * @tparam PwmDuration `std::duration` type.
         * @param duty_cycle   Duty cycle. Must be between 0 and 100.
         * @param duration     Duration for which to keep the given duty cycle.
         */
        template <typename PwmDuration>
        void set_duty_cycle(pwm_duty_cycle_t duty_cycle, PwmDuration duration);

    private:
        /**
         * @brief          Thread function that does the PWM emulation.
         * @param this_ptr Pointer to the owning instance.
         */
        static void thread_func(pwm_emulator* this_ptr) noexcept;

    private:
        /**
         * @brief GPIO output line over which PWM is emulated.
         */
        output_line _line;

        // Parameters
        /**
         * @brief Minimum pulse width.
         */
        pwm_duration _min_pulse_width;

        /**
         * @brief Maximum pulse width.
         */
        pwm_duration _max_pulse_width;

        /**
         * @brief Calculated period.
         */
        pwm_duration _period;

        // Sync
        /**
         * @brief Mutex needed for `_control_condition`.
         */
        std::mutex _control_mutex;

        /**
         * @brief Condition variable used to save CPU cycles when the duty cycle is min or max.
         */
        std::condition_variable _control_condition;

        // Controllables
        /**
         * @brief Duty cycle.
         */
        std::atomic<pwm_duty_cycle_t> _duty_cycle;

        /**
         * @brief "Quit requested" flag.
         */
        std::atomic<bool> _quit;

        /**
         * @brief The thread on which PWM is emulated.
         */
        std::thread _thread;

        /**
         * @brief Break const level sleeps periodically to prevent notification misses.
         */
        const std::chrono::milliseconds _const_level_period = std::chrono::milliseconds(200);
    };


    // --------------------------------------------------------------

} }
