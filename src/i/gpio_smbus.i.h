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
#include <mutex>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "../root/size.h"
#include "gpio_base.i.h"


namespace abc {

	using gpio_smbus_functionality_t	= unsigned long;
	using gpio_smbus_address_t			= std::uint8_t;
	using gpio_smbus_register_t			= std::uint8_t;
	using gpio_smbus_clock_frequency_t	= std::uint64_t;


	// --------------------------------------------------------------


	template <typename Log>
	class gpio_smbus_target;


	/**
	 * @brief							SMBus (I2C).
	 * @tparam Log						Logging facility.
	 */
	template <typename Log = null_log>
	class gpio_smbus {
	public:
		/**
		 * @brief						Constructor. Identifies the SMBus device by number - `/dev/i2c-0`.
		 * @param dev_i2c_pos			SMBus device number.
		 * @param log					Pointer to a `Log` instance. May be `nullptr`.
		 */
		gpio_smbus(int dev_i2c_pos, Log* log = nullptr);

		/**
		 * @brief						Construct a new gpio smbus object
		 * @param path					Constructor. Identifies the SMBus device by path - `/dev/i2c-0`.
		 * @param log					Pointer to a `Log` instance. May be `nullptr`.
		 */
		gpio_smbus(const char* path, Log* log = nullptr);

		/**
		 * @brief						Move constructor.
		 */
		gpio_smbus(gpio_smbus<Log>&& other) noexcept = default;

		/**
		 * @brief						Deleted.
		 */
		gpio_smbus(const gpio_smbus<Log>& other) = delete;

		/**
		 * @brief						Destructor.
		 */
		~gpio_smbus() noexcept;

	private:
		/**
		 * @brief						Initializer.
		 * @param path					SMBus device path.
		 */
		void init(const char* path);

	public:
		/**
		 * @brief						Returns the device path.
		 */
		const char* path() const noexcept;

		/**
		 * @brief						Returns	the functionality bits.
		 */
		gpio_smbus_functionality_t functionality() const noexcept;

	public:
		/**
		 * @brief						Send a signal with no data to a target's register.
		 * @param target				Target/HAT.
		 * @param reg					Register on the target.
		 * @return						`true` = success. `false` = error.
		 */
		bool put_nodata(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg) noexcept;

		/**
		 * @brief						Send a byte (8 bits) to a target's register.
		 * @param target				Target/HAT.
		 * @param reg					Register on the target.
		 * @param byte					Data.
		 * @return						`true` = success. `false` = error.
		 */
		bool put_byte(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg, std::uint8_t byte) noexcept;

		/**
		 * @brief						Send a word (16 bits) to a target's register.
		 * @param target				Target/HAT.
		 * @param reg					Register on the target.
		 * @param word					Data.
		 * @return						`true` = success. `false` = error.
		 */
		bool put_word(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg, std::uint16_t word) noexcept;

		/**
		 * @brief						Send a block/array to a target's register.
		 * @param target				Target/HAT.
		 * @param reg					Register on the target.
		 * @param block					Data buffer.
		 * @param size					Size of the data buffer.
		 * @return						`true` = success. `false` = error.
		 */
		bool put_block(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg, const void* block, std::size_t size) noexcept;

		/**
		 * @brief						Receive a byte (8 bits) from a target with no register.
		 * @param target				Target/HAT.
		 * @param byte					Data.
		 * @return						`true` = success. `false` = error.
		 */
		bool get_noreg(const gpio_smbus_target<Log>& target, std::uint8_t& byte) noexcept;

		/**
		 * @brief						Receive a word (16 bits) from a target with no register.
		 * @param target				Target/HAT.
		 * @param word					Data.
		 * @return						`true` = success. `false` = error.
		 */
		bool get_noreg_2(const gpio_smbus_target<Log>& target, std::uint16_t& word) noexcept;

		/**
		 * @brief						Receive a byte (8 bits) from a target's register.
		 * @param target				Target/HAT.
		 * @param reg					Register on the target.
		 * @param byte					Data.
		 * @return						`true` = success. `false` = error.
		 */
		bool get_byte(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg, std::uint8_t& byte) noexcept;

