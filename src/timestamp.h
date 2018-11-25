#include <chrono>
#include "log.h"


namespace abc {
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

	template <typename Clock = std::chrono::system_clock, typename Duration = typename Clock::duration>
	class timestamp {
	private:
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

		static constexpr time_count_t nanoseconds_per_day = static_cast<time_count_t>(nanosecond_count) * second_count * minute_count * hour_count;

		static constexpr date_count_t days_per_1_year = 365;
		static constexpr date_count_t days_per_1_year_leap = days_per_1_year + 1;
		static constexpr date_count_t days_per_4_years = 3 * days_per_1_year + days_per_1_year_leap;
		static constexpr date_count_t days_per_100_years_leap = 25 * days_per_4_years;
		static constexpr date_count_t days_per_100_years = days_per_100_years_leap - 1;
		static constexpr date_count_t days_per_400_years = days_per_100_years_leap + 3 * days_per_100_years;

	public:
		year_t			year() const noexcept { return _year; }
		month_t			month() const noexcept { return _month; }
		day_t			day() const noexcept { return _day; }

		hour_t			hours() const noexcept { return _hours; }
		minute_t		minutes() const noexcept { return _minutes; }
		second_t		seconds() const noexcept { return _seconds; }
		millisecond_t	milliseconds() const noexcept { return _milliseconds; }
		microsecond_t	microseconds() const noexcept { return _microseconds; }
		nanosecond_t	nanoseconds() const noexcept { return _nanoseconds; }

	public:
		timestamp() noexcept 
			: timestamp(Clock::now()) {
		}

		timestamp(std::chrono::time_point<Clock> tp) noexcept {
			reset(tp);
		}

		void reset(std::chrono::time_point<Clock> tp) noexcept {
			std::chrono::nanoseconds tp_nanoseconds_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch());
			reset(tp_nanoseconds_since_epoch.count());
		}

		void reset(time_count_t nanoseconds_since_epoch) noexcept {
			date_count_t days_since_epoch = static_cast<date_count_t>(nanoseconds_since_epoch / nanoseconds_per_day);
			reset_date(days_since_epoch);

			time_count_t nanoseconds_since_midnight = nanoseconds_since_epoch % nanoseconds_per_day;
			reset_time(nanoseconds_since_midnight);
		}

