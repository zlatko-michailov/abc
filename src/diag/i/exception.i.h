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

#include <cstddef>
#include <stdexcept>

#include "../tag.h"


namespace abc { namespace diag {

    // logic_error
    // |-- assert_error
    //     |-- expect_error
    //     |-- ensure_error
    //  
    // runtime_error


    // --------------------------------------------------------------


    /**
     * @brief            Wrapper around `Exception` that logs upon constructions to track the origin of the exception.
     * @tparam Exception Base exception type.
     * @tparam Log       Logging facility.
     */
    template <typename Exception, typename LogPtr = std::nullptr_t>
    class exception
        : public Exception {

    public:
        /**
         * @brief         Constructor.
         * @param origin  Thrower's origin.
         * @param message Error message.
         * @param tag     Unique tag.
         * @param log     Pointer to a `log_ostream` instance.
         */
        exception(const char* origin, const char* message, tag_t tag, LogPtr&& log = nullptr);

        /**
         * @brief Copy constructor.
         */
        exception(const exception& other) noexcept = default;

    public:
        /**
         * @brief Returns the tag passed in to the constructor.
         */
        tag_t tag() const noexcept;

    private:
        tag_t _tag;
    };


    // --------------------------------------------------------------


    class assert_error
        : public std::logic_error {

    public:
        /**
         * @brief         Constructor.
         * @param message Error message.
         */
        assert_error(const char* message)
            : std::logic_error(message) {
        }

        /**
         * @brief Copy constructor.
         */
        assert_error(const assert_error& other) noexcept = default;
    };


    // --------------------------------------------------------------


    class expect_error
        : public assert_error {

    public:
        /**
         * @brief         Constructor.
         * @param message Error message.
         */
        expect_error(const char* message)
            : assert_error(message) {
        }

        /**
         * @brief Copy constructor.
         */
        expect_error(const expect_error& other) noexcept = default;
    };


    // --------------------------------------------------------------


    class ensure_error
        : public assert_error {

    public:
        /**
         * @brief         Constructor.
         * @param message Error message.
         */
        ensure_error(const char* message)
            : assert_error(message) {
        }

        /**
         * @brief Copy constructor.
         */
        ensure_error(const ensure_error& other) noexcept = default;
    };


    // --------------------------------------------------------------

} }
