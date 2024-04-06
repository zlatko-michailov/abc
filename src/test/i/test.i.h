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
     * @brief Temporary accessor passed into each test method to perform verification and logging.
     */
    class context 
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief               Constructor.
         * @param category_name Test category name.
         * @param method_name   Test method name.
         * @param log           Pointer to a `diag::log_ostream` instance.
         * @param seed          Randomization seed. Used to repeat a previous test run.
         * @param process_path  The path to the test process.
         */
        context(const char* category_name, const char* method_name, diag::log_ostream* log, seed_t seed, const char* process_path);

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

        const char*         category_name;
        const char*         method_name;
        seed_t              seed;
        const char*         process_path;
        std::string         suborigin;

    public:        
        /**
         * @brief Returns the `diag::log_ostream` pointer.
         */
        diag::log_ostream* log() const noexcept;
    };


    // --------------------------------------------------------------


    /**
     * @brief Bare test method.
     */
    using method = std::function<bool(context&)>;

    /**
     * @brief Pair of a name and a test method.
     */
    using named_method = std::pair<std::string, method>;

    /**
     * @brief Bare collection of named test methods.
     */
    using category = std::vector<named_method>;

    /**
     * @brief Pair of a name and a collection of named test methods.
     */
    using named_category = std::pair<std::string, category>;

    /**
     * @brief Bare collection of named test categories.
     */
    using named_categories = std::vector<named_category>;


    // --------------------------------------------------------------


    /**
     * @brief              Complete test suite.
     * @tparam ProcessStr  String type for process_path.
     */
    template <typename ProcessStr>
    class suite
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief Default constructor.
         */
        suite() noexcept = default;

        /**
         * @brief              Constructor. Vector version.
         * @param categories   Collection of named categories.
         * @param log          Pointer to a `diag::log_ostream` instance.
         * @param seed         Randomization seed. Used to repeat a previous test run.
         * @param process_path The path to the test process.
         */
        suite(named_categories&& categories, diag::log_ostream* log, seed_t seed, ProcessStr&& process_path);

        /**
         * @brief              Constructor. initializer list version.
         * @param init         Initializer list of named categories.
         * @param log          Pointer to a `diag::log_ostream` instance.
         * @param seed         Randomization seed. Used to repeat a previous test run.
         * @param process_path The path to the test process.
         */
        suite(std::initializer_list<named_category> init, diag::log_ostream* log, seed_t seed, ProcessStr&& process_path);

        /**
         * @brief  Executes all test methods of all test categories.
         * @return `true` = pass; `false` = fail.
         */
        bool run() noexcept;

        named_categories categories;
        seed_t           seed;
        ProcessStr       process_path;

    private:
        void srand() noexcept;
    };

} }
