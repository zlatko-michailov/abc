#include <thread>
#include <ostream>

#include <iostream>

#include "../src/timestamp.h"
#include "../src/pool.h"
#include "../src/process.h"
#include "../src/host.itf.h"
#include "../src/log.h"


constexpr abc::category_t	test_category	= 0x1234;
constexpr abc::tag_t		test_tag		= 0x567890ab;


void test_log(abc::log& log) {
	log.push(abc::severity::trace, test_category, test_tag);

	log.push(abc::severity::trace, test_category, test_tag, "1 of 4: inline");
	log.push(abc::severity::trace, test_category, test_tag, "2 of 4: %s", "UTF-8");
	log.push(abc::severity::trace, test_category, test_tag, "3 of 4: %ls", L"wide");
}


void test_timestamp(abc::log& log, abc::date_count_t days_since_epoch) {
	abc::timestamp ts;

	if (days_since_epoch >= 0) {
		ts.reset_date(days_since_epoch);
	}

	log.push(abc::severity::warning, test_category, test_tag, "ts=%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%9.9u",
		ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds(), ts.nanoseconds());

	abc::timestamp ts2(ts);
	log.push(abc::severity::warning, test_category, test_tag, "ts2=%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%9.9u",
		ts2.year(), ts2.month(), ts2.day(), ts2.hours(), ts2.minutes(), ts2.seconds(), ts2.nanoseconds());

	abc::timestamp ts3 = ts;
	log.push(abc::severity::warning, test_category, test_tag, "ts3=%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%9.9u",
		ts3.year(), ts3.month(), ts3.day(), ts3.hours(), ts3.minutes(), ts3.seconds(), ts3.nanoseconds());

	abc::timestamp ts4 = ts.coerse_minutes(3);
	log.push(abc::severity::warning, test_category, test_tag, "ts4=%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%9.9u",
		ts4.year(), ts4.month(), ts4.day(), ts4.hours(), ts4.minutes(), ts4.seconds(), ts4.nanoseconds());
}


void test_pool() {
	abc::pool<std::int32_t> upool(abc::pool<std::int32_t>::unlimited);
	std::cout << "upinst1 begin" << std::endl;
	abc::instance<abc::pool<std::int32_t>> upinst1(upool);
	std::cout << "upinst1 end" << std::endl;
	std::cout << "upinst2 begin" << std::endl;
	abc::instance<abc::pool<std::int32_t>> upinst2(upool);
	std::cout << "upinst2 end" << std::endl;

	abc::pool<std::int32_t> pool(abc::pool<std::int32_t>::singleton);
	std::cout << "pinst1 begin" << std::endl;
	abc::instance<abc::pool<std::int32_t>> pinst1(pool);
	std::cout << "pinst1 end" << std::endl;

	std::cout << "pinst2 begin" << std::endl;
	try {
		abc::instance<abc::pool<std::int32_t>> pinst2(pool);
	}
	catch(...) {
		std::cout << "pinst2 exceeded the pool capacity" << std::endl;
	}
	std::cout << "pinst2 end" << std::endl;
}


void test_process() {
	std::cout << "[" << abc::process::current().kind() << " " << abc::process::current().id() << "] Starting test..." << std::endl;

	abc::program program;

	program.emplace_back_daemon(program, [=](abc::daemon&, abc::process_cycle_t cycle) {
		std::cout << "\t[" << abc::process::current().kind() << " " << abc::process::current().id() << "] Starting..." << std::endl;
		std::cout << "\t[" << abc::process::current().kind() << " " << abc::process::current().id() << "] Started." << std::endl;
	}, 0, 0);
	program.emplace_back_daemon(program, [=](abc::daemon&, abc::process_cycle_t cycle) {
		std::cout << "\t[" << abc::process::current().kind() << " " << abc::process::current().id() << "] Starting..." << std::endl;
		std::cout << "\t[" << abc::process::current().kind() << " " << abc::process::current().id() << "] Started." << std::endl;
	}, 0, 0);
	program.emplace_back_daemon(program, [=](abc::daemon&, abc::process_cycle_t cycle) {
		std::cout << "\t[" << abc::process::current().kind() << " " << abc::process::current().id() << "] Starting..." << std::endl;
		std::cout << "\t[" << abc::process::current().kind() << " " << abc::process::current().id() << "] Started." << std::endl;
	}, 0, 0);

	program.start();
}


int main() {
	abc::log console_log(stdout);

	test_log(console_log);

	abc::log file_log("out/log", 3);
	test_log(file_log);

	test_timestamp(console_log, -1);
	test_timestamp(console_log, 0);
	test_timestamp(console_log, 1);

	test_timestamp(console_log, 31 - 1);
	test_timestamp(console_log, 31);
	test_timestamp(console_log, 31 + 1);

	test_timestamp(console_log, 31 + 28 - 1);
	test_timestamp(console_log, 31 + 28);
	test_timestamp(console_log, 31 + 28 + 1);

	test_timestamp(console_log, 31 + 28 + 31 - 1);
	test_timestamp(console_log, 31 + 28 + 31);
	test_timestamp(console_log, 31 + 28 + 31 + 1);

	test_timestamp(console_log, 365 - 1);
	test_timestamp(console_log, 365);
	test_timestamp(console_log, 365 + 1);

	test_pool();
	test_process();

	return 0;
}
