/*
MIT License

Copyright (c) 2018-2022 Zlatko Michailov

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

#include "gpio_base.i.h"
#include "gpio_smbus.i.h"


namespace abc {

	using gpio_smbus_motion_channel_t = std::uint16_t;

	namespace gpio_smbus_motion_channel {
		constexpr gpio_smbus_motion_channel_t accel_x		= 0x0001;
		constexpr gpio_smbus_motion_channel_t accel_y		= 0x0002;
		constexpr gpio_smbus_motion_channel_t accel_z		= 0x0004;

		constexpr gpio_smbus_motion_channel_t gyro_x		= 0x0008;
		constexpr gpio_smbus_motion_channel_t gyro_y		= 0x0010;
		constexpr gpio_smbus_motion_channel_t gyro_z		= 0x0020;

		constexpr gpio_smbus_motion_channel_t temperature	= 0x0040;

		constexpr gpio_smbus_motion_channel_t all			= 0x0080 - 1;
		constexpr gpio_smbus_motion_channel_t mask			= 0xffff;
	}


	// --------------------------------------------------------------


	using gpio_smbus_motion_value_t = double;

	/**
	 * @brief					Values, ready to use, from the motion sensor.
	 */
	struct gpio_smbus_motion_values {
		/**
		 * @brief				Acceleration. Units: cm/sec^2. Range: -/+16 g.
		 */
		gpio_smbus_motion_value_t accel_x		= 0.0;
		gpio_smbus_motion_value_t accel_y		= 0.0;
		gpio_smbus_motion_value_t accel_z		= 0.0;

		/**
		 * @brief				Gyro. Units: degrees/sec. Range: -/+2000.
		 */
		gpio_smbus_motion_value_t gyro_x		= 0.0;
		gpio_smbus_motion_value_t gyro_y		= 0.0;
		gpio_smbus_motion_value_t gyro_z		= 0.0;

		/**
		 * @brief				Temperature. Units: degrees C.
		 */
		gpio_smbus_motion_value_t temperature	= 0.0;
	};


	using gpio_smbus_motion_measurement_t = std::int16_t;

	/**
	 * @brief					Raw measurements from the sensor's channels.
	 */
	struct gpio_smbus_motion_measurements {
		gpio_smbus_motion_measurement_t accel_x		= 0;
		gpio_smbus_motion_measurement_t accel_y		= 0;
		gpio_smbus_motion_measurement_t accel_z		= 0;

		gpio_smbus_motion_measurement_t gyro_x		= 0;
		gpio_smbus_motion_measurement_t gyro_y		= 0;
		gpio_smbus_motion_measurement_t gyro_z		= 0;

		gpio_smbus_motion_measurement_t temperature	= 0;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					Motion sensor MPU-6000/MPU-6050, a.k.a. MPU-60X0, sensor connected over SMBus.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log = null_log>
	class gpio_smbus_motion {
	private:
		static constexpr gpio_smbus_register_t				reg_pwr_mgmt_1		= 0x6b;

		static constexpr gpio_smbus_register_t				reg_config			= 0x1a;
		static constexpr gpio_smbus_register_t				reg_config_gyro		= 0x1b;
		static constexpr gpio_smbus_register_t				reg_config_accel	= 0x1c;

		static constexpr gpio_smbus_register_t				reg_accel_x			= 0x3b;
		static constexpr gpio_smbus_register_t				reg_accel_y			= 0x3d;
		static constexpr gpio_smbus_register_t				reg_accel_z			= 0x3f;

		static constexpr gpio_smbus_register_t				reg_gyro_x			= 0x43;
		static constexpr gpio_smbus_register_t				reg_gyro_y			= 0x45;
		static constexpr gpio_smbus_register_t				reg_gyro_z			= 0x47;

		static constexpr gpio_smbus_register_t				reg_temperature		= 0x41;


		// --------------------------------------------------------------


		static constexpr gpio_smbus_motion_measurement_t	g					= 100.0 * 9.8067;	// cm/sec^2
		static constexpr gpio_smbus_motion_measurement_t	max_measurement		= 0x7fffU;
		static constexpr gpio_smbus_motion_measurement_t	max_accel			= 16 * g;			// 16 g
		static constexpr gpio_smbus_motion_measurement_t	max_gyro			= 2000;				// 2000 degrees / sec


		// --------------------------------------------------------------


		static constexpr gpio_smbus_address_t				addr				= 0x68;
		static constexpr gpio_smbus_clock_frequency_t		clock_frequency		= 1 * std::kilo::num; // Not true, but doesn't matter.
		static constexpr bool								requires_byte_swap	= true;


		// --------------------------------------------------------------


	public:
		/**
		 * @brief				Constructor.
		 * @param smbus			Pointer to a `gpio_smbus` instance.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		gpio_smbus_motion(gpio_smbus<Log>* smbus, Log* log = nullptr);

		/**
		 * @brief				Constructor.
		 * @param smbus			Pointer to a `gpio_smbus` instance.
		 * @param smbus_target	SMBus target representing the motion sensor.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		gpio_smbus_motion(gpio_smbus<Log>* smbus, const gpio_smbus_target<Log>& smbus_target, Log* log = nullptr);

		/**
		 * @brief				Move constructor.
		 */
		gpio_smbus_motion(gpio_smbus_motion<Log>&& other) noexcept = default;

		/**
		 * @brief				Deleted.
		 */
		gpio_smbus_motion(const gpio_smbus_motor<Log>& other) = delete;

	public:
		/**
		 * @brief				Takes a snapshot of the sensor channel values.
		 * @details				These values will be subtracted from further readings.
		 *						This method should be called when the object is still and ideally horizontal.
		 * @param mask			Bit mask of channels of interest.
		*/
		void calibrate(gpio_smbus_motion_channel_t mask) noexcept;

		/**
		 * @brief				Gets the normalized values of the sensor's channels.
		 * @param mask			Bit mask of channels of interest.
		 * @param values		Normalized values returned.
		*/
		void get_values(gpio_smbus_motion_channel_t mask, gpio_smbus_motion_values& values) noexcept;

		/**
		 * @brief				Gets the raw measurements of the sensor's channels.
		 * @param mask			Bit mask of channels of interest.
		 * @param measurements	Raw measurements returned.
		*/
		void get_measurements(gpio_smbus_motion_channel_t mask, gpio_smbus_motion_measurements& measurements) noexcept;

		/**
		 * @brief				Converts raw measurements to a normalized values.
		 * @param mask			Bit mask of channels of interest.
		 * @param measurements	Raw measurements.
		 * @param calibration	Raw calibration measurements.
		 * @param values		Normalized values returned.
		*/
		void get_values_from_measurements(gpio_smbus_motion_channel_t mask, const gpio_smbus_motion_measurements& measurements, const gpio_smbus_motion_measurements& calibration, gpio_smbus_motion_values& values) noexcept;

		/**
		 * @brief				Converts a raw measurement to a normalized value.
		 * @param measurement	Raw measurement.
		 * @param calibration	Raw calibration measurement.
		 * @param max_value		Max value for this channel.
		*/
		gpio_smbus_motion_value_t get_value_from_measurement(gpio_smbus_motion_measurement_t measurement, gpio_smbus_motion_measurement_t calibration, gpio_smbus_motion_value_t max_value) noexcept;

		/**
		 * @brief				Returns a reference to the raw calibration measurements.
		*/
		const gpio_smbus_motion_measurements& calibration() const noexcept;

	private:
		/**
		 * @brief				Pointer to the `gpio_smbus` instance passed in to the constructor.
		 */
		gpio_smbus<Log>* _smbus;

		/**
		 * @brief				Copy of the `gpio_smbus_target` passed in to the constructor.
		 */
		gpio_smbus_target<Log> _smbus_target;

		/**
		 * @brief				Raw calibration measurements.
		 */
		gpio_smbus_motion_measurements _calibration;

		/**
		 * @brief				The log passed in to the constructor.
		 */
		Log* _log;
	};


	// --------------------------------------------------------------

}
