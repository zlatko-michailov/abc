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

#include <iostream>

#include "../root/ascii.h"
#include "i/vector_streambuf.i.h"

namespace abc { namespace stream {

    template <typename Char>
    inline basic_vector_streambuf<Char>::basic_vector_streambuf(std::size_t inital_capacity)
        : base(nullptr, 0, 0, nullptr, 0, 0)
        , _vector(inital_capacity, ascii::ends) {

        base::reset(_vector.data(), 0, 0, _vector.size(), _vector.data(), 0, 0, _vector.size());
    }


    template <typename Char>
    inline basic_vector_streambuf<Char>::basic_vector_streambuf(basic_vector_streambuf<Char>&& other) noexcept
        : base(nullptr, 0, 0, nullptr, 0, 0)
        , _vector(std::move(other._vector)) {

        base::reset(_vector.data(), 0, other.current_get_pos(), other.end_get_pos(), _vector.data(), 0, other.current_put_pos(), _vector.size());
        static_cast<base&>(other).reset(nullptr, 0, 0, 0, nullptr, 0, 0, 0);
    }


    template <typename Char>
    inline void basic_vector_streambuf<Char>::reset() noexcept {
        base::reset(_vector.data(), 0, 0, _vector.size(), _vector.data(), 0, 0, _vector.size());
    }


    template <typename Char>
    inline bool basic_vector_streambuf<Char>::try_ensure_capacity(std::size_t available) noexcept {
        try {
            ensure_capacity(available);
        }
        catch (...) {
            return false;
        }

        return true;
    }


    template <typename Char>
    inline void basic_vector_streambuf<Char>::ensure_capacity(std::size_t available) {
        std::size_t total = base::current_put_pos() + available;

        if (_vector.size() < total) {
            _vector.resize(total);
            base::reset(_vector.data(), 0, base::current_get_pos(), _vector.size(), _vector.data(), 0, base::current_put_pos(), _vector.size());
        }
    }


    template <typename Char>
    inline const std::vector<Char>& basic_vector_streambuf<Char>::vector() const noexcept {
        return _vector;
    }

} }

