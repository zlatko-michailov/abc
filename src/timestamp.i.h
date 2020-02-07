/*
MIT License

Copyright (c) 2018 Zlatko Michailov 

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


namespace abc {

	template <typename Clock>
	class timestamp;


	typedef int32_t date_count_t;
	typedef int64_t time_count_t;

	typedef int16_t year_t;
	typedef int16_t month_t;
	typedef int16_t day_t;
	typedef int16_t hour_t;
	typedef int16_t minute_t;
	typedef int16_t second_t;
	typedef int16_t millisecond_t;
	typedef int32_t microsecond_t;
	typedef int32_t nanosecond_t;


	// --------------------------------------------------------------


	template <typename Clock = std::chrono::system_clock>
	class timestamp {
	public:
		timestamp() noexcept;
		timestamp(std::chrono::time_point<Clock> tp) noexcept;

	public:
		void reset(std::chrono::time_point<Clock> tp) noexcept;
		void reset(time_count_t nanoseconds_since_epoch) noexcept;

		void reset_date(date_count_t days_since_epoch) noexcept;
		void reset_time(time_count_t nanoseconds_since_midnight) noexcept;

	public:
		year_t			year()			const noexcept { return _year; }
		month_t			month()			const noexcept { return _month; }
		day_t			day()			const noexcept { return _day; }

		hour_t			hours()			const noexcept { return _hours; }
		minute_t		minutes()		const noexcept { return _minutes; }
		second_t		seconds()		const noexcept { return _seconds; }
		millisecond_t	milliseconds()	const noexcept { return _milliseconds; }
		microsecond_t	microseconds()	const noexcept { return _microseconds; }
		nanosecond_t	nanoseconds()	const noexcept { return _nanoseconds; }

	public:
		bool operator==(const timestamp<Clock>& other) const noexcept;
		bool operator!=(const timestamp<Clock>& other) const noexcept;
		bool operator> (const timestamp<Clock>& other) const noexcept;
		bool operator>=(const timestamp<Clock>& other) const noexcept;
		bool operator< (const timestamp<Clock>& other) const noexcept;
		bool operator<=(const timestamp<Clock>& other) const noexcept;

	public:
		timestamp<Clock> coerse_minutes(std::chrono::minutes::rep minutes) const noexcept;

	private:
		bool reset_date_if_done(date_count_t days_since_epoch, date_count_t& year, date_count_t& month, date_count_t& day, date_count_t& remaining_days, date_count_t days_in_1_month) noexcept;
		void reset_date(date_count_t days_since_epoch, year_t year, month_t month, day_t day) noexcept;
		void reset_time(time_count_t nanoseconds_since_midnight, hour_t hours, minute_t minutes, second_t seconds, nanosecond_t nanoseconds) noexcept;

	private:
		date_count_t	_days_since_epoch;
		time_count_t	_nanoseconds_since_midnight;

	private:
		year_t			_year;
		month_t			_month;
		day_t			_day;

		hour_t			_hours;
		minute_t		_minutes;
		second_t		_seconds;
		millisecond_t	_milliseconds;
		microsecond_t	_microseconds;
		nanosecond_t	_nanoseconds;
	};

}
