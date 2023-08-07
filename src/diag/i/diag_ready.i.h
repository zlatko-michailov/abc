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

#include "log.i.h"


namespace abc {

    /**
     * @brief            Diagnostics facility used by classes.
     * @tparam OriginStr String type for origin.
     * @tparam LogPtr    Pointer type to `log_ostream`.
     */
    template <typename OriginStr, typename LogPtr = std::nullptr_t>
    class diag_ready {

    protected:
        /**
         * @brief         Constructor.
         * @param origin  Origin string.
         * @param log     Log pointer.
         */
        diag_ready(OriginStr&& origin, LogPtr&& log) noexcept;

        /**
         * @brief Move constructor.
         */
        diag_ready(diag_ready&& other) noexcept;

        /**
         * @brief Deleted.
         */
        diag_ready(const diag_ready& other) = delete;

    protected:
        /**
         * @brief           Write a formatted message.
         * @param suborigin Entry suborigin, e.g. method.
         * @param severity  Entry severity.
         * @param tag       Entry tag.
         * @param format    Message format.
         * @param ...       Message arguments.
         */
        void put_any(const char* suborigin, severity_t severity, tag_t tag, const char* format, ...) noexcept;

        /**
         * @brief           Write a formatted message.
         * @param suborigin Entry suborigin, e.g. method.
         * @param severity  Entry severity.
         * @param tag       Entry tag.
         * @param format    Message format.
         * @param vlist     Message arguments.
         */
        void put_anyv(const char* suborigin, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept;

        /**
         * @brief             Write binary buffer as a sequence of hexadecimal bytes.
         * @param suborigin   Entry suborigin, e.g. method.
         * @param severity    Entry severity.
         * @param tag         Entry tag.
         * @param buffer      Data buffer.
         * @param buffer_size Content size.
         */
        void put_binary(const char* suborigin, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept;

        /**
         * @brief          Puts a new line.
         * @param severity Entry severity.
         */
        void put_blank_line(severity_t severity) noexcept;

        /**
         * @brief            Throws an exception with the given base type, message, and tag.
         * @tparam Exception Exception base type.
         * @param message    Message.
         * @param tag        Origination tag.
         */
        template <typename Exception>
        void throw_exception(const char* message, tag_t tag);

        /**
         * @brief            Throws an exception derived from `assert_error` if `condition` is `false`.
         * @details          Use this facility to assert general assumptions in the middle of a method.
         * @param condition  Asserted condition.
         * @param message    Message.
         * @param tag        Origination tag.
         */
        void assert(bool condition, const char* message, tag_t tag);

        /**
         * @brief            Throws an exception derived from `expect_error` if `condition` is `false`.
         * @details          Use this facility to assert assumptions about expected/input state.
         * @param condition  Asserted condition.
         * @param message    Message.
         * @param tag        Origination tag.
         */
        void expect(bool condition, const char* message, tag_t tag);

        /**
         * @brief            Throws an exception derived from `ensure_error` if `condition` is `false`.
         * @details          Use this facility to assert assumptions about ensured/output state.
         * @param condition  Asserted condition.
         * @param message    Message.
         * @param tag        Origination tag.
         */
        void ensure(bool condition, const char* message, tag_t tag);

    private:
        OriginStr _origin;
        LogPtr _log;
    };

}