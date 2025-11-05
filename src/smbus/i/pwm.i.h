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
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "../../root/size.h"
#include "../../diag/i/diag_ready.i.h"
#include "controller.i.h"
#include "pwm_base.i.h"


namespace abc { namespace smbus {

    /**
     * @brief PWM peripheral connected over SMBus. For motors and servo, it is better to use `motor` and `servo`.
     */
    class pwm
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

        public:
        /**
         * @brief                     Constructor for servo-like PWM peripherals.
         * @tparam PulseWidthDuration `std::duration` type of the pulse width duration.
         * @param controller          Pointer to an SMBus controller.
         * @param target              SMBus target representing the HAT to which the peripheral is connected.
         * @param min_pulse_width     Minimum pulse width duration.
         * @param max_pulse_width     Maximum pulse width duration.
         * @param frequency           Peripheral frequency.
         * @param reg_pwm             Duty cycle register on the HAT.
         * @param reg_autoreload      ARR register on the HAT.
         * @param reg_prescaler       Prescaler register on the HAT.
         * @param log                 `diag::log_ostream` pointer. May be `nullptr`.
         */
        template <typename PulseWidthDuration>
        pwm(controller* controller, const target& target,
            PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width,
            pwm_pulse_frequency_t frequency,
            register_t reg_pwm, register_t reg_autoreload, register_t reg_prescaler,
            diag::log_ostream* log = nullptr);

        /**
         * @brief                Constructor for motor-like PWM peripherals.
         * @param controller     Pointer to an SMBus controller.
         * @param target         SMBus target representing the HAT to which the peripheral is connected.
         * @param frequency      Duty cycle frequency.
         * @param reg_pwm        Duty cycle register on the HAT.
         * @param reg_autoreload ARR register on the HAT.
         * @param reg_prescaler  Prescaler register on the HAT.
         * @param log            `diag::log_ostream` pointer. May be `nullptr`.
         */
        pwm(controller* controller, const target& target,
                    pwm_pulse_frequency_t frequency,
                    register_t reg_pwm, register_t reg_autoreload, register_t reg_prescaler,
                    diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        pwm(pwm&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        pwm(const pwm& other) = delete;

    public:
        /**
         * @brief            Sets the duty cycle and returns immediately.
         * @param duty_cycle Duty cycle. Must be between 0 and 100.
         */
        void set_duty_cycle(pwm_duty_cycle_t duty_cycle);

        /**
         * @brief              Sets the duty cycle and waits for the given duration.
         * @tparam PwmDuration `std::duration` type.
         * @param duty_cycle   Duty cycle. Must be between 0 and 100.
         * @param duration     Sleep duration.
         */
        template <typename PwmDuration>
        void set_duty_cycle(pwm_duty_cycle_t duty_cycle, PwmDuration duration);

    private:
        /**
         * @brief        Calculates ARR from a PWM period.
         * @details      Returns a number that is close to sqrt(period) and is round to 1000.
         * @param period PWM period.
         */
        static clock_frequency_t get_autoreload_from_period(clock_frequency_t period) noexcept;

    private:
        /**
         * @brief Pointer to the `controller` instance passed in to the constructor.
         */
        controller* _controller;

        /**
         * @brief Copy of the target passed in to the constructor.
         */
        target _target;

        /**
         * @brief Minimum pulse width if passed in to the constructor.
         */
        clock_frequency_t _min_pulse_width;

        /**
         * @brief Maximum pulse width if passed in to the constructor.
         */
        clock_frequency_t _max_pulse_width;

        /**
         * @brief PWM frequency passed in to the constructor.
         */
        pwm_pulse_frequency_t _frequency;

        /**
         * @brief Calculated PWM period.
         */
        clock_frequency_t _period;

        /**
         * @brief Calculated ARR.
         */
        clock_frequency_t _autoreload;

        /**
         * @brief Calculated prescaler.
         */
        clock_frequency_t _prescaler;

        /**
         * @brief PWM register on the HAT.
         */
        register_t _reg_pwm;

        /**
         * @brief ARR register on the HAT.
         */
        register_t _reg_autoreload;

        /**
         * @brief Prescaler register on the HAT.
         */
        register_t _reg_prescaler;
    };


    // --------------------------------------------------------------

} }