		/**
		 * @brief						Receive a word (16 bits) from a target's register.
		 * @param target				Target/HAT.
		 * @param reg					Register on the target.
		 * @param word					Data.
		 * @return						`true` = success. `false` = error.
		 */
		bool get_word(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg, std::uint16_t& word) noexcept;

		/**
		 * @brief						Receive a block/array from a target's register.
		 * @param target				Target/HAT.
		 * @param reg					Register on the target.
		 * @param block					Data buffer.
		 * @param size					Size of the data buffer.
		 * @return						`true` = success. `false` = error.
		 */
		bool get_block(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg, void* block, std::size_t& size) noexcept;

	private:
		/**
		 * @brief						Ensure the SMBus is currently targeting the target's address.
		 * @param addr					Target's address. 
		 * @return						`true` = success. `false` = error.
		 */
		bool ensure_address(gpio_smbus_address_t addr) noexcept;

		/**
		 * @brief						Calls `ioctl()` while a mutex is being acquired.
		 * @tparam Arg					Argument type.
		 * @param arg					Argument value. 
		 * @return						The return value from `ioctl()`.
		 */
		template <typename Arg>
		int safe_ioctl(int command, Arg arg) noexcept;

	private:
		/**
		 * @brief						Swap the bytes of a word.
		 * @param word					Input word.
		 * @return						The word with its two bytes swapped. 
		 */
		static std::uint16_t swap_bytes(std::uint16_t word) noexcept;

	private:
		/**
		 * @brief						Copy of the SMBus device path.
		 */
		char _path[gpio_max_path];

		/**
		 * @brief						File descriptor of the SMBus device.
		 */
		gpio_fd_t _fd;

		/**
		 * @brief						Functionality bits.
		 */
		gpio_smbus_functionality_t _functionality;

		/**
		 * @brief						Current target address.
		 */
		gpio_smbus_address_t _addr;

		/**
		 * @brief						ioctl() mutex. 
		 */
		std::mutex _ioctl_mutex;

		/**
		 * @brief						The log passed in to the constructor.
		 */
		Log* _log;
	};


	// --------------------------------------------------------------


	/**
	 * @brief							SMBus target identification and properties.
	 * @tparam Log						Logging facility.
	 */
	template <typename Log = null_log>
	class gpio_smbus_target {
	public:
		/**
		 * @brief						Constructor.
		 * @param addr					Target address.
		 * @param clock_frequency		Frequency of the target's clock.
		 * @param requires_byte_swap	true = bytes must be swapped before sending and after receiving. false = no swap is needed.
		 * @param log					Pointer to a `Log` instance. May be `nullptr`.
		 */
		gpio_smbus_target(gpio_smbus_address_t addr, gpio_smbus_clock_frequency_t clock_frequency, bool requires_byte_swap, Log* log = nullptr);

		/**
		 * @brief						Move constructor.
		 */
		gpio_smbus_target(gpio_smbus_target&& other) noexcept = default;

		/**
		 * @brief						Copy constructor.
		 */
		gpio_smbus_target(const gpio_smbus_target& other) = default;

	public:
		/**
		 * @brief						Returns the target's address.
		 */
		gpio_smbus_address_t address() const noexcept;

		/**
		 * @brief						Returns the frequency of the target's clock.
		 */
		gpio_smbus_clock_frequency_t clock_frequency() const noexcept;

		/**
		 * @brief						Returns the flag whether a byte swap is needed.
		 */
		bool requires_byte_swap() const noexcept;

	private:
		/**
		 * @brief						Target's address.
		 */
		gpio_smbus_address_t _addr;

		/**
		 * @brief						Frequency of the target's clock.
		 */
		gpio_smbus_clock_frequency_t _clock_frequency;

		/**
		 * @brief						Flag whether a byte swap is needed.
		 */
		bool _requires_byte_swap;

		/**
		 * @brief						The log passed in to the constructor.
		 */
		Log* _log;
	};


	// --------------------------------------------------------------

}
