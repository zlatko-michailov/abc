#pragma once

#include <string>
#include <functional>
#include <map>
#include <utility>
#include <initializer_list>

#include "log.h"


namespace abc {

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
		test_context(const char* category_name, const char* method_name, Log& log, unsigned seed) noexcept;

		const char*		category_name;
		const char* 	method_name;
		Log&			log;
		unsigned		seed;
	};


	template <typename Log>
	struct test_suite {
		//test_suite(std::unordered_map<std::string, test_category<Log>>&& categories) noexcept;
		test_suite(std::initializer_list<std::pair<std::string, test_category<Log>>> init, Log&& log, unsigned seed) noexcept;

		bool run() noexcept;

		std::unordered_map<std::string, test_category<Log>>		categories;
		Log														log;
		unsigned												seed;
	};


	// --------------------------------------------------------------


	template <typename Log>
	test_context<Log>::test_context(const char* category_name, const char* method_name, Log& log, unsigned seed) noexcept
		: category_name(category_name)
		, method_name(method_name)
		, log(log)
		, seed(seed) {
	}


	/*template <typename Log>
	inline test_suite<Log>::test_suite(std::unordered_map<std::string, test_category<Log>>&& categories, Log&& log, unsigned seed) noexcept
		: categories(std::move(categories))
		, log(std::move(log))
		, seed(seed) {
	}*/

	template <typename Log>
	inline test_suite<Log>::test_suite(std::initializer_list<std::pair<std::string, test_category<Log>>> init, Log&& log, unsigned seed) noexcept
		: categories(init.begin(), init.end())
		, log(std::move(log))
		, seed(seed) {
	}


	template <typename Log>
	inline bool test_suite<Log>::run() noexcept {
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
				catch(...) {
					method_passed = false;
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
			log.push_back(category::any, severity::critical, tag::none, "%s%sPASS%s", color::begin, color::green, color::end);
		}
		else {
			log.push_back(category::any, severity::critical, tag::none, "%s%sFAIL%s", color::begin, color::red, color::end);
		}

		log.push_back_blank(category::any, severity::critical);

		return all_passed;
	}

}
