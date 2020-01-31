#include <iostream>

#include "../src/timestamp.h"
#include "../src/log.h"


int main() {
	abc::timestamp ts1;

	std::cout << ts1.year() << "-" << ts1.month() << "-" << ts1.day() << std::endl;

	std::cout << std::endl;

	abc::log debug(std::move(abc::log_container::ostream(std::clog.rdbuf())), std::move(abc::log_view::debug()), std::move(abc::log_filter::none()));
	debug.push_back(abc::category::abc::base, abc::severity::critical, 0x101, "foo=%d", 42);
	debug.push_back(abc::category::abc::base, abc::severity::warning, 0x102, "bar=%d", 99);
	debug.push_back(abc::category::abc::base, abc::severity::important, 0x103, "foo=%d", 42);
	debug.push_back(abc::category::abc::base, abc::severity::optional, 0x104, "bar=%d", 99);
	debug.push_back(abc::category::abc::base, abc::severity::important, 0x105, "foo=%d", 42);
	debug.push_back(abc::category::abc::base, abc::severity::warning, 0x106, "bar=%d", 99);
	debug.push_back(abc::category::abc::base, abc::severity::critical, 0x107, "bar=%d", 99);

	std::cout << std::endl;

	abc::log diag(std::move(abc::log_container::ostream(std::clog.rdbuf())), std::move(abc::log_view::diag()), std::move(abc::log_filter::none()));
	diag.push_back(abc::category::abc::base, abc::severity::critical, 0x101, "foo=%d", 42);
	diag.push_back(abc::category::abc::base, abc::severity::warning, 0x102, "bar=%d", 99);
	diag.push_back(abc::category::abc::base, abc::severity::important, 0x103, "foo=%d", 42);
	diag.push_back(abc::category::abc::base, abc::severity::optional, 0x104, "bar=%d", 99);
	diag.push_back(abc::category::abc::base, abc::severity::important, 0x105, "foo=%d", 42);
	diag.push_back(abc::category::abc::base, abc::severity::warning, 0x106, "bar=%d", 99);
	diag.push_back(abc::category::abc::base, abc::severity::critical, 0x107, "bar=%d", 99);

	std::cout << std::endl;

	abc::log test(std::move(abc::log_container::ostream(std::clog.rdbuf())), std::move(abc::log_view::test()), std::move(abc::log_filter::none()));
	test.push_back(abc::category::abc::base, abc::severity::critical, 0x101, "foo=%d", 42);
	test.push_back(abc::category::abc::base, abc::severity::warning, 0x102, "bar=%d", 99);
	test.push_back(abc::category::abc::base, abc::severity::important, 0x103, "foo=%d", 42);
	test.push_back(abc::category::abc::base, abc::severity::optional, 0x104, "bar=%d", 99);
	test.push_back(abc::category::abc::base, abc::severity::important, 0x105, "foo=%d", 42);
	test.push_back(abc::category::abc::base, abc::severity::warning, 0x106, "bar=%d", 99);
	test.push_back(abc::category::abc::base, abc::severity::critical, 0x107, "bar=%d", 99);

	std::cout << std::endl;

	abc::log blank(std::move(abc::log_container::ostream(std::clog.rdbuf())), std::move(abc::log_view::blank()), std::move(abc::log_filter::none()));
	blank.push_back(abc::category::any, abc::severity::off, 0, "foo=%d", 42);
	blank.push_back(abc::category::any, abc::severity::off, 0, "bar=%d", 99);
	blank.push_back(abc::category::any, abc::severity::off, 0, "foo=%d", 42);
	blank.push_back(abc::category::any, abc::severity::off, 0, "bar=%d", 99);
	blank.push_back(abc::category::any, abc::severity::off, 0, "foo=%d", 42);
	blank.push_back(abc::category::any, abc::severity::off, 0, "bar=%d", 99);
	blank.push_back(abc::category::any, abc::severity::off, 0, "bar=%d", 99);

	std::cout << std::endl;

	abc::log fdebug(std::move(abc::log_container::file("out/diag")), std::move(abc::log_view::debug()), std::move(abc::log_filter::none()));
	fdebug.push_back(abc::category::abc::base, abc::severity::critical, 0x101, "foo=%d", 12);
	fdebug.push_back(abc::category::abc::base, abc::severity::warning, 0x102, "bar=%d", 13);
	fdebug.push_back(abc::category::abc::base, abc::severity::important, 0x103, "foo=%d", 14);
	fdebug.push_back(abc::category::abc::base, abc::severity::optional, 0x104, "bar=%d", 15);
	fdebug.push_back(abc::category::abc::base, abc::severity::important, 0x105, "foo=%d", 16);
	fdebug.push_back(abc::category::abc::base, abc::severity::warning, 0x106, "bar=%d", 17);
	fdebug.push_back(abc::category::abc::base, abc::severity::critical, 0x107, "bar=%d", 18);

	std::cout << std::endl;

	std::filebuf fb;
	std::ostream os(&fb);
	fb.open("out/foobar.txt", std::ios_base::out);
	os << "123" << std::endl << "99" << std::endl;

	return 0;
}
