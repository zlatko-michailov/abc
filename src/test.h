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

	using seed_t = unsigned;

	namespace seed {
		constexpr seed_t random	= 0;
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	struct test_context {
		test_context(const char* category_name, const char* method_name, const LogPtr& log_ptr, seed_t seed = seed::random) noexcept;

		template <typename Value>
		bool are_equal(const Value& actual, const Value& expected, tag_t tag, const char* format);
		bool are_equal(const char* actual, const char* expected, tag_t tag);
		bool are_equal(const void* actual, const void* expected, std::size_t size, tag_t tag);

		const char*		category_name;
		const char* 	method_name;
		LogPtr			log_ptr;
		seed_t			seed;
	};


	// --------------------------------------------------------------


	template <typename LogPtr>
	using test_method = std::function<bool(test_context<LogPtr>&)>;

	template <typename LogPtr>
	using named_test_method = std::pair<std::string, test_method<LogPtr>>;

	template <typename LogPtr>
	using test_category = std::vector<named_test_method<LogPtr>>;

	template <typename LogPtr>
	using named_test_category = std::pair<std::string, test_category<LogPtr>>;


	// --------------------------------------------------------------


	template <typename LogPtr>
	struct test_suite {
		test_suite() noexcept = default;
		test_suite(std::vector<named_test_category<LogPtr>>&& categories, const LogPtr& log_ptr, seed_t seed) noexcept;
		test_suite(std::initializer_list<named_test_category<LogPtr>> init, const LogPtr& log_ptr, seed_t seed) noexcept;

		bool run() noexcept;

		std::vector<named_test_category<LogPtr>>	categories;
		LogPtr										log_ptr;
		seed_t										seed;

	private:
		void srand() noexcept;
	};


	// --------------------------------------------------------------


	template <typename LogPtr>
	test_context<LogPtr>::test_context(const char* category_name, const char* method_name, const LogPtr& log_ptr, seed_t seed) noexcept
		: category_name(category_name)
		, method_name(method_name)
		, log_ptr(log_ptr)
		, seed(seed) {
	}


	template <typename LogPtr>
	template <typename Value>
	inline bool test_context<LogPtr>::are_equal(const Value& actual, const Value& expected, tag_t tag, const char* format) {
		bool are_equal = actual == expected;

		char line_format[size::k2];
		if (!are_equal) {
			std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Fail: are_equal(actual=%s, expected=%s)", format, format);
			if (log_ptr != nullptr) {
				log_ptr->put_any(category::any, severity::important, tag, line_format, actual, expected);
			}
		}
		else {
			std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Pass: are_equal(actual=%s, expected=%s)", format, format);
			if (log_ptr != nullptr) {
				log_ptr->put_any(category::any, severity::optional, tag, line_format, actual, expected);
			}
		}

		return are_equal;
	}


	template <typename LogPtr>
	inline bool test_context<LogPtr>::are_equal(const char* actual, const char* expected, tag_t tag) {
		bool are_equal = std::strcmp(actual, expected) == 0;

		char line_format[size::k2];
		if (!are_equal) {
			std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Fail: are_equal(actual=%%s, expected=%%s)");
			if (log_ptr != nullptr) {
				log_ptr->put_any(category::any, severity::important, tag, line_format, actual, expected);
			}
		}
		else {
			std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Pass: are_equal(actual=%%s, expected=%%s)");
			if (log_ptr != nullptr) {
				log_ptr->put_any(category::any, severity::optional, tag, line_format, actual, expected);
			}
		}

		return are_equal;
	}


	template <typename LogPtr>
	inline bool test_context<LogPtr>::are_equal(const void* actual, const void* expected, std::size_t size, tag_t tag) {
		bool are_equal = std::memcmp(actual, expected, size) == 0;

		std::size_t offset = 0;
		for (;;) {
			std::size_t original_offset = offset;

			line_ostream<size::k2> line_actual;
			if (line_actual.put_binary(actual, size, offset) == 0) {
				break;
			};

			line_ostream<size::k2> line_expected;
			line_expected.put_binary(expected, size, original_offset);

			char line_format[size::k2];
			if (!are_equal) {
				std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Fail: are_equal(actual=%%s, expected=%%s)");
				if (log_ptr != nullptr) {
					log_ptr->put_any(category::any, severity::important, tag, line_format, line_actual.get(), line_expected.get());
				}
			}
			else {
				std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Pass: are_equal(actual=%%s, expected=%%s)");
				if (log_ptr != nullptr) {
					log_ptr->put_any(category::any, severity::optional, tag, line_format, line_actual.get(), line_expected.get());
				}
			}
		}

		return are_equal;
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline test_suite<LogPtr>::test_suite(std::vector<named_test_category<LogPtr>>&& categories, const LogPtr& log_ptr, seed_t seed) noexcept
		: categories(std::move(categories))
		, log_ptr(log_ptr)
		, seed(seed) {
	}

	template <typename LogPtr>
	inline test_suite<LogPtr>::test_suite(std::initializer_list<named_test_category<LogPtr>> init, const LogPtr& log_ptr, seed_t seed) noexcept
		: categories(init.begin(), init.end())
		, log_ptr(log_ptr)
		, seed(seed) {
	}


	template <typename LogPtr>
	inline bool test_suite<LogPtr>::run() noexcept {
		srand();

		bool all_passed = true;

		if (log_ptr != nullptr) {
			log_ptr->put_blank_line();
			log_ptr->put_blank_line();
		}

		// Category
		for (auto category_it = categories.begin(); category_it != categories.end(); category_it++) {
			bool category_passed = true;

			if (log_ptr != nullptr) {
				log_ptr->put_any(category::any, severity::critical, tag::none, ">>   %s%s%s%s", color::begin, color::cyan, category_it->first.c_str(), color::end);
			}

			// Method
			for (auto method_it = category_it->second.begin(); method_it != category_it->second.end(); method_it++) {
				bool method_passed = true;

				if (log_ptr != nullptr) {
					log_ptr->put_any(category::any, severity::warning, tag::none, ">>   %s", method_it->first.c_str());
				}

				try {
					test_context<LogPtr> context(category_it->first.c_str(), method_it->first.c_str(), log_ptr, seed);
					method_passed = method_it->second(context);
				}
				catch(const std::exception& ex) {
					method_passed = false;
					if (log_ptr != nullptr) {
						log_ptr->put_any(category::any, severity::critical, tag::none, "    %s%sEXCEPTION%s %s", color::begin, color::red, color::end, ex.what());
					}
				}

				if (method_passed) {
					if (log_ptr != nullptr) {
						log_ptr->put_any(category::any, severity::critical, tag::none, "  %s%sPASS%s %s", color::begin, color::green, color::end, method_it->first.c_str());
					}
				}
				else {
					if (log_ptr != nullptr) {
						log_ptr->put_any(category::any, severity::critical, tag::none, "  %s%sFAIL%s %s", color::begin, color::red, color::end, method_it->first.c_str());
					}
				}

				category_passed = category_passed && method_passed;
			} // method

			if (category_passed) {
				if (log_ptr != nullptr) {
					log_ptr->put_any(category::any, severity::critical, tag::none, "%s%sPASS%s %s%s%s%s", color::begin, color::green, color::end, color::begin, color::cyan, category_it->first.c_str(), color::end);
				}
			}
			else {
				if (log_ptr != nullptr) {
					log_ptr->put_any(category::any, severity::critical, tag::none, "%s%sFAIL%s %s%s%s%s", color::begin, color::red, color::end, color::begin, color::cyan, category_it->first.c_str(), color::end);
				}
			}

			if (log_ptr != nullptr) {
				log_ptr->put_blank_line();
			}
			all_passed = all_passed && category_passed;
		} // category

		if (all_passed) {
			if (log_ptr != nullptr) {
				log_ptr->put_any(category::any, severity::critical, tag::none, "%s%sPASS%s seed=%u", color::begin, color::green, color::end, seed);
			}
		}
		else {
			if (log_ptr != nullptr) {
				log_ptr->put_any(category::any, severity::critical, tag::none, "%s%sFAIL%s seed=%u", color::begin, color::red, color::end, seed);
			}
		}

		if (log_ptr != nullptr) {
			log_ptr->put_blank_line();
			log_ptr->put_blank_line();
		}

		return all_passed;
	}


	template <typename LogPtr>
	inline void test_suite<LogPtr>::srand() noexcept {
		if (seed == seed::random) {
			std::srand(static_cast<seed_t>(std::chrono::system_clock::now().time_since_epoch().count() % RAND_MAX));
			seed = std::rand();
		}

		std::srand(seed);
	}

}
