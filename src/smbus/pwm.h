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
#include <chrono>
#include <cmath>
#include <linux/gpio.h>

#include "../diag/diag_ready.h"
#include "controller.h"
#include "i/pwm.i.h"


namespace abc { namespace smbus {

    template <typename PulseWidthDuration>
    inline pwm::pwm(controller* controller, const target& target,
                    PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width,
                    pwm_pulse_frequency_t frequency,
                    register_t reg_pwm, register_t reg_autoreload, register_t reg_prescaler,
                    diag::log_ostream* log)
        : diag_base("abc::smbus::pwm", log)
        , _controller(controller)
        , _target(target)
        , _min_pulse_width(std::chrono::duration_cast<pwm_duration>(min_pulse_width).count())
        , _max_pulse_width(std::chrono::duration_cast<pwm_duration>(max_pulse_width).count())
        , _frequency(frequency)
        , _period(0)
        , _autoreload(0)
        , _prescaler(0)
        , _reg_pwm(reg_pwm)
        , _reg_autoreload(reg_autoreload)
        , _reg_prescaler(reg_prescaler) {

        constexpr const char* suborigin = "pwm()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10705, "Begin:");

        diag_base::expect(suborigin, controller != nullptr, 0x10706, "controller != nullptr");

        // Adjust min/max_pulse_width to lock frequency.
        {
            diag_base::put_any(suborigin, diag::severity::debug, 0x10707, "(1) min=%lu, max=%lu", (long)_min_pulse_width, (long)_max_pulse_width);

            _min_pulse_width = (_min_pulse_width * _target.clock_frequency()) / gpio_pwm_duration::period::den;
            _max_pulse_width = (_max_pulse_width * _target.clock_frequency()) / gpio_pwm_duration::period::den;

            diag_base::put_any(suborigin, diag::severity::debug, 0x10708, "(2) min=%lu, max=%lu", (long)_min_pulse_width, (long)_max_pulse_width);
        }

        // Calculate auto_reload and prescaler.
        _period = _target.clock_frequency() / _frequency;
        _autoreload = get_autoreload_from_period(_period);
        _prescaler = _period / _autoreload;

        // Adjust min/max_pulse_width by auto_reload and prescaler.
        if (_max_pulse_width == 0) {
            _max_pulse_width = _autoreload;
        }
        else {
            _min_pulse_width /= _prescaler;
            _max_pulse_width /= _prescaler;
        }

        diag_base::put_any(suborigin, diag::severity::debug, 0x10709, "(3) period=%lu, autoreload=%lu, prescaler=%lu, min=%lu, max=%lu",
                (long)_period, (long)_autoreload, (long)_prescaler, (long)_min_pulse_width, (long)_max_pulse_width);

        _controller->put_word(_target, _reg_autoreload, _autoreload - 1);
        _controller->put_word(_target, _reg_prescaler, _prescaler - 1);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1070a, "End:");
    }


    inline pwm::pwm(controller* controller, const target& target,
                    pwm_pulse_frequency_t frequency,
                    register_t reg_pwm, register_t reg_autoreload, register_t reg_prescaler,
                    diag::log_ostream* log)
        : pwm(controller, target, pwm_duration(0), pwm_duration(0), frequency, reg_pwm, reg_autoreload, reg_prescaler, log) {
    }


    inline void pwm::set_duty_cycle(pwm_duty_cycle_t duty_cycle) {
        constexpr const char* suborigin = "set_duty_cycle()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::expect(suborigin, duty_cycle >= pwm_duty_cycle::min, 0x1070b, "duty_cycle >= pwm_duty_cycle::min");
        diag_base::expect(suborigin, duty_cycle <= pwm_duty_cycle::max, 0x1070c, "duty_cycle <= pwm_duty_cycle::max");

        clock_frequency_t capture_compare = pwm_duty_cycle::min;

        if (duty_cycle == pwm_duty_cycle::min) {
            capture_compare = 0;
        }
        else if (duty_cycle == pwm_duty_cycle::max) {
            capture_compare = _autoreload;
        }
        else {
            capture_compare = _min_pulse_width + (duty_cycle * (_max_pulse_width - _min_pulse_width)) / pwm_duty_cycle::max;
        }

        diag_base::put_any(suborigin, diag::severity::debug, 0x1070d, "capture_compare = %lu", (long)capture_compare);

        _controller->put_word(_target, _reg_pwm, capture_compare);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename PwmDuration>
    inline void pwm::set_duty_cycle(pwm_duty_cycle_t duty_cycle, PwmDuration duration) {
        constexpr const char* suborigin = "set_duty_cycle(duration)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        set_duty_cycle(duty_cycle);
        std::this_thread::sleep_for(duration);
        set_duty_cycle(pwm_duty_cycle::min);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline clock_frequency_t pwm::get_autoreload_from_period(clock_frequency_t period) noexcept {
        std::uint64_t u64_period = static_cast<std::uint64_t>(period);

        int autoreload_bit_count = 0;
        while (u64_period != 0) {
            u64_period >>= 2;
            autoreload_bit_count++;
        }

        return static_cast<clock_frequency_t>(((1ULL << autoreload_bit_count) / 1000ULL) * 1000ULL);
    }


    // --------------------------------------------------------------

} }
