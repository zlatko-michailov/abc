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

#include <cstdint>
#include <fstream>
#include <chrono>
#include <ios>
#include <string>

#include "../../root/i/timestamp.i.h"


namespace abc { namespace stream {

    /**
     * @brief        `streambuf` specialization that is backed by files whose names are made of timestamps.
     * @tparam Clock  Clock to generate a timestamp.
     */
    template <typename Clock = std::chrono::system_clock>
    class multifile_streambuf
        : public std::filebuf {

        using base = std::filebuf;

    public:
        /**
         * @brief      Constructor.
         * @param path Path to an existing folder 
         * @param mode `std::ios_base::openmode`
         */
        multifile_streambuf(std::string&& path, std::ios_base::openmode mode = std::ios_base::out);

        /**
         * @brief Move constructor.
         */
        multifile_streambuf(multifile_streambuf&& other) noexcept;

        /**
         * @brief Deleted.
         */
        multifile_streambuf(const multifile_streambuf& other) = delete;

    public:
        /**
         * @brief Closes the current file, and opens a new file.
         */
        void reopen();

        /**
         * @brief Returns the full path of the current file.
         */
        const std::string& path() const noexcept;

    private:
        /**
         * @brief Updates the file name to the current timestamp.
         */
        void update_filename();

    private:
        /**
         * @brief The full path to the file.
         */
        std::string _path;

        /**
         * @brief Iterator pointing at the place in the path where the file name starts.
         */
        std::string::const_iterator _filename_start;

        /**
         * @brief `std::ios_base::openmode`.
         */
        std::ios_base::openmode _mode;
    };


    // --------------------------------------------------------------


    /**
     * @brief        `multifile_streambuf` specialization that is backed by files whose names are made of timestamps.
     * @details      Automatically closes and reopens a new file when the given time duration has passed. 
     * @tparam Clock Clock to generate a timestamp.
     */
    template <typename Clock = std::chrono::system_clock>
    class duration_multifile_streambuf
        : public multifile_streambuf<Clock> {

        using base = multifile_streambuf<Clock>;

    public:
        /**
         * @brief          Constructor.
         * @param duration Duration limit of the file.
         * @param path     Path to an existing folder 
         * @param mode     `std::ios_base::openmode`
         */
        duration_multifile_streambuf(typename Clock::duration duration, std::string&& path, std::ios_base::openmode mode = std::ios_base::out);

        /**
         * @brief Move constructor.
         */
        duration_multifile_streambuf(duration_multifile_streambuf&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        duration_multifile_streambuf(const duration_multifile_streambuf& other) = delete;

    public:
        /**
         * @brief Closes the current file, and opens a new file.
         */
        void reopen();

    protected:
        /**
         * @brief Handler that checks the duration, and calls `reopen()` if needed.
         */
        virtual int sync() override;

    private:
        /**
         * @brief The duration passed in to the constructor.
         */
        const typename Clock::duration _duration;

        /**
         * @brief The creation timestamp of the current file.
         */
        timestamp<Clock> _ts;
    };


    // --------------------------------------------------------------


    /**
     * @brief        `multifile_streambuf` specialization that is backed by files whose names are made of timestamps.
     * @details      Automatically closes and reopens a new file when the given time duration has passed. 
     * @tparam Clock Clock to generate a timestamp.
     */
    template <typename Clock = std::chrono::system_clock>
    class size_multifile_streambuf
        : public multifile_streambuf<Clock> {

        using base = multifile_streambuf<Clock>;

    public:
        /**
         * @brief      Constructor.
         * @param size Size limit of the file.
         * @param path Path to an existing folder 
         * @param mode `std::ios_base::openmode`
         */
        size_multifile_streambuf(std::size_t size, std::string&& path, std::ios_base::openmode mode = std::ios_base::out);

        /**
         * @brief Move constructor.
         */
        size_multifile_streambuf(size_multifile_streambuf&& other) noexcept = default;

        /**
         * @brief Deleted.
         */
        size_multifile_streambuf(const size_multifile_streambuf& other) = delete;

    public:
        /**
         * @brief Closes the current file, and opens a new file.
         */
        void reopen();

    protected:
        /**
         * @brief       Handler that counts the chars put out at once.
         * @param s     Char sequence to put out. 
         * @param count Count of chars.
         */
        virtual std::streamsize xsputn(const char* s, std::streamsize count) override;

        /**
         * @brief Handler that checks the size, and calls `reopen()` if needed.
         */
        virtual int sync() override;

    protected:
        /**
         * @brief Overrides the default pcount calculation.
         */
        std::size_t pcount() const noexcept;

    private:
        /**
         * @brief The size passed in to the constructor.
         */
        const std::size_t _size;

        /**
         * @brief Current file size.
         */
        std::size_t _current_size;
    };

} }
