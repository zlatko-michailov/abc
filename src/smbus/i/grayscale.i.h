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
#include "adc.i.h"


namespace abc { namespace smbus {

    /**
     * @brief Bundle of grayscale sensor values.
     */
    struct grayscale_values {
        std::uint16_t left;
        std::uint16_t center;
        std::uint16_t right;
    };


    /**
     * @brief Grayscale sensor connected over SMBus.
     */
    class grayscale
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief            Constructor.
         * @param controller Pointer to an SMBus controller.
         * @param target     SMBus target representing the HAT to which the servo is connected.
         * @param reg_left   HAT register of the left sensor.
         * @param reg_center HAT register of the center sensor.
         * @param reg_right  HAT register of the right sensor.
         * @param log        `diag::log_ostream` pointer. May be `nullptr`.
         */
        grayscale(controller* controller, const target& target,
            register_t reg_left, register_t reg_center, register_t reg_right,
            diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        grayscale(grayscale&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        grayscale(const grayscale& other) = delete;

    public:
        /**
         * @brief Gets the current values of the three sensors.
         */
        grayscale_values get_values();

    private:
        /**
         * @brief ADC of the left sensor.
         */
        adc _adc_left;

        /**
         * @brief ADC of the center sensor.
         */
        adc _adc_center;

        /**
         * @brief ADC of the right sensor.
         */
        adc _adc_right;
    };


    // --------------------------------------------------------------

} }
