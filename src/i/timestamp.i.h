/*
MIT License

Copyright (c) 2018-2021 Zlatko Michailov 

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

	using date_count_t	= std::int32_t;
	using time_count_t	= std::int64_t;

	using year_t		= std::int16_t;
	using month_t		= std::int16_t;
	using day_t			= std::int16_t;
	using hour_t		= std::int16_t;
	using minute_t		= std::int16_t;
	using second_t		= std::int16_t;
	using millisecond_t	= std::int16_t;
	using microsecond_t	= std::int32_t;
	using nanosecond_t	= std::int32_t;


	// --------------------------------------------------------------


	template <typename Clock = std::chrono::system_clock>
	class timestamp : public std::chrono::time_point<Clock> {
		using base = std::chrono::time_point<Clock>;

	public:
		timestamp(std::nullptr_t) noexcept;
		timestamp() noexcept;
		timestamp(const std::chrono::time_point<Clock>& tp) noexcept;

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
