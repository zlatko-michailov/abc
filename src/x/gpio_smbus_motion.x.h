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

#include <thread>

#include "../exception.h"
#include "../log.h"
#include "../i/gpio.i.h"


namespace abc {

	template <typename Log>
	inline gpio_smbus_motion<Log>::gpio_smbus_motion(gpio_smbus<Log>* smbus, Log* log)
		: gpio_smbus_motion<Log>(smbus, gpio_smbus_target<Log>(addr, clock_frequency, requires_byte_swap), log) {
	}


	template <typename Log>
	inline gpio_smbus_motion<Log>::gpio_smbus_motion(gpio_smbus<Log>* smbus, const gpio_smbus_target<Log>& smbus_target, Log* log)
		: _smbus(smbus)
		, _smbus_target(smbus_target)
		, _log(log) {
		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_motion::gpio_smbus_motion() Start.");
		}

		if (smbus == nullptr) {
			throw exception<std::logic_error, Log>("gpio_smbus_motion::gpio_smbus_motion() smbus == nullptr", __TAG__);
		}

		_smbus->put_byte(_smbus_target, reg_pwr_mgmt_1, 0x00);			// internal 8MHz oscillator
		_smbus->put_byte(_smbus_target, reg_config, 0x03);				// Filter - 44Hz, 5ms delay
		_smbus->put_byte(_smbus_target, reg_config_accel, 0x03 << 3);	// +/-16g
		_smbus->put_byte(_smbus_target, reg_config_gyro, 0x03 << 3);	// +/-2000 degrees/sec

