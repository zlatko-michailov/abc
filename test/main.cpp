#include <thread>
#include <ostream>

#include <iostream>

#include "../src/base.h"
#include "../src/async.h"
#include "../src/log.h"
#include "../src/timestamp.h"
#include "../src/streambuf.h"


constexpr abc::category_t	test_category	= 0x1234;
constexpr abc::tag_t		test_tag		= 0x567890ab;


void test_log(abc::basic_log& log) {
	log.min_severity = abc::severity::info;
	log.filed_mask = (abc::field::all & ~abc::field::status);

	log.push(abc::severity::info, test_category, test_tag, abc::status::success);

	log.push(abc::severity::info, test_category, test_tag, abc::status::not_found, "1 of 4: inline");
	log.push(abc::severity::info, test_category, test_tag, abc::status::not_found, "2 of 4: %s", "UTF-8");
	log.push(abc::severity::info, test_category, test_tag, abc::status::not_found, "3 of 4: %ls", L"wide");
	log.push_async(abc::severity::info, test_category, test_tag, abc::status::not_found, std::move(std::string("4 of 4: async")));

	abc::arraybuf<50> buf;
	std::ostream s(&buf);
	s << std::hex << std::this_thread::get_id();
	log.push(abc::severity::info, test_category, test_tag, abc::status::success, "thread_id=%s", buf.c_str());

	abc::arraybuf<50> buf2;
	std::ostream s2(&buf2);
	s2 << std::hex << "1234_abc " << std::hex << "xyz_5678";
	log.push(abc::severity::info, test_category, test_tag, abc::status::success, "mismatch=%s", buf2.c_str());

	log.push(abc::severity::info, test_category, test_tag, abc::status::success);
}


void test_timestamp(abc::date_count_t days_since_epoch) {
	abc::timestamp ts;

	if (days_since_epoch >= 0) {
		ts.reset_date(days_since_epoch);
	}

	abc::log::diag.push(abc::severity::warning, test_category, test_tag, abc::status::success, "ts=%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%9.9u",
		ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds(), ts.nanoseconds());

	abc::timestamp ts2(ts);
	abc::log::diag.push(abc::severity::warning, test_category, test_tag, abc::status::success, "ts2=%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%9.9u",
		ts2.year(), ts2.month(), ts2.day(), ts2.hours(), ts2.minutes(), ts2.seconds(), ts2.nanoseconds());

	abc::timestamp ts3 = ts;
	abc::log::diag.push(abc::severity::warning, test_category, test_tag, abc::status::success, "ts3=%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%9.9u",
		ts3.year(), ts3.month(), ts3.day(), ts3.hours(), ts3.minutes(), ts3.seconds(), ts3.nanoseconds());

	abc::timestamp ts4 = ts.coerse_minutes(3);
	abc::log::diag.push(abc::severity::warning, test_category, test_tag, abc::status::success, "ts4=%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%9.9u",
		ts4.year(), ts4.month(), ts4.day(), ts4.hours(), ts4.minutes(), ts4.seconds(), ts4.nanoseconds());
}


int main() {
	test_log(abc::log::diag);

	abc::log flog("out/log", 3);
	test_log(flog);

	test_timestamp(-1);
	test_timestamp(0);
	test_timestamp(1);

	test_timestamp(31 - 1);
	test_timestamp(31);
	test_timestamp(31 + 1);

	test_timestamp(31 + 28 - 1);
	test_timestamp(31 + 28);
	test_timestamp(31 + 28 + 1);

	test_timestamp(31 + 28 + 31 - 1);
	test_timestamp(31 + 28 + 31);
	test_timestamp(31 + 28 + 31 + 1);

	test_timestamp(365 - 1);
	test_timestamp(365);
	test_timestamp(365 + 1);

	abc::result<int> r1(42);
	abc::log::diag.push(abc::severity::info, test_category, test_tag, abc::status::success, "%x: %d", r1.status, r1.value);

	abc::result<void> r2(abc::status::bad_state);
	abc::log::diag.push(abc::severity::info, test_category, test_tag, abc::status::success, "%x", r2.status);


	abc::async::start<int>([&c = std::cout]() -> int { c << "ASYNC \n"; return 42;})
		.value.then([&c = std::cout](int v) -> int { c << "ASYNC " << v << "\n"; return 53;})
		;//.value.then([&c = std::cout](int v) -> int { c << "ASYNC " << v << "\n"; return 65;})
		//.value.then([&c = std::cout](int v) -> int { c << "ASYNC " << v << "\n"; return 78;});
	std::cin.get();

	return 0;
}
