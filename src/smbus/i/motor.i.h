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
#include <ratio>
#include <chrono>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "../../root/size.h"
#include "../../diag/i/diag_ready.i.h"
#include "../../gpio/i/chip.i.h"
#include "../../gpio/i/line.i.h"
#include "controller.i.h"
#include "pwm.i.h"


namespace abc { namespace smbus {

    /**
     * @brief Wrapper around `pwm` representing a motor connected over SMBus.
     */
    class motor
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief                    Constructor.
         * @param chip               Pointer to a `chip` instance where the direction line is.
         * @param direction_line_pos Chip-specific position of the direction line.
         * @param controller         Pointer to an SMBus controller.
         * @param target             SMBus target representing the HAT to which the servo is connected.
         * @param frequency          Peripheral frequency.
         * @param reg_pwm            Duty cycle register on the HAT for the motor connection.
         * @param reg_autoreload     ARR register on the HAT for the motor connection.
         * @param reg_prescaler      Prescaler register on the HAT for the motor connection.
         * @param log                `diag::log_ostream` pointer. May be `nullptr`.
         */
        motor(const gpio::chip* chip, gpio::line_pos_t direction_line_pos,
            controller* controller, const target& target,
            pwm_pulse_frequency_t frequency,
            register_t reg_pwm, register_t reg_autoreload, register_t reg_prescaler,
            diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        motor(motor&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        motor(const motor& other) = delete;

    public:
        /**
         * @brief         Set the direction on the motor.
         * @param forward `true` = forward. `false` = backward.
         */
        void set_forward(bool forward);

        /**
         * @brief Returns whether the motor is set to turn forward.
         */
        bool is_forward() const noexcept;

        /**
         * @brief            Sets the duty cycle on the motor.
         * @param duty_cycle Duty cycle. Must be between 0 and 100.
         */
        void set_duty_cycle(pwm_duty_cycle_t duty_cycle);

        /**
         * @brief Returns the duty cycle on the motor.
         */
        pwm_duty_cycle_t get_duty_cycle() const noexcept;

    private:
        /**
         * @brief `gpio::output_line` instance representing the direction line.
         */
        gpio::output_line _direction_line;

        /**
         * @brief `pwm` instance representing the PWM peripheral.
         */
        pwm _pwm;

        /**
         * @brief Current direction of the motor.
         */
        bool _forward;

        /**
         * @brief Current duty cycle on the motor.
         */
        pwm_duty_cycle_t _duty_cycle;
    };


    // --------------------------------------------------------------

} }
