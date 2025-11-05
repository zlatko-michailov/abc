/*
MIT License

Copyright (c) 2018-2025 Zlatko Michailov 

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

#include "../root/util.h"
#include "../diag/diag_ready.h"
#include "../diag/log.h"
#include "i/test.i.h"


namespace abc { namespace test {

    inline context::context(const char* category_name, const char* method_name, diag::log_ostream* log, seed_t seed, const char* process_path)
        : diag_base("abc::test::context", log)
        , category_name(category_name)
        , method_name(method_name)
        , seed(seed)
        , process_path(process_path)
        , suborigin(category_name) {

        suborigin.append("::").append(method_name);
    }


    template <typename Value>
    inline bool context::are_equal(const Value& actual, const Value& expected, diag::tag_t tag, const char* format) {
        // Compare.
        bool are_equal = actual == expected;

        // Format the outcome.
        char line_format[size::k2];
        if (!are_equal) {
            std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Fail: are_equal(actual=%s, expected=%s)", format, format);
            diag_base::put_any(suborigin.c_str(), abc::diag::severity::important, tag, line_format, actual, expected);
        }
        else {
            std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Pass: are_equal(actual=%s, expected=%s)", format, format);
            diag_base::put_any(suborigin.c_str(), abc::diag::severity::optional, tag, line_format, actual, expected);
        }

        return are_equal;
    }


    inline bool context::are_equal(const char* actual, const char* expected, diag::tag_t tag) {
        // Compare.
        bool are_equal = std::strcmp(actual, expected) == 0;

        // Format the outcome.
        char line_format[size::k2];
        if (!are_equal) {
            std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Fail: are_equal(actual=%%s, expected=%%s)");
            diag_base::put_any(suborigin.c_str(), abc::diag::severity::important, tag, line_format, actual, expected);
        }
        else {
            std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Pass: are_equal(actual=%%s, expected=%%s)");
            diag_base::put_any(suborigin.c_str(), abc::diag::severity::optional, tag, line_format, actual, expected);
        }

        return are_equal;
    }


    inline bool context::are_equal(const void* actual, const void* expected, std::size_t size, diag::tag_t tag) {
        // Compare.
        bool are_equal = std::memcmp(actual, expected, size) == 0;

        // Format the outcome.
        std::size_t offset = 0;
        for (;;) {
            std::size_t original_offset = offset;

            abc::stream::line_ostream line_actual;
            if (line_actual.put_binary(actual, size, offset) == 0) {
                break;
            };

            abc::stream::line_ostream line_expected;
            line_expected.put_binary(expected, size, original_offset);

            char line_format[size::k2];
            if (!are_equal) {
                std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Fail: are_equal(actual=%%s, expected=%%s)");
                diag_base::put_any(suborigin.c_str(), abc::diag::severity::important, tag, line_format, line_actual.get(), line_expected.get());
            }
            else {
                std::snprintf(line_format, sizeof(line_format) / sizeof(char), "Pass: are_equal(actual=%%s, expected=%%s)");
                diag_base::put_any(suborigin.c_str(), abc::diag::severity::optional, tag, line_format, line_actual.get(), line_expected.get());
            }
        }

        return are_equal;
    }


    inline diag::log_ostream* context::log() const noexcept {
        return diag_base::log();
    }


    // --------------------------------------------------------------


    template <typename ProcessStr>
    inline suite<ProcessStr>::suite(named_categories&& categories, diag::log_ostream* log, seed_t seed, ProcessStr&& process_path)
        : diag_base("abc::test::suite", log)
        , categories(std::move(categories))
        , seed(seed)
        , process_path(std::move(process_path)) {
    }


    template <typename ProcessStr>
    inline suite<ProcessStr>::suite(std::initializer_list<named_category> init, diag::log_ostream* log, seed_t seed, ProcessStr&& process_path)
        : diag_base("abc::test::suite", log)
        , categories(init.begin(), init.end())
        , seed(seed)
        , process_path(std::move(process_path)) {
    }


    template <typename ProcessStr>
    inline bool suite<ProcessStr>::run() noexcept {
        constexpr const char* suborigin = "run";

        srand();

        bool all_passed = true;

        diag_base::put_blank_line(diag::severity::critical);
        diag_base::put_blank_line(diag::severity::critical);
        diag_base::put_blank_line(diag::severity::critical);

        // Category
        for (auto category_itr = categories.begin(); category_itr != categories.end(); category_itr++) {
            bool category_passed = true;

            diag_base::put_any(suborigin, diag::severity::critical, diag::tag::none, ">>   %s%s%s%s", diag::color::begin, diag::color::cyan, category_itr->first.c_str(), diag::color::end);

            // Method
            for (auto method_itr = category_itr->second.begin(); method_itr != category_itr->second.end(); method_itr++) {
                bool method_passed = true;

                diag_base::put_any(suborigin, diag::severity::warning, diag::tag::none, ">>   %s", method_itr->first.c_str());

                try {
                    context context(category_itr->first.c_str(), method_itr->first.c_str(), diag_base::log(), seed, c_str(process_path));

                    // Execute
                    method_passed = method_itr->second(context);
                }
                catch(const std::exception& ex) {
                    method_passed = false;
                    diag_base::put_any(suborigin, diag::severity::critical, diag::tag::none, "    %s%sEXCEPTION%s %s", diag::color::begin, diag::color::red, diag::color::end, ex.what());
                }

                // Method outcome
                if (method_passed) {
                    diag_base::put_any(suborigin, diag::severity::critical, diag::tag::none, "  %s%sPASS%s %s", diag::color::begin, diag::color::green, diag::color::end, method_itr->first.c_str());
                }
                else {
                    diag_base::put_any(suborigin, diag::severity::critical, diag::tag::none, "  %s%sFAIL%s %s", diag::color::begin, diag::color::red, diag::color::end, method_itr->first.c_str());
                }

                category_passed = category_passed && method_passed;
            } // method

            // Category outcome
            if (category_passed) {
                diag_base::put_any(suborigin, diag::severity::critical, diag::tag::none, "%s%sPASS%s %s%s%s%s", diag::color::begin, diag::color::green, diag::color::end, diag::color::begin, diag::color::cyan, category_itr->first.c_str(), diag::color::end);
            }
            else {
                diag_base::put_any(suborigin, diag::severity::critical, diag::tag::none, "%s%sFAIL%s %s%s%s%s", diag::color::begin, diag::color::red, diag::color::end, diag::color::begin, diag::color::cyan, category_itr->first.c_str(), diag::color::end);
            }

            diag_base::put_blank_line(diag::severity::critical);
            all_passed = all_passed && category_passed;
        } // category

        // Summary
        diag_base::put_blank_line(diag::severity::critical);
        diag_base::put_any(suborigin, diag::severity::critical, diag::tag::none, ">>   %s%ssummary%s", diag::color::begin, diag::color::cyan, diag::color::end);
        diag_base::put_any(suborigin, diag::severity::warning, diag::tag::none, "seed = %u", seed);

        // Suite outcome
        if (all_passed) {
            diag_base::put_any(suborigin, diag::severity::critical, diag::tag::none, "%s%sPASS%s %s%ssummary%s", diag::color::begin, diag::color::green, diag::color::end, diag::color::begin, diag::color::cyan, diag::color::end);
        }
        else {
            diag_base::put_any(suborigin, diag::severity::critical, diag::tag::none, "%s%sFAIL%s %s%ssummary%s", diag::color::begin, diag::color::red, diag::color::end, diag::color::begin, diag::color::cyan, diag::color::end);
        }

        diag_base::put_blank_line(diag::severity::critical);
        diag_base::put_blank_line(diag::severity::critical);
        diag_base::put_blank_line(diag::severity::critical);

        return all_passed;
    }


    template <typename ProcessStr>
    inline void suite<ProcessStr>::srand() noexcept {
        if (seed == seed::random) {
            std::srand(static_cast<seed_t>(std::chrono::system_clock::now().time_since_epoch().count() % RAND_MAX));
            seed = std::rand();
        }

        std::srand(seed);
    }

} }
