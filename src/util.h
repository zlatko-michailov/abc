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
#include <utility>
#include <deque>
#include <map>


namespace abc {

    /**
     * @brief Constructs and returns a temporary copy (`xvalue`) of the given source instance.
     */
    template <typename T>
    inline T copy(const T& source) {
        return T(source);
    }


    // --------------------------------------------------------------


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


    // --------------------------------------------------------------


 // Comparison of std::pair<T1, T2> has been removed in C++ 20.
#if __cplusplus >= 202002L
    /**
     * @brief Returns `true` iff both items of one pair are equal to the corresponding items of the other pair.
     */
    template <typename T1, typename T2>
    bool operator == (const std::pair<T1, T2>& left, const std::pair<T1, T2>& right) noexcept {
        return left.first == right.first && left.second == right.second;
    }


    /**
     * @brief Returns the opposite of `operator ==`.
     */
    template <typename T1, typename T2>
    bool operator != (const std::pair<T1, T2>& left, const std::pair<T1, T2>& right) noexcept {
        return !(left == right);
    }
#endif


    /**
     * @brief Returns `true` iff the two containers have the same items in the same order.
     */
    template <typename Container>
    bool are_equal(const Container& left, const Container& right) noexcept {
        if (left.size() != right.size()) {
            return false;
        }

        if (left.empty() && right.empty()) {
            return true;
        }

        typename Container::const_iterator left_itr  = left.cbegin();
        typename Container::const_iterator right_itr = right.cbegin();

        while (left_itr != left.cend() && right_itr != right.cend()) {
            if (*left_itr != *right_itr) {
                return false;
            }

            left_itr++;
            right_itr++;
        }

        return true;
    }


    /**
     * @brief Returns `true` iff the two containers have the same items in the same order.
     */
    template <typename T>
    bool operator == (const std::deque<T>& left, const std::deque<T>& right) noexcept {
        return are_equal(left, right);
    }


    /**
     * @brief Returns the opposite of `operator ==`.
     */
    template <typename T>
    bool operator != (const std::deque<T>& left, const std::deque<T>& right) noexcept {
        return !are_equal(left, right);
    }


    /**
     * @brief Returns `true` iff the two containers have the same items in the same order.
     */
    template <typename K, typename V>
    bool operator == (const std::map<K, V>& left, const std::map<K, V>& right) noexcept {
        return are_equal(left, right);
    }


    /**
     * @brief Returns the opposite of `operator ==`.
     */
    template <typename K, typename V>
    bool operator != (const std::map<K, V>& left, const std::map<K, V>& right) noexcept {
        return !are_equal(left, right);
    }


    // --------------------------------------------------------------


    template <typename Predicate, typename ...Args>
    bool retry(std::size_t count, Predicate&& predicate, Args&&... args) {
        for (std::size_t c = 0; c < count; c++) {
            if (predicate(std::forward<Args>(args)...)) {
                return true;
            }
        }

        return false;
    }

}
