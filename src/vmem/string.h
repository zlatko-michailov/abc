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

#include <string>

#include "list.h"
#include "i/string.i.h"


namespace abc { namespace vmem {

    template <typename Char>
    inline constexpr const char* basic_string_streambuf<Char>::origin() noexcept {
        return "abc::vmem::basic_string_streambuf";
    }


    template <typename Char>
    inline basic_string_streambuf<Char>::basic_string_streambuf(String* string, diag::log_ostream* log)
        : base()
        , diag_base(abc::copy(origin()), log)
        , _string(string)
        , _get_itr(nullptr)
        , _get_ch(0)
        , _put_ch(0) {

        constexpr const char* suborigin = "basic_string_streambuf()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: string=%p", string);

        diag_base::expect(suborigin, string != nullptr, 0x107b3, "string != nullptr");

        base::setg(&_get_ch, &_get_ch + 1, &_get_ch + 1);
        base::setp(&_put_ch, &_put_ch + 1);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename Char>
    inline basic_string_streambuf<Char>::basic_string_streambuf(basic_string_streambuf&& other) noexcept
        : base()
        , diag_base(other.log())
        , _string(other._string)
        , _get_itr(std::move(other._get_itr))
        , _get_ch(other._get_ch)
        , _put_ch(other._put_ch) {

        constexpr const char* suborigin = "basic_string_streambuf(move)";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: other._string=%p", other._string);

        base::setg(&_get_ch, &_get_ch + 1, &_get_ch + 1);
        base::setp(&_put_ch, &_put_ch + 1);

        other._string = nullptr;
        other._log = nullptr;
        other._get_itr = nullptr;
        other.setg(nullptr, nullptr, nullptr);
        other.setp(nullptr, nullptr);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    template <typename Char>
    inline typename std::basic_streambuf<Char>::int_type basic_string_streambuf<Char>::underflow() {
        if (_get_itr == nullptr) {
            _get_itr = _string->begin();
        }

        typename base::int_type ch = std::char_traits<Char>::eof();

        if (_get_itr.can_deref()) {
            ch = _get_ch = *_get_itr++;

            base::setg(&_get_ch, &_get_ch, &_get_ch + 1);
        }

        return ch;
    }


    template <typename Char>
    inline typename std::basic_streambuf<Char>::int_type basic_string_streambuf<Char>::overflow(typename base::int_type ch) {
        _string->push_back(_put_ch);
        _string->push_back(ch);

        base::setp(&_put_ch, &_put_ch + 1);

        return ch;
    }

    template <typename Char>
    inline int basic_string_streambuf<Char>::sync() {
        if (base::pptr() != &_put_ch) {
            _string->push_back(_put_ch);
        }

        base::setp(&_put_ch, &_put_ch + 1);

        return 0;
    }

} }
