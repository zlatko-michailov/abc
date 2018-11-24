#include <chrono>

namespace abc {
	typedef int16_t year_t;
	typedef int8_t month_t;
	typedef int8_t day_t;
	typedef int8_t hour_t;
	typedef int8_t minute_t;
	typedef int8_t second_t;
	typedef int16_t millisecond_t;
	typedef int32_t microsecond_t;
	typedef int32_t nanosecond_t;


	//constexpr day_t day_count = 365.2425;
	//constexpr date_count_t day_ratio = hour_count * hour_ratio;

	//constexpr month_t max_month = 12;
	//constexpr month_t min_month = 1;
	//constexpr month_t month_count = max_month - min_month + 1;
	//constexpr date_count_t month_ratio = hour_count * hour_ratio;

	//constexpr year_t min_year = 1970;


	template <typename Clock, typename Duration = typename Clock::duration>
	class timestamp {
	private:
		typedef int32_t date_count_t;
		typedef int64_t time_count_t;

		constexpr nanosecond_t max_nanosecond = 999999999;
		constexpr nanosecond_t min_nanosecond = 0;
		constexpr nanosecond_t nanosecond_count = max_nanosecond - min_nanosecond + 1;

		constexpr second_t max_second = 59;
		constexpr second_t min_second = 0;
		constexpr second_t second_count = max_second - min_second + 1;

		constexpr minute_t max_minute = 59;
		constexpr minute_t min_minute = 0;
		constexpr minute_t minute_count = max_minute - min_minute + 1;

		constexpr hour_t max_hour = 23;
		constexpr hour_t min_hour = 0;
		constexpr hour_t hour_count = max_hour - min_hour + 1;

		constexpr day_t min_day = 1;

		constexpr month_t max_month = 12;
		constexpr month_t min_month = 1;
		constexpr month_t month_count = max_month - min_month + 1;

		constexpr year_t min_year = 1970;

		constexpr time_count_t nanoseconds_per_day = static_cast<time_count_t>(nanosecond_count) * second_count * minute_count * hour_count;

		constexpr date_count_t days_per_1_year = 365;
		constexpr date_count_t days_per_1_year_leap = days_per_1_year + 1;
		constexpr date_count_t days_per_4_years = 3 * days_per_1_year + days_per_1_year_leap;
		constexpr date_count_t days_per_100_years_leap = 25 * days_per_4_years;
		constexpr date_count_t days_per_100_years = days_per_100_years_leap - 1;
		constexpr date_count_t days_per_400_years = days_per_100_years_leap + 3 * days_per_100_years;

		/*
		// Until C++ 20
		typedef std::chrono::duration<date_count_t, std::ratio<31556952>> years;
		typedef std::chrono::duration<date_count_t, std::ratio<2629746>> months;
		typedef std::chrono::duration<date_count_t, std::ratio<86400>> days;
		*/

	public:
		timestamp() noexcept 
			: datetime(Clock::now()) {
		}

		timestamp(std::chrono::time_point<Clock> tp) noexcept {
			/*
			years years_since_epoch = std::chrono::duration_cast<years>(tp.time_since_epoch());
			_years = min_year + static_cast<year_t>(years_since_epoch.count());

			months months_since_epoch = std::chrono::duration_cast<months>(tp.time_since_epoch());
			_years = min_year + static_cast<year_t>(years_since_epoch.count());


			std::chrono::nanoseconds tp_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch());

			int64_t nanoseconds_since_epoch = tp_since_epoch.count();
			_nanoseconds = min_nanosecond + static_cast<nanosecond_t>(nanoseconds_since_epoch % nanosecond_count);

			int64_t seconds_since_epoch = nanoseconds_since_epoch / nanosecond_count;
			_seconds = min_second + static_cast<second_t>(seconds_since_epoch % second_count);

			int64_t minutes_since_epoch = seconds_since_epoch / second_count;
			_minutes = min_minute + static_cast<minute_t>(minutes_since_epoch % minute_count);

			int64_t hours_since_epoch = minutes_since_epoch / minute_count;
			_hours = min_hour + static_cast<hour_t>(hours_since_epoch % hour_count);

			int64_t days_since_epoch = hours_since_epoch / hour_count;
			_days = min_day + static_cast<day_t>(days_since_epoch % day_count);
			*/

			reset(tp);
		}

		void reset(std::chrono::time_point<Clock> tp) noexcept {
			time_count_t nanoseconds_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch());

			date_count_t days_since_epoch = static_cast<date_count_t>(nanoseconds_since_epoch / nanoseconds_per_day);
			time_count_t nanoseconds_since_midnight = nanoseconds_since_epoch % nanoseconds_per_day;

			reset_date(days_since_epoch);
			reset_date(nanoseconds_since_midnight);
		}

	private:
		void reset_date(date_count_t days_since_epoch) noexcept {
			year_t year = min_year;
			month_t month = min_month;
			day_t day = min_day;

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
				year += 400 * (remaining_days / days_per_400_years);
				remaining_days = remaining_days % days_per_400_years;
			}

			// The first century bounary is leap.
			if (remaining_days >= days_per_100_years_leap) {
				year += 100;
				remaining_days -= days_per_100_years_leap;
			}

			// The following century boundaries are not leap.
			// We artificailly make them leap.
			if (remaining_days >= days_per_100_years) {
				remaining_days += (remaining_days / days_per_100_years);
			}

			// Now we have homogenious 4-year chunks.
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
				remaining_days = remaining_days % days_per_1_year
			}

			// The remaining days are within 1 year (and we are based at Mar 1).

			// Mar
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Apr
			if (reset_date_if_done(year, month, day, remaining_days, 30)) {
				return;
			}

			// May
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Jun
			if (reset_date_if_done(year, month, day, remaining_days, 30)) {
				return;
			}

			// Jul
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Aug
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Sep
			if (reset_date_if_done(year, month, day, remaining_days, 30)) {
				return;
			}

			// Oct
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Nov
			if (reset_date_if_done(year, month, day, remaining_days, 30)) {
				return;
			}

			// Dec
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Jan
			if (reset_date_if_done(year, month, day, remaining_days, 31)) {
				return;
			}

			// Feb
			if (reset_date_if_done(year, month, day, remaining_days, 29)) {
				return;
			}

			// TODO: assert unreachable
		}

		bool reset_date_if_done(date_count_t& year, date_count_t& month, date_count_t& day, date_count_t& remaining_days, date_count_t days_in_1_month) noexcept {
			if (remaining_days < days_in_1_month) {
				day += remaining_days;

				reset_date(year, month, day);
				return true;
			}

			if (++month > max_month) {
				year++;
				month = min_month
			}

			remaining_days -= days_in_1_month;

			return false;
		}

		void reset_date(year_t year, month_t month, day_t day) noexcept {
			_year = year;
			_month = month;
			_day = day;
		}

		void reset_time(time_count_t /*nanoseconds_since_midnight*/) noexcept {
		}

	private:
		year_t			_year;
		month_t			_month;
		day_t			_day;

		hour_t			_hours;
		minute_t		_minutes;
		second_t		_seconds;
		nanosecond_t	_nanoseconds;
	};
}
