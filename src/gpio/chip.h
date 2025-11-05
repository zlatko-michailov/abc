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

#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../diag/diag_ready.h"
#include "i/chip.i.h"



namespace abc { namespace gpio {

    inline chip_info::chip_info() noexcept
        : chip_info_base{ }
        , is_valid(false) {
    }


    inline line_info::line_info() noexcept
        : line_info_base{ }
        , is_valid(false) {
    }


    // --------------------------------------------------------------


    inline chip::chip(int dev_gpiochip_pos, const char* consumer, diag::log_ostream* log)
        : diag_base("abc::gpio::chip", log) {

        char path[max_path];
        std::snprintf(path, max_path, "/dev/gpiochip%d", dev_gpiochip_pos);

        init(path, consumer);
    }


    inline chip::chip(const char* path, const char* consumer, diag::log_ostream* log)
        : diag_base("abc::gpio::chip", log) {

        init(path, consumer);
    }


    inline void chip::init(const char* path, const char* consumer) {
        constexpr const char* suborigin = "init()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x106b9, "Begin: consumer='%s'", consumer);

        diag_base::expect(suborigin, path != nullptr, 0x106ba, "path != nullptr");
        diag_base::expect(suborigin, std::strlen(path) < max_path, 0x106bb, "std::strlen(path) < max_path");
        diag_base::expect(suborigin, consumer != nullptr, 0x106bc, "consumer != nullptr");
        diag_base::expect(suborigin, std::strlen(consumer) < max_consumer, 0x106bd, "std::strlen(path) < max_consumer");

        fd_t fd = open(path, O_RDONLY);
        diag_base::expect(suborigin, fd >= 0, 0x106be, "fd >= 0");
        close(fd);

        std::strncpy(_path, path, max_path);
        std::strncpy(_consumer, consumer, max_consumer);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106bf, "End:");
    }


    inline const char* chip::path() const noexcept {
        return _path;
    }


    inline const char* chip::consumer() const noexcept {
        return _consumer;
    }


    inline gpio::chip_info chip::chip_info() const {
        constexpr const char* suborigin = "chip_info()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x106c0, "Begin:");

        gpio::chip_info info;
        info.is_valid = false;

        fd_t fd = open(_path, O_RDONLY);
        diag_base::expect(suborigin, fd >= 0, 0x106c1, "fd >= 0");

        int stat = ::ioctl(fd, ioctl::get_chip_info, &info);
        close(fd);
        diag_base::expect(suborigin, stat >= 0, 0x106c2, "stat >= 0");

        info.is_valid = true;
        diag_base::put_any(suborigin, diag::severity::callstack, 0x106c3, "End:");

        return info;
    }


    inline gpio::line_info chip::line_info(line_pos_t pos) const {
        constexpr const char* suborigin = "line_info()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x106c4, "Begin: pos=%u", pos);

        gpio::line_info info;
        info.is_valid = false;
#if ((__ABC__GPIO_VER) == 2)
        info.offset = pos;
#else
        info.line_offset = pos;
#endif

        fd_t fd = open(_path, O_RDONLY);
        diag_base::expect(suborigin, fd >= 0, 0x106c5, "fd >= 0");

        int stat = ::ioctl(fd, ioctl::get_line_info, &info);
        close(fd);
        diag_base::expect(suborigin, stat >= 0, 0x106c6, "stat >= 0");

        info.is_valid = true;
        diag_base::put_any(suborigin, diag::severity::callstack, 0x106c7, "End:");

        return info;
    }


    // --------------------------------------------------------------

} }
