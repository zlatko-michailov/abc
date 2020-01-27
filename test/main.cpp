#include <iostream>

#include "../src/timestamp.h"
#include "../src/log.h"


int main() {
	abc::timestamp ts1;

	std::cout << ts1.year() << "-" << ts1.month() << "-" << ts1.day() << std::endl;

	std::cout << std::endl;

	abc::log diag(std::move(abc::log_container::ostream()), std::move(abc::log_view::diag()), std::move(abc::log_filter::none()));
	diag.push_back(abc::category::abc::base, abc::severity::critical, 0x01, "foo=%d", 42);
	diag.push_back(abc::category::abc::base, abc::severity::warning, 0x02, "bar=%d", 99);
	diag.push_back(abc::category::abc::base, abc::severity::important, 0x03, "foo=%d", 42);
	diag.push_back(abc::category::abc::base, abc::severity::optional, 0x04, "bar=%d", 99);
	diag.push_back(abc::category::abc::base, abc::severity::important, 0x05, "foo=%d", 42);
	diag.push_back(abc::category::abc::base, abc::severity::warning, 0x06, "bar=%d", 99);
	diag.push_back(abc::category::abc::base, abc::severity::critical, 0x07, "bar=%d", 99);

	std::cout << std::endl;

	abc::log test(std::move(abc::log_container::ostream()), std::move(abc::log_view::test()), std::move(abc::log_filter::none()));
	test.push_back(abc::category::abc::base, abc::severity::critical, 0x01, "foo=%d", 42);
	test.push_back(abc::category::abc::base, abc::severity::warning, 0x02, "bar=%d", 99);
	test.push_back(abc::category::abc::base, abc::severity::important, 0x03, "foo=%d", 42);
	test.push_back(abc::category::abc::base, abc::severity::optional, 0x04, "bar=%d", 99);
	test.push_back(abc::category::abc::base, abc::severity::important, 0x05, "foo=%d", 42);
	test.push_back(abc::category::abc::base, abc::severity::warning, 0x06, "bar=%d", 99);
	test.push_back(abc::category::abc::base, abc::severity::critical, 0x07, "bar=%d", 99);

	std::cout << std::endl;

	abc::log blank(std::move(abc::log_container::ostream()), std::move(abc::log_view::blank()), std::move(abc::log_filter::none()));
	blank.push_back(abc::category::any, abc::severity::off, 0, "foo=%d", 42);
	blank.push_back(abc::category::any, abc::severity::off, 0, "bar=%d", 99);
	blank.push_back(abc::category::any, abc::severity::off, 0, "foo=%d", 42);
	blank.push_back(abc::category::any, abc::severity::off, 0, "bar=%d", 99);
	blank.push_back(abc::category::any, abc::severity::off, 0, "foo=%d", 42);
	blank.push_back(abc::category::any, abc::severity::off, 0, "bar=%d", 99);
	blank.push_back(abc::category::any, abc::severity::off, 0, "bar=%d", 99);

	std::cout << std::endl;

	return 0;
}
