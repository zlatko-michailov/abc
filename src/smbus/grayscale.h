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

#include "../diag/diag_ready.h"
#include "controller.h"
#include "i/grayscale.i.h"


namespace abc { namespace smbus {

    inline grayscale::grayscale(controller* controller, const target& target,
                register_t reg_left, register_t reg_center, register_t reg_right,
                diag::log_ostream* log)
        : diag_base("abc::smbus::grayscale", log)
        , _controller(controller)
        , _target(target)
        , _reg_left(reg_left)
        , _reg_center(reg_center)
        , _reg_right(reg_right) {

        constexpr const char* suborigin = "grayscale()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10749, "Begin:");

        diag_base::expect(suborigin, controller != nullptr, 0x1074a, "controller != nullptr");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10703, "End:");
    }


    inline grayscale_values grayscale::get_values() {
        static constexpr std::uint16_t zero = 0x0000;

        grayscale_values values{ };

        _controller->put_word(_target, _reg_left, zero);
        values.left = _controller->get_noreg_word(_target);

        _controller->put_word(_target, _reg_center, zero);
        values.center = _controller->get_noreg_word(_target);

        _controller->put_word(_target, _reg_right, zero);
        values.right = _controller->get_noreg_word(_target);

        return values;
    }


    // --------------------------------------------------------------

} }
