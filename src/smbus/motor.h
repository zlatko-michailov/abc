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

#include "../diag/diag_ready.h"
#include "../gpio/chip.h"
#include "../gpio/line.h"
#include "controller.h"
#include "pwm.h"
#include "i/motor.i.h"


namespace abc { namespace smbus {

    inline motor::motor(const gpio::chip* chip, gpio::line_pos_t direction_line_pos,
                controller* controller, const target& target,
                pwm_pulse_frequency_t frequency,
                register_t reg_pwm, register_t reg_autoreload, register_t reg_prescaler,
                diag::log_ostream* log)
        : diag_base("abc::smbus::motor", log)
        , _direction_line(chip, direction_line_pos, log)
        , _pwm(controller, target, frequency, reg_pwm, reg_autoreload, reg_prescaler, log)
        , _forward(true)
        , _duty_cycle(pwm_duty_cycle::min) {

        constexpr const char* suborigin = "motor()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10704, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void motor::set_forward(bool forward) {
        _forward = forward;
        _direction_line.put_level(_forward ? gpio::level::low : gpio::level::high);
    }


    inline bool motor::is_forward() const noexcept {
        return _forward;
    }


    inline void motor::set_duty_cycle(pwm_duty_cycle_t duty_cycle) {
        _pwm.set_duty_cycle(duty_cycle);
    }


    // --------------------------------------------------------------

} }
