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

#include <chrono>
#include <cmath>

#include "../diag/diag_ready.h"
#include "motion.h"
#include "i/motion_tracker.i.h"


namespace abc { namespace smbus {

    template <typename DistanceScale>
    inline motion_tracker<DistanceScale>::motion_tracker(motion* motion, diag::log_ostream* log)
        : diag_base("abc::smbus::motion_tracker", log)
        , _motion(motion)
        , _depth(0)
        , _width(0)
        , _direction(0)
        , _speed(0)
        , _run(false)
        , _quit(false)
        , _thread(thread_func, this) {

        constexpr const char* suborigin = "motion_tracker()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10753, "Begin:");

        diag_base::expect(suborigin, motion != nullptr, 0x10754, "motion != nullptr");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10755, "End:");
    }


    template <typename DistanceScale>
    motion_tracker<DistanceScale>::motion_tracker(motion_tracker&& other) noexcept
        : diag_base(std::move(other))
        , _motion(other._motion)
        , _depth(other._depth.load())
        , _width(other._width.load())
        , _direction(other._direction.load())
        , _speed(other._speed.load())
        , _run(other._run.load())
        , _quit(other._quit.load())
        , _thread(std::move(other._thread)) {

        // Clear the other instance.
        other._motion = nullptr;
        other._depth = 0;
        other._width = 0;
        other._direction = 0;
        other._speed = 0;
        other._run = false;
        other._quit = true;
    }


