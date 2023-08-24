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

#include "../../diag/i/diag_ready.i.h"
#include "../../diag/i/log.i.h"


namespace abc { namespace test {

    using seed_t = unsigned;

    namespace seed {
        constexpr seed_t random = 0;
    }


    // --------------------------------------------------------------


    /**
     * @brief         Temporary accessor passed into each test method to perform verification and logging.
     * @tparam LogPtr Pointer to a logging facility.
     */
    template <typename LogPtr>
    class context
        : protected diag::diag_ready<const char*, const LogPtr&> {

    protected:
        using diag_base = diag::diag_ready<const char*, const LogPtr&>;

    public:
        /**
         * @brief               Constructor.
         * @param category_name Test category name.
         * @param method_name   Test method name.
         * @param log           Pointer to a `log_ostream` instance.
         * @param seed          Randomization seed. Used to repeat a previous test run.
         * @param process_path  The path to the test process.
         */
        context(const char* category_name, const char* method_name, const LogPtr& log, seed_t seed, const char* process_path);

        /**
         * @brief          Verifies an actual value matches the expected one.
         * @tparam Value   Type of the value.
         * @param actual   Actual value.
         * @param expected Expected value.
         * @param tag      Unique tag.
         * @param format   C-style format to print a value of type `Value`.
         * @return         `true` = pass; `false` = fail.
         */
        template <typename Value>
        bool are_equal(const Value& actual, const Value& expected, diag::tag_t tag, const char* format);

        /**
         * @brief          Verifies an actual string value matches the expected one.
         * @param actual   Actual string value.
         * @param expected Expected string value.
         * @param tag      Unique tag.
         * @return         `true` = pass; `false` = fail.
         */
        bool are_equal(const char* actual, const char* expected, diag::tag_t tag);

        /**
         * @brief          Verifies an actual binary blob matches the expected one.
         * @param actual   Actual blob.
         * @param expected Expected blob.
         * @param size     Size of the blob.
         * @param tag      Unique tag.
         * @return         `true` = pass; `false` = fail.
         */
        bool are_equal(const void* actual, const void* expected, std::size_t size, diag::tag_t tag);

        const char*   category_name;
        const char*   method_name;
        const LogPtr& log;
        seed_t        seed;
        const char*   process_path;
        std::string   suborigin;
    };


    // --------------------------------------------------------------


    /**
     * @brief         Bare test method.
     * @tparam LogPtr Pointer to a logging facility.
     */
    template <typename LogPtr>
    using method = std::function<bool(context<LogPtr>&)>;

    /**
     * @brief         Pair of a name and a test method.
     * @tparam LogPtr Pointer to a logging facility.
     */
    template <typename LogPtr>
    using named_method = std::pair<std::string, method<LogPtr>>;

    /**
     * @brief         Bare collection of named test methods.
     * @tparam LogPtr Pointer to a logging facility.
     */
    template <typename LogPtr>
    using category = std::vector<named_method<LogPtr>>;

    /**
     * @brief         Pair of a name and a collection of named test methods.
     * @tparam LogPtr Pointer to a logging facility.
     */
    template <typename LogPtr>
    using named_category = std::pair<std::string, category<LogPtr>>;


    // --------------------------------------------------------------


    /**
     * @brief              Complete test suite.
     * @tparam ProcessStr  String type for process_path.
     * @tparam LogPtr      Pointer to a logging facility.
     */
    template <typename ProcessStr, typename LogPtr>
    class suite
        : protected diag::diag_ready<const char*, LogPtr> {

    protected:
        using diag_base = diag::diag_ready<const char*, LogPtr>;

    public:
        /**
         * @brief Default constructor.
         */
        suite() noexcept = default;

        /**
         * @brief              Constructor. Vector version.
         * @param categories   Collection of named categories.
         * @param log          Pointer to a `log_ostream` instance.
         * @param seed         Randomization seed. Used to repeat a previous test run.
         * @param process_path The path to the test process.
         */
        suite(std::vector<named_category<LogPtr>>&& categories, LogPtr&& log, seed_t seed, ProcessStr&& process_path);

        /**
         * @brief              Constructor. initializer list version.
         * @param init         Initializer list of named categories.
         * @param log          Pointer to a `log_ostream` instance.
         * @param seed         Randomization seed. Used to repeat a previous test run.
         * @param process_path The path to the test process.
         */
        suite(std::initializer_list<named_category<LogPtr>> init, LogPtr&& log, seed_t seed, ProcessStr&& process_path);

        /**
         * @brief  Executes all test methods of all test categories.
         * @return `true` = pass; `false` = fail.
         */
        bool run() noexcept;

        std::vector<named_category<LogPtr>> categories;
        seed_t                              seed;
        ProcessStr                          process_path;

    private:
        void srand() noexcept;
    };

} }
