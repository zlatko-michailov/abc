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

#include <cstdint>
#include <chrono>
#include <ratio>


namespace abc { namespace test {

	class clock {
	public:
		using rep			= std::int64_t;
		using period		= std::nano;
		using duration		= std::chrono::duration<rep, period>;
		using time_point	= std::chrono::time_point<clock>;

	public:
		static time_point now() noexcept;
	};


	inline clock::time_point clock::now() noexcept {
		constexpr clock::rep nanoseconds_per_millisecond = 1000LL * 1000LL;
		constexpr clock::rep nanoseconds_per_second = 1000LL * nanoseconds_per_millisecond;
		constexpr clock::rep nanoseconds_per_minute = 60LL * nanoseconds_per_second;
		constexpr clock::rep nanoseconds_per_hour = 60LL * nanoseconds_per_minute;
		constexpr clock::rep nanoseconds_per_day = 24LL * nanoseconds_per_hour;
		constexpr clock::rep nanoseconds_per_year = 365LL * nanoseconds_per_day;

		// 2020-10-15 12:34:56.789
		return time_point(
			std::chrono::nanoseconds(
				50LL * nanoseconds_per_year +
				300LL * nanoseconds_per_day +
				12LL * nanoseconds_per_hour +
				34LL * nanoseconds_per_minute +
				56LL * nanoseconds_per_second +
				789LL * nanoseconds_per_millisecond));
	}
}}

