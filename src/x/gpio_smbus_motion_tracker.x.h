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

#include <chrono>
#include <cmath>

#include "../log.h"
#include "../i/gpio_smbus_motion_tracker.i.h"


namespace abc {

	template <typename DistanceScale, typename Log>
	inline gpio_smbus_motion_tracker<DistanceScale, Log>::gpio_smbus_motion_tracker(gpio_smbus_motion<Log>* motion, Log* log)
		: _motion(motion)
		, _depth(0)
		, _width(0)
		, _direction(0)
		, _speed(0)
		, _run(false)
		, _quit(false)
		, _log(log)
		, _thread(thread_func, this) {
		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_motion_tracker::gpio_smbus_motion_tracker() Start.");
		}

		if (motion == nullptr) {
			throw exception<std::logic_error, Log>("gpio_smbus_motion_tracker::gpio_smbus_motion_tracker() motion", __TAG__);
		}

		if (log != nullptr) {
			log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_motion_tracker::gpio_smbus_motion_tracker() Done.");
		}
	}


	template <typename DistanceScale, typename Log>
	inline gpio_smbus_motion_tracker<DistanceScale, Log>::~gpio_smbus_motion_tracker() noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_motion_tracker::~gpio_smbus_motion_tracker() Start.");
		}

		_quit = true;

		// The thread may be sleeping. Notify the condition to wake it up.
		_control_condition.notify_all();

		// Wait for the child thread to finish. std::~thread() terminates the process if the thread is running.
		_thread.join();

		if (_log != nullptr) {
			_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_motion_tracker::~gpio_smbus_motion_tracker() Done.");
		}
	}


	template <typename DistanceScale, typename Log>
	inline bool gpio_smbus_motion_tracker<DistanceScale, Log>::is_running() const noexcept {
		return _run;
	}


	template <typename DistanceScale, typename Log>
	inline void gpio_smbus_motion_tracker<DistanceScale, Log>::start() {
		_run = true;

		// The thread may be sleeping. Notify the condition to wake it up.
		_control_condition.notify_all();
	}


	template <typename DistanceScale, typename Log>
	inline void gpio_smbus_motion_tracker<DistanceScale, Log>::stop() {
		_run = false;

		// The thread is running. There is no need to notify it.
	}


	template <typename DistanceScale, typename Log>
	inline gpio_smbus_motion_value_t gpio_smbus_motion_tracker<DistanceScale, Log>::depth() const noexcept {
		return _depth;
	}


	template <typename DistanceScale, typename Log>
	inline gpio_smbus_motion_value_t gpio_smbus_motion_tracker<DistanceScale, Log>::width() const noexcept {
		return _width;
	}


	template <typename DistanceScale, typename Log>
	inline gpio_smbus_motion_value_t gpio_smbus_motion_tracker<DistanceScale, Log>::direction() const noexcept {
		return _direction;
	}


	template <typename DistanceScale, typename Log>
	inline gpio_smbus_motion_value_t gpio_smbus_motion_tracker<DistanceScale, Log>::speed() const noexcept {
		return _speed;
	}


	template <typename DistanceScale, typename Log>
	inline void gpio_smbus_motion_tracker<DistanceScale, Log>::set_depth(gpio_smbus_motion_value_t value) noexcept {
		_depth = value;
	}


	template <typename DistanceScale, typename Log>
	inline void gpio_smbus_motion_tracker<DistanceScale, Log>::set_width(gpio_smbus_motion_value_t value) noexcept {
		_width = value;
	}


	template <typename DistanceScale, typename Log>
	inline void gpio_smbus_motion_tracker<DistanceScale, Log>::set_direction(gpio_smbus_motion_value_t value) noexcept {
		_direction = value;
	}


	template <typename DistanceScale, typename Log>
	inline void gpio_smbus_motion_tracker<DistanceScale, Log>::set_speed(gpio_smbus_motion_value_t value) noexcept {
		_speed = value;
	}


	template <typename DistanceScale, typename Log>
	inline void gpio_smbus_motion_tracker<DistanceScale, Log>::thread_func(gpio_smbus_motion_tracker<DistanceScale, Log>* this_ptr) noexcept {
		using clock = std::chrono::steady_clock;

		if (this_ptr->_log != nullptr) {
			this_ptr->_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_motion_tracker::thread_func() Start.");
		}

		// Previous motion values;
		clock::time_point prev_time_point;
		gpio_smbus_motion_value_t prev_accel;
		gpio_smbus_motion_value_t prev_gyro;

		for (;;) {
			bool quit = this_ptr->_quit;
			bool run = this_ptr->_run;

			if (quit) {
				if (this_ptr->_log != nullptr) {
					this_ptr->_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_motion_tracker::thread_func() Quitting (from running).");
				}

				break;
			}

			if (!run) {
				if (this_ptr->_log != nullptr) {
					this_ptr->_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_motion_tracker::thread_func() Stopping.");
				}

				// Reset kept measurements.
				this_ptr->_speed = 0;
				prev_accel = 0;
				prev_gyro = 0;

				// Sleep to let the inertia die.
				std::this_thread::sleep_for(std::chrono::milliseconds(200));

				// Sleep until the owning instance fires the condition.
				{
					std::unique_lock<std::mutex> lock(this_ptr->_control_mutex);
					this_ptr->_control_condition.wait(lock);

					quit = this_ptr->_quit;
					run = this_ptr->_run;
				}

				if (quit) {
					if (this_ptr->_log != nullptr) {
						this_ptr->_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_motion_tracker::thread_func() Quitting (from sleeping).");
					}

					break;
				}

				if (this_ptr->_log != nullptr) {
					this_ptr->_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_motion_tracker::thread_func() Starting.");
				}

				prev_time_point = clock::time_point();
			}

			// Running.
			// Read the current motion values regardless.
			gpio_smbus_motion_values values;
			this_ptr->_motion->get_values(abc::gpio_smbus_motion_channel::accel_x | abc::gpio_smbus_motion_channel::gyro_z, values);

			gpio_smbus_motion_value_t curr_accel = (values.accel_x * gpio_smbus_motion_const::g * DistanceScale::den) / DistanceScale::num;
			gpio_smbus_motion_value_t curr_gyro = values.gyro_z;

			// Snap the current time point.
			clock::time_point curr_time_point = clock::now();

			// Read the atomic members once.
			gpio_smbus_motion_value_t prev_depth = this_ptr->_depth;
			gpio_smbus_motion_value_t prev_width = this_ptr->_width;
			gpio_smbus_motion_value_t prev_speed = this_ptr->_speed;
			gpio_smbus_motion_value_t prev_direction = this_ptr->_direction;

			if (prev_time_point.time_since_epoch().count() != 0) {
				// There is a previous set of measurements.
				// Do the calculations.

				// Calculate seconds as a floating point number.
				std::chrono::microseconds::rep microsec = std::chrono::duration_cast<std::chrono::microseconds>(curr_time_point - prev_time_point).count();
				gpio_smbus_motion_value_t sec = (static_cast<gpio_smbus_motion_value_t>(microsec) / std::micro::den) * std::micro::num;
				
				gpio_smbus_motion_value_t accel_accel = (curr_accel - prev_accel) / sec;
				gpio_smbus_motion_value_t distance = (prev_speed * sec) + (prev_accel * sec * sec / 2.0) + (accel_accel * sec * sec * sec / 6.0);

				gpio_smbus_motion_value_t gyro_accel = (curr_gyro - prev_gyro) / sec;
				gpio_smbus_motion_value_t gyro = (prev_gyro * sec) + (gyro_accel * sec * sec / 2.0);

				gpio_smbus_motion_value_t direction_rad = deg_to_rad(prev_direction);

				if (std::abs(gyro) < 0.000001) { // 0.001 degrees / sec
					// Straight line.
					this_ptr->_depth = prev_depth + distance * std::cos(direction_rad);
					this_ptr->_width = prev_width + distance * std::sin(direction_rad);
				}
				else {
					// Arch.
					gpio_smbus_motion_value_t gyro_rad = deg_to_rad(gyro);
					gpio_smbus_motion_value_t radius = distance / gyro_rad;
					gpio_smbus_motion_value_t straight_depth = radius * std::sin(gyro_rad);
					gpio_smbus_motion_value_t straight_width = radius * (1 - std::cos(gyro_rad));

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

		if (this_ptr->_log != nullptr) {
			this_ptr->_log->put_any(category::abc::gpio, severity::abc::optional, __TAG__, "gpio_smbus_motion_tracker::thread_func() Done.");
		}
	}


	template <typename DistanceScale, typename Log>
	inline  gpio_smbus_motion_value_t gpio_smbus_motion_tracker<DistanceScale, Log>::deg_to_rad(gpio_smbus_motion_value_t deg) noexcept {
		return gpio_smbus_motion_const::pi * deg / 180.0;
	}

}
