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

#include <cstdarg>
#include <cstring>

#include "vector_streambuf.h"
#include "stream.h"
#include "i/table_stream.i.h"


namespace abc {

    inline table_ostream::table_ostream(std::streambuf* sb)
        : base(sb) {
    }


    inline table_ostream::table_ostream(table_ostream&& other)
        : base(std::move(other)) {
    }


    inline void table_ostream::put_line(const char* line, std::size_t line_size) noexcept {
        if (line_size == size::strlen) {
            line_size = std::strlen(line);
        }

        try {
            base::write(line, line_size);
            base::flush();
        }
        catch (...) {
        }
    }


    inline void table_ostream::put_blank_line() noexcept {
        try {
            *this << ascii::endl;
            base::flush();
        }
        catch (...) {
        }
    }


    // --------------------------------------------------------------


    inline line_ostream::line_ostream()
        : line_ostream(nullptr) {
    }


    inline line_ostream::line_ostream(table_ostream* table)
        : base(&_sb)
        , _table(table)
        , _sb(size::_256) {
    }


    inline line_ostream::line_ostream(line_ostream&& other) noexcept
        : base(std::move(other))
        , _table(other._table)
        , _sb(std::move(other._sb)) {
    }


    inline line_ostream::~line_ostream() noexcept {
        flush();
    }


    inline const char* line_ostream::get() noexcept {
        if (!_sb.try_ensure_capacity(1)) {
            return "";
        }

        put_ends();

        return _sb.vector().data();
    }


    inline void line_ostream::flush() noexcept {
        if (_sb.current_put_pos() > 0) {
            if (!_sb.try_ensure_capacity(2)) {
                return;
            }

            _sb.sputc(ascii::endl);
            put_ends();

            if (_table != nullptr) {
                _table->put_line(_sb.begin_put_ptr(), _sb.current_put_pos());
            }

            _sb.reset();
        }
    }


    inline void line_ostream::put_any(const char* format, ...) noexcept {
        std::va_list vlist;
        va_start(vlist, format);

        put_anyv(format, vlist);

        va_end(vlist);
    }


    inline void line_ostream::put_anyv(const char* format, std::va_list vlist) noexcept {
        std::va_list vlist_copy;
        va_copy(vlist_copy, vlist);
        int pc = std::vsnprintf(nullptr, 0, format, vlist_copy);

        if (!_sb.try_ensure_capacity(pc + 1)) {
            return;
        }

        pc = std::vsnprintf(_sb.begin_put_ptr() + _sb.current_put_pos(), pc + 1, format, vlist);
        _sb.move_current_put_pos(pc);
    }


    inline bool line_ostream::put_binary(const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset) noexcept {
        // 0000: 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f  abcdefghijklmnop
        constexpr std::size_t half_chunk_size = 8;
        constexpr std::size_t chunk_size = half_chunk_size * 2;
        constexpr std::size_t local_size = 5 + (chunk_size * 3) + 1 + 2 + chunk_size;
        constexpr char hex[] = "0123456789abcdef";
        constexpr char blank = ' ';
        constexpr char nonprint = '.';
        constexpr char head = ':';

        if (buffer_size <= buffer_offset) {
            return false;
        }

        if ((buffer_offset % chunk_size) != 0) {
            return false;
        }

        if (!_sb.try_ensure_capacity(local_size + 1)) {
            return false;
        }

        const std::uint8_t* chunk = static_cast<const std::uint8_t*>(buffer) + buffer_offset;
        std::size_t local_offset = 0;
        char* line = _sb.begin_put_ptr() + _sb.current_put_pos();
        bool hasMore = true;

        // 0000:
        line[local_offset++] = hex[(buffer_offset >> 12) & 0xf];
        line[local_offset++] = hex[(buffer_offset >>  8) & 0xf];
        line[local_offset++] = hex[(buffer_offset >>  4) & 0xf];
        line[local_offset++] = hex[buffer_offset & 0xf];
        line[local_offset++] = head;

        // 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f
        for (int half = 0; half < 2; half++) {
            line[local_offset++] = blank;

            for (std::size_t chunk_offset = 0; chunk_offset < half_chunk_size; chunk_offset++) {
                std::size_t source_offset = half * half_chunk_size + chunk_offset;

                if (buffer_offset + source_offset < buffer_size) {
                    line[local_offset++] = hex[(chunk[source_offset] >> 4) & 0xf];
                    line[local_offset++] = hex[chunk[source_offset] & 0xf];
                }
                else {
                    line[local_offset++] = blank;
                    line[local_offset++] = blank;
                    hasMore = false;
                }
    
                line[local_offset++] = blank;
            }
        }

        line[local_offset++] = blank;

        //  abcdefghijklmnop
        for (std::size_t chunk_offset = 0; chunk_offset < chunk_size; chunk_offset++) {
            if (buffer_offset + chunk_offset < buffer_size) {
                if (std::isprint(chunk[chunk_offset])) {
                    line[local_offset++] = chunk[chunk_offset];
                }
                else {
                    line[local_offset++] = nonprint;
                }
            }
            else {
                line[local_offset++] = blank;
            }
        }

        line[local_offset++] = ascii::ends;

        _sb.move_current_put_pos(local_size);
        buffer_offset += chunk_size;
        return hasMore;
    }


    template <typename Clock>
    inline void line_ostream::put_timestamp(const timestamp<Clock>& ts, const char* format) noexcept {
        put_any(format, ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds(), ts.milliseconds());
    }


    inline void line_ostream::put_thread_id(std::thread::id thread_id, const char* format) noexcept {
        char buf[17];
        buffer_streambuf sb(nullptr, 0, 0, buf, 0, sizeof(buf));

        try {
            std::ostream stream(&sb);
            stream << std::hex << thread_id << ascii::ends;
        }
        catch (...) {
            buf[0] = ascii::ends;
        }

        put_any(format, buf);
    }


    inline void line_ostream::put_blank() noexcept {
        if (_table != nullptr) {
            _table->put_blank_line();
        }
    }


    inline void line_ostream::put_ends() noexcept {
        if (_sb.try_ensure_capacity(1)) {
            _sb.sputc(ascii::ends);
            _sb.move_current_put_pos(-1);
        }
    }
}
