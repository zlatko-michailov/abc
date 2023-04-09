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

#include <cstdint>
#include <linux/gpio.h>

#include "gpio_base.i.h"
#include "gpio_chip.i.h"


namespace abc {

	/**
	 * @brief				Base GPIO line. Should not be used directly. Either `gpio_input_line` or `gpio_output_line` should be used.
	 * @tparam Log			Logging facility.
	 */
	template <typename Log = null_log>
	class gpio_line {
	public:
		/**
		 * @brief			Constructor.
		 * @param chip		Pointer to the owning `gpio_chip` instance.
		 * @param pos		Chip-specific line position.
		 * @param flags		Line flags.
		 * @param log		Pointer to a `Log` instance. May be `nullptr`.
		 */
		gpio_line(const gpio_chip<Log>* chip, gpio_line_pos_t pos, gpio_line_flags_t flags, Log* log = nullptr);

		/**
		 * @brief			Move constructor.
		 */
		gpio_line(gpio_line<Log>&& other) noexcept = default;

		/**
		 * @brief			Deleted.
		 */
		gpio_line(const gpio_line<Log>& other) = delete;

		/**
		 * @brief			Destructor.
		 */
		virtual ~gpio_line() noexcept;

	public:
		/**
		 * @brief			Returns the current level on the line.
		 */
		gpio_level_t get_level() const noexcept;

		/**
		 * @brief			Wait until the current level on the line matches the expected one.
		 * @tparam Duration	`std::duration` type.
		 * @param level		Expected level.
		 * @param timeout	Timeout.
		 * @return			The expected level if there was match, or `invalid` if the wait timed out. 
		 */
		template <typename Duration>
		gpio_level_t expect_level(gpio_level_t level, Duration timeout) const noexcept;

		/**
		 * @brief			Set the current level on the line.
		 * @param level		Desired level.
		 * @return			The desired level upon success, or `invalid` upon failure.
		 */
		gpio_level_t put_level(gpio_level_t level) const noexcept;

		/**
		 * @brief			Sets the current level on the line, and blocks for the specified duration
		 * @tparam Duration	`std::duration` type.
		 * @param level		Desired level.
		 * @param duration	Duration.
		 * @return			The desired level upon success, or `invalid` upon failure.
		 */
		template <typename Duration>
		gpio_level_t put_level(gpio_level_t level, Duration duration) const noexcept;

	private:
		/**
		 * @brief			The line's device file descriptor.
		 */
		gpio_fd_t _fd = -1;

		/**
		 * @brief			The log passed in to the constructor.
		 */
		Log* _log;
	};


	// --------------------------------------------------------------


	/**
	 * @brief				GPIO input line.
	 * @tparam Log			Logging facility.
	 */
	template <typename Log = null_log>
	class gpio_input_line : public gpio_line<Log> {
		using base = gpio_line<Log>;

	public:
		/**
		 * @brief			Constructor.
		 * @see gpio_line
		 */
		gpio_input_line(const gpio_chip<Log>* chip, gpio_line_pos_t pos, Log* log = nullptr);

	public:
		/**
		 * @brief			Deleted.
		 */
		gpio_level_t put_level(gpio_level_t level) const noexcept = delete;

		/**
		 * @brief			Deleted.
		 */
		template <typename Duration>
		gpio_level_t put_level(gpio_level_t level, Duration duration) const noexcept = delete;
	};


	// --------------------------------------------------------------


	/**
	 * @brief				GPIO output line.
	 * @tparam Log			Logging facility.
	 */
	template <typename Log = null_log>
	class gpio_output_line : public gpio_line<Log> {
		using base = gpio_line<Log>;

	public:
		/**
		 * @brief			Constructor.
		 * @see gpio_line
		 */
		gpio_output_line(const gpio_chip<Log>* chip, gpio_line_pos_t pos, Log* log = nullptr);

	public:
		/**
		 * @brief			Deleted.
		 */
		gpio_level_t get_level() const noexcept = delete;

		/**
		 * @brief			Deleted.
		 */
		template <typename Duration>
		gpio_level_t expect_level(gpio_level_t level, Duration timeout) const noexcept = delete;
	};


	// --------------------------------------------------------------

}
