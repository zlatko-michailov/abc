/*
MIT License

Copyright (c) 2018-2026 Zlatko Michailov

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

#include "../../diag/i/diag_ready.i.h"
#include "base.i.h"
#include "chip.i.h"
#include "line.i.h"


namespace abc { namespace gpio {

    /**
     * @brief                Combination of an `output_line` (trigger) and an `input_line` (echo) to measure distance to the nearest obstacle.
     * @tparam DistanceScale `std::ratio` type for the distance.
     */
    template <typename DistanceScale>
    class ultrasonic
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief                  Constructor.
         * @param chip             Pointer to the GPIO chip where the lines are surfaced.
         * @param trigger_line_pos Chip-specific position of the output (trigger) line.
         * @param echo_line_pos    Chip-specific position of the input (echo) line.
         * @param log              `diag::log_ostream` pointer. May be `nullptr`.
         */
        ultrasonic(const chip* chip, line_pos_t trigger_line_pos, line_pos_t echo_line_pos, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        ultrasonic(ultrasonic<DistanceScale>&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        ultrasonic(const ultrasonic<DistanceScale>& other) = delete;

    public:
        /**
         * @brief              Measures the distance to the nearest obstacle.
         * @param max_distance Maximum distance that this call should wait for.
         * @return             Actual distance upon success. `max_distance` otherwise.  
         */
        std::size_t measure_distance(std::size_t max_distance) const noexcept;

    private:
        /**
         * @brief           Static helper that calculates the distance sound travels for the given time.
         * @tparam Duration `std::duration` type.
         * @param duration  Duration of time.
         * @return          Distance = sonic speed * `duration`.
         */
        template <typename Duration>
        static std::size_t sonic_distance(Duration duration) noexcept;

        /**
         * @brief           Static helper that calculates the time needed for sound to travel the given distance.
         * @tparam Duration `std::duration` type.
         * @param distance  Distance.
         * @return          Duration = `distance` / sonic speed.
         */
        template <typename Duration>
        static Duration sonic_duration(std::size_t distance) noexcept;

    private:
        /**
         * @brief Output (trigger) line.
         */
        output_line _trigger_line;

        /**
         * @brief Input (echo) line.
         */
        input_line _echo_line;
    };


    // --------------------------------------------------------------

} }
