/*
MIT License

Copyright (c) 2018-2025 Zlatko Michailov 

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

#include "i/timestamp.i.h"


namespace abc {

    static constexpr nanosecond_t max_nanosecond = 999999999;
    static constexpr nanosecond_t min_nanosecond = 0;
    static constexpr nanosecond_t nanosecond_count = max_nanosecond - min_nanosecond + 1;

    static constexpr microsecond_t max_microsecond = 999999;
    static constexpr microsecond_t min_microsecond = 0;
    static constexpr microsecond_t microsecond_count = max_microsecond - min_microsecond + 1;

    static constexpr millisecond_t max_millisecond = 999;
    static constexpr millisecond_t min_millisecond = 0;
    static constexpr millisecond_t millisecond_count = max_millisecond - min_millisecond + 1;

    static constexpr second_t max_second = 59;
    static constexpr second_t min_second = 0;
    static constexpr second_t second_count = max_second - min_second + 1;

    static constexpr minute_t max_minute = 59;
    static constexpr minute_t min_minute = 0;
    static constexpr minute_t minute_count = max_minute - min_minute + 1;

    static constexpr hour_t max_hour = 23;
    static constexpr hour_t min_hour = 0;
    static constexpr hour_t hour_count = max_hour - min_hour + 1;

    static constexpr day_t min_day = 1;

    static constexpr month_t max_month = 12;
    static constexpr month_t min_month = 1;
    static constexpr month_t month_count = max_month - min_month + 1;

    static constexpr year_t min_year = 1970;

    static constexpr time_count_t nanoseconds_per_minute    = static_cast<time_count_t>(nanosecond_count) * second_count;
    static constexpr time_count_t nanoseconds_per_day        = static_cast<time_count_t>(nanosecond_count) * second_count * minute_count * hour_count;

    static constexpr date_count_t days_per_1_year = 365;
    static constexpr date_count_t days_per_1_year_leap = days_per_1_year + 1;
    static constexpr date_count_t days_per_4_years = 3 * days_per_1_year + days_per_1_year_leap;
    static constexpr date_count_t days_per_100_years_leap = 25 * days_per_4_years;
    static constexpr date_count_t days_per_100_years = days_per_100_years_leap - 1;
    static constexpr date_count_t days_per_400_years = days_per_100_years_leap + 3 * days_per_100_years;


    // --------------------------------------------------------------


    template <typename Clock>
    inline timestamp<Clock>::timestamp(std::nullptr_t) noexcept
        : base(Clock::duration::zero()) {

        reset_date(0);
        reset_time(0);
    }


    template <typename Clock>
    inline timestamp<Clock>::timestamp() noexcept
        : timestamp(Clock::now()) {
    }


    template <typename Clock>
    inline timestamp<Clock>::timestamp(const std::chrono::time_point<Clock>& tp) noexcept
        : base(tp) {

        reset(tp);
    }


    template <typename Clock>
    inline void timestamp<Clock>::reset(std::chrono::time_point<Clock> tp) noexcept {
        std::chrono::nanoseconds tp_nanoseconds_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch());
        reset(tp_nanoseconds_since_epoch.count());
    }


    template <typename Clock>
    inline void timestamp<Clock>::reset(time_count_t nanoseconds_since_epoch) noexcept {
        date_count_t days_since_epoch = static_cast<date_count_t>(nanoseconds_since_epoch / nanoseconds_per_day);
        reset_date(days_since_epoch);

        time_count_t nanoseconds_since_midnight = nanoseconds_since_epoch % nanoseconds_per_day;
        reset_time(nanoseconds_since_midnight);
    }


    template <typename Clock>
    inline bool timestamp<Clock>::operator==(const timestamp<Clock>& other) const noexcept {
        return _days_since_epoch == other._days_since_epoch
            && _nanoseconds_since_midnight == other._nanoseconds_since_midnight;
    }


    template <typename Clock>
    inline bool timestamp<Clock>::operator!=(const timestamp<Clock>& other) const noexcept {
        return !operator==(other);
    }


    template <typename Clock>
    inline bool timestamp<Clock>::operator> (const timestamp<Clock>& other) const noexcept {
        return (_days_since_epoch > other._days_since_epoch)
            || (_days_since_epoch == other._days_since_epoch
                && _nanoseconds_since_midnight > other._nanoseconds_since_midnight);
    }


    template <typename Clock>
    inline bool timestamp<Clock>::operator>=(const timestamp<Clock>& other) const noexcept {
        return (_days_since_epoch > other._days_since_epoch)
            || (_days_since_epoch == other._days_since_epoch
                && _nanoseconds_since_midnight >= other._nanoseconds_since_midnight);
    }


    template <typename Clock>
    inline bool timestamp<Clock>::operator< (const timestamp<Clock>& other) const noexcept {
        return !operator>=(other);
    }


    template <typename Clock>
    inline bool timestamp<Clock>::operator<=(const timestamp<Clock>& other) const noexcept {
        return !operator>(other);
    }


    template <typename Clock>
    inline void timestamp<Clock>::reset_date(date_count_t days_since_epoch) noexcept {
        date_count_t year = min_year;
        date_count_t month = min_month;
        date_count_t day = min_day;

        date_count_t remaining_days = days_since_epoch;

        // Since Feb 29 is the phantom day of the year, we'll make it the last day of the year by moving the epoch to Mar 1, 1970.
        // We can do that, unless we are in Jan or Feb 1970.

        // Jan
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 31)) {
            return;
        }

        // Feb
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 28)) {
            return;
        }

        // Now the epoch is Mar 1, 1970.

        // There is a well-known number of days per 400 years.
        if (remaining_days >= days_per_400_years) {
            year += 400 * (remaining_days / days_per_400_years);
            remaining_days = remaining_days % days_per_400_years;
        }

        // The first century boundary is leap.
        if (remaining_days >= days_per_100_years_leap) {
            year += 100;
            remaining_days -= days_per_100_years_leap;
        }

        // The following century boundaries are not leap.
        // We artificially make them leap.
        if (remaining_days >= days_per_100_years) {
            remaining_days += (remaining_days / days_per_100_years);
        }

        // Now we have homogeneous 4-year chunks.
        if (remaining_days >= days_per_4_years) {
            year += 4 * (remaining_days / days_per_4_years);
            remaining_days = remaining_days % days_per_4_years;
        }

        // Out of any remaining years, the first one is not leap.
        if (remaining_days >= days_per_1_year) {
            year++;
            remaining_days -= days_per_1_year;
        }

        // The second year is leap.
        if (remaining_days >= days_per_1_year_leap) {
            year++;
            remaining_days -= days_per_1_year_leap;
        }

        // The remaining year(s) are not leap.
        if (remaining_days >= days_per_1_year) {
            year += (remaining_days / days_per_1_year);
            remaining_days = remaining_days % days_per_1_year;
        }

        // The remaining days are within 1 year (and we are based at Mar 1).

        // Mar
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 31)) {
            return;
        }

        // Apr
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 30)) {
            return;
        }

        // May
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 31)) {
            return;
        }

        // Jun
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 30)) {
            return;
        }

        // Jul
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 31)) {
            return;
        }

        // Aug
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 31)) {
            return;
        }

        // Sep
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 30)) {
            return;
        }

        // Oct
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 31)) {
            return;
        }

        // Nov
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 30)) {
            return;
        }

        // Dec
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 31)) {
            return;
        }

        // Jan
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 31)) {
            return;
        }

        // Feb
        if (reset_date_if_done(days_since_epoch, year, month, day, remaining_days, 29)) {
            return;
        }

        // TODO: diag 'unreachable'
    }


    template <typename Clock>
    inline void timestamp<Clock>::reset_time(time_count_t nanoseconds_since_midnight) noexcept {
        time_count_t remaining = nanoseconds_since_midnight % nanoseconds_per_day;

        time_count_t nanoseconds = remaining % nanosecond_count;
        remaining = remaining / nanosecond_count;

        time_count_t seconds = remaining % second_count;
        remaining = remaining / second_count;

        time_count_t minutes = remaining % minute_count;
        remaining = remaining / minute_count;

        time_count_t hours = remaining % hour_count;

        reset_time(nanoseconds_since_midnight, hours, minutes, seconds, nanoseconds);
    }


    template <typename Clock>
    inline bool timestamp<Clock>::reset_date_if_done(date_count_t days_since_epoch, date_count_t& year, date_count_t& month, date_count_t& day, date_count_t& remaining_days, date_count_t days_in_1_month) noexcept {
        if (remaining_days < days_in_1_month) {
            day += remaining_days;

            reset_date(days_since_epoch, year, month, day);
            return true;
        }

        if (++month > max_month) {
            year++;
            month = min_month;
        }

        remaining_days -= days_in_1_month;

        return false;
    }


    template <typename Clock>
    inline void timestamp<Clock>::reset_date(date_count_t days_since_epoch, year_t year, month_t month, day_t day) noexcept {
        _days_since_epoch = days_since_epoch;

        _year = year;
        _month = month;
        _day = day;
    }


    template <typename Clock>
    inline void timestamp<Clock>::reset_time(time_count_t nanoseconds_since_midnight, hour_t hours, minute_t minutes, second_t seconds, nanosecond_t nanoseconds) noexcept {
        _nanoseconds_since_midnight = nanoseconds_since_midnight;

        _hours = hours;
        _minutes = minutes;
        _seconds = seconds;
        _milliseconds = nanoseconds / microsecond_count;
        _microseconds = nanoseconds / millisecond_count;
        _nanoseconds = nanoseconds;
    }

}
