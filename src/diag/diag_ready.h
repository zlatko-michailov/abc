/*
MIT License

Copyright (c) 2018-2023 Zlatko Michailov 

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

#include "../../util.h"
#include "i/diag_ready.i.h"
#include "log.h"
#include "exception.h"


namespace abc { namespace diag {

    template <typename OriginStr, typename LogPtr>
    inline diag_ready<OriginStr, LogPtr>::diag_ready(OriginStr&& origin, LogPtr&& log) noexcept
        : _origin(std::move(origin))
        , _log(std::move(log)) {
    }


    template <typename OriginStr, typename LogPtr>
    inline diag_ready<OriginStr, LogPtr>::diag_ready(diag_ready&& other) noexcept
        : diag_ready(std::move(other._origin), std::move(other._log)) {
    }


    template <typename OriginStr, typename LogPtr>
    inline void diag_ready<OriginStr, LogPtr>::put_any(const char* suborigin, severity_t severity, tag_t tag, const char* format, ...) noexcept {
        va_list vlist;
        va_start(vlist, format);

        put_anyv(suborigin, severity, tag, format, vlist);

        va_end(vlist);
    }


    template <typename OriginStr, typename LogPtr>
    inline void diag_ready<OriginStr, LogPtr>::put_anyv(const char* suborigin, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept {
        if (_log != nullptr) {
            _log->put_any(c_str(_origin), suborigin, severity, tag, format, vlist);
        }
    }


    template <typename OriginStr, typename LogPtr>
    inline void diag_ready<OriginStr, LogPtr>::put_binary(const char* suborigin, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept {
        if (_log != nullptr) {
            _log->put_binary(c_str(_origin), suborigin, severity, tag, buffer, buffer_size);
        }
    }


    template <typename OriginStr, typename LogPtr>
    inline void diag_ready<OriginStr, LogPtr>::put_blank_line(severity_t severity) noexcept {
        if (_log != nullptr) {
            _log->put_blank_line(c_str(_origin), severity);
        }
    }


    template <typename OriginStr, typename LogPtr>
    template <typename Exception>
    inline void diag_ready<OriginStr, LogPtr>::throw_exception(const char* suborigin, tag_t tag, const char* format, ...) {
        va_list vlist;
        va_start(vlist, format);

        throw_exceptionv(suborigin, tag, format, vlist);

        va_end(vlist);
    }


    template <typename OriginStr, typename LogPtr>
    template <typename Exception>
    inline void diag_ready<OriginStr, LogPtr>::throw_exceptionv(const char* suborigin, tag_t tag, const char* format, va_list vlist) {
        char message[size::k2];
        std::vsnprintf(message, sizeof(message) / sizeof(char), format, vlist);

        throw exception<Exception, LogPtr>(c_str(_origin), suborigin, tag, message, _log);
    }


    template <typename OriginStr, typename LogPtr>
    inline void diag_ready<OriginStr, LogPtr>::assert(const char* suborigin, bool condition, tag_t tag, const char* format, ...) {
        va_list vlist;
        va_start(vlist, format);

        assertv(suborigin, tag, format, vlist);

        va_end(vlist);
    }


    template <typename OriginStr, typename LogPtr>
    inline void diag_ready<OriginStr, LogPtr>::assertv(const char* suborigin, bool condition, tag_t tag, const char* format, va_list vlist) {
        if (!condition) {
            throw_exceptionv<assert_error>(suborigin, tag, format, vlist);
        }
    }


    template <typename OriginStr, typename LogPtr>
    inline void diag_ready<OriginStr, LogPtr>::expect(const char* suborigin, bool condition, tag_t tag, const char* format, ...) {
        va_list vlist;
        va_start(vlist, format);

        expectv(suborigin, condition, tag, format, vlist);

        va_end(vlist);
    }


    template <typename OriginStr, typename LogPtr>
    inline void diag_ready<OriginStr, LogPtr>::expectv(const char* suborigin, bool condition, tag_t tag, const char* format, va_list vlist) {
        if (!condition) {
            throw_exception<expect_error>(suborigin, tag, format, vlist);
        }
    }


    template <typename OriginStr, typename LogPtr>
    inline void diag_ready<OriginStr, LogPtr>::ensure(const char* suborigin, bool condition, tag_t tag, const char* format, ...) {
        va_list vlist;
        va_start(vlist, format);

        ensurev(suborigin, condition, tag, format, vlist);

        va_end(vlist);
    }

    template <typename OriginStr, typename LogPtr>
    inline void diag_ready<OriginStr, LogPtr>::ensurev(const char* suborigin, bool condition, tag_t tag, const char* format, va_list vlist) {
        if (!condition) {
            throw_exception<ensure_error>(suborigin, tag, format, vlist);
        }
    }

    template <typename OriginStr, typename LogPtr>
    inline const LogPtr& diag_ready<OriginStr, LogPtr>::log() const noexcept {
        return _log;
    }

} }
