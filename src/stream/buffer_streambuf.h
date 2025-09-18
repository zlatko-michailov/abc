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

#include "i/buffer_streambuf.i.h"


namespace abc { namespace stream {

    template <typename Char>
    inline basic_buffer_streambuf<Char>::basic_buffer_streambuf(Char* get_buffer, std::size_t begin_get_pos, std::size_t end_get_pos, Char* put_buffer, std::size_t begin_put_pos, std::size_t end_put_pos) noexcept
        : basic_buffer_streambuf<Char>(&get_buffer[begin_get_pos], &get_buffer[end_get_pos], &put_buffer[begin_put_pos], &put_buffer[end_put_pos]) {
    }


    template <typename Char>
    inline basic_buffer_streambuf<Char>::basic_buffer_streambuf(Char* begin_get_ptr, Char* end_get_ptr, Char* begin_put_ptr, Char* end_put_ptr) noexcept
        : base() {

        reset(begin_get_ptr, begin_get_ptr, end_get_ptr, begin_put_ptr, 0, end_put_ptr);
    }


    template <typename Char>
    inline basic_buffer_streambuf<Char>::basic_buffer_streambuf(basic_buffer_streambuf<Char>&& other) noexcept
        : base() {

        reset(other.eback(), other.gptr(), other.egptr(), other.pbase(), other.pptr() - other.pbase(), other.epptr());
    }


    template <typename Char>
    inline void basic_buffer_streambuf<Char>::reset(Char* get_buffer, std::size_t begin_get_pos, std::size_t current_get_pos, std::size_t end_get_pos, Char* put_buffer, std::size_t begin_put_pos, std::size_t current_put_pos, std::size_t end_put_pos) noexcept {
        reset(get_buffer + begin_get_pos, get_buffer + current_get_pos, get_buffer + end_get_pos, put_buffer + begin_put_pos, current_put_pos, put_buffer + end_put_pos);
    }


    template <typename Char>
    inline void basic_buffer_streambuf<Char>::reset(Char* begin_get_ptr, Char* current_get_ptr, Char* end_get_ptr, Char* begin_put_ptr, std::size_t current_put_pos, Char* end_put_ptr) noexcept {
        base::setg(begin_get_ptr, current_get_ptr, end_get_ptr);
        base::setp(begin_put_ptr, end_put_ptr);
        base::pbump(current_put_pos);
    }


    template <typename Char>
    inline Char* basic_buffer_streambuf<Char>::begin_get_ptr() const {
        return base::eback();
    }


    template <typename Char>
    inline std::size_t basic_buffer_streambuf<Char>::current_get_pos() const {
        return base::gptr() - base::eback();
    }


    template <typename Char>
    inline std::size_t basic_buffer_streambuf<Char>::end_get_pos() const {
        return base::egptr() - base::eback();
    }


    template <typename Char>
    inline void basic_buffer_streambuf<Char>::move_current_get_pos(int count) {
        base::gbump(count);
    }


    template <typename Char>
    inline Char* basic_buffer_streambuf<Char>::begin_put_ptr() const {
        return base::pbase();
    }


    template <typename Char>
    inline std::size_t basic_buffer_streambuf<Char>::current_put_pos() const {
        return base::pptr() - base::pbase();
    }


    template <typename Char>
    inline std::size_t basic_buffer_streambuf<Char>::end_put_pos() const {
        return base::epptr() - base::pbase();
    }


    template <typename Char>
    inline void basic_buffer_streambuf<Char>::move_current_put_pos(int count) {
        base::pbump(count);
    }

} }
