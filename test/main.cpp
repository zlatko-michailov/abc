#include "../src/base.h"
#include "../src/log.h"
#include "../src/timestamp.h"


constexpr abc::category_t	test_category	= 0x1234;
constexpr abc::tag_t		test_tag		= 0x567890ab;


template <typename Char>
void test_log(abc::basic_log& log, const Char* message) {
	log.min_severity = abc::severity::info;

	log.push(abc::severity::info, test_category, test_tag, abc::status::success);
	log.push(abc::severity::info, test_category, test_tag, abc::status::not_found, message);
}


void test_timestamp(abc::date_count_t days_since_epoch) {
	abc::timestamp ts;

	if (days_since_epoch >= 0) {
		ts.reset_date(days_since_epoch);
	}

	abc::log::diag.push(abc::severity::warning, test_category, test_tag, abc::status::success, "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%9.9u",
		ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds(), ts.nanoseconds());
}


int main() {
	test_log(abc::log::diag, "UTF-8 console");

	test_log(abc::log::diag, L"Wide char console");

	abc::log flog("out/log.txt");
	test_log(flog, "UTF-8 file");

	abc::log wflog("out/wlog.txt");
	test_log(wflog, L"Wide char file");

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

	return 0;
}
