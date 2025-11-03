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

#include <ratio>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "../../diag/i/diag_ready.i.h"
#include "motion.i.h"


namespace abc { namespace smbus {

    /**
     * @brief                Continuous motion tracker.
     * @details              Continuously polls a given motion sensor, and calculates relative location, direction, and speed.
     * @tparam DistanceScale `std::ratio` type for scaling distance-related metrics - depth, width, and speed.
     */
    template <typename DistanceScale>
    class motion_tracker
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief        Constructor.
         * @details      All metrics are initialized to 0.
         * @param motion Pointer to a `motion` instance.
         * @param log    `diag::log_ostream` pointer. May be `nullptr`.
         */
        motion_tracker(motion* motion, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        motion_tracker(motion_tracker&& other) noexcept;

        /**
         * @brief Deleted.
         */
        motion_tracker(const motion_tracker<DistanceScale>& other) = delete;

        /**
         * @brief Destructor.
         */
        virtual ~motion_tracker() noexcept;

    public:
        /**
         * @brief Returns `true` if the tracker is running.
         */
        bool is_running() const noexcept;

        /**
         * @brief   Starts/resumes tracking.
         * @details Metrics are not reset.
         */
        void start();

        /**
         * @brief   Stops/suspends tracking.
         * @details Metrics are not reset.
         */
        void stop();

    public:
        /**
         * @brief Returns the distance along.
         */
        motion_value_t depth() const noexcept;

        /**
         * @brief Returns the distance across.
         */
        motion_value_t width() const noexcept;

        /**
         * @brief Returns the degrees of deviation.
         */
        motion_value_t direction() const noexcept;

        /**
         * @brief Returns the speed.
         */
        motion_value_t speed() const noexcept;

    public:
        /**
         * @brief   Sets the current depth.
         * @details This can be used to make an adjustment based on an alternative sensor, like GPS.
         */
        void set_depth(motion_value_t value) noexcept;

        /**
         * @brief   Sets the current width.
         * @details This can be used to make an adjustment based on an alternative sensor, like GPS.
         */
        void set_width(motion_value_t value) noexcept;

        /**
         * @brief   Sets the current direction.
         * @details This can be used to make an adjustment based on an alternative sensor, like GPS.
         */
        void set_direction(motion_value_t value) noexcept;

        /**
         * @brief   Sets the current speed.
         * @details This can be used to make an adjustment based on an alternative sensor, like GPS.
         */
        void set_speed(motion_value_t value) noexcept;

    private:
        /**
         * @brief          Thread function that does the continuous motion tracking.
         * @param this_ptr Pointer to the owning instance.
         */
        static void thread_func(motion_tracker<DistanceScale>* this_ptr) noexcept;

        /**
         * @brief     Converts degrees to radians.
         * @param deg Degrees.
         */
        static motion_value_t deg_to_rad(motion_value_t deg) noexcept;

    private:
        /**
         * @brief Pointer to the `motion` instance passed in to the constructor.
         */
        motion* _motion;

        /**
         * @brief Current depth.
         */
        std::atomic<motion_value_t> _depth;

        /**
         * @brief Current width.
         */
        std::atomic<motion_value_t> _width;

        /**
         * @brief Current direction.
         */
        std::atomic<motion_value_t> _direction;

        /**
         * @brief Current speed.
         */
        std::atomic<motion_value_t> _speed;

        /**
         * @brief Mutex needed for `_control_condition`.
         */
        std::mutex _control_mutex;

        /**
         * @brief Condition variable used to save CPU cycles when the duty cycle is min or max.
         */
        std::condition_variable _control_condition;

        /**
         * @brief "Run" state.
         */
        std::atomic<bool> _run;

        /**
         * @brief "Quit requested" flag.
         */
        std::atomic<bool> _quit;

        /**
         * @brief                The thread on which motion is tracked.
         */
        std::thread _thread;
    };


    // --------------------------------------------------------------

} }
