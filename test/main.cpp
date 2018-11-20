#include "../src/status.h"
#include "../src/log.h"

constexpr abc::category_t	test_category	= 0x1234;
constexpr abc::tag_t		test_tag		= 0x567890ab;

template <typename Char>
void test_log(abc::basic_log<Char>& log, const Char* message) {
	log.push(test_category, test_tag, abc::status::success);
	log.push(test_category, test_tag, abc::status::not_found, message);
}

int main() {
	test_log(abc::log::global, "UTF-8 console");

	test_log(abc::wlog::global, L"Wide char console");

	abc::flog flog("out/log.txt");
	test_log(flog, "UTF-8 file");

	abc::wflog wflog("out/wlog.txt");
	test_log(wflog, L"Wide char file");

	return 0;
}
