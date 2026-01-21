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

#include "../diag/diag_ready.h"
#include "controller.h"
#include "i/grayscale.i.h"


namespace abc { namespace smbus {

    inline grayscale::grayscale(controller* controller, const target& target,
                register_t reg_left, register_t reg_center, register_t reg_right,
                diag::log_ostream* log)
        : diag_base("abc::smbus::grayscale", log)
        , _adc_left(controller, target, reg_left, log)
        , _adc_center(controller, target, reg_center, log)
        , _adc_right(controller, target, reg_right, log) {

        constexpr const char* suborigin = "grayscale()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10749, "Begin:");

        diag_base::expect(suborigin, controller != nullptr, 0x1074a, "controller != nullptr");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10703, "End:");
    }


    inline grayscale_values grayscale::get_values() {
        grayscale_values values{ };

        values.left = _adc_left.get_value();
        values.center = _adc_center.get_value();
        values.right = _adc_right.get_value();

        return values;
    }


    // --------------------------------------------------------------

} }
