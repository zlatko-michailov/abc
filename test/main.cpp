#include "../src/status.h"
#include "../src/log.h"

constexpr abc::category_t	test_category	= 0x1234;
constexpr abc::tag_t		test_tag		= 0x567890ab;


int main() {
	abc::log::global.push(test_category, test_tag, abc::status::success);
	abc::log::global.push(test_category, test_tag, abc::status::not_found, "UTF-8 message");
	abc::wlog::global.push(test_category, test_tag, abc::status::not_ready, L"Wide char message");

	return 0;
}
