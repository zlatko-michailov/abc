/*
MIT License

Copyright (c) 2018-2026 Zlatko Michailov

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

#include "../../root/size.h"
#include "../../diag/i/diag_ready.i.h"
#include "../../concurrent/i/mutex.i.h"


namespace abc { namespace smbus {

    using functionality_t   = unsigned long;
    using address_t         = std::uint8_t;
    using register_t        = std::uint8_t;
    using clock_frequency_t = std::uint64_t;


    // --------------------------------------------------------------


    class target;


    /**
     * @brief SMBus (I2C).
     */
    class controller
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

        using fd_t = int;
        static constexpr std::size_t max_path = GPIO_MAX_NAME_SIZE;

    public:
        /**
         * @brief             Constructor. Identifies the SMBus device by number - `/dev/i2c-0`.
         * @param dev_i2c_pos SMBus device number.
         * @param log         `diag::log_ostream` pointer. May be `nullptr`.
         */
        controller(int dev_i2c_pos, diag::log_ostream* log = nullptr);

        /**
         * @brief      Construct a new gpio smbus object
         * @param path Device path - `/dev/i2c-0`.
         * @param log  `diag::log_ostream` pointer. May be `nullptr`.
         */
        controller(const char* path, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        controller(controller&& other) noexcept;

        /**
         * @brief Deleted.
         */
        controller(const controller& other) = delete;

        /**
         * @brief Destructor.
         */
        ~controller() noexcept;

    private:
        /**
         * @brief      Initializer.
         * @param path SMBus device path.
         */
        void init(const char* path);

    public:
        /**
         * @brief Returns the device path.
         */
        const char* path() const noexcept;

        /**
         * @brief Returns the functionality bits.
         */
        functionality_t functionality() const noexcept;

        /**
         * @brief   Returns the operation mutex. This mutex must be locked before performing any SMBus operation.
         * @details When an operation is compound (involving multiple SMBus operations), this mutex must be held for the entire duration of the compound operation.
         */
        concurrent::mutex& mutex() noexcept;

    public:
        /**
         * @brief        Send a signal with no data to a target's register.
         * @param target Target/HAT.
         * @param reg    Register on the target.
         */
        void put_nodata(const target& target, register_t reg);

        /**
         * @brief        Send a byte (8 bits) to a target's register.
         * @param target Target/HAT.
         * @param reg    Register on the target.
         * @param byte   Data.
         */
        void put_byte(const target& target, register_t reg, std::uint8_t byte);

        /**
         * @brief        Send a word (16 bits) to a target's register.
         * @param target Target/HAT.
         * @param reg    Register on the target.
         * @param word   Data.
         */
        void put_word(const target& target, register_t reg, std::uint16_t word);

        /**
         * @brief        Send a block/array to a target's register.
         * @param target Target/HAT.
         * @param reg    Register on the target.
         * @param block  Data buffer.
         * @param size   Size of the data buffer.
         */
        void put_block(const target& target, register_t reg, const void* block, std::size_t size);

        /**
         * @brief        Receive a byte (8 bits) from a target with no register.
         * @param target Target/HAT.
         * @return       Data.
         */
        std::uint8_t get_noreg_byte(const target& target);

        /**
         * @brief        Receive a word (16 bits) from a target with no register.
         * @param target Target/HAT.
         * @return       Data.
         */
        std::uint16_t get_noreg_word(const target& target);

        /**
         * @brief        Receive a byte (8 bits) from a target's register.
         * @param target Target/HAT.
         * @param reg    Register on the target.
         * @return       Data.
         */
        std::uint8_t get_byte(const target& target, register_t reg);

        /**
         * @brief        Receive a word (16 bits) from a target's register.
         * @param target Target/HAT.
         * @param reg    Register on the target.
         * @return       Data.
         */
        std::uint16_t get_word(const target& target, register_t reg);

        /**
         * @brief        Receive a block/array from a target's register.
         * @param target Target/HAT.
         * @param reg    Register on the target.
         * @param block  Data buffer.
         * @param size   Size of the data buffer.
         * @return       The size of the received data.
         */
        std::size_t get_block(const target& target, register_t reg, void* block, std::size_t size);

    private:
        /**
         * @brief      Ensure the SMBus is currently targeting the target's address.
         * @param addr Target's address.
         * @param tag  Origination tag.
         */
        void ensure_address(address_t addr, diag::tag_t tag);

        /**
         * @brief      Calls `ioctl()` while a mutex is being acquired.
         * @tparam Arg Argument type.
         * @param arg  Argument value. 
         * @param tag  Origination tag.
         */
        template <typename Arg>
        void ensure_ioctl(int command, Arg arg, diag::tag_t tag);

    private:
        /**
         * @brief      Swap the bytes of a word.
         * @param word Input word.
         * @return     The word with its two bytes swapped. 
         */
        static std::uint16_t swap_bytes(std::uint16_t word) noexcept;

    private:
        /**
         * @brief Copy of the SMBus device path.
         */
        char _path[max_path];

        /**
         * @brief File descriptor of the SMBus device.
         */
        fd_t _fd;

        /**
         * @brief Functionality bits.
         */
        functionality_t _functionality;

        /**
         * @brief Current target address.
         */
        address_t _addr;

        /**
         * @brief Operation mutex. 
         */
        concurrent::mutex _mutex;
    };


    // --------------------------------------------------------------


    /**
     * @brief SMBus target identification and properties.
     */
    class target {
    public:
        /**
         * @brief                    Constructor.
         * @param addr               Target address.
         * @param clock_frequency    Frequency of the target's clock.
         * @param requires_byte_swap true = bytes must be swapped before sending and after receiving. false = no swap is needed.
         */
        target(address_t addr, clock_frequency_t clock_frequency, bool requires_byte_swap);

        /**
         * @brief Move constructor.
         */
        target(target&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        target(const target& other) = default;

    public:
        /**
         * @brief Returns the target's address.
         */
        address_t address() const noexcept;

        /**
         * @brief Returns the frequency of the target's clock.
         */
        clock_frequency_t clock_frequency() const noexcept;

        /**
         * @brief Returns the flag whether a byte swap is needed.
         */
        bool requires_byte_swap() const noexcept;

    private:
        /**
         * @brief Target's address.
         */
        address_t _addr;

        /**
         * @brief Frequency of the target's clock.
         */
        clock_frequency_t _clock_frequency;

        /**
         * @brief Flag whether a byte swap is needed.
         */
        bool _requires_byte_swap;
    };


    // --------------------------------------------------------------

} }
