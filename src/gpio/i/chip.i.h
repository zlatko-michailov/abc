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

#include "../../root/size.h"
#include "../../diag/i/diag_ready.i.h"
#include "base.i.h"


namespace abc { namespace gpio {

    /**
     * @brief Wrapper around the corresponding Linux kernel struct.
     */
    struct chip_info
        : public chip_info_base {

        /**
         * @brief Constructor. Zeroes out the base struct.
         */
        chip_info() noexcept;

        /**
         * @brief Flag whether the struct has been successfully populated.
         */
        bool is_valid; //// TODO: Remove, unused.
    };


    /**
     * @brief Wrapper around the corresponding Linux kernel struct.
     */
    struct line_info
        : public line_info_base {

        /**
         * @brief Constructor. Zeroes out the base struct.
         */
        line_info() noexcept;

        /**
         * @brief Flag whether the struct has been successfully populated.
         */
        bool is_valid; //// TODO: Remove, unused.
    };


    // --------------------------------------------------------------


    /**
     * @brief GPIO chip.
     */
    class chip
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief                  Constructor. Identifies the GPIO chip device by number - `/dev/gpiochip0`.
         * @param dev_gpiochip_pos GPIO chip device number.
         * @param consumer         Label of the consumer that is using this device.
         * @param log              `diag::log_ostream` pointer. May be `nullptr`.
         */
        chip(int dev_gpiochip_pos, const char* consumer, diag::log_ostream* log = nullptr);

        /**
         * @brief          Constructor. Identifies the GPIO chip device by path - `/dev/gpiochip0`.
         * @param path     GPIO chip device path.
         * @param consumer Label of the consumer that is using this device.
         * @param log      `diag::log_ostream` pointer. May be `nullptr`.
         */
        chip(const char* path, const char* consumer, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        chip(chip&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        chip(const chip& other) = default;

    private:
        /**
         * @brief          Internal initializer. Called from the constructor. 
         * @param path     GPIO chip device path.
         * @param consumer Label of the consumer that is using this device.
         */
        void init(const char* path, const char* consumer);

    public:
        /**
         * @brief                    Returns the GPIO chip device path.
         */
        const char* path() const noexcept;

        /**
         * @brief                    Returns the label of the consumer that is using this device.
         */
        const char* consumer() const noexcept;

    public:
        /**
         * @brief Returns the `chip_info` for this chip.
         */
        gpio::chip_info chip_info() const;

        /**
         * @brief     Returns the `line_info` for the identified line.
         * @param pos Chip-specific position of the requested line.
         */
        gpio::line_info line_info(line_pos_t pos) const;

    private:
        /**
         * @brief Copy of the GPIO chip device path.
         */
        char _path[max_path];

        /**
         * @brief Copy of the consumer label.
         */
        char _consumer[max_consumer];
    };


    // --------------------------------------------------------------

} }
