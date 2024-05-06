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

#include "i/buffer_streambuf.i.h"


namespace abc {

    template <typename Char>
    inline basic_buffer_streambuf<Char>::basic_buffer_streambuf(Char* get_buffer, std::size_t get_begin_pos, std::size_t get_end_pos, Char* put_buffer, std::size_t put_begin_pos, std::size_t put_end_pos) noexcept
        : basic_buffer_streambuf<Char>(&get_buffer[get_begin_pos], &get_buffer[get_end_pos], &put_buffer[put_begin_pos], &put_buffer[put_end_pos]) {
    }


    template <typename Char>
    inline basic_buffer_streambuf<Char>::basic_buffer_streambuf(Char* get_begin_ptr, Char* get_end_ptr, Char* put_begin_ptr, Char* put_end_ptr) noexcept
        : base() {

        reset(get_begin_ptr, get_begin_ptr, get_end_ptr, put_begin_ptr, 0, put_end_ptr);
    }


    template <typename Char>
    inline basic_buffer_streambuf<Char>::basic_buffer_streambuf(basic_buffer_streambuf<Char>&& other) noexcept
        : base() {

        reset(other.eback(), other.gptr(), other.egptr(), other.pbase(), other.pptr() - other.pbase(), other.epptr());
    }


    template <typename Char>
    inline void basic_buffer_streambuf<Char>::reset(Char* get_buffer, std::size_t get_begin_pos, std::size_t get_current_pos, std::size_t get_end_pos, Char* put_buffer, std::size_t put_begin_pos, std::size_t put_current_pos, std::size_t put_end_pos) noexcept {
        reset(get_buffer + get_begin_pos, get_buffer + get_current_pos, get_buffer + get_end_pos, put_buffer + put_begin_pos, put_current_pos, put_buffer + put_end_pos);
    }


    template <typename Char>
    inline void basic_buffer_streambuf<Char>::reset(Char* get_begin_ptr, Char* get_current_ptr, Char* get_end_ptr, Char* put_begin_ptr, std::size_t put_current_pos, Char* put_end_ptr) noexcept {
        base::setg(get_begin_ptr, get_current_ptr, get_end_ptr);
        base::setp(put_begin_ptr, put_end_ptr);
        base::pbump(put_current_pos);
    }


    template <typename Char>
    inline Char* basic_buffer_streambuf<Char>::get_begin_ptr() const {
        return base::eback();
    }


    template <typename Char>
    inline std::size_t basic_buffer_streambuf<Char>::get_current_pos() const {
        return base::gptr() - base::eback();
    }


    template <typename Char>
    inline std::size_t basic_buffer_streambuf<Char>::get_end_pos() const {
        return base::egptr() - base::eback();
    }


    template <typename Char>
    inline void basic_buffer_streambuf<Char>::move_get_current_pos(int count) {
        base::gbump(count);
    }


    template <typename Char>
    inline Char* basic_buffer_streambuf<Char>::put_begin_ptr() const {
        return base::pbase();
    }


    template <typename Char>
    inline std::size_t basic_buffer_streambuf<Char>::put_current_pos() const {
        return base::pptr() - base::pbase();
    }


    template <typename Char>
    inline std::size_t basic_buffer_streambuf<Char>::put_end_pos() const {
        return base::epptr() - base::pbase();
    }


    template <typename Char>
    inline void basic_buffer_streambuf<Char>::move_put_current_pos(int count) {
        base::pbump(count);
    }

}