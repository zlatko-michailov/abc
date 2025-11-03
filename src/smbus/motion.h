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

#include "../diag/diag_ready.h"
#include "controller.h"
#include "i/motion.i.h"


namespace abc { namespace smbus {

    inline motion::motion(controller* controller, diag::log_ostream* log)
        : motion(controller, target(addr, clock_frequency, requires_byte_swap), log) {
    }


    inline motion::motion(controller* controller, const target& target, diag::log_ostream* log)
        : diag_base("abc::smbus::motion", log)
        , _controller(controller)
        , _target(target) {

        constexpr const char* suborigin = "motion()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1074b, "Begin:");

        diag_base::expect(suborigin, controller != nullptr, 0x1074c, "controller != nullptr");

        _controller->put_byte(_target, reg_pwr_mgmt_1,   0x00);      // internal 8MHz oscillator
        _controller->put_byte(_target, reg_config,       0x03);      // Filter - 44Hz, 5ms delay
        _controller->put_byte(_target, reg_config_accel, 0x03 << 3); // +/-16g
        _controller->put_byte(_target, reg_config_gyro,  0x03 << 3); // +/-2000 degrees/sec

        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1074d, "End:");
    }


    inline void motion::calibrate(motion_channel_t mask) {
        constexpr const char* suborigin = "calibrate()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        motion_measurements measurements{ };

        constexpr int reps_skip = 5;
        constexpr int reps_take = 20;
        for (int rep = 0; rep < reps_skip + reps_take; rep++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            motion_measurements temp = get_measurements(mask & ~motion_channel::temperature);

            if (rep < reps_skip) {
                continue;
            }

            diag_base::put_any(suborigin, diag::severity::debug, 0x1074e, "mask=%x, accel_x=%x, accel_y=%x, accel_z=%x, gyro_x=%x, gyro_y=%x, gyro_z=%x, temp=%x",
                mask, temp.accel_x, temp.accel_y, temp.accel_z, temp.gyro_x, temp.gyro_y, temp.gyro_z, temp.temperature);

            measurements.accel_x += temp.accel_x;
            measurements.accel_y += temp.accel_y;
            measurements.accel_z += temp.accel_z;

            measurements.gyro_x += temp.gyro_x;
            measurements.gyro_y += temp.gyro_y;
            measurements.gyro_z += temp.gyro_z;
        }

        _calibration.accel_x = measurements.accel_x / reps_take;
        _calibration.accel_y = measurements.accel_y / reps_take;
        _calibration.accel_z = measurements.accel_z / reps_take;

        _calibration.gyro_x = measurements.gyro_x / reps_take;
        _calibration.gyro_y = measurements.gyro_y / reps_take;
        _calibration.gyro_z = measurements.gyro_z / reps_take;

        diag_base::put_any(suborigin, diag::severity::debug, 0x1074f, "mask=%x, accel_x=%x, accel_y=%x, accel_z=%x, gyro_x=%x, gyro_y=%x, gyro_z=%x, temp=%x",
            mask, _calibration.accel_x, _calibration.accel_y, _calibration.accel_z, _calibration.gyro_x, _calibration.gyro_y, _calibration.gyro_z, _calibration.temperature);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline motion_values motion::get_values(motion_channel_t mask) {
        constexpr const char* suborigin = "get_values()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        motion_measurements measurements = get_measurements(mask);

        motion_values values = get_values_from_measurements(mask, measurements, _calibration);

        diag_base::put_any(suborigin, diag::severity::debug, 0x10750, "mask=%x, accel_x=%.3f, accel_y=%.3f, accel_z=%.3f, gyro_x=%.3f, gyro_y=%.3f, gyro_z=%.3f, temp=%.2f",
            mask, values.accel_x, values.accel_y, values.accel_z, values.gyro_x, values.gyro_y, values.gyro_z, values.temperature);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        return values;
    }


    inline motion_measurements motion::get_measurements(motion_channel_t mask) {
        constexpr const char* suborigin = "get_measurements()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        motion_measurements measurements = { };

        if ((mask & motion_channel::accel_x) != 0) {
            measurements.accel_x = static_cast<motion_measurement_t>(_controller->get_word(_target, reg_accel_x));
        }

        if ((mask & motion_channel::accel_y) != 0) {
            measurements.accel_y = static_cast<motion_measurement_t>(_controller->get_word(_target, reg_accel_y));
        }

        if ((mask & motion_channel::accel_z) != 0) {
            measurements.accel_z = static_cast<motion_measurement_t>(_controller->get_word(_target, reg_accel_z));
        }

        if ((mask & motion_channel::gyro_x) != 0) {
            measurements.gyro_x = static_cast<motion_measurement_t>(_controller->get_word(_target, reg_gyro_x));
        }

        if ((mask & motion_channel::gyro_y) != 0) {
            measurements.gyro_y = static_cast<motion_measurement_t>(_controller->get_word(_target, reg_gyro_y));
        }

        if ((mask & motion_channel::gyro_z) != 0) {
            measurements.gyro_z = static_cast<motion_measurement_t>(_controller->get_word(_target, reg_gyro_z));
        }

        if ((mask & motion_channel::temperature) != 0) {
            measurements.temperature = static_cast<motion_measurement_t>(_controller->get_word(_target, reg_temperature));
        }

        diag_base::put_any(suborigin, diag::severity::debug, 0x10751, "mask=%x, accel_x=%x, accel_y=%x, accel_z=%x, gyro_x=%x, gyro_y=%x, gyro_z=%x, temp=%x",
            mask, measurements.accel_x, measurements.accel_y, measurements.accel_z, measurements.gyro_x, measurements.gyro_y, measurements.gyro_z, measurements.temperature);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        return measurements;
    }


    inline motion_values motion::get_values_from_measurements(motion_channel_t mask, const motion_measurements& measurements, const motion_measurements& calibration) noexcept {
        constexpr const char* suborigin = "get_values_from_measurements()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        motion_values values = { };

        if ((mask & motion_channel::accel_x) != 0) {
            values.accel_x = get_value_from_measurement(measurements.accel_x, calibration.accel_x, max_accel);
        }

        if ((mask & motion_channel::accel_y) != 0) {
            values.accel_y = get_value_from_measurement(measurements.accel_y, calibration.accel_y, max_accel);
        }

        if ((mask & motion_channel::accel_z) != 0) {
            values.accel_z = get_value_from_measurement(measurements.accel_z, calibration.accel_z, max_accel);
        }

        if ((mask & motion_channel::gyro_x) != 0) {
            values.gyro_x = get_value_from_measurement(measurements.gyro_x, calibration.gyro_x, max_gyro);
        }

        if ((mask & motion_channel::gyro_y) != 0) {
            values.gyro_y = get_value_from_measurement(measurements.gyro_y, calibration.gyro_y, max_gyro);
        }

        if ((mask & motion_channel::gyro_z) != 0) {
            values.gyro_z = get_value_from_measurement(measurements.gyro_z, calibration.gyro_z, max_gyro);
        }

        if ((mask & motion_channel::temperature) != 0) {
            values.temperature = static_cast<motion_value_t>(static_cast<motion_measurement_t>(measurements.temperature)) / 340.0 + 36.53;
        }

        diag_base::put_any(suborigin, diag::severity::debug, 0x10752, "mask=%x, accel_x=%.3f, accel_y=%.3f, accel_z=%.3f, gyro_x=%.3f, gyro_y=%.3f, gyro_z=%.3f, temp=%.2f",
            mask, values.accel_x, values.accel_y, values.accel_z, values.gyro_x, values.gyro_y, values.gyro_z, values.temperature);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        return values;
    }


    inline motion_value_t motion::get_value_from_measurement(motion_measurement_t measurement, motion_measurement_t calibration, motion_value_t max_value) noexcept {
        motion_value_t value = max_value * static_cast<motion_value_t>(measurement - calibration) / max_measurement;

        return value;
    }


    inline const motion_measurements& motion::calibration() const noexcept {
        return _calibration;
    }


    // --------------------------------------------------------------

} }
