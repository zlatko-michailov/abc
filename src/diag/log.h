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

#include <cstdarg>
#include <cstring>

#include "../root/util.h"
#include "../root/timestamp.h"
#include "../stream/buffer_streambuf.h"
#include "../stream/table_stream.h"
#include "i/log.i.h"


namespace abc { namespace diag {

    inline log_line_ostream::log_line_ostream()
        : base() {
    }


    inline log_line_ostream::log_line_ostream(abc::stream::table_ostream* table)
        : base(table) {
    }


    inline log_line_ostream::log_line_ostream(log_line_ostream&& other) noexcept
        : base(std::move(other)) {
    }


    // --------------------------------------------------------------


    template <typename Clock>
    inline debug_line_ostream<Clock>::debug_line_ostream()
        : ctor_base() {
    }


    template <typename Clock>
    inline debug_line_ostream<Clock>::debug_line_ostream(abc::stream::table_ostream* table)
        : ctor_base(table) {
    }


    template <typename Clock>
    inline debug_line_ostream<Clock>::debug_line_ostream(debug_line_ostream&& other) noexcept
        : ctor_base(std::move(other)) {
    }


    template <typename Clock>
    inline void debug_line_ostream<Clock>::put_any(const char* origin, const char* suborigin, severity_t severity, tag_t tag, const char* format, ...) noexcept {
        va_list vlist;
        va_start(vlist, format);

        put_anyv(origin, suborigin, severity, tag, format, vlist);

        va_end(vlist);
    }


