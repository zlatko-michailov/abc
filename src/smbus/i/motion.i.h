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
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "../../root/size.h"
#include "../../diag/i/diag_ready.i.h"
#include "controller.i.h"


namespace abc { namespace smbus {

    using motion_channel_t = std::uint16_t;

    namespace motion_channel {
        constexpr motion_channel_t accel_x     = 0x0001;
        constexpr motion_channel_t accel_y     = 0x0002;
        constexpr motion_channel_t accel_z     = 0x0004;

        constexpr motion_channel_t gyro_x      = 0x0008;
        constexpr motion_channel_t gyro_y      = 0x0010;
        constexpr motion_channel_t gyro_z      = 0x0020;

        constexpr motion_channel_t temperature = 0x0040;

        constexpr motion_channel_t all         = 0x0080 - 1;
        constexpr motion_channel_t mask        = 0xffff;
    }


    // --------------------------------------------------------------


    using motion_value_t = double;

    /**
     * @brief Values, ready to use, from the motion sensor.
     */
    struct motion_values {
        /**
         * @brief Acceleration. Units: cm/sec^2. Range: -/+16 g.
         */
        motion_value_t accel_x = 0.0;
        motion_value_t accel_y = 0.0;
        motion_value_t accel_z = 0.0;

        /**
         * @brief Gyro. Units: degrees/sec. Range: -/+2000.
         */
        motion_value_t gyro_x = 0.0;
        motion_value_t gyro_y = 0.0;
        motion_value_t gyro_z = 0.0;

        /**
         * @brief Temperature. Units: degrees C.
         */
        motion_value_t temperature = 0.0;
    };


    // --------------------------------------------------------------


    using motion_measurement_t = std::int16_t;

    /**
     * @brief Raw measurements from the sensor's channels.
     */
    struct motion_measurements {
        motion_measurement_t accel_x = 0;
        motion_measurement_t accel_y = 0;
        motion_measurement_t accel_z = 0;

        motion_measurement_t gyro_x = 0;
        motion_measurement_t gyro_y = 0;
        motion_measurement_t gyro_z = 0;

        motion_measurement_t temperature = 0;
    };


    // --------------------------------------------------------------


    namespace motion_const {
        /**
         * @brief The standard gravity acceleration constant.
         */
        static constexpr motion_value_t g = 9.80665; // m / sec^2

        /**
         * @brief The pi constant = 180 degrees in radians.
         */
        static constexpr motion_value_t pi = 3.14159265; // rad
    }


    // --------------------------------------------------------------


    /**
     * @brief   Motion sensor MPU-6000/MPU-6050, a.k.a. MPU-60X0, sensor connected over SMBus.
     * @details Accelerations: range = [-16 .. +16], units = g (1 g = 9.80665 m / sec^2).
     *          Gyros: range = [-2000 .. +2000], units = degrees / sec.
     */
    class motion
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    private:
        static constexpr register_t reg_pwr_mgmt_1   = 0x6b;

        static constexpr register_t reg_config       = 0x1a;
        static constexpr register_t reg_config_gyro  = 0x1b;
        static constexpr register_t reg_config_accel = 0x1c;

        static constexpr register_t reg_accel_x      = 0x3b;
        static constexpr register_t reg_accel_y      = 0x3d;
        static constexpr register_t reg_accel_z      = 0x3f;

        static constexpr register_t reg_gyro_x       = 0x43;
        static constexpr register_t reg_gyro_y       = 0x45;
        static constexpr register_t reg_gyro_z       = 0x47;

        static constexpr register_t reg_temperature   = 0x41;


        // --------------------------------------------------------------


        static constexpr motion_measurement_t max_measurement = 0x7fffU;
        static constexpr motion_measurement_t max_accel       = 16;      // 16 g
        static constexpr motion_measurement_t max_gyro        = 2000;    // 2000 degrees / sec


        // --------------------------------------------------------------


        static constexpr address_t         addr               = 0x68;
        static constexpr clock_frequency_t clock_frequency    = 1 * std::kilo::num; // Not true, but doesn't matter.
        static constexpr bool              requires_byte_swap = true;


        // --------------------------------------------------------------


    public:
        /**
         * @brief            Constructor.
         * @param controller Pointer to an SMBus controller.
         * @param log        `diag::log_ostream` pointer. May be `nullptr`.
         */
        motion(controller* controller, diag::log_ostream* log = nullptr);

        /**
         * @brief            Constructor.
         * @param controller Pointer to an SMBus controller.
         * @param target     SMBus target representing the HAT to which the peripheral is connected.
         * @param log        Pointer to a `Log` instance. May be `nullptr`.
         */
        motion(controller* controller, const target& target, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        motion(motion&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        motion(const motion& other) = delete;

    public:
        /**
         * @brief      Takes a snapshot of the sensor channel values.
         * @details    These values will be subtracted from further readings.
         *             This method should be called when the object is still and ideally horizontal.
         * @param mask Bit mask of channels of interest.
        */
        void calibrate(motion_channel_t mask);

        /**
         * @brief      Gets the normalized values of the sensor's channels.
         * @details    See the class description for information on the ranges.
         * @param mask Bit mask of channels of interest.
         * @return     Normalized values.
        */
        motion_values get_values(motion_channel_t mask);

        /**
         * @brief      Gets the raw measurements of the sensor's channels.
         * @details    See the class description for information on the ranges.
         * @param mask Bit mask of channels of interest.
         * @return     Raw measurements returned.
        */
        motion_measurements get_measurements(motion_channel_t mask);

        /**
         * @brief              Converts raw measurements to a normalized values.
         * @param mask         Bit mask of channels of interest.
         * @param measurements Raw measurements.
         * @param calibration  Raw calibration measurements.
         * @return             Normalized values returned.
        */
        motion_values get_values_from_measurements(motion_channel_t mask, const motion_measurements& measurements, const motion_measurements& calibration) noexcept;

        /**
         * @brief             Converts a raw measurement to a normalized value.
         * @param measurement Raw measurement.
         * @param calibration Raw calibration measurement.
         * @param max_value   Max value for this channel.
         * @return            Normalized value returned.
        */
        motion_value_t get_value_from_measurement(motion_measurement_t measurement, motion_measurement_t calibration, motion_value_t max_value) noexcept;

        /**
         * @brief Returns a reference to the raw calibration measurements.
        */
        const motion_measurements& calibration() const noexcept;

    private:
        /**
         * @brief Pointer to the `gpio_smbus` instance passed in to the constructor.
         */
        controller* _controller;

        /**
         * @brief Copy of the `target` passed in to the constructor.
         */
        target _target;

        /**
         * @brief Raw calibration measurements.
         */
        motion_measurements _calibration;
    };


    // --------------------------------------------------------------

} }