		std::this_thread::sleep_for(std::chrono::milliseconds(20));

		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_motion::gpio_smbus_motion() Start.");
		}
	}


	template <typename Log>
	inline void gpio_smbus_motion<Log>::calibrate(gpio_smbus_motion_channel_t mask) noexcept {
		gpio_smbus_motion_measurements measurements{ };

		constexpr int reps_skip = 5;
		constexpr int reps_take = 20;
		for (int rep = 0; rep < reps_skip + reps_take; rep++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			gpio_smbus_motion_measurements temp{ };
			get_measurements(mask & ~gpio_smbus_motion_channel::temperature, temp);

			if (rep < reps_skip) {
				continue;
			}

			_log->put_any(category::abc::gpio, severity::abc::debug, __TAG__, "gpio_smbus_motion::calibrate() mask=%x, accel_x=%x, accel_y=%x, accel_z=%x, gyro_x=%x, gyro_y=%x, gyro_z=%x, temp=%x",
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

		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::debug, __TAG__, "gpio_smbus_motion::calibrate() mask=%x, accel_x=%x, accel_y=%x, accel_z=%x, gyro_x=%x, gyro_y=%x, gyro_z=%x, temp=%x",
				mask, _calibration.accel_x, _calibration.accel_y, _calibration.accel_z, _calibration.gyro_x, _calibration.gyro_y, _calibration.gyro_z, _calibration.temperature);
		}
	}


	template <typename Log>
	inline void gpio_smbus_motion<Log>::get_values(gpio_smbus_motion_channel_t mask, gpio_smbus_motion_values& values) noexcept {
		gpio_smbus_motion_measurements measurements;
		get_measurements(mask, measurements);

		get_values_from_measurements(mask, measurements, _calibration, values);

		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::debug, __TAG__, "gpio_smbus_motion::get_values() mask=%x, accel_x=%.3f, accel_y=%.3f, accel_z=%.3f, gyro_x=%.3f, gyro_y=%.3f, gyro_z=%.3f, temp=%.2f",
				mask, values.accel_x, values.accel_y, values.accel_z, values.gyro_x, values.gyro_y, values.gyro_z, values.temperature);
		}
	}


	template <typename Log>
	inline void gpio_smbus_motion<Log>::get_measurements(gpio_smbus_motion_channel_t mask, gpio_smbus_motion_measurements& measurements) noexcept {
		measurements = { };
		std::uint16_t temp_ui16;

		if ((mask & gpio_smbus_motion_channel::accel_x) != 0) {
			_smbus->get_word(_smbus_target, reg_accel_x, temp_ui16);
			measurements.accel_x = static_cast<gpio_smbus_motion_measurement_t>(temp_ui16);
		}

		if ((mask & gpio_smbus_motion_channel::accel_y) != 0) {
			_smbus->get_word(_smbus_target, reg_accel_y, temp_ui16);
			measurements.accel_y = static_cast<gpio_smbus_motion_measurement_t>(temp_ui16);
		}

		if ((mask & gpio_smbus_motion_channel::accel_z) != 0) {
			_smbus->get_word(_smbus_target, reg_accel_z, temp_ui16);
			measurements.accel_z = static_cast<gpio_smbus_motion_measurement_t>(temp_ui16);
		}

		if ((mask & gpio_smbus_motion_channel::gyro_x) != 0) {
			_smbus->get_word(_smbus_target, reg_gyro_x, temp_ui16);
			measurements.gyro_x = static_cast<gpio_smbus_motion_measurement_t>(temp_ui16);
		}

		if ((mask & gpio_smbus_motion_channel::gyro_y) != 0) {
			_smbus->get_word(_smbus_target, reg_gyro_y, temp_ui16);
			measurements.gyro_y = static_cast<gpio_smbus_motion_measurement_t>(temp_ui16);
		}

		if ((mask & gpio_smbus_motion_channel::gyro_z) != 0) {
			_smbus->get_word(_smbus_target, reg_gyro_z, temp_ui16);
			measurements.gyro_z = static_cast<gpio_smbus_motion_measurement_t>(temp_ui16);
		}

		if ((mask & gpio_smbus_motion_channel::temperature) != 0) {
			_smbus->get_word(_smbus_target, reg_temperature, temp_ui16);
			measurements.temperature = static_cast<gpio_smbus_motion_measurement_t>(temp_ui16);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::debug, __TAG__, "gpio_smbus_motion::get_measurements() mask=%x, accel_x=%x, accel_y=%x, accel_z=%x, gyro_x=%x, gyro_y=%x, gyro_z=%x, temp=%x",
				mask, measurements.accel_x, measurements.accel_y, measurements.accel_z, measurements.gyro_x, measurements.gyro_y, measurements.gyro_z, measurements.temperature);
		}
	}


	template <typename Log>
	inline void gpio_smbus_motion<Log>::get_values_from_measurements(gpio_smbus_motion_channel_t mask, const gpio_smbus_motion_measurements& measurements, const gpio_smbus_motion_measurements& calibration, gpio_smbus_motion_values& values) noexcept {
		values = { };

		if ((mask & gpio_smbus_motion_channel::accel_x) != 0) {
			values.accel_x = get_value_from_measurement(measurements.accel_x, calibration.accel_x, max_accel);
		}

		if ((mask & gpio_smbus_motion_channel::accel_y) != 0) {
			values.accel_y = get_value_from_measurement(measurements.accel_y, calibration.accel_y, max_accel);
		}

		if ((mask & gpio_smbus_motion_channel::accel_z) != 0) {
			values.accel_z = get_value_from_measurement(measurements.accel_z, calibration.accel_z, max_accel);
		}

		if ((mask & gpio_smbus_motion_channel::gyro_x) != 0) {
			values.gyro_x = get_value_from_measurement(measurements.gyro_x, calibration.gyro_x, max_gyro);
		}

		if ((mask & gpio_smbus_motion_channel::gyro_y) != 0) {
			values.gyro_y = get_value_from_measurement(measurements.gyro_y, calibration.gyro_y, max_gyro);
		}

		if ((mask & gpio_smbus_motion_channel::gyro_z) != 0) {
			values.gyro_z = get_value_from_measurement(measurements.gyro_z, calibration.gyro_z, max_gyro);
		}

		if ((mask & gpio_smbus_motion_channel::temperature) != 0) {
			values.temperature = static_cast<gpio_smbus_motion_value_t>(static_cast<gpio_smbus_motion_measurement_t>(measurements.temperature)) / 340.0 + 36.53;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::debug, __TAG__, "gpio_smbus_motion::get_values_from_measurements() mask=%x, accel_x=%.3f, accel_y=%.3f, accel_z=%.3f, gyro_x=%.3f, gyro_y=%.3f, gyro_z=%.3f, temp=%.2f",
				mask, values.accel_x, values.accel_y, values.accel_z, values.gyro_x, values.gyro_y, values.gyro_z, values.temperature);
		}
	}


	template <typename Log>
	inline gpio_smbus_motion_value_t gpio_smbus_motion<Log>::get_value_from_measurement(gpio_smbus_motion_measurement_t measurement, gpio_smbus_motion_measurement_t calibration, gpio_smbus_motion_value_t max_value) noexcept {
		gpio_smbus_motion_value_t value = max_value * static_cast<gpio_smbus_motion_value_t>(measurement - calibration) / max_measurement;

		return value;
	}


	template <typename Log>
	inline const gpio_smbus_motion_measurements& gpio_smbus_motion<Log>::calibration() const noexcept {
		return _calibration;
	}


	// --------------------------------------------------------------

}
