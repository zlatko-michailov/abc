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
#include <cstdio>

#include "../root/timestamp.h"
#include "../diag/exception.h"
#include "i/multifile_streambuf.i.h"


namespace abc { namespace stream {

    template <typename Clock>
    inline multifile_streambuf<Clock>::multifile_streambuf(std::string&& path, std::ios_base::openmode mode)
        : base()
        , _path(std::move(path))
        , _mode(mode) {

        static constexpr char path_separator = '/';

        // Ensure a trailing separator.
        if (_path.back() != path_separator) {
            _path.push_back(path_separator);
        }

        _filename_start = _path.cend();

        reopen();
    }


    template <typename Clock>
    inline multifile_streambuf<Clock>::multifile_streambuf(multifile_streambuf&& other) noexcept
        : base(std::move(static_cast<base&&>(other)))
        , _path(std::move(other._path))
        , _filename_start(std::move(other._filename_start))
        , _mode(other._mode) {
    }


    template <typename Clock>
    inline void multifile_streambuf<Clock>::reopen() {
        if (base::is_open()) {
            base::close();
        }

        update_filename();
        base::open(_path, _mode);
    }


    template <typename Clock>
    inline const std::string& multifile_streambuf<Clock>::path() const noexcept {
        return _path;
    }


    template <typename Clock>
    inline void multifile_streambuf<Clock>::update_filename() {
        static constexpr const char* filename_format = "%4.4u%2.2u%2.2u_%2.2u%2.2u%2.2u.txt";
        static constexpr std::size_t filename_length = 19;

        timestamp<Clock> ts;

        char filename[filename_length + 1];
        std::snprintf(filename, sizeof(filename), filename_format, ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds());

        _path.replace(_filename_start, _path.cend(), filename);
    }


    // --------------------------------------------------------------


    template <typename Clock>
    inline duration_multifile_streambuf<Clock>::duration_multifile_streambuf(typename Clock::duration duration, std::string&& path, std::ios_base::openmode mode)
        : base(std::move(path), mode)
        , _duration(duration)
        , _ts() {
    }


    template <typename Clock>
    inline void duration_multifile_streambuf<Clock>::reopen() {
        base::reopen();

        _ts = timestamp<Clock>();
    }


    template <typename Clock>
    inline int duration_multifile_streambuf<Clock>::sync() {
        base::sync();

        // This is where we check if we have to start a bew file.
        timestamp<Clock> ts;
        if (ts - _ts >= _duration) {
            reopen();
        }

        return 0;
    }


    // --------------------------------------------------------------


    template <typename Clock>
    inline size_multifile_streambuf<Clock>::size_multifile_streambuf(std::size_t size, std::string&& path, std::ios_base::openmode mode)
        : base(std::move(path), mode)
        , _size(size)
        , _current_size(0) {
    }


    template <typename Clock>
    inline void size_multifile_streambuf<Clock>::reopen() {
        base::reopen();

        _current_size = 0;
    }


    template <typename Clock>
    inline std::streamsize size_multifile_streambuf<Clock>::xsputn(const char* s, std::streamsize count) {
        _current_size += count;

        return base::xsputn(s, count);
    }


    template <typename Clock>
    inline int size_multifile_streambuf<Clock>::sync() {
        _current_size += pcount();

        base::sync();

        // This is where we check if we have to start a bew file.
        if (_current_size >= _size) {
            reopen();
        }

        return 0;
    }


    template <typename Clock>
    inline std::size_t size_multifile_streambuf<Clock>::pcount() const noexcept {
        return base::pptr() - base::pbase();
    }


    // --------------------------------------------------------------
} }
