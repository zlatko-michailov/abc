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

#include <cstddef>
#include <streambuf>
#include <vector>

#include "buffer_streambuf.i.h"


namespace abc {

    /**
     * @brief       A combination of an `abc::basic_buffer_streambuf` and an underlying `std::vector`.
     * @tparam Char Character type.
     */
    template <typename Char>
    class basic_vector_streambuf
        : public basic_buffer_streambuf<Char> {

        using base = basic_buffer_streambuf<Char>;

    public:
        /**
         * @brief                  Constructor.
         * @param initial_capacity Initial capacity of the underlying `std::vector`.
         */
        basic_vector_streambuf(std::size_t inital_capacity);

        /**
         * @brief Move constructor.
         */
        basic_vector_streambuf(basic_vector_streambuf<Char>&& other) noexcept;

        /**
         * @brief Deleted.
         */
        basic_vector_streambuf(const basic_vector_streambuf<Char>& other) = delete;

    public:
        /**
         * @brief           Tries to ensure `available` chars of buffer capacity.
         * @param available Desired available capacity.
         * @return          `true` = success, `false` = failure.
         */
        bool try_ensure_capacity(std::size_t available) noexcept; 

        /**
         * @brief           Ensure `available` chars of buffer capacity. Throwable version.
         * @param available Desired available capacity.
         */
        void ensure_capacity(std::size_t available);

    public:
        /**
         * @brief Returns a const reference to the underlying `std::vector`.
         */
        const std::vector<Char>& vector() const noexcept;

    private:
        std::vector<Char> _vector;
    };


    // --------------------------------------------------------------


    /**
     * @brief `std::streambuf` implementation over a `std::vector<char>`.
     */
    using vector_streambuf = basic_vector_streambuf<char>;

}

