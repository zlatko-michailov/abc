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

	template <typename T>
	void vmem_init(T& dest) noexcept {
		std::memset(&dest, 0, sizeof(T));
	}


	template <typename T>
	void vmem_copy(T& dest, const T& src) noexcept {
		std::memmove(&dest, &src, sizeof(T));
	}


	template <typename T>
	bool vmem_is_less(const T& left, const T& right) noexcept {
		return left < right;
	}


	template <typename T>
	bool vmem_is_less_or_equal(const T& left, const T& right) noexcept {
		return left <= right;
	}


	template <typename T>
	bool vmem_are_equal(const T& left, const T& right) noexcept {
		return left == right;
	}

}
