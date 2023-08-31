/*
MIT License

Copyright (c) 2018-2023 Zlatko Michailov 

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


#include "inc/timestamp.h"


static bool test_timestamp_properties(test_context& context, const abc::timestamp<>& ts, abc::year_t year, abc::month_t month, abc::day_t day, abc::hour_t hours, abc::minute_t minutes, abc::second_t seconds, abc::millisecond_t milliseconds, abc::microsecond_t microseconds, abc::nanosecond_t nanoseconds);

static constexpr abc::time_count_t nanoseconds_per_day = static_cast<abc::time_count_t>(abc::nanosecond_count) * abc::second_count * abc::minute_count * abc::hour_count;


bool test_null_timestamp(test_context& context) {
    abc::timestamp<> ts(nullptr);

    return test_timestamp_properties(context, ts, 1970, 1, 1, 0, 0, 0, 0, 0, 0);
}


bool test_before_year_2000_before_mar_1_timestamp(test_context& context) {
    abc::time_count_t date_1995_1_31 = (25 * 365 + 6 + 30) * nanoseconds_per_day;
    abc::timestamp<> ts(nullptr);
    ts.reset(date_1995_1_31);

    return test_timestamp_properties(context, ts, 1995, 1, 31, 0, 0, 0, 0, 0, 0);
}


bool test_before_year_2000_after_mar_1_timestamp(test_context& context) {
    abc::time_count_t date_1995_3_10 = (25 * 365 + 6 + 31 + 28 + 9) * nanoseconds_per_day;
    abc::timestamp<> ts(nullptr);
    ts.reset(date_1995_3_10);

    return test_timestamp_properties(context, ts, 1995, 3, 10, 0, 0, 0, 0, 0, 0);
}


bool test_after_year_2000_before_mar_1_timestamp(test_context& context) {
    abc::time_count_t date_2010_2_16 = (40 * 365 + 10 + 31 + 15) * nanoseconds_per_day;
    abc::timestamp<> ts(nullptr);
    ts.reset(date_2010_2_16);

    return test_timestamp_properties(context, ts, 2010, 2, 16, 0, 0, 0, 0, 0, 0);
}


bool test_after_year_2000_after_mar_1_timestamp(test_context& context) {
    abc::time_count_t date_2010_4_15 = (40 * 365 + 10 + 31 + 28 + 31 + 14) * nanoseconds_per_day;
    abc::timestamp<> ts(nullptr);
    ts.reset(date_2010_4_15);

    return test_timestamp_properties(context, ts, 2010, 4, 15, 0, 0, 0, 0, 0, 0);
}


static bool test_timestamp_properties(test_context& context, const abc::timestamp<>& ts, abc::year_t year, abc::month_t month, abc::day_t day, abc::hour_t hours, abc::minute_t minutes, abc::second_t seconds, abc::millisecond_t milliseconds, abc::microsecond_t microseconds, abc::nanosecond_t nanoseconds) {
    bool passed = true;
    passed = context.are_equal<std::int32_t>(ts.year(),         year,         0x1002e, "%d") && passed;
    passed = context.are_equal<std::int32_t>(ts.month(),        month,        0x1002f, "%d") && passed;
    passed = context.are_equal<std::int32_t>(ts.day(),          day,          0x10030, "%d") && passed;

    passed = context.are_equal<std::int32_t>(ts.hours(),        hours,        0x10031, "%d") && passed;
    passed = context.are_equal<std::int32_t>(ts.minutes(),      minutes,      0x10032, "%d") && passed;
    passed = context.are_equal<std::int32_t>(ts.seconds(),      seconds,      0x10033, "%d") && passed;
    passed = context.are_equal<std::int32_t>(ts.milliseconds(), milliseconds, 0x10034, "%d") && passed;
    passed = context.are_equal<std::int32_t>(ts.microseconds(), microseconds, 0x10035, "%d") && passed;
    passed = context.are_equal<std::int32_t>(ts.nanoseconds(),  nanoseconds,  0x10036, "%d") && passed;

    return passed;
}