		void reset_date(date_count_t days_since_epoch) noexcept {
			date_count_t year = min_year;
			date_count_t month = min_month;
			date_count_t day = min_day;

			date_count_t remaining_days = days_since_epoch;

			// Since Feb 29 is the phantom day of the year, we'll make it the last day of the year by moving the epoch to Mar 1, 1970.
			// We can do that, unless we are in Jan or Feb 1970.

			// Jan
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Feb
			if (reset_date_if_done(year, month, day, remaining_days, 28)) {
				return;
			}

			// Now the eopch is Mar 1, 1970.

			// There is a well-known number of days per 400 years.
			if (remaining_days >= days_per_400_years) {
				abc::log::diag.push(abc::category::log, 0x0011, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
				year += 400 * (remaining_days / days_per_400_years);
				remaining_days = remaining_days % days_per_400_years;
				abc::log::diag.push(abc::category::log, 0x0011, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
			}

			// The first century bounary is leap.
			if (remaining_days >= days_per_100_years_leap) {
				abc::log::diag.push(abc::category::log, 0x0021, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
				year += 100;
				remaining_days -= days_per_100_years_leap;
				abc::log::diag.push(abc::category::log, 0x0022, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
			}

			// The following century boundaries are not leap.
			// We artificailly make them leap.
			if (remaining_days >= days_per_100_years) {
				abc::log::diag.push(abc::category::log, 0x0031, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
				remaining_days += (remaining_days / days_per_100_years);
				abc::log::diag.push(abc::category::log, 0x0032, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
			}

			// Now we have homogenious 4-year chunks.
			if (remaining_days >= days_per_4_years) {
				abc::log::diag.push(abc::category::log, 0x0041, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
				year += 4 * (remaining_days / days_per_4_years);
				remaining_days = remaining_days % days_per_4_years;
				abc::log::diag.push(abc::category::log, 0x0042, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
			}

			// Out of any remaining years, the first one is not leap.
			if (remaining_days >= days_per_1_year) {
				abc::log::diag.push(abc::category::log, 0x0051, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
				year++;
				remaining_days -= days_per_1_year;
				abc::log::diag.push(abc::category::log, 0x0052, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
			}

			// The second year is leap.
			if (remaining_days >= days_per_1_year_leap) {
				abc::log::diag.push(abc::category::log, 0x0061, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
				year++;
				remaining_days -= days_per_1_year_leap;
				abc::log::diag.push(abc::category::log, 0x0062, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
			}

			// The remaining year(s) are not leap.
			if (remaining_days >= days_per_1_year) {
				abc::log::diag.push(abc::category::log, 0x0071, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
				year += (remaining_days / days_per_1_year);
				remaining_days = remaining_days % days_per_1_year;
				abc::log::diag.push(abc::category::log, 0x0072, abc::status::debug, "remaing_days=%d, year=%d", remaining_days, year);
			}

			// The remaining days are within 1 year (and we are based at Mar 1).

			// Mar
			abc::log::diag.push(abc::category::log, 0x0081, abc::status::debug, "remaing_days=%d, year=%d, month=%d, day=%d", remaining_days, year, month, day);
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Apr
			abc::log::diag.push(abc::category::log, 0x0082, abc::status::debug, "remaing_days=%d, year=%d, month=%d, day=%d", remaining_days, year, month, day);
			if (reset_date_if_done(year, month, day, remaining_days, 30)) {
				return;
			}

			// May
			abc::log::diag.push(abc::category::log, 0x0083, abc::status::debug, "remaing_days=%d, year=%d, month=%d, day=%d", remaining_days, year, month, day);
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Jun
			abc::log::diag.push(abc::category::log, 0x0084, abc::status::debug, "remaing_days=%d, year=%d, month=%d, day=%d", remaining_days, year, month, day);
			if (reset_date_if_done(year, month, day, remaining_days, 30)) {
				return;
			}

			// Jul
			abc::log::diag.push(abc::category::log, 0x0085, abc::status::debug, "remaing_days=%d, year=%d, month=%d, day=%d", remaining_days, year, month, day);
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Aug
			abc::log::diag.push(abc::category::log, 0x0086, abc::status::debug, "remaing_days=%d, year=%d, month=%d, day=%d", remaining_days, year, month, day);
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Sep
			abc::log::diag.push(abc::category::log, 0x0087, abc::status::debug, "remaing_days=%d, year=%d, month=%d, day=%d", remaining_days, year, month, day);
			if (reset_date_if_done(year, month, day, remaining_days, 30)) {
				return;
			}

			// Oct
			abc::log::diag.push(abc::category::log, 0x0088, abc::status::debug, "remaing_days=%d, year=%d, month=%d, day=%d", remaining_days, year, month, day);
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Nov
			abc::log::diag.push(abc::category::log, 0x0089, abc::status::debug, "remaing_days=%d, year=%d, month=%d, day=%d", remaining_days, year, month, day);
			if (reset_date_if_done(year, month, day, remaining_days, 30)) {
				return;
			}

			// Dec
			abc::log::diag.push(abc::category::log, 0x008a, abc::status::debug, "remaing_days=%d, year=%d, month=%d, day=%d", remaining_days, year, month, day);
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Jan
			abc::log::diag.push(abc::category::log, 0x008b, abc::status::debug, "remaing_days=%d, year=%d, month=%d, day=%d", remaining_days, year, month, day);
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Feb
			abc::log::diag.push(abc::category::log, 0x008c, abc::status::debug, "remaing_days=%d, year=%d, month=%d, day=%d", remaining_days, year, month, day);
			if (reset_date_if_done(year, month, day, remaining_days, 29)) {
				return;
			}

			// TODO: assert unreachable
			abc::log::diag.push(abc::category::log, 0x008d, abc::status::debug, "remaing_days=%d, year=%d, month=%d, day=%d", remaining_days, year, month, day);
		}

		void reset_time(time_count_t nanoseconds_since_midnight) noexcept {
			// TODO: assert if nanoseconds_since_midnight >= nanoseconds_per_day
			time_count_t remaining = nanoseconds_since_midnight % nanoseconds_per_day;
			abc::log::diag.push(abc::category::log, 0x0201, abc::status::debug, "remaing=%lld", remaining);

			time_count_t nanoseconds = remaining % nanosecond_count;
			remaining = remaining / millisecond_count;

			time_count_t microseconds = remaining % microsecond_count;
			remaining = remaining / millisecond_count;

			time_count_t milliseconds = remaining % millisecond_count;
			remaining = remaining / millisecond_count;

			time_count_t seconds = remaining % second_count;
			remaining = remaining / second_count;

			time_count_t minutes = remaining % minute_count;
			remaining = remaining / minute_count;

			time_count_t hours = remaining % hour_count;

			abc::log::diag.push(abc::category::log, 0x0202, abc::status::debug, "hours=%lld, minutes=%lld, seconds=%lld, milliseconds=%lld, microsconds=%lld, nanoseconds=%lld", 
				hours, minutes, seconds, milliseconds, microseconds, nanoseconds);

			reset_time(hours, minutes, seconds, nanoseconds);
		}

	private:
		bool reset_date_if_done(date_count_t& year, date_count_t& month, date_count_t& day, date_count_t& remaining_days, date_count_t days_in_1_month) noexcept {
			if (remaining_days < days_in_1_month) {
				day += remaining_days;

				abc::log::diag.push(abc::category::log, 0x0100, abc::status::debug, "year=%d, month=%d, day=%d", year, month, day);
				reset_date(year, month, day);
				return true;
			}

			if (++month > max_month) {
				year++;
				month = min_month;
			}

			remaining_days -= days_in_1_month;
			abc::log::diag.push(abc::category::log, 0x00ff, abc::status::debug, "year=%d, month=%d, day=%d", year, month, day);

			return false;
		}

		void reset_date(year_t year, month_t month, day_t day) noexcept {
			_year = year;
			_month = month;
			_day = day;
		}

		void reset_time(hour_t hours, minute_t minutes, second_t seconds, nanosecond_t nanoseconds) noexcept {
			_hours = hours;
			_minutes = minutes;
			_seconds = seconds;
			_milliseconds = nanoseconds / microsecond_count;
			_microseconds = nanoseconds / millisecond_count;
			_nanoseconds = nanoseconds;
		}

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
