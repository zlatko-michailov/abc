/*
MIT License

Copyright (c) 2018-2024 Zlatko Michailov

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

#include "../root/size.h"
#include "gpio_base.i.h"


namespace abc {

	/**
	 * @brief		Wrapper around the corresponding Linux kernel struct.
	 */
	struct gpio_chip_info : public chip_info_base {
		/**
		 * @brief	Constructor. Zeroes out the base struct.
		 */
		gpio_chip_info() noexcept;

		/**
		 * @brief	Flag whether the struct has been successfully populated.
		 */
		bool is_valid;
	};


	/**
	 * @brief		Wrapper around the corresponding Linux kernel struct.
	 */
	struct gpio_line_info : public line_info_base {
		/**
		 * @brief	Constructor. Zeroes out the base struct.
		 */
		gpio_line_info() noexcept;

		/**
		 * @brief	Flag whether the struct has been successfully populated.
		 */
		bool is_valid;
	};


	// --------------------------------------------------------------


	/**
	 * @brief						GPIO chip.
	 * @tparam Log					Logging facility.
	 */
	template <typename Log = null_log>
	class gpio_chip {
	public:
		/**
		 * @brief					Constructor. Identifies the GPIO chip device by number - `/dev/gpiochip0`.
		 * @param dev_gpiochip_pos	GPIO chip device number.
		 * @param consumer			Label of the consumer that is using this device.
		 * @param log				Pointer to a `Log` instance. May be `nullptr`.
		 */
		gpio_chip(int dev_gpiochip_pos, const char* consumer = "abc", Log* log = nullptr);

		/**
		 * @brief					Constructor. Identifies the GPIO chip device by path - `/dev/gpiochip0`.
		 * @param path				GPIO chip device path.
		 * @param consumer			Label of the consumer that is using this device.
		 * @param log				Pointer to a `Log` instance. May be `nullptr`.
		 */
		gpio_chip(const char* path, const char* consumer = "abc", Log* log = nullptr);

		/**
		 * @brief					Move constructor.
		 */
		gpio_chip(gpio_chip<Log>&& other) noexcept = default;

		/**
		 * @brief					Copy constructor.
		 */
		gpio_chip(const gpio_chip<Log>& other) = default;

	private:
		/**
		 * @brief					Internal initializer. Called from the constructor. 
		 * @param path				GPIO chip device path.
		 * @param consumer			Label of the consumer that is using this device.
		 */
		void init(const char* path, const char* consumer);

	public:
		/**
		 * @brief					Returns the GPIO chip device path.
		 */
		const char* path() const noexcept;

		/**
		 * @brief					Returns the label of the consumer that is using this device.
		 */
		const char* consumer() const noexcept;

	public:
		/**
		 * @brief					Returns the `gpio_chip_info` for this chip.
		 */
		gpio_chip_info chip_info() const noexcept;

		/**
		 * @brief					Returns the `gpio_line_info` for the identified line.
		 * @param pos				Chip-specific position of the requested line.
		 */
		gpio_line_info line_info(line_pos_t pos) const noexcept;

	private:
		/**
		 * @brief					Copy of the GPIO chip device path.
		 */
		char _path[max_path];

		/**
		 * @brief					Copy of the consumer label.
		 */
		char _consumer[max_consumer];

		/**
		 * @brief					The log passed in to the constructor.
		 */
		Log* _log;
	};


	// --------------------------------------------------------------

}
