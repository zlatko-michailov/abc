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

#include "../diag/diag_ready.h"
#include "chip.h"
#include "line.h"
#include "i/pwm_emulator.i.h"


namespace abc { namespace gpio {

    template <typename PulseWidthDuration>
    inline pwm_emulator::pwm_emulator(const chip* chip, line_pos_t line_pos, PulseWidthDuration min_pulse_width, PulseWidthDuration max_pulse_width, pwm_pulse_frequency_t frequency, diag::log_ostream* log)
        : diag_base("abc::gpio::pwm_emulator", log)
        , _line(chip, line_pos, log)
        , _min_pulse_width(std::chrono::duration_cast<gpio_pwm_duration>(min_pulse_width))
        , _max_pulse_width(std::chrono::duration_cast<gpio_pwm_duration>(max_pulse_width))
        , _frequency(frequency)
        , _period(pwm_period(frequency))
        , _duty_cycle(0)
        , _quit(false)
        , _thread(thread_func, this) {

        constexpr const char* suborigin = "pwm_emulator()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x106ce, "Begin:");

        diag_base::expect(suborigin, min_pulse_width <= max_pulse_width, 0x106cf, "min_pulse_width <= max_pulse_width");
        diag_base::expect(suborigin, max_pulse_width <= _period, 0x106d0, "min_pulse_width <= max_pulse_width");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106d1, "End:");
    }


    inline pwm_emulator::pwm_emulator(const chip* chip, line_pos_t line_pos, pwm_pulse_frequency_t frequency, diag::log_ostream* log)
        : pwm_emulator(chip, line_pos, pwm_duration(0), pwm_period(frequency), frequency, log) {
    }


    inline pwm_emulator::~pwm_emulator() noexcept {
        constexpr const char* suborigin = "~pwm_emulator()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x106d2, "Begin:");

        {
            _quit = true;

            _control_condition.notify_all();
        }

        _thread.join();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106d3, "End:");
    }


    inline void pwm_emulator::set_duty_cycle(pwm_duty_cycle_t duty_cycle) {
        constexpr const char* suborigin = "set_duty_cycle()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: _duty_cycle=%u, duty_cycle=%u", _duty_cycle, duty_cycle);

        if (duty_cycle == _duty_cycle) {
            diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: (noop)");
            return;
        }

        diag_base::expect(suborigin, duty_cycle >= pwm_duty_cycle::min, 0x106d4, "duty_cycle >= pwm_duty_cycle::min");
        diag_base::expect(suborigin, duty_cycle <= pwm_duty_cycle::max, 0x106d5, "duty_cycle <= pwm_duty_cycle::max");

        bool should_notify = (_duty_cycle == pwm_duty_cycle::min || _duty_cycle == pwm_duty_cycle::max);

        {
            _duty_cycle = duty_cycle;

            if (should_notify) {
                _control_condition.notify_all();
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename PwmDuration>
    inline void pwm_emulator::set_duty_cycle(pwm_duty_cycle_t duty_cycle, PwmDuration duration) {
        constexpr const char* suborigin = "set_duty_cycle(duration)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        set_duty_cycle(duty_cycle);
        std::this_thread::sleep_for(duration);
        set_duty_cycle(pwm_duty_cycle::min);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void pwm_emulator::thread_func(pwm_emulator* this_ptr) noexcept {
        using clock = std::chrono::steady_clock;

        constexpr const char* suborigin = "thread_func()";
        this_ptr->put_any(suborigin, diag::severity::callstack, 0x106d6, "Begin:");

        bool quit = this_ptr->_quit;
        pwm_duty_cycle_t duty_cycle = this_ptr->_duty_cycle;

        for (;;) {
            if (quit) {
            this_ptr->put_any(suborigin, diag::severity::optional, 0x106d7, "Quitting.");

                this_ptr->_line.put_level(level::low);
                break;
            }

            if (duty_cycle == pwm_duty_cycle::min || duty_cycle == pwm_duty_cycle::max) {
                // Constant level:
                // Set the level, and block until the duty_cycle changes.
                level_t level = duty_cycle != pwm_duty_cycle::min ? level::high : level::low;
                this_ptr->_line.put_level(level);
                {
                    std::unique_lock<std::mutex> lock(this_ptr->_control_mutex);
                    this_ptr->_control_condition.wait_for(lock, this_ptr->const_level_period);

                    quit = this_ptr->_quit;
                    duty_cycle = this_ptr->_duty_cycle;
                }
            }
            else {
                // Alternating level:
                // Calculate the time points when the level should change, and use the longer interval to refresh the control variables. 
                pwm_duration high_duration = this_ptr->_min_pulse_width + duty_cycle * (this_ptr->_max_pulse_width - this_ptr->_min_pulse_width) / pwm_duty_cycle::max;
                pwm_duration low_duration  = this_ptr->_period - high_duration;

                typename clock::time_point start_time_point = clock::now();
                typename clock::time_point high_end_time_point = start_time_point + high_duration;
                typename clock::time_point low_end_time_point = high_end_time_point + low_duration;

                // High level.
                this_ptr->_line.put_level(level::high);
                if (high_duration >= low_duration) {
                    quit = this_ptr->_quit;
                    duty_cycle = this_ptr->_duty_cycle;
                }
                std::this_thread::sleep_until(high_end_time_point);

                // Low level.
                this_ptr->_line.put_level(level::low);
                if (high_duration < low_duration) {
                    quit = this_ptr->_quit;
                    duty_cycle = this_ptr->_duty_cycle;
                }
                std::this_thread::sleep_until(low_end_time_point);
            }
        }

        this_ptr->put_any(suborigin, diag::severity::callstack, 0x106d8, "End:");
    }


    // --------------------------------------------------------------

} }
