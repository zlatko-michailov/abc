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

#include <thread>

#include "../diag/diag_ready.h"
#include "chip.h"
#include "line.h"
#include "chip.h"
#include "i/ultrasonic.i.h"


namespace abc { namespace gpio {

    using clock = std::chrono::steady_clock;
    using microseconds = std::chrono::microseconds;
    constexpr std::size_t sonic_speed = 343; // meters per second


    template <typename DistanceScale>
    inline ultrasonic<DistanceScale>::ultrasonic(const chip* chip, line_pos_t trigger_line_pos, line_pos_t echo_line_pos, diag::log_ostream* log)
        : diag_base("abc::gpio::ultrasonic", log)
        , _trigger_line(chip, trigger_line_pos, log)
        , _echo_line(chip, echo_line_pos, log) {

        constexpr const char* suborigin = "ultrasonic()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1070f, "Begin: trigger_line_pos=%u, echo_line_pos=%u", trigger_line_pos, echo_line_pos);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename DistanceScale>
    inline std::size_t ultrasonic<DistanceScale>::measure_distance(std::size_t max_distance) const noexcept {
        static constexpr microseconds added_timeout = microseconds(3000);
        static const microseconds timeout = added_timeout + sonic_duration<microseconds>(2 * max_distance); // back and forth

        constexpr const char* suborigin = "measure_distance()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: max_distance=%zu, us=%llu", max_distance, (long long)timeout.count());

        // Clear and send a pulse.
        _trigger_line.put_level(level::low, microseconds(10));
        _trigger_line.put_level(level::high, microseconds(10));
        _trigger_line.put_level(level::low);

        // Start the clock.
        microseconds time_left(timeout);
        clock::time_point echo_not_ready_tp = clock::now();

        // Make sure there is no echo in progress.
        level_t level = _echo_line.wait_for_level(level::low, time_left);
        clock::time_point echo_ready_tp = clock::now();
        diag_base::put_any(suborigin, diag::severity::debug, __TAG__, "1: level=%u, us=%llu", level, (long long)std::chrono::duration_cast<microseconds>(echo_ready_tp - echo_not_ready_tp).count());

        // Wait until the echo starts.
        if (level != level::invalid) {
            time_left -= std::chrono::duration_cast<microseconds>(echo_ready_tp - echo_not_ready_tp);
            level = _echo_line.wait_for_level(level::high, time_left);
        }
        clock::time_point echo_start_tp = clock::now();
        diag_base::put_any(suborigin, diag::severity::debug, __TAG__, "2: level=%u, us=%llu", level, (long long)std::chrono::duration_cast<microseconds>(echo_start_tp - echo_ready_tp).count());

        // Wait until the echo ends.
        if (level != level::invalid) {
            time_left -= std::chrono::duration_cast<microseconds>(echo_start_tp - echo_ready_tp);
            level = _echo_line.wait_for_level(level::low, time_left);
        }
        clock::time_point echo_end_tp = clock::now();
        diag_base::put_any(suborigin, diag::severity::debug, __TAG__, "3: level=%u, us=%llu", level, (long long)std::chrono::duration_cast<microseconds>(echo_end_tp - echo_start_tp).count());

        std::size_t distance = max_distance;
        if (level != level::invalid) {
            std::size_t distance_x2 = sonic_distance(std::chrono::duration_cast<microseconds>(echo_end_tp - echo_start_tp));
            distance = distance_x2 / 2;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: distance=%zu", distance);

        return distance;
    }


    template <typename DistanceScale>
    template <typename Duration>
    inline std::size_t ultrasonic<DistanceScale>::sonic_distance(Duration duration) noexcept {
        return sonic_speed * duration.count() * (Duration::period::num * DistanceScale::den) / (Duration::period::den * DistanceScale::num);
    }


    template <typename DistanceScale>
    template <typename Duration>
    inline Duration ultrasonic<DistanceScale>::sonic_duration(std::size_t distance) noexcept {
        return Duration(distance * (DistanceScale::num * Duration::period::den) / (sonic_speed * DistanceScale::den * Duration::period::num));
    }


    // --------------------------------------------------------------

} }
