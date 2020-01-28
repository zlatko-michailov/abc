#include <iostream>

#include "../src/timestamp.h"
#include "../src/log.h"


int main() {
	abc::timestamp ts1;

	std::cout << ts1.year() << "-" << ts1.month() << "-" << ts1.day() << std::endl;

	std::cout << std::endl;

	abc::log debug(std::move(abc::log_container::ostream()), std::move(abc::log_view::debug()), std::move(abc::log_filter::none()));
	debug.push_back(abc::category::abc::base, abc::severity::critical, 0x101, "foo=%d", 42);
	debug.push_back(abc::category::abc::base, abc::severity::warning, 0x102, "bar=%d", 99);
	debug.push_back(abc::category::abc::base, abc::severity::important, 0x103, "foo=%d", 42);
	debug.push_back(abc::category::abc::base, abc::severity::optional, 0x104, "bar=%d", 99);
	debug.push_back(abc::category::abc::base, abc::severity::important, 0x105, "foo=%d", 42);
	debug.push_back(abc::category::abc::base, abc::severity::warning, 0x106, "bar=%d", 99);
	debug.push_back(abc::category::abc::base, abc::severity::critical, 0x107, "bar=%d", 99);

	std::cout << std::endl;

	abc::log diag(std::move(abc::log_container::ostream()), std::move(abc::log_view::diag()), std::move(abc::log_filter::none()));
	diag.push_back(abc::category::abc::base, abc::severity::critical, 0x101, "foo=%d", 42);
	diag.push_back(abc::category::abc::base, abc::severity::warning, 0x102, "bar=%d", 99);
	diag.push_back(abc::category::abc::base, abc::severity::important, 0x103, "foo=%d", 42);
	diag.push_back(abc::category::abc::base, abc::severity::optional, 0x104, "bar=%d", 99);
	diag.push_back(abc::category::abc::base, abc::severity::important, 0x105, "foo=%d", 42);
	diag.push_back(abc::category::abc::base, abc::severity::warning, 0x106, "bar=%d", 99);
	diag.push_back(abc::category::abc::base, abc::severity::critical, 0x107, "bar=%d", 99);

	std::cout << std::endl;

	abc::log test(std::move(abc::log_container::ostream()), std::move(abc::log_view::test()), std::move(abc::log_filter::none()));
	test.push_back(abc::category::abc::base, abc::severity::critical, 0x101, "foo=%d", 42);
	test.push_back(abc::category::abc::base, abc::severity::warning, 0x102, "bar=%d", 99);
	test.push_back(abc::category::abc::base, abc::severity::important, 0x103, "foo=%d", 42);
	test.push_back(abc::category::abc::base, abc::severity::optional, 0x104, "bar=%d", 99);
	test.push_back(abc::category::abc::base, abc::severity::important, 0x105, "foo=%d", 42);
	test.push_back(abc::category::abc::base, abc::severity::warning, 0x106, "bar=%d", 99);
	test.push_back(abc::category::abc::base, abc::severity::critical, 0x107, "bar=%d", 99);

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
