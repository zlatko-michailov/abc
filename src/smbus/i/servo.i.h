/*
MIT License

Copyright (c) 2018-2026 Zlatko Michailov

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
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "../../root/size.h"
#include "../../diag/i/diag_ready.i.h"
#include "controller.i.h"
#include "pwm.i.h"


namespace abc { namespace smbus {

    /**
     * @brief              Wrapper around `pwm` representing a servo connected over SMBus.
     * @tparam PwmDuration `std::duration` type of the duty cycle duration.
     */
    template <typename PwmDuration>
    class servo
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief                     Constructor.
         * @tparam PulseWidthDuration `std::duration` type of the pulse width duration.
         * @param controller          Pointer to an SMBus controller.
         * @param target              SMBus target representing the HAT to which the servo is connected.
         * @param min_pulse_width     Minimum pulse width duration.
         * @param max_pulse_width     Maximum pulse width duration.
         * @param pwm_duration        Duty cycle duration.
         * @param frequency           Peripheral frequency.
         * @param reg_pwm             Duty cycle register on the HAT for the servo connection.
         * @param reg_autoreload      ARR register on the HAT for the servo connection.
         * @param reg_prescaler       Prescaler register on the HAT for the servo connection.
         * @param log                 `diag::log_ostream` pointer. May be `nullptr`.
         */
        template <typename PulseWidthDuration>
        servo(controller* controller, const target& target,
            PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width,
            PwmDuration pwm_duration,
            pwm_pulse_frequency_t frequency,
            register_t reg_pwm, register_t reg_autoreload, register_t reg_prescaler,
            diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        servo(servo<PwmDuration>&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        servo(const servo<PwmDuration>& other) = delete;

    public:
        /**
         * @brief            Sets the duty cycle object
         * @param duty_cycle Duty cycle. Must be between 0 and 100.
         */
        void set_duty_cycle(pwm_duty_cycle_t duty_cycle);

    private:
        /**
         * @brief `pwm` instance representing the PWM peripheral.
         */
        pwm _pwm;

        /**
         * @brief Duty cycle duration.
         */
        PwmDuration _pwm_duration;
    };


    // --------------------------------------------------------------

} }
