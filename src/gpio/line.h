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

#include <cstring>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../diag/diag_ready.h"
#include "chip.h"
#include "i/line.i.h"


namespace abc { namespace gpio {

    using clock = std::chrono::steady_clock;


    inline line::line(const char* origin, const chip* chip, line_pos_t pos, line_flags_t flags, diag::log_ostream* log)
        : diag_base(copy(origin), log) {

        constexpr const char* suborigin = "line()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x106c8, "Begin: pos=%u, flags=0x%llx", pos, (unsigned long long)flags);

        diag_base::expect(suborigin, chip != nullptr, 0x106c9, "chip != nullptr");

        fd_t fd = open(chip->path(), O_RDONLY);
        diag_base::expect(suborigin, fd >= 0, 0x106ca, "fd >= 0");

        line_request line_request{ };
#if ((__ABC__GPIO_VER) == 2)
        line_request.num_lines = 1;
        line_request.offsets[0] = pos;
        std::strncpy(line_request.consumer, chip->consumer(), max_consumer);
        line_request.config.flags = flags;
#else
        line_request.lines = 1;
        line_request.lineoffsets[0] = pos;
        std::strncpy(line_request.consumer_label, chip->consumer(), max_consumer);
        line_request.flags = flags;
#endif

        int ret = ::ioctl(fd, ioctl::get_line, &line_request);
        close(fd);
        diag_base::expect(suborigin, ret >= 0, 0x106cb, "ret >= 0");

        _fd = line_request.fd;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106cd, "End: _fd=%d", _fd);
    }


    inline line::~line() noexcept {
        constexpr const char* suborigin = "~line()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        if (_fd >= 0) {
            close(_fd);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline level_t line::get_level() const {
        constexpr const char* suborigin = "get_level()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        line_values values{ };
#if ((__ABC__GPIO_VER) == 2)
        values.mask = level::mask;
#endif        

        int ret = ::ioctl(_fd, ioctl::get_line_values, &values);
        diag_base::expect(suborigin, ret >= 0, __TAG__, "ret >= 0");

        level_t level;
#if ((__ABC__GPIO_VER) == 2)
        level = (values.bits & level::mask);
#else
        level = (values.values[0] & level::mask);
#endif

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: level=%u", level);

        return level;
    }


    template <typename Duration>
    level_t line::wait_for_level(level_t level, Duration timeout) const {
        constexpr const char* suborigin = "wait_for_level()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        clock::time_point start_tp = clock::now();
        clock::time_point current_tp = clock::now();
        level_t current_level = get_level();

        while (current_level != level) {
            if (std::chrono::duration_cast<Duration>(current_tp - start_tp) > timeout) {
                current_level = level::invalid;
                break;
            }

            current_tp = clock::now();
            current_level = get_level();
        }

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: current_level=%u", current_level);

        return current_level;
    }


    inline level_t line::put_level(level_t level) const {
        constexpr const char* suborigin = "put_level()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: level=%u", level);

        diag_base::expect(suborigin, (level & ~level::mask) == 0, __TAG__, "(level & ~level::mask) == 0");

        line_values values{ };
#if ((__ABC__GPIO_VER) == 2)
        values.mask = level::mask;
        values.bits = (level & level::mask);
#else
        values.values[0] = level;
#endif

        int ret = ::ioctl(_fd, ioctl::set_line_values, &values);
        diag_base::expect(suborigin, ret >= 0, __TAG__, "ret >= 0");

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: level=%u", level);

        return level;
    }


    template <typename Duration>
    inline level_t line::put_level(level_t level, Duration duration) const {
        constexpr const char* suborigin = "put_level(duration)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: level=%u", level);

        level = put_level(level);
        std::this_thread::sleep_for(duration);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End: level=%u", level);

        return level;
    }


    // --------------------------------------------------------------


    inline input_line::input_line(const chip* chip, line_pos_t pos, diag::log_ostream* log)
        : base("input_line", chip, pos, line_flags::input, log) {
    }


    // --------------------------------------------------------------


    inline output_line::output_line(const chip* chip, line_pos_t pos, diag::log_ostream* log)
        : base("output_line", chip, pos, line_flags::output, log) {
    }


    // --------------------------------------------------------------

} }
