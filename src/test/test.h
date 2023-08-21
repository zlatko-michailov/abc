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

#include "i/test.i.h"
#include "../diag/log.h"


namespace abc { namespace test {

    template <typename LogPtr>
    context<LogPtr>::context(const char* category_name, const char* method_name, const LogPtr& log, seed_t seed, const char* process_path) noexcept
        : category_name(category_name)
        , method_name(method_name)
        , log(log)
        , seed(seed)
        , process_path(process_path) {
    }


    template <typename LogPtr>
    template <typename Value>
    inline bool context<LogPtr>::are_equal(const Value& actual, const Value& expected, diag::tag_t tag, const char* format) {
        // Compare.
        bool are_equal = actual == expected;

        // Format the outcome.
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


    template <typename LogPtr>
    inline bool context<LogPtr>::are_equal(const char* actual, const char* expected, diag::tag_t tag) {
        // Compare.
        bool are_equal = std::strcmp(actual, expected) == 0;

        // Format the outcome.
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


    template <typename LogPtr>
    inline bool context<LogPtr>::are_equal(const void* actual, const void* expected, std::size_t size, diag::tag_t tag) {
        // Compare.
        bool are_equal = std::memcmp(actual, expected, size) == 0;

        // Format the outcome.
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


    template <typename ProcessStr, typename LogPtr>
    inline suite<ProcessStr, LogPtr>::suite(std::vector<named_category<LogPtr>>&& categories, LogPtr&& log, seed_t seed, ProcessStr&& process_path) noexcept
        : categories(std::move(categories))
        , log(std::move(log))
        , seed(seed)
        , process_path(std::move(process_path)) {
    }


    template <typename ProcessStr, typename LogPtr>
    inline suite<ProcessStr, LogPtr>::suite(std::initializer_list<named_category<LogPtr>> init, LogPtr&& log, seed_t seed, ProcessStr&& process_path) noexcept
        : categories(init.begin(), init.end())
        , log(std::move(log))
        , seed(seed)
        , process_path(std::move(process_path)) {
    }


    template <typename ProcessStr, typename LogPtr>
    inline bool suite<ProcessStr, LogPtr>::run() noexcept {
        srand();

        bool all_passed = true;

        if (log != nullptr) {
            log->put_blank_line(category::any, severity::critical);
            log->put_blank_line(category::any, severity::critical);
            log->put_blank_line(category::any, severity::critical);
        }

        // Category
        for (auto category_itr = categories.begin(); category_itr != categories.end(); category_itr++) {
            bool category_passed = true;

            if (log != nullptr) {
                log->put_any(category::any, severity::critical, diag::tag::none, ">>   %s%s%s%s", color::begin, color::cyan, category_itr->first.c_str(), color::end);
            }

            // Method
            for (auto method_itr = category_itr->second.begin(); method_itr != category_itr->second.end(); method_itr++) {
                bool method_passed = true;

                if (log != nullptr) {
                    log->put_any(category::any, severity::warning, diag::tag::none, ">>   %s", method_itr->first.c_str());
                }

                try {
                    context<LogPtr> context(category_itr->first.c_str(), method_itr->first.c_str(), log, seed, c_str(process_path));

                    // Execute
                    method_passed = method_itr->second(context);
                }
                catch(const std::exception& ex) {
                    method_passed = false;
                    if (log != nullptr) {
                        log->put_any(category::any, severity::critical, diag::tag::none, "    %s%sEXCEPTION%s %s", color::begin, color::red, color::end, ex.what());
                    }
                }

                // Method outcome
                if (method_passed) {
                    if (log != nullptr) {
                        log->put_any(category::any, severity::critical, diag::tag::none, "  %s%sPASS%s %s", color::begin, color::green, color::end, method_itr->first.c_str());
                    }
                }
                else {
                    if (log != nullptr) {
                        log->put_any(category::any, severity::critical, diag::tag::none, "  %s%sFAIL%s %s", color::begin, color::red, color::end, method_itr->first.c_str());
                    }
                }

                category_passed = category_passed && method_passed;
            } // method

            // Category outcome
            if (category_passed) {
                if (log != nullptr) {
                    log->put_any(category::any, severity::critical, diag::tag::none, "%s%sPASS%s %s%s%s%s", color::begin, color::green, color::end, color::begin, color::cyan, category_itr->first.c_str(), color::end);
                }
            }
            else {
                if (log != nullptr) {
                    log->put_any(category::any, severity::critical, diag::tag::none, "%s%sFAIL%s %s%s%s%s", color::begin, color::red, color::end, color::begin, color::cyan, category_itr->first.c_str(), color::end);
                }
            }

            if (log != nullptr) {
                log->put_blank_line(category::any, severity::critical);
            }
            all_passed = all_passed && category_passed;
        } // category

        // Summary
        if (log != nullptr) {
            log->put_blank_line(category::any, severity::critical);
            log->put_any(category::any, severity::critical, diag::tag::none, ">>   %s%ssummary%s", color::begin, color::cyan, color::end);
            log->put_any(category::any, severity::warning, diag::tag::none, "seed = %u", seed);
        }

        // Suite outcome
        if (all_passed) {
            if (log != nullptr) {
                log->put_any(category::any, severity::critical, diag::tag::none, "%s%sPASS%s %s%ssummary%s", color::begin, color::green, color::end, color::begin, color::cyan, color::end);
            }
        }
        else {
            if (log != nullptr) {
                log->put_any(category::any, severity::critical, diag::tag::none, "%s%sFAIL%s %s%ssummary%s", color::begin, color::red, color::end, color::begin, color::cyan, color::end);
            }
        }

        if (log != nullptr) {
            log->put_blank_line(category::any, severity::critical);
            log->put_blank_line(category::any, severity::critical);
            log->put_blank_line(category::any, severity::critical);
        }

        return all_passed;
    }


    template <typename ProcessStr, typename LogPtr>
    inline void suite<ProcessStr, LogPtr>::srand() noexcept {
        if (seed == seed::random) {
            std::srand(static_cast<seed_t>(std::chrono::system_clock::now().time_since_epoch().count() % RAND_MAX));
            seed = std::rand();
        }

        std::srand(seed);
    }

} }
