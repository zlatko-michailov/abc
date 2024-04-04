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

#include <streambuf>
#include <ostream>
#include <thread>
#include <vector>

#include "../size.h"
#include "../buffer_streambuf.h"
#include "timestamp.i.h"
#include "stream.i.h"


namespace abc {

    /**
     * @brief Output stream that puts lines.
     */
    class table_ostream
        : public ostream {

        using base = ostream;

    public:
        /**
         * @brief New line literal.
         */
        static constexpr char endl = '\n';

    public:
        /**
         * @brief    Constructor.
         * @param sb Pointer to a `std::streambuf` implementation. 
         */
        table_ostream(std::streambuf* sb);

        /**
         * @brief Move constructor.
         */
        table_ostream(table_ostream&& other);

        /**
         * @brief Deleted.
         */
        table_ostream(const table_ostream& other) = delete;

    public:
        /**
         * @brief           Puts a line.
         * @details         A new line char is assumed to already contain a trailing new line char.
         * @param line      Buffer to put.
         * @param line_size Buffer size. Optional for null-terminated strings.
         */
        void put_line(const char* line, std::size_t line_size = size::strlen) noexcept;

        /**
         * @brief Puts a new line.
         */
        void put_blank_line() noexcept;
    };


    // --------------------------------------------------------------


    /**
     * @brief   Output stream that puts chars into a line buffer.
     * @details The built line buffer is put through a `table_ostream` upon `flush()`.
     */
    class line_ostream
        : public ostream {

        using base = ostream;

    public:
        /**
         * @brief New line literal.
         */
        static constexpr char endl = '\n';

        /**
         * @brief End of stream/string literal.
         */
        static constexpr char ends = '\0';

    public:
        /**
         * @brief Default constructor.
         */
        line_ostream();

        /**
         * @brief       Constructor.
         * @param table Pointer to a `table_ostream` implementation where the built line buffer should be put upon `flush()`.
         */
        line_ostream(table_ostream* table);

        /**
         * @brief Move constructor.
         */
        line_ostream(line_ostream&& other) noexcept;

        /**
         * @brief Deleted.
         */
        line_ostream(const line_ostream& other) = delete;

        /**
         * @brief Destructor.
         */
        ~line_ostream() noexcept;

    public:
        /**
         * @brief Appends a null char, and returns the line buffer.
         */
        const char* get() noexcept;

        /**
         * @brief Appends a new-line char and a null char, and puts the line buffer to the `table_ostream`.
         */
        void flush() noexcept;

    public:
        /**
         * @brief        Puts a formatted string.
         * @param format Format.
         * @param ...    Variable arguments.
         */
        void put_any(const char* format, ...) noexcept;

        /**
         * @brief        Puts a formatted string.
         * @param format Format.
         * @param vlist  Variable arguments.
         */
        void put_anyv(const char* format, va_list vlist) noexcept;

        /**
         * @brief               Puts a binary buffer.
         * @param buffer        Buffer.
         * @param buffer_size   Buffer size.
         * @param buffer_offset Start offset. Gets updated.
         */
        bool put_binary(const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset) noexcept;

    public:
        /**
         * @brief        Puts a formatted `timestamp` value.
         * @tparam Clock Clock of the timestamp.
         * @param ts     `timestamp` value.
         * @param format Format.
         */
        template <typename Clock>
        void put_timestamp(const timestamp<Clock>& ts, const char* format) noexcept;

        /**
         * @brief           Puts a thread ID.
         * @param thread_id Thread ID.
         * @param format    Should be a variation of `"%s"`. Optional.
         */
        void put_thread_id(std::thread::id thread_id, const char* format = "%s") noexcept;

        /**
         * @brief Puts a new line.
         */
        void put_blank() noexcept; 

    private:
        /**
         * @brief  Tries to ensure `available` chars of buffer capacity.
         * @return `true` = success, `false` = failure.
         */
        bool try_ensure_capacity(std::size_t available) noexcept; 

    private:
        /**
         * @brief The `table_ostream` pointer passed in to the constructor.
         */
        table_ostream* _table;

        /**
         * @brief Line buffer.
         */
        std::vector<char> _buffer;

        /**
         * @brief `buffer_streambuf` around the line buffer, which must be passed to the base constructor.
         */
        buffer_streambuf _sb;

        /**
         * @brief Count of chars written to the buffer.
         */
        std::size_t _pcount;
    };

}
