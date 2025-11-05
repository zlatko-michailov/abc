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

#include <cstdint>

#include "../../diag/i/diag_ready.i.h"
#include "base.i.h"
#include "chip.i.h"


namespace abc { namespace gpio {

    /**
     * @brief Base GPIO line. Should not be used directly. Either `input_line` or `output_line` should be used.
     */
    class line
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    protected:
        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param chip   Pointer to the owning `chip` instance.
         * @param pos    Chip-specific line position.
         * @param flags  Line flags.
         * @param log    `diag::log_ostream` pointer. May be `nullptr`.
         */
        line(const char* origin, const chip* chip, line_pos_t pos, line_flags_t flags, diag::log_ostream* log = nullptr);

    public:
        /**
         * @brief       Constructor.
         * @param chip  Pointer to the owning `chip` instance.
         * @param pos   Chip-specific line position.
         * @param flags Line flags.
         * @param log   `diag::log_ostream` pointer. May be `nullptr`.
         */
        //// TODO: REMOVE? line(const chip* chip, line_pos_t pos, line_flags_t flags, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        line(line&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        line(const line& other) = delete;

        /**
         * @brief Destructor.
         */
        virtual ~line() noexcept;

    public:
        /**
         * @brief Returns the current level on the line.
         */
        level_t get_level() const;

        /**
         * @brief           Wait until the current level on the line matches the expected one.
         * @tparam Duration `std::duration` type.
         * @param level     Expected level.
         * @param timeout   Timeout.
         * @return          The expected level if there was match, or `invalid` if the wait timed out. 
         */
        template <typename Duration>
        level_t wait_for_level(level_t level, Duration timeout) const;

        /**
         * @brief       Set the current level on the line.
         * @param level Desired level.
         * @return      The desired level upon success, or `invalid` upon failure.
         */
        level_t put_level(level_t level) const;

        /**
         * @brief           Sets the current level on the line, and blocks for the specified duration
         * @tparam Duration `std::duration` type.
         * @param level     Desired level.
         * @param duration  Duration.
         * @return          The desired level upon success, or `invalid` upon failure.
         */
        template <typename Duration>
        level_t put_level(level_t level, Duration duration) const;

    private:
        /**
         * @brief The line's device file descriptor.
         */
        fd_t _fd = -1;
    };


    // --------------------------------------------------------------


    /**
     * @brief GPIO input line.
     */
    class input_line
        : public line {

        using base = line;
        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief Constructor.
         * @see `line`
         */
        input_line(const chip* chip, line_pos_t pos, diag::log_ostream* log = nullptr);

    public:
        /**
         * @brief Deleted.
         */
        level_t put_level(level_t level) const noexcept = delete;

        /**
         * @brief Deleted.
         */
        template <typename Duration>
        level_t put_level(level_t level, Duration duration) const noexcept = delete;
    };


    // --------------------------------------------------------------


    /**
     * @brief GPIO output line.
     */
    class output_line
        : public line {

        using base = line;
        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief Constructor.
         * @see `line`
         */
        output_line(const chip* chip, line_pos_t pos, diag::log_ostream* log = nullptr);

    public:
        /**
         * @brief Deleted.
         */
        level_t get_level() const noexcept = delete;

        /**
         * @brief Deleted.
         */
        template <typename Duration>
        level_t wait_for_level(level_t level, Duration timeout) const noexcept = delete;
    };


    // --------------------------------------------------------------

} }
