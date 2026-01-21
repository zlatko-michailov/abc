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

#include <cstddef>
#include <cstdint>
#include <chrono>


namespace abc {

    using date_count_t  = std::int32_t;
    using time_count_t  = std::int64_t;

    using year_t        = std::int16_t;
    using month_t       = std::int16_t;
    using day_t         = std::int16_t;
    using hour_t        = std::int16_t;
    using minute_t      = std::int16_t;
    using second_t      = std::int16_t;
    using millisecond_t = std::int16_t;
    using microsecond_t = std::int32_t;
    using nanosecond_t  = std::int32_t;


    // --------------------------------------------------------------


    /**
     * @brief        Wrapper around `std::chrono::time_point` that has convenience getters for date and time parts.
     * @tparam Clock Clock for the `std::chrono::time_point`.
     */
    template <typename Clock = std::chrono::system_clock>
    class timestamp
        : public std::chrono::time_point<Clock> {

        using base = std::chrono::time_point<Clock>;

    public:
        /**
         * @brief   Constructor.
         * @details Initializes with epoch/`0`.
         */
        timestamp(std::nullptr_t) noexcept;

        /**
         * @brief   Constructor.
         * @details Initializes with the current time.
         */
        timestamp() noexcept;

        /**
         * @brief    Constructor.
         * @details  Initializes with a given time point.
         * @param tp Time point.
         */
        timestamp(const std::chrono::time_point<Clock>& tp) noexcept;

    public:
        /**
         * @brief    Resets with a given time point.
         * @param tp Time point.
         */
        void reset(std::chrono::time_point<Clock> tp) noexcept;

        /**
         * @brief                         Resets both the date and the time members with the given count of nanoseconds since epoch.
         * @param nanoseconds_since_epoch Count of nanoseconds since epoch.
         */
        void reset(time_count_t nanoseconds_since_epoch) noexcept;

        /**
         * @brief                  Resets the date members with the given count of days since epoch.
         * @param days_since_epoch Count of days since epoch.
         */
        void reset_date(date_count_t days_since_epoch) noexcept;

        /**
         * @brief                            Resets the time members with the given count of nanoseconds since midnight.
         * @param nanoseconds_since_midnight Count of nanoseconds since midnight.
         */
        void reset_time(time_count_t nanoseconds_since_midnight) noexcept;

    public:
        /**
         * @brief Returns the year part of the date.
         */
        year_t year() const noexcept { return _year; }

        /**
         * @brief Returns the month part of the date.
         */
        month_t month() const noexcept { return _month; }

        /**
         * @brief Returns the day part of the date.
         */
        day_t day() const noexcept { return _day; }

        /**
         * @brief Returns the hours part of the time.
         */
        hour_t hours() const noexcept { return _hours; }

        /**
         * @brief Returns the minutes part of the time.
         */
        minute_t minutes() const noexcept { return _minutes; }

        /**
         * @brief Returns the seconds part of the time.
         */
        second_t seconds() const noexcept { return _seconds; }

        /**
         * @brief Returns the milliseconds part of the time.
         */
        millisecond_t milliseconds() const noexcept { return _milliseconds; }

        /**
         * @brief Returns the microseconds part of the time.
         */
        microsecond_t microseconds() const noexcept { return _microseconds; }

        /**
         * @brief Returns the nanoseconds part of the time.
         */
        nanosecond_t nanoseconds() const noexcept { return _nanoseconds; }

    public:
        /**
         * @brief Operator `==`.
         */
        bool operator==(const timestamp<Clock>& other) const noexcept;

        /**
         * @brief Operator `!=`.
         */
        bool operator!=(const timestamp<Clock>& other) const noexcept;

        /**
         * @brief Operator `>`.
         */
        bool operator> (const timestamp<Clock>& other) const noexcept;

        /**
         * @brief Operator `>=`.
         */
        bool operator>=(const timestamp<Clock>& other) const noexcept;

        /**
         * @brief Operator `<`.
         */
        bool operator< (const timestamp<Clock>& other) const noexcept;

        /**
         * @brief Operator `<=`.
         */
        bool operator<=(const timestamp<Clock>& other) const noexcept;

    private:
        /**
         * @brief                  Resets the date parts to the given values if the remaining days fall within a month. Otherwise, it moves to the next month.
         * @param days_since_epoch Total days since epoch.
         * @param year             Year.
         * @param month            Month.
         * @param day              Day.
         * @param remaining_days   Remaining days.
         * @param days_in_1_month  The count of days in the month that is being considered.
         * @return                 `true` = the date parts were reset. `false` = the process should continue.
         */
        bool reset_date_if_done(date_count_t days_since_epoch, date_count_t& year, date_count_t& month, date_count_t& day, date_count_t& remaining_days, date_count_t days_in_1_month) noexcept;

        /**
         * @brief                  Unconditionally resets the date parts to the given values.
         * @param days_since_epoch Total days since epoch.
         * @param year             Year.
         * @param month            Month.
         * @param day              Day.
         */
        void reset_date(date_count_t days_since_epoch, year_t year, month_t month, day_t day) noexcept;

        /**
         * @brief                            Unconditionally resets the time parts to the given values.
         * @param nanoseconds_since_midnight Nanoseconds since midnight.
         * @param hours                      Hours.
         * @param minutes                    Minutes.
         * @param seconds                    Seconds.
         * @param nanoseconds                Nanoseconds.
         */
        void reset_time(time_count_t nanoseconds_since_midnight, hour_t hours, minute_t minutes, second_t seconds, nanosecond_t nanoseconds) noexcept;

    private:
        date_count_t  _days_since_epoch;
        time_count_t  _nanoseconds_since_midnight;

    private:
        year_t        _year;
        month_t       _month;
        day_t         _day;

        hour_t        _hours;
        minute_t      _minutes;
        second_t      _seconds;
        millisecond_t _milliseconds;
        microsecond_t _microseconds;
        nanosecond_t  _nanoseconds;
    };

}
