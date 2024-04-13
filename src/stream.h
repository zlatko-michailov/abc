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

#include "i/stream.i.h"


namespace abc {

    template <typename Stream>
    inline stream<Stream>::stream(std::streambuf* sb)
        : base(sb) {
    }


    template <typename Stream>
    inline stream<Stream>::stream(stream&& other) noexcept
        : base(std::move(other)) {

        base::rdbuf(other.rdbuf());
    }


    template <typename Stream>
    inline void stream<Stream>::reset() {
        base::clear(base::goodbit);
    }


    template <typename Stream>
    inline std::streambuf* stream<Stream>::rdbuf() const {
        return base::rdbuf();
    }


    template <typename Stream>
    inline bool stream<Stream>::eof() const {
        return base::eof();
    }


    template <typename Stream>
    inline bool stream<Stream>::good() const {
        return base::good();
    }


    template <typename Stream>
    inline bool stream<Stream>::bad() const {
        return base::bad();
    }


    template <typename Stream>
    inline bool stream<Stream>::fail() const {
        return base::fail();
    }


    template <typename Stream>
    inline bool stream<Stream>::operator!() const {
        return base::operator!();
    }


    template <typename Stream>
    inline stream<Stream>::operator bool() const {
        return base::operator bool();
    }


    template <typename Stream>
    inline bool stream<Stream>::is_good() const {
        return base::good() && !Stream::eof();
    }


    template <typename Stream>
    inline void stream<Stream>::set_bad() {
        base::clear(base::badbit | base::failbit);
    }


    template <typename Stream>
    inline void stream<Stream>::set_bad_if(bool condition) {
        if (condition) {
            set_bad();
        }
    }


    template <typename Stream>
    inline void stream<Stream>::set_fail() {
        base::setstate(base::failbit);
    }


    template <typename Stream>
    inline void stream<Stream>::set_fail_if(bool condition) {
        if (condition) {
            set_fail();
        }
    }


    template <typename Stream>
    inline void stream<Stream>::set_eof() {
        base::setstate(base::eofbit);
    }


    template <typename Stream>
    inline void stream<Stream>::set_eof_if(bool condition) {
        if (condition) {
            set_eof();
        }
    }


    // --------------------------------------------------------------


    inline istream::istream(std::streambuf* sb)
        : base(sb)
        , _gcount(0) {
    }


    inline istream::istream(istream&& other) noexcept
        : base(std::move(other)) {
    }


    inline void istream::reset() {
        base::reset();
        _gcount = 0;
    }


    inline std::size_t istream::gcount() const noexcept {
        return _gcount;
    }


    inline void istream::set_gcount(std::size_t gcount) noexcept {
        _gcount = gcount;
    }


    // --------------------------------------------------------------


    inline ostream::ostream(std::streambuf* sb)
        : base(sb) {
    }


    inline ostream::ostream(ostream&& other) noexcept
        : base(std::move(other)) {
    }


    inline void ostream::flush() {
        base::flush();
    }

}

