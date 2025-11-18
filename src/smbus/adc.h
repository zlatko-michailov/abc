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

#include "../diag/diag_ready.h"
#include "controller.h"
#include "i/adc.i.h"


namespace abc { namespace smbus {

    inline adc::adc(controller* controller, const target& target, register_t reg, diag::log_ostream* log)
        : diag_base("abc::smbus::adc", log)
        , _controller(controller)
        , _target(target)
        , _reg(reg) {

        constexpr const char* suborigin = "adc()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::expect(suborigin, controller != nullptr, __TAG__, "controller != nullptr");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline std::uint16_t adc::get_value() {
        static constexpr std::uint16_t zero = 0x0000;

        std::uint32_t value;
        {
            std::lock_guard<abc::concurrent::mutex> lock(_controller->mutex());

            _controller->put_word(_target, _reg, zero);
            value = _controller->get_noreg_word(_target);
        }

        return value;
    }


    // --------------------------------------------------------------

} }