    template <typename DistanceScale>
    inline motion_tracker<DistanceScale>::~motion_tracker() noexcept {
        constexpr const char* suborigin = "~motion_tracker()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10756, "Begin:");

        _quit = true;

        // The thread may be sleeping. Notify the condition to wake it up.
        _control_condition.notify_all();

        // Wait for the child thread to finish. std::~thread() terminates the process if the thread is running.
        _thread.join();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10757, "End:");
    }


    template <typename DistanceScale>
    inline bool motion_tracker<DistanceScale>::is_running() const noexcept {
        return _run;
    }


    template <typename DistanceScale>
    inline void motion_tracker<DistanceScale>::start() {
        _run = true;

        // The thread may be sleeping. Notify the condition to wake it up.
        _control_condition.notify_all();
    }


    template <typename DistanceScale>
    inline void motion_tracker<DistanceScale>::stop() {
        _run = false;

        // The thread is running. There is no need to notify it.
    }


    template <typename DistanceScale>
    inline motion_value_t motion_tracker<DistanceScale>::depth() const noexcept {
        return _depth;
    }


    template <typename DistanceScale>
    inline motion_value_t motion_tracker<DistanceScale>::width() const noexcept {
        return _width;
    }


    template <typename DistanceScale>
    inline motion_value_t motion_tracker<DistanceScale>::direction() const noexcept {
        return _direction;
    }


    template <typename DistanceScale>
    inline motion_value_t motion_tracker<DistanceScale>::speed() const noexcept {
        return _speed;
    }


    template <typename DistanceScale>
    inline void motion_tracker<DistanceScale>::set_depth(motion_value_t value) noexcept {
        _depth = value;
    }


    template <typename DistanceScale>
    inline void motion_tracker<DistanceScale>::set_width(motion_value_t value) noexcept {
        _width = value;
    }


    template <typename DistanceScale>
    inline void motion_tracker<DistanceScale>::set_direction(motion_value_t value) noexcept {
        _direction = value;
    }


    template <typename DistanceScale>
    inline void motion_tracker<DistanceScale>::set_speed(motion_value_t value) noexcept {
        _speed = value;
    }


    template <typename DistanceScale>
    inline void motion_tracker<DistanceScale>::thread_func(motion_tracker<DistanceScale>* this_ptr) noexcept {
        using clock = std::chrono::steady_clock;

        constexpr const char* suborigin = "thread_func()";
        this_ptr->put_any(suborigin, diag::severity::callstack, 0x10758, "Begin:");

        // Previous motion values;
        clock::time_point prev_time_point;
        motion_value_t prev_accel;
        motion_value_t prev_gyro;

        for (;;) {
            bool quit = this_ptr->_quit;
            bool run = this_ptr->_run;

            if (quit) {
                this_ptr->put_any(suborigin, diag::severity::optional, 0x10759, "Quitting (from running).");

                break;
            }

            if (!run) {
                this_ptr->put_any(suborigin, diag::severity::optional, 0x1075a, "Stopping.");

                // Reset kept measurements.
                this_ptr->_speed = 0;
                prev_accel = 0;
                prev_gyro = 0;

                // Sleep to let the inertia dies.
                std::this_thread::sleep_for(std::chrono::milliseconds(200));

                // Sleep until the owning instance fires the condition.
                {
                    std::unique_lock<std::mutex> lock(this_ptr->_control_mutex);
                    this_ptr->_control_condition.wait(lock);

                    quit = this_ptr->_quit;
                    run = this_ptr->_run;
                }

                if (quit) {
                    this_ptr->put_any(suborigin, diag::severity::optional, 0x1075b, "Quitting (from sleeping).");

                    break;
                }

                this_ptr->put_any(suborigin, diag::severity::optional, 0x1075c, "Starting.");

                prev_time_point = clock::time_point();
            }

            // Running.
            // Read the current motion values regardless.
            motion_values values = this_ptr->_motion->get_values(abc::smbus::motion_channel::accel_x | abc::smbus::motion_channel::gyro_z);

            motion_value_t curr_accel = (values.accel_x * motion_const::g * DistanceScale::den) / DistanceScale::num;
            motion_value_t curr_gyro = values.gyro_z;

            // Snap the current time point.
            clock::time_point curr_time_point = clock::now();

            // Read the atomic members once.
            motion_value_t prev_depth = this_ptr->_depth;
            motion_value_t prev_width = this_ptr->_width;
            motion_value_t prev_speed = this_ptr->_speed;
            motion_value_t prev_direction = this_ptr->_direction;

            if (prev_time_point.time_since_epoch().count() != 0) {
                // There is a previous set of measurements.
                // Do the calculations.

                // Calculate seconds as a floating point number.
                std::chrono::microseconds::rep microsec = std::chrono::duration_cast<std::chrono::microseconds>(curr_time_point - prev_time_point).count();
                motion_value_t sec = (static_cast<motion_value_t>(microsec) / std::micro::den) * std::micro::num;
                
                motion_value_t accel_accel = (curr_accel - prev_accel) / sec;
                motion_value_t distance = (prev_speed * sec) + (prev_accel * sec * sec / 2.0) + (accel_accel * sec * sec * sec / 6.0);

                motion_value_t gyro_accel = (curr_gyro - prev_gyro) / sec;
                motion_value_t gyro = (prev_gyro * sec) + (gyro_accel * sec * sec / 2.0);

                motion_value_t direction_rad = deg_to_rad(prev_direction);

                if (std::abs(gyro) < 0.000001) { // 0.001 degrees / sec
                    // Straight line.
                    this_ptr->_depth = prev_depth + distance * std::cos(direction_rad);
                    this_ptr->_width = prev_width + distance * std::sin(direction_rad);
                }
                else {
                    // Arch.
                    motion_value_t gyro_rad = deg_to_rad(gyro);
                    motion_value_t radius = distance / gyro_rad;
                    motion_value_t straight_depth = radius * std::sin(gyro_rad);
                    motion_value_t straight_width = radius * (1 - std::cos(gyro_rad));

                    this_ptr->_depth = prev_depth + straight_depth * std::cos(direction_rad) - straight_width * std::sin(direction_rad);
                    this_ptr->_width = prev_width + straight_depth * std::sin(direction_rad) + straight_width * std::cos(direction_rad);
                    this_ptr->_direction = prev_direction + gyro;
                }

                this_ptr->_speed = prev_speed + (prev_accel * sec) + (accel_accel * sec * sec / 2.0);
            }

            prev_time_point = curr_time_point;
            prev_accel = curr_accel;
            prev_gyro = curr_gyro;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        this_ptr->put_any(suborigin, diag::severity::callstack, 0x1075d, "End:");
    }


    template <typename DistanceScale>
    inline  motion_value_t motion_tracker<DistanceScale>::deg_to_rad(motion_value_t deg) noexcept {
        return (motion_const::pi * deg) / 180.0;
    }

} }
