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

#include <streambuf>

#include "list.i.h"


namespace abc { namespace vmem {

    /**
     * @brief       Virtually contiguous generic string.
     * @tparam Char Character.
     */
    template <typename Char>
    using basic_string = list<Char>;


    /**
     * @brief Virtually contiguous `char` string.
     */
    using string = basic_string<char>;


    // --------------------------------------------------------------


    /**
     * @brief       Generic string iterator.
     * @tparam Char Character.
     */
    template <typename Char>
    using basic_string_iterator = list_iterator<Char>;


    /**
     * @brief       Generic string const iterator.
     * @tparam Char Character.
     */
    template <typename Char>
    using basic_string_const_iterator = list_const_iterator<Char>;


    /**
     * @brief String iterator.
     */
    using string_iterator = basic_string_iterator<char>;


    /**
     * @brief String const iterator.
     */
    using string_const_iterator = basic_string_const_iterator<char>;


    // --------------------------------------------------------------


    /**
     * @brief       `std::streambuf` specialization that is backed by a vmem string.
     * @tparam Char Character.
     */
    template <typename Char>
    class basic_string_streambuf
        : public std::basic_streambuf<Char>
        , protected diag::diag_ready<const char*> {

        using base = std::basic_streambuf<Char>;
        using diag_base = diag::diag_ready<const char*>;
        using String = basic_string<Char>;
        using Iterator = basic_string_iterator<Char>;

    private:
        static constexpr const char* origin() noexcept;

    public:
        /**
         * @brief        Constructor.
         * @param string Pointer to a `vmem::basic_string<Char>` instance.
         * @param log    Pointer to a `log_ostream` instance.
         */
        basic_string_streambuf(String* string, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        basic_string_streambuf(basic_string_streambuf&& other) noexcept;

        /**
         * @brief Deleted.
         */
        basic_string_streambuf(const basic_string_streambuf& other) = delete;

    protected:
        /**
         * @brief  Handler that reads a char from the string.
         * @return The char received.
         */
        virtual typename std::basic_streambuf<Char>::int_type underflow() override;

        /**
         * @brief    Handler that appends a char to the string.
         * @param ch Char to be sent.
         * @return   `ch`
         */
        virtual typename std::basic_streambuf<Char>::int_type overflow(typename base::int_type ch) override;

        /**
         * @brief  Flushes.
         * @return `0`
         */
        virtual int sync() override;

    private:
        /**
         * @brief The `String` pointer passed in to the constructor.
         */
        String* _string;

        /**
         * @brief 'get' iterator.
         */
        Iterator _get_itr;

        /**
         * @brief Cached 'get' char.
         */
        Char _get_ch;

        /**
         * @brief Cached 'put' char.
         */
        Char _put_ch;
    };


    // --------------------------------------------------------------


    /**
     * @brief `std::streambuf` specialization that is backed by a `char` vmmem string.
     */
    using string_streambuf = basic_string_streambuf<char>;

} }