    template <typename Clock>
    inline void debug_line_ostream<Clock>::put_anyv(const char* origin, const char* suborigin, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept {
        put_props(origin, suborigin, severity, tag);

        base::put_anyv(format, vlist);
    }


    template <typename Clock>
    inline void debug_line_ostream<Clock>::put_binary(const char* origin, const char* suborigin, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept {
        std::size_t buffer_offset = 0;
        bool hasMore = true;

        while (hasMore) {
            if (buffer_offset != 0) {
                base::flush();
            }

            put_props(origin, suborigin, severity, tag);

            hasMore = base::put_binary(buffer, buffer_size, buffer_offset);
        }
    }

    template <typename Clock>
    inline void debug_line_ostream<Clock>::put_props(const char* origin, const char* suborigin, severity_t severity, tag_t tag) noexcept {
        base::put_timestamp(timestamp<Clock>(), "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u |");
        base::put_thread_id(std::this_thread::get_id(), " %16s |");
        base::put_any(" %1.1x |", (unsigned)severity);
        base::put_any(" %16llx |", (unsigned long long)tag);
        base::put_any(" %s |", origin);
        base::put_any(" %s | ", suborigin);
    }


    // --------------------------------------------------------------


    template <typename Clock>
    inline diag_line_ostream<Clock>::diag_line_ostream()
        : ctor_base() {
    }


    template <typename Clock>
    inline diag_line_ostream<Clock>::diag_line_ostream(abc::stream::table_ostream* table)
        : ctor_base(table) {
    }


    template <typename Clock>
    inline diag_line_ostream<Clock>::diag_line_ostream(diag_line_ostream&& other) noexcept
        : ctor_base(std::move(other)) {
    }


    template <typename Clock>
    inline void diag_line_ostream<Clock>::put_any(const char* origin, const char* suborigin, severity_t severity, tag_t tag, const char* format, ...) noexcept {
        va_list vlist;
        va_start(vlist, format);

        put_anyv(origin, suborigin, severity, tag, format, vlist);

        va_end(vlist);
    }


    template <typename Clock>
    inline void diag_line_ostream<Clock>::put_anyv(const char* origin, const char* suborigin, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept {
        put_props(origin, suborigin, severity, tag);

        base::put_anyv(format, vlist);
    }


    template <typename Clock>
    inline void diag_line_ostream<Clock>::put_binary(const char* origin, const char* suborigin, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept {
        std::size_t buffer_offset = 0;
        bool hasMore = true;

        while (hasMore) {
            if (buffer_offset != 0) {
                base::flush();
            }

            put_props(origin, suborigin, severity, tag);

            hasMore = base::put_binary(buffer, buffer_size, buffer_offset);
        }
    }

    template <typename Clock>
    inline void diag_line_ostream<Clock>::put_props(const char* origin, const char* suborigin, severity_t severity, tag_t tag) noexcept {
        base::put_timestamp(timestamp<Clock>(), "%4.4u-%2.2u-%2.2uT%2.2u:%2.2u:%2.2u.%3.3uZ,");
        base::put_thread_id(std::this_thread::get_id(), "%s,");
        base::put_any("%.1x,", (unsigned)severity);
        base::put_any("%llx,", (unsigned long long)tag);
        base::put_any("%s,", origin);
        base::put_any("%s,", suborigin);
    }


    // --------------------------------------------------------------


    template <typename Clock>
    inline test_line_ostream<Clock>::test_line_ostream()
        : ctor_base() {
    }


    template <typename Clock>
    inline test_line_ostream<Clock>::test_line_ostream(abc::stream::table_ostream* table)
        : ctor_base(table) {
    }


    template <typename Clock>
    inline test_line_ostream<Clock>::test_line_ostream(test_line_ostream&& other) noexcept
        : ctor_base(std::move(other)) {
    }


    template <typename Clock>
    inline void test_line_ostream<Clock>::put_any(const char* origin, const char* suborigin, severity_t severity, tag_t tag, const char* format, ...) noexcept {
        va_list vlist;
        va_start(vlist, format);

        put_anyv(origin, suborigin, severity, tag, format, vlist);

        va_end(vlist);
    }


    template <typename Clock>
    inline void test_line_ostream<Clock>::put_anyv(const char* origin, const char* suborigin, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept {
        put_props(origin, suborigin, severity, tag);

        base::put_anyv(format, vlist);
    }


    template <typename Clock>
    inline void test_line_ostream<Clock>::put_binary(const char* origin, const char* suborigin, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept {
        std::size_t buffer_offset = 0;
        bool hasMore = true;

        while (hasMore) {
            if (buffer_offset != 0) {
                base::flush();
            }

            put_props(origin, suborigin, severity, tag);

            hasMore = base::put_binary(buffer, buffer_size, buffer_offset);
        }
    }

    template <typename Clock>
    inline void test_line_ostream<Clock>::put_props(const char* origin, const char* suborigin, severity_t severity, tag_t tag) noexcept {
        base::put_timestamp(timestamp<Clock>(), "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u ");

        char buf_severity[2 * severity::debug + 1];
        severity = severity <= severity::debug ? severity : severity::debug;
        std::memset(buf_severity, ' ', 2 * severity);
        buf_severity[2 * (severity - 1)] = '\0';
        base::put_any(buf_severity);

        if (severity::is_higher_or_equal(severity, severity::callstack)) {
            base::put_any("%s::%s [0x%llx] ", origin, suborigin, (unsigned long long)tag);
        }
    }


    // --------------------------------------------------------------


    template <typename OriginPrefixStr>
    inline str_log_filter<OriginPrefixStr>::str_log_filter(OriginPrefixStr&& origin_prefix, severity_t min_severity) noexcept
        : _origin_prefix(std::move(origin_prefix))
        , _min_severity(min_severity) {
    }


    template <typename OriginPrefixStr>
    inline const OriginPrefixStr& str_log_filter<OriginPrefixStr>::origin_prefix() const noexcept {
        return _origin_prefix;
    }


    template <typename OriginPrefixStr>
    inline severity_t str_log_filter<OriginPrefixStr>::min_severity() const noexcept {
        return _min_severity;
    }


    template <typename OriginPrefixStr>
    inline void str_log_filter<OriginPrefixStr>::origin_prefix(OriginPrefixStr&& origin_prefix) noexcept {
        _origin_prefix = std::move(origin_prefix);
    }


    template <typename OriginPrefixStr>
    inline void str_log_filter<OriginPrefixStr>::min_severity(severity_t min_severity) noexcept {
        _min_severity = min_severity;
    }


    template <typename OriginPrefixStr>
    inline bool str_log_filter<OriginPrefixStr>::is_enabled(const char* origin, severity_t severity) const noexcept {
        if (!severity::is_higher_or_equal(severity, _min_severity)) {
            return false;
        }

        if (is_empty_str(_origin_prefix)) {
            return true;
        }

        if (origin == nullptr) {
            return false;
        }

        return std::strlen(origin) >= str_length(_origin_prefix)
            && std::strncmp(origin, c_str(_origin_prefix), str_length(_origin_prefix)) == 0;
    }


    // --------------------------------------------------------------


    inline log_ostream::log_ostream(log_line_ostream* line, const log_filter* filter)
        : _line(line)
        , _filter(filter) {
    }


    inline log_ostream::log_ostream(log_ostream&& other) noexcept
        : _line(other._line)
        , _filter(other._filter) {
    }


    inline const log_filter* log_ostream::filter() const noexcept {
        return _filter;
    }


    inline void log_ostream::put_any(const char* origin, const char* suborigin, severity_t severity, tag_t tag, const char* format, ...) noexcept {
        va_list vlist;
        va_start(vlist, format);

        put_anyv(origin, suborigin, severity, tag, format, vlist);

        va_end(vlist);
    }


    inline void log_ostream::put_anyv(const char* origin, const char* suborigin, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept {
        if (_filter == nullptr || _filter->is_enabled(origin, severity)) {
            if (_line != nullptr) {
                std::lock_guard<std::mutex> lock(_put_mutex);

                _line->put_anyv(origin, suborigin, severity, tag, format, vlist);
                _line->flush();
            }
        }
    }


    inline void log_ostream::put_binary(const char* origin, const char* suborigin, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept {
        if (_filter == nullptr || _filter->is_enabled(origin, severity)) {
            if (_line != nullptr) {
                std::lock_guard<std::mutex> lock(_put_mutex);

                _line->put_binary(origin, suborigin, severity, tag, buffer, buffer_size);
                _line->flush();
            }
        }
    }


    inline void log_ostream::put_blank_line(const char* origin, severity_t severity) noexcept {
        if (_filter == nullptr || _filter->is_enabled(origin, severity)) {
            if (_line != nullptr) {
                std::lock_guard<std::mutex> lock(_put_mutex);

                _line->put_blank();
                _line->flush();
            }
        }
    }


    // --------------------------------------------------------------


    inline bool severity::is_higher(severity_t severity, severity_t other) noexcept {
        return severity < other;
    }


    inline bool severity::is_higher_or_equal(severity_t severity, severity_t other) noexcept {
        return severity <= other;
    }

} }
