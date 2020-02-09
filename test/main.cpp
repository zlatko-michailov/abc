#include <iostream>

#include "../src/test.h"
#include "../src/timestamp.h"
#include "../src/log.h"
#include "../src/streambuf.h"

#include "timestamp.h"


int main() {
	abc::timestamp ts1(nullptr);
	std::cout << ts1.year() << "-" << ts1.month() << "-" << ts1.day() << std::endl;

	abc::timestamp ts2;
	std::cout << ts2.year() << "-" << ts2.month() << "-" << ts2.day() << std::endl;

	char s[31];
	memset(s, '+', 30);
	abc::streambuf_adapter streambuf(s, 0, 30);
	std::ostream stream(&streambuf);
	stream << "thread_id=" << std::hex << std::this_thread::get_id() << '\0';
	std::cout << s << std::endl;

	std::cout << std::endl;

	abc::log debug(std::move(abc::log_container::ostream(std::clog.rdbuf())), std::move(abc::log_view::debug<>()), std::move(abc::log_filter::none()));
	debug.push_back(abc::category::abc::base, abc::severity::critical, 0x101, "foo=%d", 42);
	debug.push_back(abc::category::abc::base, abc::severity::warning, 0x102, "bar=%d", 99);
	debug.push_back(abc::category::abc::base, abc::severity::important, 0x103, "foo=%d", 42);
	debug.push_back(abc::category::abc::base, abc::severity::optional, 0x104, "bar=%d", 99);
	debug.push_back(abc::category::abc::base, abc::severity::important, 0x105, "foo=%d", 42);
	debug.push_back(abc::category::abc::base, abc::severity::warning, 0x106, "bar=%d", 99);
	debug.push_back(abc::category::abc::base, abc::severity::critical, 0x107, "bar=%d", 99);

	std::cout << std::endl;

	abc::log diag(std::move(abc::log_container::ostream(std::clog.rdbuf())), std::move(abc::log_view::diag<>()), std::move(abc::log_filter::none()));
	diag.push_back(abc::category::abc::base, abc::severity::critical, 0x101, "foo=%d", 42);
	diag.push_back(abc::category::abc::base, abc::severity::warning, 0x102, "bar=%d", 99);
	diag.push_back(abc::category::abc::base, abc::severity::important, 0x103, "foo=%d", 42);
	diag.push_back(abc::category::abc::base, abc::severity::optional, 0x104, "bar=%d", 99);
	diag.push_back(abc::category::abc::base, abc::severity::important, 0x105, "foo=%d", 42);
	diag.push_back(abc::category::abc::base, abc::severity::warning, 0x106, "bar=%d", 99);
	diag.push_back(abc::category::abc::base, abc::severity::critical, 0x107, "bar=%d", 99);

	std::cout << std::endl;

	abc::log test(std::move(abc::log_container::ostream(std::clog.rdbuf())), std::move(abc::log_view::test<>()), std::move(abc::log_filter::none()));
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

	abc::log fdebug(std::move(abc::log_container::file("out/diag")), std::move(abc::log_view::debug<>()), std::move(abc::log_filter::none()));
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

	{
		abc::test_log test_log1(std::move(abc::log_container::ostream(std::clog.rdbuf())), std::move(abc::log_view::test<>()), std::move(abc::log_filter::severity(abc::severity::important)));

		abc::test_suite<> test_suite1 ( {
			{ "hacks", {
				{ "hack1", abc::test::timestamp::test_null_timestamp },
				{ "hack2", [=](abc::test_context<>& context) { return false; } },
				{ "hack3", [=](abc::test_context<>& context) { return true; } },
			} },
			{ "tests", {
				{ "hack1", [=](abc::test_context<>& context) { return false; } },
				{ "hack2", [=](abc::test_context<>& context) { return true; } },
				{ "hack3", [=](abc::test_context<>& context) { return true; } },
			} },
			{ "passes", {
				{ "hack1", [=](abc::test_context<>& context) { return true; } },
				{ "hack2", [=](abc::test_context<>& context) { return true; } },
				{ "hack3", [=](abc::test_context<>& context) { return true; } },
			} },
		},
		std::move(test_log1),
		0);

		test_suite1.run();
	}
	
	{
		abc::test_log test_log2(std::move(abc::log_container::ostream(std::clog.rdbuf())), std::move(abc::log_view::test<>()), std::move(abc::log_filter::severity(abc::severity::important)));

		std::map<std::string, abc::test_category<>> categories = {
			{ "hacks", {
				{ "hack1", [=](abc::test_context<>& context) { return true; } },
				{ "hack2", [=](abc::test_context<>& context) { return false; } },
				{ "hack3", [=](abc::test_context<>& context) { return true; } },
			} },
			{ "tests", {
				{ "hack1", [=](abc::test_context<>& context) { return false; } },
				{ "hack2", [=](abc::test_context<>& context) { return true; } },
				{ "hack3", [=](abc::test_context<>& context) { return true; } },
			} },
			{ "passes", {
				{ "hack1", [=](abc::test_context<>& context) { return true; } },
				{ "hack2", [=](abc::test_context<>& context) { return true; } },
				{ "hack3", [=](abc::test_context<>& context) { return true; } },
			} },
		};

		abc::test_suite<> test_suite2(std::move(categories), std::move(test_log2), 0);
	}


	abc::test_log test_log(
		std::move(abc::log_container::ostream(std::clog.rdbuf())),
		std::move(abc::log_view::test<>()),
		std::move(abc::log_filter::severity(abc::severity::optional)));

		abc::test_suite<> test_suite ( {
			{ "timestamp", {
				{ "test_null_timestamp", abc::test::timestamp::test_null_timestamp },
			} },
		},
		std::move(test_log),
		0);

		test_suite.run();
	
	return 0;
}
