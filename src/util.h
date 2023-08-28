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

#include <cstring>


namespace abc {

    /**
     * @brief Constructs and returns a temporary copy (`xvalue`) of the given source instance.
     */
    template <typename T>
    inline T copy(const T& source) {
        return T(source);
    }


    /**
     * @brief Returns a C-style string.
     */
    inline const char* c_str(const char* str) noexcept {
        return str;
    }


    /**
     * @brief Returns a C-style string.
     */
    inline const char* c_str(const std::string& str) noexcept {
        return str.c_str();
    }


    /**
     * @brief Returns `true` if a string is empty.
     */
    inline bool is_empty_str(const char* str) noexcept {
        return str == nullptr
            || *str == '\0';
    }


    /**
     * @brief Returns `true` if a string is empty.
     */
    inline bool is_empty_str(const std::string& str) noexcept {
        return str.empty();
    }


    /**
     * @brief Returns the length of a string.
     */
    inline std::size_t str_length(const char* str) noexcept {
        return str == nullptr ? 0 : std::strlen(str);
    }


    /**
     * @brief Returns the length of a string.
     */
    inline std::size_t str_length(const std::string& str) noexcept {
        return str.length();
    }

}
