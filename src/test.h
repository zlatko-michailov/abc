/*
MIT License

Copyright (c) 2018-2023 Zlatko Michailov 

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
#include <vector>
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


	/**
	 * @brief					Utility passed into each test method to perform verification and logging.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log>
	struct test_context {
		/**
		 * @brief				Constructor.
		 * @param category_name	Test category name.
		 * @param method_name	Test method name.
		 * @param log			Pointer to a `log_ostream` instance.
		 * @param seed			Randomization seed. Used to repeat a previous test run.
		 */
		test_context(const char* category_name, const char* method_name, Log* log, seed_t seed = seed::random) noexcept;

		/**
		 * @brief				Verifies an actual value matches the expected one.
		 * @tparam Value		Type of the value.
		 * @param actual		Actual value.
		 * @param expected		Expected value.
		 * @param tag			Unique tag.
		 * @param format		C-style format to print a value of type `Value`.
		 * @return				`true` = pass; `false` = fail.
		 */
		template <typename Value>
		bool are_equal(const Value& actual, const Value& expected, tag_t tag, const char* format);

		/**
		 * @brief				Verifies an actual string value matches the expected one.
		 * @param actual		Actual string value.
		 * @param expected		Expected string value.
		 * @param tag			Unique tag.
		 * @return				`true` = pass; `false` = fail.
		 */
		bool are_equal(const char* actual, const char* expected, tag_t tag);

		/**
		 * @brief				Verifies an actual binary blob matches the expected one.
		 * @param actual		Actual blob.
		 * @param expected		Expected blob.
		 * @param size			Siz of the blob.
		 * @param tag			Unique tag.
		 * @return				`true` = pass; `false` = fail.
		 */
		bool are_equal(const void* actual, const void* expected, std::size_t size, tag_t tag);

		const char*		category_name;
		const char* 	method_name;
		Log*			log;
		seed_t			seed;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					Bare test method.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log>
	using test_method = std::function<bool(test_context<Log>&)>;

	/**
	 * @brief					Pair of a name and a test method.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log>
	using named_test_method = std::pair<std::string, test_method<Log>>;

	/**
	 * @brief					Bare collection of named test methods.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log>
	using test_category = std::vector<named_test_method<Log>>;

	/**
	 * @brief					Pair of a name and a collection of named test methods.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log>
	using named_test_category = std::pair<std::string, test_category<Log>>;


	// --------------------------------------------------------------


	/**
	 * @brief					Complete test suite.
	 * @tparam Log				Logging facility.
	 */
	template <typename Log>
	struct test_suite {
		/**
		 * @brief				Default constructor.
		 */
		test_suite() noexcept = default;

		/**
		 * @brief				Constructor. Vector version.
		 * @param categories	Collection of named categories.
		 * @param log			Pointer to a `log_ostream` instance.
		 * @param seed			Randomization seed. Used to repeat a previous test run.
		 */
		test_suite(std::vector<named_test_category<Log>>&& categories, Log* log, seed_t seed) noexcept;

		/**
		 * @brief				Constructor. initializer list version.
		 * @param init			Initializer list of named categories.
		 * @param log			Pointer to a `log_ostream` instance.
		 * @param seed			Randomization seed. Used to repeat a previous test run.
		 */
		test_suite(std::initializer_list<named_test_category<Log>> init, Log* log, seed_t seed) noexcept;

		/**
		 * @brief				Executes all test methods of all test categories.
		 * @return				`true` = pass; `false` = fail.
		 */
		bool run() noexcept;

		std::vector<named_test_category<Log>>	categories;
		Log*									log;
		seed_t									seed;

	private:
		void srand() noexcept;
	};


	// --------------------------------------------------------------


	template <typename Log>
	test_context<Log>::test_context(const char* category_name, const char* method_name, Log* log, seed_t seed) noexcept
		: category_name(category_name)
		, method_name(method_name)
		, log(log)
		, seed(seed) {
	}


	template <typename Log>
	template <typename Value>
	inline bool test_context<Log>::are_equal(const Value& actual, const Value& expected, tag_t tag, const char* format) {
		bool are_equal = actual == expected;

		char line_format[size::k2];
		if (!are_equal) {
			std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Fail: are_equal(actual=%s, expected=%s)", format, format);
			if (log != nullptr) {
				log->put_any(category::any, severity::important, tag, line_format, actual, expected);
			}
		}
		else {
			std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Pass: are_equal(actual=%s, expected=%s)", format, format);
			if (log != nullptr) {
				log->put_any(category::any, severity::optional, tag, line_format, actual, expected);
			}
		}

		return are_equal;
	}


	template <typename Log>
	inline bool test_context<Log>::are_equal(const char* actual, const char* expected, tag_t tag) {
		bool are_equal = std::strcmp(actual, expected) == 0;

		char line_format[size::k2];
		if (!are_equal) {
			std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Fail: are_equal(actual=%%s, expected=%%s)");
			if (log != nullptr) {
				log->put_any(category::any, severity::important, tag, line_format, actual, expected);
			}
		}
		else {
			std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Pass: are_equal(actual=%%s, expected=%%s)");
			if (log != nullptr) {
				log->put_any(category::any, severity::optional, tag, line_format, actual, expected);
			}
		}

		return are_equal;
	}


	template <typename Log>
	inline bool test_context<Log>::are_equal(const void* actual, const void* expected, std::size_t size, tag_t tag) {
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
				if (log != nullptr) {
					log->put_any(category::any, severity::important, tag, line_format, line_actual.get(), line_expected.get());
				}
			}
			else {
				std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Pass: are_equal(actual=%%s, expected=%%s)");
				if (log != nullptr) {
					log->put_any(category::any, severity::optional, tag, line_format, line_actual.get(), line_expected.get());
				}
			}
		}

		return are_equal;
	}


	// --------------------------------------------------------------


	template <typename Log>
	inline test_suite<Log>::test_suite(std::vector<named_test_category<Log>>&& categories, Log* log, seed_t seed) noexcept
		: categories(std::move(categories))
		, log(log)
		, seed(seed) {
	}

	template <typename Log>
	inline test_suite<Log>::test_suite(std::initializer_list<named_test_category<Log>> init, Log* log, seed_t seed) noexcept
		: categories(init.begin(), init.end())
		, log(log)
		, seed(seed) {
	}


	template <typename Log>
	inline bool test_suite<Log>::run() noexcept {
		srand();

		bool all_passed = true;

		if (log != nullptr) {
			log->put_blank_line();
			log->put_blank_line();
			log->put_blank_line();
		}

		// Category
		for (auto category_it = categories.begin(); category_it != categories.end(); category_it++) {
			bool category_passed = true;

			if (log != nullptr) {
				log->put_any(category::any, severity::critical, tag::none, ">>   %s%s%s%s", color::begin, color::cyan, category_it->first.c_str(), color::end);
			}

			// Method
			for (auto method_it = category_it->second.begin(); method_it != category_it->second.end(); method_it++) {
				bool method_passed = true;

				if (log != nullptr) {
					log->put_any(category::any, severity::warning, tag::none, ">>   %s", method_it->first.c_str());
				}

				try {
					test_context<Log> context(category_it->first.c_str(), method_it->first.c_str(), log, seed);
					method_passed = method_it->second(context);
				}
				catch(const std::exception& ex) {
					method_passed = false;
					if (log != nullptr) {
						log->put_any(category::any, severity::critical, tag::none, "    %s%sEXCEPTION%s %s", color::begin, color::red, color::end, ex.what());
					}
				}

				if (method_passed) {
					if (log != nullptr) {
						log->put_any(category::any, severity::critical, tag::none, "  %s%sPASS%s %s", color::begin, color::green, color::end, method_it->first.c_str());
					}
				}
				else {
					if (log != nullptr) {
						log->put_any(category::any, severity::critical, tag::none, "  %s%sFAIL%s %s", color::begin, color::red, color::end, method_it->first.c_str());
					}
				}

				category_passed = category_passed && method_passed;
			} // method

			if (category_passed) {
				if (log != nullptr) {
					log->put_any(category::any, severity::critical, tag::none, "%s%sPASS%s %s%s%s%s", color::begin, color::green, color::end, color::begin, color::cyan, category_it->first.c_str(), color::end);
				}
			}
			else {
				if (log != nullptr) {
					log->put_any(category::any, severity::critical, tag::none, "%s%sFAIL%s %s%s%s%s", color::begin, color::red, color::end, color::begin, color::cyan, category_it->first.c_str(), color::end);
				}
			}

			if (log != nullptr) {
				log->put_blank_line();
			}
			all_passed = all_passed && category_passed;
		} // category

		// Summary
		if (log != nullptr) {
			log->put_blank_line();
			log->put_any(category::any, severity::critical, tag::none, ">>   %s%ssummary%s", color::begin, color::cyan, color::end);
			log->put_any(category::any, severity::warning, tag::none, "seed = %u", seed);
		}

		if (all_passed) {
			if (log != nullptr) {
				log->put_any(category::any, severity::critical, tag::none, "%s%sPASS%s %s%ssummary%s", color::begin, color::green, color::end, color::begin, color::cyan, color::end);
			}
		}
		else {
			if (log != nullptr) {
				log->put_any(category::any, severity::critical, tag::none, "%s%sFAIL%s %s%ssummary%s", color::begin, color::red, color::end, color::begin, color::cyan, color::end);
			}
		}

		if (log != nullptr) {
			log->put_blank_line();
			log->put_blank_line();
			log->put_blank_line();
		}

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
