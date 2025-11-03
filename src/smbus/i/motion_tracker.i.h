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
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "gpio_smbus_motion.i.h"


namespace abc {

	/**
	 * @brief					Continuous motion tracker.
	 * @details					Continuously polls a given motion sensor, and calculates relative location, direction, and speed.
	 * @tparam DistanceScale	`std::ratio` type for scaling distance-related metrics - depth, width, and speed.
	 * @tparam Log				Logging facility.
	 */
	template <typename DistanceScale, typename Log = null_log>
	class gpio_smbus_motion_tracker {
	public:
		/**
		 * @brief				Constructor.
		 * @details				All metrics are initialized to 0.
		 * @param motion		Pointer to a `gpio_smbus_motion` instance.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		gpio_smbus_motion_tracker(gpio_smbus_motion<Log>* motion, Log* log = nullptr);

		/**
		 * @brief				Move constructor.
		 */
		gpio_smbus_motion_tracker(gpio_smbus_motion_tracker<DistanceScale, Log>&& other) noexcept = default;

		/**
		 * @brief				Deleted.
		 */
		gpio_smbus_motion_tracker(const gpio_smbus_motion_tracker<DistanceScale, Log>& other) = delete;

		/**
		 * @brief				Destructor.
		 */
		virtual ~gpio_smbus_motion_tracker() noexcept;

	public:
		/**
		 * @brief				Returns `true` if the tracker is running.
		 */
		bool is_running() const noexcept;

		/**
		 * @brief				Starts/resumes tracking.
		 * @details				Metrics are not reset.
		 */
		void start();

		/**
		 * @brief				Stops/suspends tracking.
		 * @details				Metrics are not reset.
		 */
		void stop();

	public:
		/**
		 * @brief				Returns the distance along.
		 */
		gpio_smbus_motion_value_t depth() const noexcept;

		/**
		 * @brief				Returns the distance across.
		 */
		gpio_smbus_motion_value_t width() const noexcept;

		/**
		 * @brief				Returns the degrees of deviation.
		 */
		gpio_smbus_motion_value_t direction() const noexcept;

		/**
		 * @brief				Returns the speed.
		 */
		gpio_smbus_motion_value_t speed() const noexcept;

	public:
		/**
		 * @brief				Sets the current depth.
		 * @details				This can be used to make an adjustment based on an alternative sensor, like GPS.
		 */
		void set_depth(gpio_smbus_motion_value_t value) noexcept;

		/**
		 * @brief				Sets the current width.
		 * @details				This can be used to make an adjustment based on an alternative sensor, like GPS.
		 */
		void set_width(gpio_smbus_motion_value_t value) noexcept;

		/**
		 * @brief				Sets the current direction.
		 * @details				This can be used to make an adjustment based on an alternative sensor, like GPS.
		 */
		void set_direction(gpio_smbus_motion_value_t value) noexcept;

		/**
		 * @brief				Sets the current speed.
		 * @details				This can be used to make an adjustment based on an alternative sensor, like GPS.
		 */
		void set_speed(gpio_smbus_motion_value_t value) noexcept;

	private:
		/**
		 * @brief				Thread function that does the continuous motion tracking.
		 * @param this_ptr		Pointer to the owning instance.
		 */
		static void thread_func(gpio_smbus_motion_tracker<DistanceScale, Log>* this_ptr) noexcept;

		/**
		 * @brief				Converts degrees to radians.
		 * @param deg			Degrees.
		 */
		static gpio_smbus_motion_value_t deg_to_rad(gpio_smbus_motion_value_t deg) noexcept;

	private:
		/**
		 * @brief				Pointer to the `gpio_smbus_motion` instance passed in to the constructor.
		 */
		gpio_smbus_motion<Log>* _motion;

		/**
		 * @brief				Current depth.
		 */
		std::atomic<gpio_smbus_motion_value_t> _depth;

		/**
		 * @brief				Current width.
		 */
		std::atomic<gpio_smbus_motion_value_t> _width;

		/**
		 * @brief				Current direction.
		 */
		std::atomic<gpio_smbus_motion_value_t> _direction;

		/**
		 * @brief				Current speed.
		 */
		std::atomic<gpio_smbus_motion_value_t> _speed;

		/**
		 * @brief				Mutex needed for `_control_condition`.
		 */
		std::mutex _control_mutex;

		/**
		 * @brief				Condition variable used to save CPU cycles when the duty cycle is min or max.
		 */
		std::condition_variable _control_condition;

		/**
		 * @brief				"Run" state.
		 */
		std::atomic<bool> _run;

		/**
		 * @brief				"Quit requested" flag.
		 */
		std::atomic<bool> _quit;

		/**
		 * @brief				The log passed in to the constructor.
		 */
		Log* _log;

		/**
		 * @brief				The thread on which motion is tracked.
		 */
		std::thread _thread;
	};


	// --------------------------------------------------------------

}
