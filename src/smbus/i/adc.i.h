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
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "../../root/size.h"
#include "../../diag/i/diag_ready.i.h"
#include "controller.i.h"


namespace abc { namespace smbus {

    /**
     * @brief Analog-to-digital converter connected over SMBus.
     */
    class adc
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief            Constructor.
         * @param controller Pointer to an SMBus controller.
         * @param target     SMBus target representing the HAT to which the servo is connected.
         * @param reg        HAT register of the ADC.
         * @param log        `diag::log_ostream` pointer. May be `nullptr`.
         */
        adc(controller* controller, const target& target, register_t reg, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        adc(adc&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        adc(const adc& other) = delete;

    public:
        /**
         * @brief Gets the current value of the ADC.
         */
        std::uint16_t get_value();

    private:
        /**
         * @brief Pointer to the `controller` instance passed in to the constructor.
         */
        controller* _controller;

        /**
         * @brief Copy of the target passed in to the constructor.
         */
        target _target;

        /**
         * @brief HAT register of the ADC.
         */
        register_t _reg;
    };


    // --------------------------------------------------------------

} }
