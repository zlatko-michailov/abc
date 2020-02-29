/*
MIT License

Copyright (c) 2018-2020 Zlatko Michailov 

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


#pragma once

#include <cstdlib>
#include <string>
#include <functional>
#include <map>
#include <utility>
#include <initializer_list>

#include "log.h"


namespace abc {

	typedef unsigned seed_t;
	namespace seed {
		constexpr unsigned random	= 0;
	}


	using test_log = log<size::k4, log_container::ostream, abc::log_view::test<>, abc::log_filter::severity>;

	template <typename Log = test_log>
	struct test_context;

	template <typename Log = test_log>
	using test_method = std::function<bool(test_context<Log>&)>;

	template <typename Log = test_log>
	using test_category = std::unordered_map<std::string, test_method<Log>>;

	template <typename Log = test_log>
	struct test_suite;


	// --------------------------------------------------------------


	template <typename Log>
	struct test_context {
		test_context(const char* category_name, const char* method_name, Log& log, seed_t seed = seed::random) noexcept;

		template <typename Value>
		bool are_equal(const Value& actual, const Value& expected, tag_t tag, const char* format);
		bool are_equal(const char* actual, const char* expected, tag_t tag);

		const char*		category_name;
		const char* 	method_name;
		Log&			log;
		unsigned		seed;
	};


	template <typename Log>
	struct test_suite {
		test_suite() noexcept = default;
		test_suite(std::unordered_map<std::string, test_category<Log>>&& categories, Log&& log, seed_t seed) noexcept;
		test_suite(std::initializer_list<std::pair<std::string, test_category<Log>>> init, Log&& log, seed_t seed) noexcept;

		bool run() noexcept;

		std::unordered_map<std::string, test_category<Log>>		categories;
		Log														log;
		seed_t													seed;

	private:
		void srand() noexcept;
	};


	// --------------------------------------------------------------


	template <typename Log>
	test_context<Log>::test_context(const char* category_name, const char* method_name, Log& log, seed_t seed) noexcept
		: category_name(category_name)
		, method_name(method_name)
		, log(log)
		, seed(seed) {
	}


	template <typename Log>
	template <typename Value>
	inline bool test_context<Log>::are_equal(const Value& actual, const Value& expected, tag_t tag, const char* format) {
		bool are_equal = actual == expected;

		char line_format[size::k1];
		if (!are_equal) {
			std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Fail: are_equal(actual=%s, expected=%s)", format, format);
			log.push_back(category::any, severity::important, tag, line_format, actual, expected);
		}
		else {
			std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Pass: are_equal(actual=%s, expected=%s)", format, format);
			log.push_back(category::any, severity::optional, tag, line_format, actual, expected);
		}

		return are_equal;
	}


	template <typename Log>
	inline bool test_context<Log>::are_equal(const char* actual, const char* expected, tag_t tag) {
		bool are_equal = std::strcmp(actual, expected) == 0;

		char line_format[size::k1];
		if (!are_equal) {
			std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Fail: are_equal(actual=%%s, expected=%%s)");
			log.push_back(category::any, severity::important, tag, line_format, actual, expected);
		}
		else {
			std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Pass: are_equal(actual=%%s, expected=%%s)");
			log.push_back(category::any, severity::optional, tag, line_format, actual, expected);
		}

		return are_equal;
	}


	template <typename Log>
	inline test_suite<Log>::test_suite(std::unordered_map<std::string, test_category<Log>>&& categories, Log&& log, seed_t seed) noexcept
		: categories(std::move(categories))
		, log(std::move(log))
		, seed(seed) {
	}

	template <typename Log>
	inline test_suite<Log>::test_suite(std::initializer_list<std::pair<std::string, test_category<Log>>> init, Log&& log, seed_t seed) noexcept
		: categories(init.begin(), init.end())
		, log(std::move(log))
		, seed(seed) {
	}


	template <typename Log>
	inline bool test_suite<Log>::run() noexcept {
		srand();

		bool all_passed = true;

		log.push_back_blank(category::any, severity::critical);

		// Category
		for (auto category_it = categories.begin(); category_it != categories.end(); category_it++) {
			bool category_passed = true;

			log.push_back(category::any, severity::critical, tag::none, ">>   %s%s%s%s", color::begin, color::cyan, category_it->first.c_str(), color::end);

			// Method
			for (auto method_it = category_it->second.begin(); method_it != category_it->second.end(); method_it++) {
				bool method_passed = true;

				log.push_back(category::any, severity::warning, tag::none, ">>   %s", method_it->first.c_str());

				try {
					test_context<Log> context(category_it->first.c_str(), method_it->first.c_str(), log, seed);
					method_passed = method_it->second(context);
				}
				catch(const std::exception& ex) {
					method_passed = false;
					log.push_back(category::any, severity::critical, tag::none, "    %s%sEXCEPTION%s %s", color::begin, color::red, color::end, ex.what());
				}

				if (method_passed) {
					log.push_back(category::any, severity::critical, tag::none, "  %s%sPASS%s %s", color::begin, color::green, color::end, method_it->first.c_str());
				}
				else {
					log.push_back(category::any, severity::critical, tag::none, "  %s%sFAIL%s %s", color::begin, color::red, color::end, method_it->first.c_str());
				}

				category_passed = category_passed && method_passed;
			} // method

			if (category_passed) {
				log.push_back(category::any, severity::critical, tag::none, "%s%sPASS%s %s%s%s%s", color::begin, color::green, color::end, color::begin, color::cyan, category_it->first.c_str(), color::end);
			}
			else {
				log.push_back(category::any, severity::critical, tag::none, "%s%sFAIL%s %s%s%s%s", color::begin, color::red, color::end, color::begin, color::cyan, category_it->first.c_str(), color::end);
			}

			log.push_back_blank(category::any, severity::critical);
			all_passed = all_passed && category_passed;
		} // category

		if (all_passed) {
			log.push_back(category::any, severity::critical, tag::none, "%s%sPASS%s seed=%u", color::begin, color::green, color::end, seed);
		}
		else {
			log.push_back(category::any, severity::critical, tag::none, "%s%sFAIL%s seed=%u", color::begin, color::red, color::end, seed);
		}

		log.push_back_blank(category::any, severity::critical);

		return all_passed;
	}

	template <typename Log>
	inline void test_suite<Log>::srand() noexcept {
		if (seed == seed::random) {
			std::srand(static_cast<seed_t>(std::chrono::system_clock::now().time_since_epoch().count() % RAND_MAX));
			seed = std::rand();
		}

		std::srand(seed);
	}

}
